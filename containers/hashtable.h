#ifndef __HASHTABLE_H__
#define __HASHTABLE_H__

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <functional>
#include <shared_mutex>
#include <type_traits>
#include <utility>
#include "avl.h"
#include "../types.h"
using namespace std;

// KVPair
template <typename Key, typename Value, typename Compare = less<Key>>
struct KVPair {
    using value_type = KVPair;

    Key           m_key;
    mutable Value m_value;

    KVPair() : m_key(), m_value() {}
    KVPair(const Key& key, const Value& value = Value{}) : m_key(key), m_value(value) {}

    bool operator< (const KVPair& other) const { return Compare{}(m_key, other.m_key); }
    bool operator> (const KVPair& other) const { return Compare{}(other.m_key, m_key); }
    bool operator==(const KVPair& other) const { return !(*this < other) && !(other < *this); }

    friend ostream& operator<<(ostream& os, const KVPair& kv) {
        return os << kv.m_key << "=>" << kv.m_value;
    }

    friend istream& operator>>(istream& is, KVPair& kv) {
        Char sep;
        if (!(is >> kv.m_key >> sep) || sep != '=') {
            is.clear(ios_base::failbit);
            return is;
        }
        Char arrow;
        if (!is.get(arrow) || arrow != '>') {
            is.clear(ios_base::failbit);
            return is;
        }
        string buffer;
        Char c;
        while (is.get(c) && c != ',' && c != ')') buffer += c;
        if (c == ',' || c == ')') is.putback(c);
        if constexpr (is_same_v<Value, string>) {
            kv.m_value = buffer;
        } else {
            istringstream parser(buffer);
            parser >> kv.m_value;
        }
        return is;
    }
};

// HashTable
template <typename Trait>
class HashTable {
public:
    using Key       = typename Trait::Key;
    using Value     = typename Trait::Value;
    using Compare   = typename Trait::Compare;
    using Entry     = typename Trait::Entry;
    using Container = typename Trait::Container;
    using Node      = typename Container::Node;

private:
    Container            m_container;
    mutable shared_mutex m_mtx;

public:
    HashTable()  = default;
    ~HashTable() = default;
    // Copy Constructor
    HashTable(const HashTable& other) {
        shared_lock<shared_mutex> lock(other.m_mtx);
        m_container = other.m_container;
    }

    HashTable(HashTable&& other) noexcept {
        unique_lock<shared_mutex> lock(other.m_mtx);
        m_container = std::move(other.m_container);
    }

    HashTable& operator=(const HashTable& other) {
        if (this != &other) {
            unique_lock<shared_mutex> lockSelf(m_mtx);
            shared_lock<shared_mutex> lockOther(other.m_mtx);
            m_container = other.m_container;
        }
        return *this;
    }

    HashTable& operator=(HashTable&& other) noexcept {
        if (this != &other) {
            unique_lock<shared_mutex> lockSelf(m_mtx);
            unique_lock<shared_mutex> lockOther(other.m_mtx);
            m_container = std::move(other.m_container);
        }
        return *this;
    }

    // m[5] = 3 (sobrecarga de operator[])
    Value& operator[](const Key& key) {
        unique_lock<shared_mutex> lock(m_mtx);
        if (Node* hit = m_container.find(Entry(key)))
            return hit->m_data.m_value;
        m_container.insert(Entry(key), Ref{});
        return m_container.find(Entry(key))->m_data.m_value;
    }

    // at: solo lectura
    const Value& at(const Key& key) const {
        shared_lock<shared_mutex> lock(m_mtx);
        Node* hit = m_container.find(Entry(key));
        if (!hit) throw out_of_range("HashTable::at: la key no existe");
        return hit->m_data.m_value;
    }

    bool contains(const Key& key) const {
        shared_lock<shared_mutex> lock(m_mtx);
        return m_container.find(Entry(key)) != nullptr;
    }

    size_t size()    const { return m_container.size(); }
    bool   isEmpty() const { return m_container.size() == 0; }

    // for (const auto& [key, value] : m)
    auto begin()       { return m_container.begin(); }
    auto end()         { return m_container.end();   }
    auto begin() const { return m_container.begin(); }
    auto end()   const { return m_container.end();   }

    // operator<<
    friend ostream& operator<<(ostream& os, const HashTable& t) {
        return os << t.m_container;
    }

    // operator>>
    friend istream& operator>>(istream& is, HashTable& t) {
        return is >> t.m_container;
    }

    // toString
    string toString() const {
        ostringstream oss;
        oss << "{ ";
        bool inicio = true;
        m_container.inorder().forEach([&](const Entry& e) {
            if (!inicio) oss << " | ";
            oss << e;
            inicio = false;
        });
        oss << " }";
        return oss.str();
    }
};

#endif // __HASHTABLE_H__
