#ifndef __DOUBLELINKEDLIST_H__
#define __DOUBLELINKEDLIST_H__

#include "linkedlist.h"

template <typename T>
class DLLNode : public LLNode<T, DLLNode<T>>{
    private:
        using Node = DLLNode<T>;
        Node *m_pPrev;
    public:
        DLLNode() : LLNode<T, DLLNode<T>>(), m_pPrev(nullptr) {}
        DLLNode(T data, Ref ref, Node *next = nullptr, Node *prev = nullptr) : LLNode<T, DLLNode<T>>(data, ref, next), m_pPrev(prev) {}

        Node*  getPrev() const     { return m_pPrev; }
        void   setPrev(Node *prev) { m_pPrev = prev; }
        Node*& getPrevRef()        { return m_pPrev; }
};

// T2: Forward y Backward Iterators
template <typename Container>
class DLLForwardIterator : public general_iterator<Container, DLLForwardIterator<Container>> {
public:
    using MySelf = DLLForwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;

    MySelf operator++() { 
        if (this->m_pNode) {
            this->m_pNode = this->m_pNode->getNext(); 
        }
        return *this; 
    }
};

template <typename Container>
class DLLBackwardIterator : public general_iterator<Container, DLLBackwardIterator<Container>> {
public:
    using MySelf = DLLBackwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;

    MySelf operator++() { 
        if (this->m_pNode) {
            this->m_pNode = this->m_pNode->getPrev(); 
        }
        return *this; 
    }
};


template <typename T>
struct AscendingDLLTrait : BaseTrait<T, less<T>, DLLNode<T>>{};

template <typename T>
struct DescendingDLLTrait : BaseTrait<T, greater<T>, DLLNode<T>>{};

template <typename Trait>
class DoubleLinkedList : public LinkedList<Trait>{
public:
    using MySelf = DoubleLinkedList<Trait>;
    using forward_iterator = DLLForwardIterator<MySelf>;
    using backward_iterator = DLLBackwardIterator<MySelf>;
    friend forward_iterator;
    friend backward_iterator;

    // T3: Hooks de iteración Bidireccional
    forward_iterator begin() { return forward_iterator(this, this->m_pRoot); }
    forward_iterator end()   { return forward_iterator(this, nullptr); }
    
    backward_iterator rbegin() { return backward_iterator(this, this->m_tail); }
    backward_iterator rend()   { return backward_iterator(this, nullptr); }

    // T4: push_front con enlace doble
    void push_front(typename Trait::value_type value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx); 
        auto oldRoot = this->m_pRoot;
        this->m_pRoot = new typename Trait::Node(value, ref, oldRoot, nullptr);
        if (this->m_size == 0) {
            this->m_tail = this->m_pRoot;
        } else {
            oldRoot->setPrev(this->m_pRoot);
        }
        this->m_size++;
    }

    // T5: push_back con enlace doble
    void push_back(typename Trait::value_type value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx); 
        auto newNode = new typename Trait::Node(value, ref, nullptr, this->m_tail);
        if (this->m_size == 0) {
            this->m_pRoot = newNode;
            this->m_tail = newNode;
        } else {
            this->m_tail->setNext(newNode);
            this->m_tail = newNode;
        }
        this->m_size++;
    }

    // T6: pop_front con enlace doble
    std::tuple<typename Trait::value_type, Ref> pop_front() override {
        unique_lock<shared_mutex> lock(this->m_mtx); 
        if (!this->m_pRoot) throw runtime_error("La lista esta vacia");
        
        auto temp = this->m_pRoot;
        auto result = std::make_tuple(temp->getData(), temp->getRef());
        
        this->m_pRoot = this->m_pRoot->getNext();
        if (this->m_pRoot) {
            this->m_pRoot->setPrev(nullptr);
        } else {
            this->m_tail = nullptr;
        }
        
        delete temp;
        this->m_size--;
        return result;
    }

    // T7: pop_back ¡Reducido drásticamente a O(1)!
    std::tuple<typename Trait::value_type, Ref> pop_back() override {
        unique_lock<shared_mutex> lock(this->m_mtx); 
        if (!this->m_pRoot) throw runtime_error("La lista esta vacia");
        
        auto temp = this->m_tail;
        auto result = std::make_tuple(temp->getData(), temp->getRef());
        
        this->m_tail = this->m_tail->getPrev();
        if (this->m_tail) {
            this->m_tail->setNext(nullptr);
        } else {
            this->m_pRoot = nullptr;
        }
        
        delete temp;
        this->m_size--;
        return result;
    }

    // T8: insert bidireccional
    void insert(const typename Trait::value_type &value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        internal_insert_dll(this->m_pRoot, nullptr, value, ref);
    }

private:
    void internal_insert_dll(typename Trait::Node* &pCurr, typename Trait::Node* pPrev, const typename Trait::value_type &value, Ref ref) {
        if (!pCurr || this->m_comp(value, pCurr->getDataRef())) {
            auto newNode = new typename Trait::Node(value, ref, pCurr, pPrev);
            if (pCurr) {
                pCurr->setPrev(newNode);
            } else {
                this->m_tail = newNode; // Si cae al final, es el nuevo tail
            }
            pCurr = newNode; // Reconecta el next del padre
            this->m_size++;
            return;
        }
        internal_insert_dll(pCurr->getNextRef(), pCurr, value, ref);
    }
public:

    DoubleLinkedList() : LinkedList<Trait>() {}

    // T9: Copy constructor
    DoubleLinkedList(const DoubleLinkedList &other) : LinkedList<Trait>() {
        shared_lock<shared_mutex> lock(other.m_mtx); 
        for (auto* curr = other.m_pRoot; curr != nullptr; curr = curr->getNext()) {
            this->push_back(curr->getData(), curr->getRef());
        }
    }

    // T9: Move constructor 
    DoubleLinkedList(DoubleLinkedList &&other) noexcept : LinkedList<Trait>(std::move(other)) {}

    // T9: Copy assignment operator
    DoubleLinkedList& operator=(const DoubleLinkedList &other) {
        LinkedList<Trait>::operator=(other); 
        return *this;
    }

    // T9: Move assignment operator
    DoubleLinkedList& operator=(DoubleLinkedList &&other) noexcept {
        LinkedList<Trait>::operator=(std::move(other));
        return *this;
    }

    // T10: operator<< (Doble visualización Fwd y Bwd para Debugging)
    friend ostream& operator<<(ostream& os, const DoubleLinkedList& list) {
        shared_lock<shared_mutex>lock(list.m_mtx); 
        os << "[";
        auto* act = list.m_pRoot;
        while(act){
            os << "(" << act->getData() << "," << act->getRef() << ")";
            if(act->getNext()) os << ",";
            act = act->getNext();
        }
        os << "] |bwd:[";
        auto* bact = list.m_tail;
        while(bact){
            os << "(" << bact->getData() << "," << bact->getRef() << ")";
            if(bact->getPrev()) os << ",";
            bact = bact->getPrev();
        }
        os << "]";
        return os;
    }

    // T10: operator>> 
    friend istream& operator>>(istream& is, DoubleLinkedList& list) {
        char ch;
        if (!(is >> ch) || ch != '[') {
            is.clear(ios_base::failbit);
            return is;
        }
        typename Trait::value_type val;
        Ref ref;
        char comma, parenClose;
        while (is >> ch && ch != ']') {
            if (ch == '(') {
                if (is >> val >> comma >> ref >> parenClose) {
                    if (comma == ',' && parenClose == ')') {
                        list.push_back(val, ref);
                    }
                }
            }
        }
        // Desechar la basura visual 
        string discard;
        getline(is, discard, ']'); 
        return is;
    }
};

#endif // __DOUBLELINKEDLIST_H__