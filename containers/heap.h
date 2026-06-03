#ifndef __HEAP_H__
#define __HEAP_H__

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <shared_mutex>
#include "vector.h"
#include "traits.h"
#include "../types.h"
using namespace std;

// Adaptar Vector a Trait
template <typename Trait>
class Heap : public Vector<Trait> {
public:
    using Base       = Vector<Trait>;
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;
    using Comp       = typename Trait::Comp;
    using Base::Base;

private:
    Comp m_comp;

    size_t parent(size_t i) const { return (i - 1) / 2; }
    size_t left  (size_t i) const { return 2 * i + 1;   }
    size_t right (size_t i) const { return 2 * i + 2;   }

    void swap(size_t i, size_t j) {
        Node tmp        = this->m_data[i];
        this->m_data[i] = this->m_data[j];
        this->m_data[j] = tmp;
    }

    // heapifyUp
    void heapifyUp(size_t i) {
        while (i > 0) {
            size_t p = parent(i);
            if (m_comp(this->m_data[i].getData(), this->m_data[p].getData())) {
                swap(i, p);
                i = p;
            } else break;
        }
    }

    // heapifyDown
    void heapifyDown(size_t i) {
        size_t best = i;
        size_t l = left(i), r = right(i);
        if (l < this->m_size && m_comp(this->m_data[l].getData(), this->m_data[best].getData()))
            best = l;
        if (r < this->m_size && m_comp(this->m_data[r].getData(), this->m_data[best].getData()))
            best = r;
        if (best != i) {
            swap(i, best);
            heapifyDown(best);
        }
    }

    string treeToString() const {
        if (this->m_size == 0) return "  (vacio)\n";
        ostringstream oss;
        size_t inicioNivel = 0;
        size_t tamNivel    = 1;
        while (inicioNivel < this->m_size) {
            oss << "  ";
            size_t fin = (inicioNivel + tamNivel < this->m_size)
                            ? inicioNivel + tamNivel : this->m_size;
            for (size_t i = inicioNivel; i < fin; ++i)
                oss << this->m_data[i].getData() << " ";
            oss << "\n";
            inicioNivel += tamNivel;
            tamNivel    *= 2;
        }
        return oss.str();
    }

public:
    // insert
    void insert(value_type value, Ref ref) {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (this->m_size == this->m_capacity) this->resize();
        this->m_data[this->m_size] = Node(value, ref);
        ++this->m_size;
        heapifyUp(this->m_size - 1);
    }

    // extract
    tuple<value_type, Ref> extract() {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (this->m_size == 0) throw runtime_error("heap vacio");
        auto resultado = make_tuple(this->m_data[0].getData(), this->m_data[0].getRef());
        this->m_data[0] = this->m_data[this->m_size - 1];
        --this->m_size;
        if (this->m_size > 0) heapifyDown(0);
        return resultado;
    }

    // peek()
    tuple<value_type, Ref> peek() const {
        shared_lock<shared_mutex> lock(this->m_mtx);
        if (this->m_size == 0) throw runtime_error("heap vacio");
        return make_tuple(this->m_data[0].getData(), this->m_data[0].getRef());
    }

    bool isEmpty() const {
        shared_lock<shared_mutex> lock(this->m_mtx);
        return this->m_size == 0;
    }

    // toString()
    string toString() const override {
        shared_lock<shared_mutex> lock(this->m_mtx);
        ostringstream oss;
        oss << "Array: [";
        for (size_t i = 0; i < this->m_size; ++i) {
            if (i) oss << ",";
            oss << "(" << this->m_data[i].getData() << "," << this->m_data[i].getRef() << ")";
        }
        oss << "]\nTree:\n" << treeToString();
        return oss.str();
    }

    // operator<<
    friend ostream& operator<<(ostream& os, const Heap& h) {
        shared_lock<shared_mutex> lock(h.m_mtx);
        os << "[";
        for (size_t i = 0; i < h.m_size; ++i) {
            if (i) os << ",";
            os << "(" << h.m_data[i].getData() << "," << h.m_data[i].getRef() << ")";
        }
        os << "]";
        return os;
    }

    // operator>>
    friend istream& operator>>(istream& is, Heap& h) {
        Char ch;
        if (!(is >> ch) || ch != '[') { is.clear(ios_base::failbit); return is; }
        value_type val; Ref ref; Char coma, parC;
        while (is >> ch && ch != ']')
            if (ch == '(')
                if (is >> val >> coma >> ref >> parC)
                    if (coma == ',' && parC == ')')
                        h.insert(val, ref);
        return is;
    }
};

#endif // __HEAP_H__
