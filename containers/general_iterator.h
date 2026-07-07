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


// snapshot_iterator
template <typename Node>
class snapshot_iterator {
public:
    using value_type     = Node;
    using container_type = std::vector<Node>;

protected:
    std::shared_ptr<container_type> m_data;
    Size                            m_pos;
    static constexpr Size           npos = static_cast<Size>(-1);

    void advance() { if (++m_pos >= m_data->size()) reset(); }   // hacia el final
    void retreat() { if (m_pos == 0) reset(); else --m_pos; }    // hacia el inicio
    void reset()   { m_data = nullptr; m_pos = npos; }

public:
    snapshot_iterator() : m_data(nullptr), m_pos(npos) {}
    snapshot_iterator(std::shared_ptr<container_type> data, Size pos)
        : m_data(std::move(data)), m_pos(pos) {}

    const Node &operator*()  const { return (*m_data)[m_pos]; }
    const Node *operator->() const { return &(*m_data)[m_pos]; }

    Bool isEnd() const { return !m_data || m_pos == npos || m_pos >= m_data->size(); }

    Bool operator==(const snapshot_iterator &other) const {
        if (isEnd() && other.isEnd()) return true;
        return m_data == other.m_data && m_pos == other.m_pos;
    }
    Bool operator!=(const snapshot_iterator &other) const { return !(*this == other); }
};

#endif
