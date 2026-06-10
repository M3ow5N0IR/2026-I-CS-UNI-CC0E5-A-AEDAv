#ifndef __HASH_H__
#define __HASH_H__

#include <iostream>
#include <cstddef> // size_t
#include <vector>
#include <list>
#include <string>
#include <sstream>
#include <functional> // hash

using namespace std;

// ----------------------------------------
// Definir Traits
// ----------------------------------------

template<typename K, typename V>
struct HashTrait{
    using KeyType   = K;
    using ValueType = V;
    using Pair      = std::pair<K, V>;
    using Bucket    = std::list<Pair>;
    using HashFunc  = std::hash<K>;
};

// ----------------------------------------
// Hash Table
// ----------------------------------------
template<typename Trait>
class HashMap{
public:
    using MySelf    = HashMap<Trait>;
    using Key       = typename Trait::Key;
    using Value     = typename Trait::Value;
    using Pair      = typename Trait::Pair;
    using Bucket    = typename Trait::Bucket;
    using HashFunc  = typename Trait::HashFunc;
    using Node      = typename Bucket::iterator; // For compatibility

private:
    size_t capacity;
    vector<Bucket> table;
    size_t current_size;
    HashFunc hash_func;

    size_t hash(const Key& key) const {
        return hash_func(key) % capacity;
    }

public:
    HashMap(size_t initial_capacity = 10) : capacity(initial_capacity), current_size(0) {
        table.resize(capacity);
    }

    void insert(const Key& key, const Value& value) {
        size_t index = hash(key);
        Bucket& bucket = table[index];
        for (auto& pair : bucket) {
            if (pair.first == key) {
                pair.second = value; // Update existing
                return;
            }
        }
        bucket.push_back({key, value});
        current_size++;
    }

    bool get(const Key& key, Value& value) const {
        size_t index = hash(key);
        const Bucket& bucket = table[index];
        for (const auto& pair : bucket) {
            if (pair.first == key) {
                value = pair.second;
                return true;
            }
        }
        return false;
    }

    bool remove(const Key& key) {
        size_t index = hash(key);
        Bucket& bucket = table[index];
        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            if (it->first == key) {
                bucket.erase(it);
                current_size--;
                return true;
            }
        }
        return false;
    }

    size_t size() const {
        return current_size;
    }

    bool isEmpty() const {
        return current_size == 0;
    }

    string toString() const {
        ostringstream oss;
        oss << "Hash Table (size: " << current_size <<", capacity: " << capacity << ")\n";
        for (size_t i = 0; i < capacity; ++i) {
            oss << "Bucket " << i << ": ";
            bool first = true;
            for (const auto& pair : table[i]) {
                if (!first) oss << " -> ";
                oss << "[" << pair.first << ": " << pair.second << "]";
                first = false;
            }
            oss << "\n";
        }
        return oss.str();
    }
};



// ----------------------------------------
// Demo Function
// ----------------------------------------
void DemoHash();

#endif // __HASH_H__