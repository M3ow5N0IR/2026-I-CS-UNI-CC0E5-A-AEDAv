#ifndef __AVL_H__
#define __AVL_H__

#include <algorithm>
#include "BinaryTree.h"

// Extender BinaryTreeNode
template<typename T>
struct AVLNode : BinaryTreeNode<T, AVLNode<T>> {
    int m_height;
    AVLNode(T data, Ref ref) : BinaryTreeNode<T, AVLNode<T>>(data, ref), m_height(1) {}
};

// AVL: hereda BinaryTree y agrega rebalanceo
template<typename Trait>
class AVL : public BinaryTree<Trait> {
public:
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;

private:
    int height(Node* n) const {
        if (!n) return 0;
        return n->m_height;
    }

    void update_height(Node* n) {
        if (!n) return;
        n->m_height = 1 + max(height(n->m_pChild[0]), height(n->m_pChild[1]));
    }

    int balance_factor(Node* n) const {
        if (!n) return 0;
        return height(n->m_pChild[0]) - height(n->m_pChild[1]);
    }

    // Rotacion derecha (casos LL / LR)
    void rotate_right(Node* &y) {
        Node* x = y->m_pChild[0];
        Node* B = x->m_pChild[1];
        x->m_pChild[1] = y;
        y->m_pChild[0] = B;
        update_height(y);
        update_height(x);
        y = x;
    }

    // Rotacion izquierda (casos RR / RL)
    void rotate_left(Node* &x) {
        Node* y = x->m_pChild[1];
        Node* B = y->m_pChild[0];
        y->m_pChild[0] = x;
        x->m_pChild[1] = B;
        update_height(x);
        update_height(y);
        x = y;
    }

    // rebalance: aplica LL, LR, RR o RL segun el factor de balance
    void rebalance(Node* &n) {
        update_height(n);
        int bf = balance_factor(n);
        if      (bf >  1 && balance_factor(n->m_pChild[0]) >= 0)  rotate_right(n);
        else if (bf >  1 && balance_factor(n->m_pChild[0]) <  0) { rotate_left (n->m_pChild[0]); rotate_right(n); }
        else if (bf < -1 && balance_factor(n->m_pChild[1]) <= 0)  rotate_left (n);
        else if (bf < -1 && balance_factor(n->m_pChild[1]) >  0) { rotate_right(n->m_pChild[1]); rotate_left (n); }
    }

protected:
    // Adaptar insert de BinaryTree
    void internal_insert(Node* &pNode, const value_type &data, Ref ref) override {
        if (!pNode) { pNode = new AVLNode<value_type>(data, ref); return; }
        auto branch = !this->m_comp(data, pNode->m_data);
        internal_insert(pNode->m_pChild[branch], data, ref);
        rebalance(pNode);
    }

    // Adaptar insert (copia preservando m_height)
    Node* internal_copy(Node* pNode) override {
        if (!pNode) return nullptr;
        auto* n = new AVLNode<value_type>(pNode->m_data, pNode->m_ref);
        n->m_height    = pNode->m_height;
        n->m_pChild[0] = internal_copy(pNode->m_pChild[0]);
        n->m_pChild[1] = internal_copy(pNode->m_pChild[1]);
        return n;
    }

public:
    AVL() : BinaryTree<Trait>() {}

    // Copy constructor: construye base vacia y delega a internal_copy virtual
    AVL(const AVL& other) : BinaryTree<Trait>() {
        shared_lock<shared_mutex> lock(other.m_mtx);
        this->m_pRoot = internal_copy(other.m_pRoot);
    }

    // Move constructor: transfiere via base
    AVL(AVL&& other) : BinaryTree<Trait>(std::move(other)) {}

    AVL& operator=(const AVL& other) {
        if (this != &other) {
            this->clear();
            shared_lock<shared_mutex> lock(other.m_mtx);
            this->m_pRoot = internal_copy(other.m_pRoot);
        }
        return *this;
    }

    AVL& operator=(AVL&& other) {
        BinaryTree<Trait>::operator=(std::move(other));
        return *this;
    }

    virtual ~AVL() {}

    // Altura y balance del arbol completo
    int height() const {
        shared_lock<shared_mutex> lock(this->m_mtx);
        return height(this->m_pRoot);
    }

    int balance() const {
        shared_lock<shared_mutex> lock(this->m_mtx);
        return balance_factor(this->m_pRoot);
    }
};

#endif // __AVL_H__
