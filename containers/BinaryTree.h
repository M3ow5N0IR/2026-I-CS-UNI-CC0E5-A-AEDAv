#ifndef __BINARYTREE_H__
#define __BINARYTREE_H__

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <climits>
#include <shared_mutex>
#include <type_traits>
#include "stack.h"
#include "vector.h"
#include "../types.h"
#include "traits.h"
using namespace std;

// Node en BinaryTree 
template<typename T, typename Self = void>
struct BinaryTreeNode {
    using value_type = T;
    using Node = conditional_t<is_void_v<Self>, BinaryTreeNode, Self>;

    T     m_data;
    Ref   m_ref;
    Node* m_pChild[2];

    BinaryTreeNode(T data, Ref ref = 0)
        : m_data(data), m_ref(ref), m_pChild{nullptr, nullptr} {}

    T&  getDataRef() { return m_data; }
    Ref getRef()     { return m_ref; }
};

// tree_iterator
template<typename Container, class IteratorBase>
class tree_iterator {
public:
    using Node       = typename Container::Node;
    using value_type = typename Container::value_type;
    using myself     = tree_iterator<Container, IteratorBase>;

protected:
    Stack<Node*> m_stack;
    size_t       m_idx;

public:
    tree_iterator(Stack<Node*> s, size_t idx) : m_stack(s), m_idx(idx) {}

    Node* getNode() const { return m_stack[m_idx]; }

    friend bool operator==(const IteratorBase& a, const IteratorBase& b) {
        return a.m_idx == b.m_idx;
    }
    friend bool operator!=(const IteratorBase& a, const IteratorBase& b) {
        return a.m_idx != b.m_idx;
    }

    value_type& operator*() { return m_stack[m_idx]->getDataRef(); }
};

// Forward iterator del arbol
template<typename Container>
class tree_forward_iterator : public tree_iterator<Container, tree_forward_iterator<Container>> {
public:
    using MySelf = tree_forward_iterator<Container>;
    using Parent = tree_iterator<Container, MySelf>;
    using Parent::Parent;

    MySelf& operator++() { ++this->m_idx; return *this; }
};

// Backward iterator 
template<typename Container>
class tree_backward_iterator : public tree_iterator<Container, tree_backward_iterator<Container>> {
public:
    using MySelf = tree_backward_iterator<Container>;
    using Parent = tree_iterator<Container, MySelf>;
    using Parent::Parent;

    MySelf& operator++() {
        if (this->m_idx == 0) this->m_idx = SIZE_MAX;
        else                  --this->m_idx;
        return *this;
    }
};

// TraversalView
template<typename FwdIt, typename BwdIt>
class TraversalView {
    FwdIt m_begin, m_end;
    BwdIt m_rbegin, m_rend;

public:
    TraversalView(FwdIt b, FwdIt e, BwdIt rb, BwdIt re)
        : m_begin(b), m_end(e), m_rbegin(rb), m_rend(re) {}

    FwdIt begin()  const { return m_begin;  }
    FwdIt end()    const { return m_end;    }
    BwdIt rbegin() const { return m_rbegin; }
    BwdIt rend()   const { return m_rend;   }

    template<typename Func, typename... Args>
    void forEach(Func func, Args&&... args) {
        for (auto it = begin(); it != end(); ++it)
            func(*it, std::forward<Args>(args)...);
    }

    template<typename Func, typename... Args>
    void rForEach(Func func, Args&&... args) {
        for (auto it = rbegin(); it != rend(); ++it)
            func(*it, std::forward<Args>(args)...);
    }
};

template<typename Trait>
class BinaryTree {
public:
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;
    using Comp       = typename Trait::Comp;
    using MySelf     = BinaryTree<Trait>;

    using FwdIt = tree_forward_iterator<MySelf>;
    using BwdIt = tree_backward_iterator<MySelf>;
    using View  = TraversalView<FwdIt, BwdIt>;

    friend FwdIt;
    friend BwdIt;

protected:
    Node*  m_pRoot;
    size_t m_size;
    Comp   m_comp;
    mutable shared_mutex m_mtx;

    // internal_insert
    virtual Node* internal_insert(Node* pNode, const value_type& data, Ref ref) {
        if (!pNode) return new Node(data, ref);
        
        auto rama = m_comp(pNode->m_data, data);
        pNode->m_pChild[rama] = internal_insert(pNode->m_pChild[rama], data, ref);
        return pNode;
    }

    // Destructor seguro
    virtual void internal_clear(Node* pNode) {
        if (!pNode) return;
        internal_clear(pNode->m_pChild[0]);
        internal_clear(pNode->m_pChild[1]);
        delete pNode;
    }

    // Copy constructor 
    virtual Node* internal_copy(Node* pNode) {
        if (!pNode) return nullptr;
        Node* clon = new Node(pNode->m_data, pNode->m_ref);
        clon->m_pChild[0] = internal_copy(pNode->m_pChild[0]);
        clon->m_pChild[1] = internal_copy(pNode->m_pChild[1]);
        return clon;
    }

    // Search recursivo
    virtual Node* internal_search(Node* pNode, const value_type& data) const {
        if (!pNode) return nullptr;
        if (!m_comp(data, pNode->m_data) && !m_comp(pNode->m_data, data))
            return pNode;
        auto rama = m_comp(pNode->m_data, data);
        return internal_search(pNode->m_pChild[rama], data);
    }

    // Recorridos
    void fill_inorder(Node* n, Stack<Node*>& s) const {
        if (!n) return;
        fill_inorder(n->m_pChild[0], s);
        s.push(n);
        fill_inorder(n->m_pChild[1], s);
    }

    void fill_preorder(Node* n, Stack<Node*>& s) const {
        if (!n) return;
        s.push(n);
        fill_preorder(n->m_pChild[0], s);
        fill_preorder(n->m_pChild[1], s);
    }

    void fill_postorder(Node* n, Stack<Node*>& s) const {
        if (!n) return;
        fill_postorder(n->m_pChild[0], s);
        fill_postorder(n->m_pChild[1], s);
        s.push(n);
    }

    // Construye TraversalView a partir del Stack 
    View make_view(Stack<Node*> s) const {
        size_t n = s.size();
        FwdIt fwdB(s, 0);
        FwdIt fwdE(s, n);
        BwdIt bwdB(s, n > 0 ? n - 1 : SIZE_MAX);
        BwdIt bwdE(s, SIZE_MAX);
        return View(fwdB, fwdE, bwdB, bwdE);
    }

    // Helper de serializacion 
    string traversalToString(Stack<Node*>& s) const {
        ostringstream oss;
        oss << "[";
        for (size_t i = 0; i < s.size(); ++i) {
            if (i) oss << ",";
            oss << "(" << s[i]->m_data << "," << s[i]->m_ref << ")";
        }
        oss << "]";
        return oss.str();
    }

public:
    BinaryTree() : m_pRoot(nullptr), m_size(0) {}

    // Copy constructor 
    BinaryTree(const BinaryTree& other) : m_pRoot(nullptr), m_size(0) {
        shared_lock<shared_mutex> lock(other.m_mtx);
        m_pRoot = internal_copy(other.m_pRoot);
        m_size  = other.m_size;
    }

    // Move constructor
    BinaryTree(BinaryTree&& other) noexcept : m_pRoot(nullptr), m_size(0) {
        unique_lock<shared_mutex> lock(other.m_mtx);
        m_pRoot = exchange(other.m_pRoot, nullptr);
        m_size  = exchange(other.m_size, 0);
    }

    BinaryTree& operator=(const BinaryTree& other) {
        if (this != &other) {
            clear();
            shared_lock<shared_mutex> lock(other.m_mtx);
            m_pRoot = internal_copy(other.m_pRoot);
            m_size  = other.m_size;
        }
        return *this;
    }

    BinaryTree& operator=(BinaryTree&& other) noexcept {
        if (this != &other) {
            clear();
            unique_lock<shared_mutex> lock(other.m_mtx);
            m_pRoot = exchange(other.m_pRoot, nullptr);
            m_size  = exchange(other.m_size, 0);
        }
        return *this;
    }

    // Destructor seguro
    virtual ~BinaryTree() { clear(); }

    void clear() {
        unique_lock<shared_mutex> lock(m_mtx);
        internal_clear(m_pRoot);
        m_pRoot = nullptr;
        m_size  = 0;
    }

    // insert
    virtual void insert(const value_type& data, Ref ref) {
        unique_lock<shared_mutex> lock(m_mtx);
        m_pRoot = internal_insert(m_pRoot, data, ref);
        ++m_size;
    }

    // Mejora libre #1: search devuelve tuple
    tuple<value_type, Ref> search(const value_type& data) const {
        shared_lock<shared_mutex> lock(m_mtx);
        Node* hit = internal_search(m_pRoot, data);
        if (!hit) throw runtime_error("elemento no encontrado");
        return make_tuple(hit->m_data, hit->m_ref);
    }

    // Complemento de search
    bool contains(const value_type& data) const {
        shared_lock<shared_mutex> lock(m_mtx);
        return internal_search(m_pRoot, data) != nullptr;
    }

    size_t size() const {
        shared_lock<shared_mutex> lock(m_mtx);
        return m_size;
    }

    // Mejora libre #2: vistas (TraversalView)
    View inorder() const {
        shared_lock<shared_mutex> lock(m_mtx);
        Stack<Node*> s;
        fill_inorder(m_pRoot, s);
        return make_view(s);
    }

    View preorder() const {
        shared_lock<shared_mutex> lock(m_mtx);
        Stack<Node*> s;
        fill_preorder(m_pRoot, s);
        return make_view(s);
    }

    View postorder() const {
        shared_lock<shared_mutex> lock(m_mtx);
        Stack<Node*> s;
        fill_postorder(m_pRoot, s);
        return make_view(s);
    }


    FwdIt begin() const { return inorder().begin(); }
    FwdIt end()   const { return inorder().end();   }

    // ToString
    string toString() const {
        shared_lock<shared_mutex> lock(m_mtx);
        Stack<Node*> s;
        string res;
        fill_inorder(m_pRoot, s);
        res += "Inorder:"   + traversalToString(s) + "\n";
        while (!s.empty()) s.pop();
        fill_preorder(m_pRoot, s);
        res += "Preorder:"  + traversalToString(s) + "\n";
        while (!s.empty()) s.pop();
        fill_postorder(m_pRoot, s);
        res += "Postorder:" + traversalToString(s);
        return res;
    }

    // operator<< 
    friend ostream& operator<<(ostream& os, const BinaryTree& arbol) {
        shared_lock<shared_mutex> lock(arbol.m_mtx);
        Stack<Node*> s;
        arbol.fill_inorder(arbol.m_pRoot, s);
        os << arbol.traversalToString(s);
        return os;
    }

    // operator>> 
    friend istream& operator>>(istream& is, BinaryTree& arbol) {
        char ch;
        if (!(is >> ch) || ch != '[') { is.clear(ios_base::failbit); return is; }
        value_type val; Ref ref; char coma, parC;
        while (is >> ch && ch != ']')
            if (ch == '(')
                if (is >> val >> coma >> ref >> parC)
                    if (coma == ',' && parC == ')')
                        arbol.insert(val, ref);
        return is;
    }

    // printTree: BFS por niveles
    void printTree(ostream& os = cout) const {
        shared_lock<shared_mutex> lock(m_mtx);
        if (!m_pRoot) { os << "(arbol vacio)" << endl; return; }
        Vector<Node*> cola(64);
        size_t inicioNivel = 0;
        cola.push_back(m_pRoot, 0);
        while (inicioNivel < cola.size()) {
            size_t finNivel = cola.size();
            bool   todoNulo = true;
            for (size_t i = inicioNivel; i < finNivel; ++i)
                if (cola[i]) { todoNulo = false; break; }
            if (todoNulo) break;
            os << "  ";
            for (size_t i = inicioNivel; i < finNivel; ++i) {
                Node* n = cola[i];
                if (n) {
                    os << n->m_data;
                    cola.push_back(n->m_pChild[0], 0);
                    cola.push_back(n->m_pChild[1], 0);
                } else {
                    os << "_";
                    cola.push_back(nullptr, 0);
                    cola.push_back(nullptr, 0);
                }
                os << " ";
            }
            os << endl;
            inicioNivel = finNivel;
        }
    }
};

#endif // __BINARYTREE_H__
