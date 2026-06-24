#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <iostream>
#include <cstddef>
#include <string>
#include <sstream>
#include <shared_mutex>
#include <mutex>
#include <utility>
#include "general_iterator.h"
#include "util.h"
#include "../types.h"
#include "traits.h"
using namespace std;

// VectorNode: par (dato, ref)
template <typename T>
class VectorNode {
public:
    using value_type = T;
private:
    T   m_data;
    Ref m_ref;
public:
    VectorNode() : m_data(T()), m_ref(Ref()) {}
    VectorNode(T data, Ref ref) : m_data(data), m_ref(ref) {}
    VectorNode(const VectorNode &other) : m_data(other.m_data), m_ref(other.m_ref) {}
    VectorNode(VectorNode &&other) : m_data(std::move(other.m_data)), m_ref(std::move(other.m_ref)) {}
    VectorNode& operator=(const VectorNode &other) {
        m_data = other.m_data;
        m_ref  = other.m_ref;
        return *this;
    }
    VectorNode& operator=(VectorNode &&other) {
        m_data = std::move(other.m_data);
        m_ref  = std::move(other.m_ref);
        return *this;
    }

    T    getData() const   { return m_data; }
    T&   getDataRef()      { return m_data; }
    void setData(T data)   { m_data = data; }
    Ref  getRef() const    { return m_ref; }
    void setRef(Ref ref)   { m_ref = ref; }
};

template <typename T>
ostream& operator<<(ostream& os, VectorNode<T>& node) {
    return os << "(" << node.getData() << "," << node.getRef() << ")";
}

// Iteradores forward/backward sobre el array crudo
template <typename Container>
class vector_forward_iterator : public general_iterator<Container, vector_forward_iterator<Container>> {
public:
    using MySelf = vector_forward_iterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;
    MySelf operator++() { this->m_pNode++; return *this; }
};

template <typename Container>
class vector_backward_iterator : public general_iterator<Container, vector_backward_iterator<Container>> {
public:
    using MySelf = vector_backward_iterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;
    MySelf operator++() { this->m_pNode--; return *this; }
};

// Vector<Trait>: contenedor secuencial con redimensionamiento automatico.
// Trait define value_type, Node y Comp (este ultimo no usado en la base).
template <typename Trait>
class Vector {
public:
    using value_type        = typename Trait::value_type;
    using Node              = typename Trait::Node;
    using Comp              = typename Trait::Comp;
    using MySelf            = Vector<Trait>;
    using forward_iterator  = vector_forward_iterator<MySelf>;
    using backward_iterator = vector_backward_iterator<MySelf>;
    friend forward_iterator;
    friend backward_iterator;

protected:
    size_t  m_capacity;
    size_t  m_size;
    Node   *m_data;
    mutable shared_mutex m_mtx;

    // resize: protected para que clases derivadas (Heap) puedan llamarlo
    void resize() {
        m_capacity = (m_capacity < 10) ? m_capacity + 10 : m_capacity * 2;
        Node *new_data = new Node[m_capacity];
        for (size_t i = 0; i < m_size; ++i) new_data[i] = m_data[i];
        delete[] m_data;
        m_data = new_data;
    }

public:
    Vector(size_t capacity = 10)
        : m_capacity(capacity), m_size(0), m_data(new Node[capacity]) {}

    Vector(const Vector& other) : m_capacity(0), m_size(0), m_data(nullptr) {
        shared_lock<shared_mutex> lock(other.m_mtx);
        m_capacity = other.m_capacity;
        m_size     = other.m_size;
        m_data     = new Node[m_capacity];
        for (size_t i = 0; i < m_size; ++i) m_data[i] = other.m_data[i];
    }

    Vector& operator=(const Vector& other) {
        if (this != &other) {
            shared_lock<shared_mutex> olock(other.m_mtx);
            unique_lock<shared_mutex> lock(m_mtx);
            delete[] m_data;
            m_capacity = other.m_capacity;
            m_size     = other.m_size;
            m_data     = new Node[m_capacity];
            for (size_t i = 0; i < m_size; ++i) m_data[i] = other.m_data[i];
        }
        return *this;
    }

    virtual ~Vector() { delete[] m_data; }

    virtual void push_back(value_type value, Ref ref) {
        unique_lock<shared_mutex> lock(m_mtx);
        if (m_size == m_capacity) resize();
        m_data[m_size++] = Node(value, ref);
    }

    virtual void pop_back() {
        unique_lock<shared_mutex> lock(m_mtx);
        if (m_size == 0) throw out_of_range("vector vacio");
        --m_size;
    }

    value_type& operator[](size_t index) {
        shared_lock<shared_mutex> lock(m_mtx);
        if (index >= m_size) throw out_of_range("indice fuera de rango");
        return m_data[index].getDataRef();
    }

    const value_type& operator[](size_t index) const {
        shared_lock<shared_mutex> lock(m_mtx);
        if (index >= m_size) throw out_of_range("indice fuera de rango");
        return m_data[index].getDataRef();
    }

    // nodeAt: acceso al nodo completo (data + ref) por indice.
    // operator[] solo expone el data; esto permite leer/escribir tambien el ref.
    Node& nodeAt(size_t index) {
        shared_lock<shared_mutex> lock(m_mtx);
        if (index >= m_size) throw out_of_range("indice fuera de rango");
        return m_data[index];
    }

    virtual size_t size() const {
        shared_lock<shared_mutex> lock(m_mtx);
        return m_size;
    }

    forward_iterator  begin()  { return forward_iterator (this, m_data); }
    forward_iterator  end()    { return forward_iterator (this, m_data + m_size); }
    backward_iterator rbegin() { return backward_iterator(this, m_data + m_size - 1); }
    backward_iterator rend()   { return backward_iterator(this, m_data - 1); }

    template <typename Func, typename... Args>
    void ForEach(Func func, Args &&...args) {
        unique_lock<shared_mutex> lock(m_mtx);
        ::ForEach(begin(), end(), func, std::forward<Args>(args)...);
    }

    template <typename Func, typename... Args>
    void ReverseForEach(Func func, Args &&...args) {
        unique_lock<shared_mutex> lock(m_mtx);
        if (m_size == 0) return;
        ::ForEach(rbegin(), rend(), func, std::forward<Args>(args)...);
    }

    virtual string toString() const {
        shared_lock<shared_mutex> lock(m_mtx);
        ostringstream oss;
        oss << "[";
        for (size_t i = 0; i < m_size; ++i) {
            if (i > 0) oss << ",";
            oss << m_data[i];
        }
        oss << "]";
        return oss.str();
    }
};

template <typename Trait>
ostream& operator<<(ostream& os, const Vector<Trait>& v) {
    return os << v.toString();
}

template <typename Trait>
istream& operator>>(istream& is, Vector<Trait>& v) {
    return is; // TODO
}

void DemoVector();
void DemoConcurrentVector();

#endif // __VECTOR_H__
