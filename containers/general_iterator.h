#ifndef __ITERATOR_H__
#define __ITERATOR_H__

#include <algorithm>
#include <utility>
#include <vector>
#include <memory>
#include "../types.h"


// general_iterator: Reutilizacion de iteradores

template <typename Container, class IteratorBase>
class general_iterator {
public:
    using Node       = typename Container::Node;
    using value_type = typename Container::value_type;
    using myself     = general_iterator<Container, IteratorBase>;

protected:
    Container *m_pContainer;
    Node      *m_pNode;

public:
    general_iterator(Container *pContainer, Node *pNode)
        : m_pContainer(pContainer), m_pNode(pNode) {}

    general_iterator(const myself &other)
        : m_pContainer(other.m_pContainer), m_pNode(other.m_pNode) {}

    general_iterator(myself &&other) {
        m_pContainer = std::move(other.m_pContainer);
        m_pNode      = std::move(other.m_pNode);
    }

    IteratorBase &operator=(IteratorBase &iter) {
        m_pContainer = std::move(iter.m_pContainer);
        m_pNode      = std::move(iter.m_pNode);
        return *(IteratorBase *)this;
    }

    Node *getNode() const { return m_pNode; }

    friend bool operator==(const IteratorBase &a, const IteratorBase &b) { return a.getNode() == b.getNode(); }
    friend bool operator!=(const IteratorBase &a, const IteratorBase &b) { return a.getNode() != b.getNode(); }

    value_type &operator*() { return m_pNode->getDataRef(); }

    IteratorBase &operator++() {
        if (m_pNode) m_pNode = m_pNode->getNext();
        return *(IteratorBase *)this;
    }
};


// circular_iterator: Reutilizacion de iteradores

template <typename Container, class IteratorBase>
class circular_iterator : public general_iterator<Container, IteratorBase> {
public:
    using Node = typename Container::Node;

protected:
    Node *m_pRoot;

public:
    circular_iterator(Container *pContainer, Node *pNode, Node *pRoot)
        : general_iterator<Container, IteratorBase>(pContainer, pNode), m_pRoot(pRoot) {}

    IteratorBase &operator++() {
        if (this->m_pNode) {
            Node *next    = this->m_pNode->getNext();
            this->m_pNode = (next == m_pRoot) ? nullptr : next;
        }
        return *(IteratorBase *)this;
    }
};


// snapshot_container
template <typename T>
struct snapshot_container {
    using Node       = T;
    using value_type = T;
};


// snapshot_iterator
template <typename Node>
class snapshot_iterator
    : public general_iterator<snapshot_container<Node>, snapshot_iterator<Node>> {
public:
    using Base = general_iterator<snapshot_container<Node>, snapshot_iterator<Node>>;

protected:
    std::shared_ptr<std::vector<Node>> m_snap;   // mantiene vivo el snapshot

    void advance() {                             // hacia el final
        if (!this->m_pNode) return;
        Node *last = m_snap->data() + m_snap->size();
        if (++this->m_pNode >= last) this->m_pNode = nullptr;
    }
    void retreat() {                             // hacia el inicio
        if (!this->m_pNode) return;
        if (this->m_pNode == m_snap->data()) this->m_pNode = nullptr;
        else --this->m_pNode;
    }

public:
    snapshot_iterator() : Base(nullptr, nullptr), m_snap(nullptr) {}
    snapshot_iterator(std::shared_ptr<std::vector<Node>> snap, Size pos)
        : Base(nullptr, (snap && pos < snap->size()) ? &(*snap)[pos] : nullptr),
          m_snap(std::move(snap)) {}

    const Node &operator*()  const { return *this->m_pNode; }
    const Node *operator->() const { return  this->m_pNode; }
};

#endif
