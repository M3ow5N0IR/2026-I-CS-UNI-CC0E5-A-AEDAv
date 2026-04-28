#ifndef __CIRCULARDOUBLELINKEDLIST_H__
#define __CIRCULARDOUBLELINKEDLIST_H__

#include "doublelinkedlist.h"

// T20: Traits para CDLL (reutilizan DLLNode que ya tiene prev/next)
template <typename T>
struct AscendingCDLLTrait : BaseTrait<T, less<T>, DLLNode<T>>{};

template <typename T>
struct DescendingCDLLTrait : BaseTrait<T, greater<T>, DLLNode<T>>{};

// T21: Forward Iterator circular con centinela
template <typename Container>
class CDLLForwardIterator : public general_iterator<Container, CDLLForwardIterator<Container>> {
public:
    using MySelf = CDLLForwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;

    MySelf operator++() {
        if (this->m_pNode) {
            this->m_pNode = this->m_pNode->getNext();
            // Centinela: si volvimos al root, paramos
            if (this->m_pContainer && this->m_pNode == this->m_pContainer->m_pRoot) {
                this->m_pNode = nullptr;
            }
        }
        return *this;
    }
};

// T21: Backward Iterator circular con centinela
template <typename Container>
class CDLLBackwardIterator : public general_iterator<Container, CDLLBackwardIterator<Container>> {
public:
    using MySelf = CDLLBackwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;

    MySelf operator++() {
        if (this->m_pNode) {
            this->m_pNode = this->m_pNode->getPrev();
            // Centinela: si volvimos al tail, paramos
            if (this->m_pContainer && this->m_pNode == this->m_pContainer->m_tail) {
                this->m_pNode = nullptr;
            }
        }
        return *this;
    }
};

// Clase principal: Circular Double Linked List
template <typename Trait>
class CircularDoubleLinkedList : public DoubleLinkedList<Trait> {
public:
    using MySelf = CircularDoubleLinkedList<Trait>;
    using forward_iterator  = CDLLForwardIterator<MySelf>;
    using backward_iterator = CDLLBackwardIterator<MySelf>;
    friend forward_iterator;
    friend backward_iterator;

    // Enganches de iteración
    forward_iterator  begin()  { return forward_iterator(this, this->m_pRoot); }
    forward_iterator  end()    { return forward_iterator(this, nullptr); }
    backward_iterator rbegin() { return backward_iterator(this, this->m_tail); }
    backward_iterator rend()   { return backward_iterator(this, nullptr); }

    // T22: Constructor por defecto
    CircularDoubleLinkedList() : DoubleLinkedList<Trait>() {}

    // T22: Copy Constructor (do-while para recorrer el anillo)
    CircularDoubleLinkedList(const CircularDoubleLinkedList &other) : DoubleLinkedList<Trait>() {
        shared_lock<shared_mutex> lock(other.m_mtx);
        if (other.m_size == 0) return;

        auto* curr = other.m_pRoot;
        do {
            this->push_back(curr->getData(), curr->getRef());
            curr = curr->getNext();
        } while (curr != other.m_pRoot);
    }

    // T22: Move Constructor
    CircularDoubleLinkedList(CircularDoubleLinkedList &&other) noexcept
        : DoubleLinkedList<Trait>(std::move(other)) {}

    // T22: Copy Assignment
    CircularDoubleLinkedList& operator=(const CircularDoubleLinkedList &other) {
        if (this != &other) {
            this->clear();
            shared_lock<shared_mutex> lock(other.m_mtx);
            if (other.m_size > 0) {
                auto* curr = other.m_pRoot;
                do {
                    this->push_back(curr->getData(), curr->getRef());
                    curr = curr->getNext();
                } while (curr != other.m_pRoot);
            }
        }
        return *this;
    }

    // T22: Move Assignment
    CircularDoubleLinkedList& operator=(CircularDoubleLinkedList &&other) noexcept {
        if (this != &other) {
            this->clear();
            DoubleLinkedList<Trait>::operator=(std::move(other));
        }
        return *this;
    }

    // T22: Destructor seguro (rompe AMBOS ciclos antes de borrar)
    virtual ~CircularDoubleLinkedList() { clear(); }

    void clear() {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (this->m_size == 0) return;

        // Romper los 2 enlaces circulares para convertirlo en lista plana
        this->m_tail->setNext(nullptr);
        this->m_pRoot->setPrev(nullptr);

        auto* current = this->m_pRoot;
        while (current) {
            auto* next = current->getNext();
            delete current;
            current = next;
        }
        this->m_pRoot = nullptr;
        this->m_tail  = nullptr;
        this->m_size  = 0;
    }

    // T23: push_front circular bidireccional
    void push_front(typename Trait::value_type value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        auto* newNode = new typename Trait::Node(value, ref, this->m_pRoot, this->m_tail);

        if (this->m_size == 0) {
            this->m_pRoot = newNode;
            this->m_tail  = newNode;
            newNode->setNext(newNode); // Círculo de 1
            newNode->setPrev(newNode);
        } else {
            this->m_pRoot->setPrev(newNode); // El antiguo root apunta atrás al nuevo
            this->m_pRoot = newNode;
            this->m_tail->setNext(this->m_pRoot);  // Reparar: tail -> root
            this->m_pRoot->setPrev(this->m_tail);   // Reparar: root -> tail
        }
        this->m_size++;
    }

    // T23: push_back circular bidireccional
    void push_back(typename Trait::value_type value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        auto* newNode = new typename Trait::Node(value, ref, this->m_pRoot, this->m_tail);

        if (this->m_size == 0) {
            this->m_pRoot = newNode;
            this->m_tail  = newNode;
            newNode->setNext(newNode);
            newNode->setPrev(newNode);
        } else {
            this->m_tail->setNext(newNode);
            this->m_tail = newNode;
            this->m_tail->setNext(this->m_pRoot);   // Reparar: tail -> root
            this->m_pRoot->setPrev(this->m_tail);    // Reparar: root -> tail
        }
        this->m_size++;
    }

    // T24: pop_front circular bidireccional
    std::tuple<typename Trait::value_type, Ref> pop_front() override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (!this->m_pRoot) throw runtime_error("La lista esta vacia");

        auto* temp = this->m_pRoot;
        auto result = std::make_tuple(temp->getData(), temp->getRef());

        if (this->m_size == 1) {
            this->m_pRoot = nullptr;
            this->m_tail  = nullptr;
        } else {
            this->m_pRoot = this->m_pRoot->getNext();
            this->m_tail->setNext(this->m_pRoot);   // Reparar: tail -> root
            this->m_pRoot->setPrev(this->m_tail);    // Reparar: root -> tail
        }

        delete temp;
        this->m_size--;
        return result;
    }

    // T24: pop_back circular bidireccional ¡O(1)!
    std::tuple<typename Trait::value_type, Ref> pop_back() override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (!this->m_pRoot) throw runtime_error("La lista esta vacia");

        auto* temp = this->m_tail;
        auto result = std::make_tuple(temp->getData(), temp->getRef());

        if (this->m_size == 1) {
            this->m_pRoot = nullptr;
            this->m_tail  = nullptr;
        } else {
            this->m_tail = this->m_tail->getPrev();  // O(1) gracias a prev
            this->m_tail->setNext(this->m_pRoot);    // Reparar: tail -> root
            this->m_pRoot->setPrev(this->m_tail);     // Reparar: root -> tail
        }

        delete temp;
        this->m_size--;
        return result;
    }

    // T25: insert circular bidireccional ordenado
    void insert(const typename Trait::value_type &value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);

        // Caso 1: Lista vacía
        if (this->m_size == 0) {
            auto* newNode = new typename Trait::Node(value, ref);
            this->m_pRoot = newNode;
            this->m_tail  = newNode;
            newNode->setNext(newNode);
            newNode->setPrev(newNode);
            this->m_size++;
            return;
        }

        // Caso 2: Insertar antes de la cabeza
        if (this->m_comp(value, this->m_pRoot->getDataRef())) {
            auto* newNode = new typename Trait::Node(value, ref, this->m_pRoot, this->m_tail);
            this->m_pRoot->setPrev(newNode);
            this->m_pRoot = newNode;
            this->m_tail->setNext(this->m_pRoot);
            this->m_pRoot->setPrev(this->m_tail);
            this->m_size++;
            return;
        }

        // Caso 3: Buscar el slot en el anillo
        auto* prev = this->m_pRoot;
        auto* curr = this->m_pRoot->getNext();

        while (curr != this->m_pRoot && !this->m_comp(value, curr->getDataRef())) {
            prev = curr;
            curr = curr->getNext();
        }

        // Insertar entre prev y curr
        auto* newNode = new typename Trait::Node(value, ref, curr, prev);
        prev->setNext(newNode);
        curr->setPrev(newNode);

        // Si se insertó al final (curr volvió al root), actualizar tail
        if (curr == this->m_pRoot) {
            this->m_tail = newNode;
            this->m_tail->setNext(this->m_pRoot);
            this->m_pRoot->setPrev(this->m_tail);
        }

        this->m_size++;
    }

    // T26: operator<< con visualización bidireccional circular
    friend ostream& operator<<(ostream& os, const CircularDoubleLinkedList& list) {
        shared_lock<shared_mutex> lock(list.m_mtx);
        os << "[";
        if (list.m_size > 0) {
            auto* act = list.m_pRoot;
            do {
                os << "(" << act->getData() << "," << act->getRef() << ")";
                act = act->getNext();
                if (act != list.m_pRoot) os << ",";
            } while (act != list.m_pRoot);
            os << "] ->root(" << list.m_pRoot->getData() << ")";
            // Imprimir reversa para validar integridad de punteros prev
            os << " |bwd:[";
            auto* bact = list.m_tail;
            do {
                os << "(" << bact->getData() << "," << bact->getRef() << ")";
                bact = bact->getPrev();
                if (bact != list.m_tail) os << ",";
            } while (bact != list.m_tail);
            os << "]";
        } else {
            os << "]";
        }
        return os;
    }

    // T26: operator>>
    friend istream& operator>>(istream& is, CircularDoubleLinkedList& list) {
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
                        list.insert(val, ref);
                    }
                }
            }
        }
        // Descartar el resto de la línea (->root... |bwd:...)
        string discard;
        getline(is, discard);
        return is;
    }

    // ForEach override: usa *this (CDLL) para que begin()/end() resuelvan al iterador con centinela
    template <typename Func, typename... Args>
    void ForEach(Func func, Args &&...args) {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (this->m_size == 0) return;
        for (auto& item : *this) {
            func(item, std::forward<Args>(args)...);
        }
    }

    // T27: circularForEach (N vueltas hacia adelante)
    template <typename Func, typename... Args>
    void circularForEach(size_t vueltas, Func func, Args &&...args) {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (this->m_size == 0 || vueltas == 0) return;

        auto* curr = this->m_pRoot;
        size_t pasos = this->m_size * vueltas;

        for (size_t i = 0; i < pasos; ++i) {
            func(curr->getDataRef(), std::forward<Args>(args)...);
            curr = curr->getNext();
        }
    }

    // T27: ReverseForEach (recorrido inverso, 1 vuelta)
    template <typename Func, typename... Args>
    void ReverseForEach(Func func, Args &&...args) {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (this->m_size == 0) return;

        auto* curr = this->m_tail;
        do {
            func(curr->getDataRef(), std::forward<Args>(args)...);
            curr = curr->getPrev();
        } while (curr != this->m_tail);
    }
};

#endif // __CIRCULARDOUBLELINKEDLIST_H__
