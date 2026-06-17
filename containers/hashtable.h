#ifndef __HASHTABLE_H__
#define __HASHTABLE_H__

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <functional>
#include <shared_mutex>
#include <utility>
#include "avl.h"
#include "../types.h"
using namespace std;

template<typename Key, typename Value>
struct HashNode : BinaryTreeNode<Key, HashNode<Key, Value>> {
    Height m_height;
    Value  m_value;

    HashNode(Key key, Ref ref)
        : BinaryTreeNode<Key, HashNode<Key, Value>>(key, ref),
          m_height(1), m_value(Value{}) {}
};

// HashBucket
template<typename Trait>
class HashBucket {
public:
    using Key   = typename Trait::Key;
    using Value = typename Trait::Value;
    using Node  = HashNode<Key, Value>;
    using AVL   = AVLTree<AscendingTrait<Node>>;

private:
    AVL m_avl;

public:
    HashBucket() = default;

    HashBucket(const HashBucket& other) : m_avl(other.m_avl) {}
    HashBucket(HashBucket&& other) noexcept : m_avl(std::move(other.m_avl)) {}

    HashBucket& operator=(const HashBucket& other) {
        m_avl = other.m_avl;
        return *this;
    }
    HashBucket& operator=(HashBucket&& other) noexcept {
        m_avl = std::move(other.m_avl);
        return *this;
    }

    // findNode
    Node* findNode(const Key& key) const {
        return m_avl.find(key);
    }

    // insertOrUpdate
    bool insertOrUpdate(const Key& key, const Value& value, Ref ref = 0) {
        Node* found = m_avl.find(key);
        if (found) {
            found->m_value = value;
            return false;
        }
        m_avl.insert(key, ref);
        Node* nuevo = m_avl.find(key);
        if (nuevo) nuevo->m_value = value;
        return true;
    }

    size_t size()    const { return m_avl.size(); }
    bool   isEmpty() const { return m_avl.size() == 0; }

    // Iteracion
    auto inorder() const { return m_avl.inorder(); }
    auto begin()   const { return m_avl.begin(); }
    auto end()     const { return m_avl.end(); }

    // reutiliza operator<< y operator>> del AVL
    friend ostream& operator<<(ostream& os, const HashBucket& b) {
        return os << b.m_avl;
    }
    friend istream& operator>>(istream& is, HashBucket& b) {
        return is >> b.m_avl;
    }
};

template<typename Key, typename Value>
struct KVPair {
    const Key& key;
    Value&     value;
};

namespace std {
    template<typename Key, typename Value>
    struct tuple_size<KVPair<Key, Value>> : integral_constant<size_t, 2> {};

    template<typename Key, typename Value>
    struct tuple_element<0, KVPair<Key, Value>> { using type = const Key; };

    template<typename Key, typename Value>
    struct tuple_element<1, KVPair<Key, Value>> { using type = Value; };
}

template<size_t I, typename Key, typename Value>
decltype(auto) get(KVPair<Key, Value>& p) {
    if constexpr (I == 0) return p.key;
    else                  return p.value;
}

template<size_t I, typename Key, typename Value>
decltype(auto) get(const KVPair<Key, Value>& p) {
    if constexpr (I == 0) return p.key;
    else                  return p.value;
}

template<typename Trait>
class HashTable {
public:
    using Key    = typename Trait::Key;
    using Value  = typename Trait::Value;
    using Node   = HashNode<Key, Value>;
    using Bucket = HashBucket<Trait>;
    using MySelf = HashTable<Trait>;

private:
    static constexpr size_t kDefaultCapacity = 17;

    Bucket              *m_buckets;
    size_t               m_capacity;
    size_t               m_size;
    mutable shared_mutex m_mtx;

    size_t bucket_index(const Key& key) const {
        return std::hash<Key>{}(key) % m_capacity;
    }

public:
    // for (const auto& [key, value] : m)
    struct Iterator {
        HashTable *m_table;
        size_t     m_bucket;
        Node      *m_node;
        typename Bucket::AVL::FwdIt m_it;
        typename Bucket::AVL::FwdIt m_end;

        Iterator(HashTable* t, size_t b)
            : m_table(t), m_bucket(b),
              m_node(nullptr),
              m_it(Stack<Node*>(), 0),
              m_end(Stack<Node*>(), 0) {
            advanceToNonEmpty();
        }

        void advanceToNonEmpty() {
            while (m_bucket < m_table->m_capacity) {
                auto view = m_table->m_buckets[m_bucket].inorder();
                m_it  = view.begin();
                m_end = view.end();
                if (m_it != m_end) {
                    m_node = static_cast<Node*>(m_it.getNode());
                    return;
                }
                ++m_bucket;
            }
            m_node = nullptr;
        }

        KVPair<Key, Value> operator*() const {
            return { m_node->m_data, m_node->m_value };
        }

        Iterator& operator++() {
            ++m_it;
            if (m_it != m_end) {
                m_node = static_cast<Node*>(m_it.getNode());
            } else {
                ++m_bucket;
                advanceToNonEmpty();
            }
            return *this;
        }

        bool operator==(const Iterator& o) const {
            return m_bucket == o.m_bucket && m_node == o.m_node;
        }
        bool operator!=(const Iterator& o) const { return !(*this == o); }
    };

    Iterator begin() { return Iterator(this, 0); }
    Iterator end()   {
        Iterator it(this, m_capacity);
        it.m_node = nullptr;
        return it;
    }

    HashTable(size_t capacity = kDefaultCapacity)
        : m_buckets(new Bucket[capacity]), m_capacity(capacity), m_size(0) {}

    // Constructor copia
    HashTable(const HashTable& other)
        : m_buckets(nullptr), m_capacity(0), m_size(0) {
        shared_lock<shared_mutex> lock(other.m_mtx);
        m_capacity = other.m_capacity;
        m_size     = other.m_size;
        m_buckets  = new Bucket[m_capacity];
        for (size_t i = 0; i < m_capacity; ++i)
            m_buckets[i] = other.m_buckets[i];
    }

    // Move constructor
    HashTable(HashTable&& other) noexcept
        : m_buckets(nullptr), m_capacity(0), m_size(0) {
        unique_lock<shared_mutex> lock(other.m_mtx);
        m_buckets  = exchange(other.m_buckets,  nullptr);
        m_capacity = exchange(other.m_capacity, 0);
        m_size     = exchange(other.m_size,     0);
    }

    HashTable& operator=(const HashTable& other) {
        if (this != &other) {
            unique_lock<shared_mutex> lock(m_mtx);
            shared_lock<shared_mutex> olock(other.m_mtx);
            delete[] m_buckets;
            m_capacity = other.m_capacity;
            m_size     = other.m_size;
            m_buckets  = new Bucket[m_capacity];
            for (size_t i = 0; i < m_capacity; ++i)
                m_buckets[i] = other.m_buckets[i];
        }
        return *this;
    }

    HashTable& operator=(HashTable&& other) noexcept {
        if (this != &other) {
            unique_lock<shared_mutex> lock(m_mtx);
            unique_lock<shared_mutex> olock(other.m_mtx);
            delete[] m_buckets;
            m_buckets  = exchange(other.m_buckets,  nullptr);
            m_capacity = exchange(other.m_capacity, 0);
            m_size     = exchange(other.m_size,     0);
        }
        return *this;
    }

    virtual ~HashTable() { delete[] m_buckets; }

    void insert(const Key& key, const Value& value, Ref ref = 0) {
        unique_lock<shared_mutex> lock(m_mtx);
        size_t idx = bucket_index(key);
        bool nuevo = m_buckets[idx].insertOrUpdate(key, value, ref);
        if (nuevo) ++m_size;
    }

    // m[5] = 3 (sobrecarga de operator[])
    Value& operator[](const Key& key) {
        unique_lock<shared_mutex> lock(m_mtx);
        size_t idx  = bucket_index(key);
        Node*  node = m_buckets[idx].findNode(key);
        if (node) return node->m_value;
        m_buckets[idx].insertOrUpdate(key, Value{});
        ++m_size;
        return m_buckets[idx].findNode(key)->m_value;
    }

    Value search(const Key& key) const {
        shared_lock<shared_mutex> lock(m_mtx);
        Node* hit = m_buckets[bucket_index(key)].findNode(key);
        if (!hit) throw runtime_error("key no encontrada");
        return hit->m_value;
    }

    bool contains(const Key& key) const {
        shared_lock<shared_mutex> lock(m_mtx);
        return m_buckets[bucket_index(key)].findNode(key) != nullptr;
    }

    size_t size()    const { shared_lock<shared_mutex> lock(m_mtx); return m_size; }
    bool   isEmpty() const { shared_lock<shared_mutex> lock(m_mtx); return m_size == 0; }

    string toString() const {
        shared_lock<shared_mutex> lock(m_mtx);
        ostringstream oss;
        oss << "{";
        bool first = true;
        for (size_t i = 0; i < m_capacity; ++i) {
            for (auto it = m_buckets[i].inorder().begin();
                 it != m_buckets[i].inorder().end(); ++it) {
                Node* n = static_cast<Node*>(it.getNode());
                if (!first) oss << ",";
                oss << n->m_data << ":" << n->m_value;
                first = false;
            }
        }
        oss << "}";
        return oss.str();
    }

    // operator<<
    friend ostream& operator<<(ostream& os, const HashTable& t) {
        shared_lock<shared_mutex> lock(t.m_mtx);
        os << "{";
        for (size_t i = 0; i < t.m_capacity; ++i)
            os << t.m_buckets[i];
        os << "}";
        return os;
    }

    // operator>>
    friend istream& operator>>(istream& is, HashTable& t) {
        Char ch;
        if (!(is >> ch) || ch != '{') { is.clear(ios_base::failbit); return is; }
        for (size_t i = 0; i < t.m_capacity; ++i) {
            is >> t.m_buckets[i];
            t.m_size += t.m_buckets[i].size();
        }
        is >> ch; // consume '}'
        return is;
    }
};

#endif // __HASHTABLE_H__
