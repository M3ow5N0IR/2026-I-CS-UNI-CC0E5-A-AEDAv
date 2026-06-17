#ifndef __AVL_H__
#define __AVL_H__

#include <type_traits>
#include "BinaryTree.h"
using namespace std;

template <typename T, typename = void>
struct has_m_value : std::false_type {};

template <typename T>
struct has_m_value<T, std::void_t<decltype(std::declval<T&>().m_value)>>
    : std::true_type {};

// Extender BinaryTreeNode: AVLNode hereda
template<typename T>
struct AVLNode : BinaryTreeNode<T, AVLNode<T>> {
    Height m_height;
    AVLNode(T data, Ref ref = 0)
        : BinaryTreeNode<T, AVLNode<T>>(data, ref), m_height(1) {}
};

// Traits especificos para AVL
template<typename T> using AscendingAVLTrait  = AscendingTrait<AVLNode<T>>;
template<typename T> using DescendingAVLTrait = DescendingTrait<AVLNode<T>>;

template<typename Trait>
class AVLTree : public BinaryTree<Trait> {
public:
    using Base       = BinaryTree<Trait>;
    using Node       = typename Base::Node;
    using value_type = typename Base::value_type;
    using Base::Base; // hereda constructores de BinaryTree

    // Copy: reusa internal_copy
    AVLTree(const AVLTree& other) : Base() {
        shared_lock<shared_mutex> lock(other.m_mtx);
        this->m_pRoot = this->internal_copy(other.m_pRoot);
        this->m_size  = other.m_size;
    }

    // Move: delega al move ctor del padre
    AVLTree(AVLTree&& other) noexcept : Base(std::move(other)) {}

    AVLTree& operator=(const AVLTree& other) {
        if (this != &other) {
            this->clear();
            shared_lock<shared_mutex> lock(other.m_mtx);
            this->m_pRoot = this->internal_copy(other.m_pRoot);
            this->m_size  = other.m_size;
        }
        return *this;
    }

    AVLTree& operator=(AVLTree&& other) noexcept {
        Base::operator=(std::move(other));
        return *this;
    }

    virtual ~AVLTree() {}

    // Altura del arbol
    Height height() const {
        shared_lock<shared_mutex> lock(this->m_mtx);
        return alturaNodo(this->m_pRoot);
    }

    // Factor de balance de la raiz
    Balance balance() const {
        shared_lock<shared_mutex> lock(this->m_mtx);
        return factorBalance(this->m_pRoot);
    }

protected:
    // Adaptar insert de BinaryTree
    Node* internal_insert(Node* pNode, const value_type& data, Ref ref) override {
        pNode = Base::internal_insert(pNode, data, ref);
        recalcularAltura(pNode);
        return rebalancear(pNode);
    }

    // Override internal_copy
    Node* internal_copy(Node* pNode) override {
        if (!pNode) return nullptr;
        Node* clon        = this->make_node(pNode->m_data, pNode->m_ref);
        clon->m_height    = pNode->m_height;
        if constexpr (has_m_value<Node>::value) {
            clon->m_value = pNode->m_value;
        }
        clon->m_pChild[0] = internal_copy(pNode->m_pChild[0]);
        clon->m_pChild[1] = internal_copy(pNode->m_pChild[1]);
        return clon;
    }

private:
    Height alturaNodo(Node* n) const { return n ? n->m_height : 0; }

    Balance factorBalance(Node* n) const {
        return n ? (Balance)alturaNodo(n->m_pChild[0]) - (Balance)alturaNodo(n->m_pChild[1]) : 0;
    }

    void recalcularAltura(Node* n) {
        if (!n) return;
        Height hIzq = alturaNodo(n->m_pChild[0]);
        Height hDer = alturaNodo(n->m_pChild[1]);
        n->m_height = 1 + (hIzq > hDer ? hIzq : hDer);
    }

    // rotar(nodo, hijoPesado)
    Node* rotar(Node* nodo, size_t hijoPesado) {
        size_t hijoLigero = 1 - hijoPesado;
        Node* pivote = nodo->m_pChild[hijoPesado];
        Node* medio  = pivote->m_pChild[hijoLigero];
        pivote->m_pChild[hijoLigero] = nodo;
        nodo->m_pChild[hijoPesado]   = medio;
        recalcularAltura(nodo);
        recalcularAltura(pivote);
        return pivote;
    }

    // rebalancear
    Node* rebalancear(Node* n) {
        Balance bf = factorBalance(n);

        if (bf > 1) {
            // izquierda pesada (LL o LR)
            if (factorBalance(n->m_pChild[0]) < 0)
                n->m_pChild[0] = rotar(n->m_pChild[0], 1); // pre-rota LR
            return rotar(n, 0); // rotacion derecha final
        }
        if (bf < -1) {
            // derecha pesada (RR o RL)
            if (factorBalance(n->m_pChild[1]) > 0)
                n->m_pChild[1] = rotar(n->m_pChild[1], 0); // pre-rota RL
            return rotar(n, 1); // rotacion izquierda final
        }
        return n;
    }
};

#endif // __AVL_H__
