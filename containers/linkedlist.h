#ifndef __LINKEDLIST_H__
#define __LINKEDLIST_H__

#include <iostream>
#include <cstddef> // size_t
#include <string>
#include <sstream>
#include <shared_mutex> // shared_mutex
#include "general_iterator.h"
#include "util.h"
#include "../types.h"
using namespace std;

// Forward iterator
template <typename Container>
class LinkedListForwardIterator : public general_iterator<Container, LinkedListForwardIterator<Container>>{
    using MySelf = LinkedListForwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;
    // T4: Impl forward iter (op++)
    MySelf operator++() { 
        this->m_pNode = this->m_pNode->getNext(); 
        return *this; 
    }
};

// Linked List Node
template <typename T>
class LLNode{
    using Node = LLNode<T>;
private:
    T   m_data;
    Node *m_next;
public:
    LLNode() : m_data(T()), m_next(nullptr) {}
    LLNode(T data) : m_data(data), m_next(nullptr) {}
    LLNode(T data, Node *next) : m_data(data), m_next(next) {}
    virtual ~LLNode() {}

    T      getData() const { return m_data; }
    T&     getDataRef()    { return m_data; }
    void   setData(T data) { m_data = data; }
    Node*  getNext() const { return m_next; }
    Node*& getNextRef()    { return m_next; }
    void   setNext(Node *next) { m_next = next; }
};

template <typename T>
struct AscendingLinkedListTrait{
    using value_type = T;
    using Node = LLNode<T>;
    using Comp = less<T>;
};

template <typename T>
struct DescendingLinkedListTrait{
    using value_type = T;
    using Node = LLNode<T>;
    using Comp = greater<T>;
};

template <typename Trait>
class LinkedList{
public:
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;
    using Comp       = typename Trait::Comp;
    using MySelf     = LinkedList<Trait>;

    using forward_iterator = LinkedListForwardIterator<MySelf>;
    // friend forward_iterator;

private:
    Node *m_pRoot = nullptr;
    Node *m_tail = nullptr;
    size_t m_size = 0;
    Comp   m_comp;
    mutable shared_mutex m_mtx;
public:
    LinkedList() {}
    // T1: Copy constructor
    LinkedList(const LinkedList &other){ 
        shared_lock<shared_mutex> lock(other.m_mtx);
        Node* pOther = other.m_pRoot;
        while(pOther) {
            Node* newNode = new Node(pOther->getData(), nullptr);
            if (!m_pRoot) {
                m_pRoot = m_tail = newNode;
            } else {
                m_tail->setNext(newNode);
                m_tail = newNode;
            }
            m_size++;
            pOther = pOther->getNext();
        }
    }
    // T2: Move constructor
    LinkedList(LinkedList &&other){ 
        unique_lock<shared_mutex> lock(other.m_mtx);
        m_pRoot = other.m_pRoot;
        m_tail  = other.m_tail;
        m_size  = other.m_size;
        
        other.m_pRoot = nullptr;
        other.m_tail  = nullptr;
        other.m_size  = 0;
    }
    
    LinkedList& operator=(const LinkedList &other){ // Copy assignment operator
        if(this == &other) return *this;
        
        scoped_lock lock(m_mtx, other.m_mtx); 
        
        Node *pCurr = m_pRoot;
        while(pCurr) {
            Node *pNext = pCurr->getNext();
            delete pCurr;
            pCurr = pNext;
        }
        m_pRoot = m_tail = nullptr;
        m_size = 0;
        
        Node* pOther = other.m_pRoot;
        while(pOther) {
            Node* newNode = new Node(pOther->getData(), nullptr);
            if (!m_pRoot) {
                m_pRoot = m_tail = newNode;
            } else {
                m_tail->setNext(newNode);
                m_tail = newNode;
            }
            m_size++;
            pOther = pOther->getNext();
        }
        return *this;
    }
    LinkedList& operator=(LinkedList &&other){ // Move assignment operator
        if(this == &other) return *this;
        
        scoped_lock lock(m_mtx, other.m_mtx); 
        
        Node *pCurr = m_pRoot;
        while(pCurr) {
            Node *pNext = pCurr->getNext();
            delete pCurr;
            pCurr = pNext;
        }
        
        m_pRoot = other.m_pRoot;
        m_tail  = other.m_tail;
        m_size  = other.m_size;
        
        other.m_pRoot = nullptr;
        other.m_tail  = nullptr;
        other.m_size  = 0;
        
        return *this;
    }
    
    // T3: Destructor seguro
    virtual ~LinkedList() {
        
        unique_lock<shared_mutex> lock(m_mtx); 
        
        Node *pCurr = m_pRoot;
        while(pCurr) {
            Node *pNext = pCurr->getNext();
            delete pCurr;
            pCurr = pNext;
        }
        m_pRoot = m_tail = nullptr;
        m_size = 0;
    }
    // T5: push_front
    virtual void push_front(value_type value, Ref ref) {
        unique_lock<shared_mutex> lock(m_mtx);
        m_pRoot = new Node(value, m_pRoot);
        if (m_size == 0) m_tail = m_pRoot;
        m_size++;
    }
    // T6: pop_front
    virtual void pop_front() {
        unique_lock<shared_mutex> lock(m_mtx);
        if (!m_pRoot) return;
        
        Node* pTemp = m_pRoot;
        m_pRoot = m_pRoot->getNext();
        delete pTemp;
        
        m_size--;
        if (m_size == 0) m_tail = nullptr;
    }
    // T7: push_back
    virtual void push_back(value_type value, Ref ref) {
        unique_lock<shared_mutex> lock(m_mtx);
        Node* newNode = new Node(value, nullptr);
        if (!m_pRoot) {
            m_pRoot = m_tail = newNode;
        } else {
            m_tail->setNext(newNode);
            m_tail = newNode;
        }
        m_size++;
    }
    // T8: pop_back
    virtual void pop_back() {
        unique_lock<shared_mutex> lock(m_mtx);
        if (!m_pRoot) return;
        
        // Caso particular: Si solo queda 1 elemento
        if (m_pRoot == m_tail) {
            delete m_pRoot;
            m_pRoot = m_tail = nullptr;
            m_size = 0;
            return;
        }
        
        // Rastreo exhaustivo: Buscar a la penúltima caja 
        Node* pCurr = m_pRoot;
        while (pCurr->getNext() != m_tail) {
            pCurr = pCurr->getNext();
        }
        
        // Aniquilamos al antiguo tail, y la penúltima caja es ascendida a Tail
        delete m_tail;
        pCurr->setNext(nullptr);
        m_tail = pCurr;
        m_size--;
    }
private:
            void    internal_insert(Node* &pParent, const value_type &value, Ref ref);
public:
    virtual void    insert(const value_type &value, Ref ref);
    
    // T9: operator[]
    virtual value_type& operator[](size_t index) {
        shared_lock<shared_mutex> lock(m_mtx); 
        if (index >= m_size) {
            throw out_of_range("Index out of bounds");
        }
        
        Node* pCurr = m_pRoot;
        for (size_t i = 0; i < index; ++i) {
            pCurr = pCurr->getNext();
        }
        return pCurr->getDataRef();
    }
    virtual size_t  size() const;
    virtual string  toString() const;

    forward_iterator begin() { return forward_iterator(this, m_pRoot); }
    forward_iterator end()   { return forward_iterator(this, nullptr); }

    // T12: ForEach (Soporte Genérico de Funciones y Argumentos Variables)
    template <typename Func, typename... Args>
    void ForEach(Func func, Args &&...  args){
        // T13
        unique_lock<shared_mutex> lock(m_mtx);
        if (m_size == 0) return; // Parche de Retorno Temprano 
        ::ForEach(begin(), end(), func, std::forward<Args>(args)... );
    }
};

template <typename Trait>
void LinkedList<Trait>::internal_insert(Node* &pPrev, const value_type &value, Ref ref){
    if(!pPrev || m_comp(value, pPrev->getDataRef())){
        pPrev = new Node(value, ref, pPrev);
        m_size++;
        if(pPrev == m_pRoot)
            m_tail = pPrev;
        return;
    }
    internal_insert(pPrev->getNextRef(), value, ref);
}

template <typename Trait>
void LinkedList<Trait>::insert(const value_type &value, Ref ref){
    // T13
    unique_lock<shared_mutex> lock(m_mtx);
    internal_insert(m_pRoot, value, ref);
}

// T10: operator>>
template <typename Trait>
istream& operator>>(istream& is, LinkedList<Trait>& list){
    char c;
    if (!(is >> c) || c != '[') {
        is.setstate(ios::failbit);
        return is;
    }
    
    while (is >> c) {
        if (c == ']') break;
        if (c == ',') continue;
        
        is.putback(c);
        
        typename Trait::value_type value; 
        if (is >> value) {
            list.insert(value, Ref()); 
        } else {
            is.setstate(ios::failbit);
            break;
        }
    }
    return is;
}

// implementacion auxiliar de lectura segura
template <typename Trait>
size_t LinkedList<Trait>::size() const {
    shared_lock<shared_mutex> lock(m_mtx);
    return m_size;
}

// implementacion auxiliar (String Format)
template <typename Trait>
string LinkedList<Trait>::toString() const {
    shared_lock<shared_mutex> lock(m_mtx);
    ostringstream oss;
    oss << "[";
    Node* pCurr = m_pRoot;
    while (pCurr) {
        if (pCurr != m_pRoot)
            oss << ",";
        oss << pCurr->getData();
        pCurr = pCurr->getNext();
    }
    oss << "]";
    return oss.str();
}

// T11: operator<<
template <typename Trait>
ostream& operator<<(ostream& os, const LinkedList<Trait>& list) {
    return os << list.toString();
}

#endif // __LINKEDLIST_H__