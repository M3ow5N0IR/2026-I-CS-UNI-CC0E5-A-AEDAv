// BTree.h

#ifndef BTREE_H
#define BTREE_H

#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <tuple>
#include <mutex>
#include <shared_mutex>
#include <utility>
#include "../types.h"
#include "BTreePage.h"
#include "traits.h"
using namespace std;

#define DEFAULT_BTREE_ORDER 3

// BTree<Trait>
template <typename Trait>
class BTree {
public:
       using value_type = typename Trait::value_type;
       using Comp       = typename Trait::Comp;
       using Page       = BTreePage<Trait>;
       using KeyNode    = typename Page::KeyNode;
       using MySelf     = BTree<Trait>;

private:
       Page   m_root;
       size_t m_order;
       size_t m_height;
       size_t m_numKeys;
       mutable shared_mutex m_mtx;

public:
       BTree(size_t order = DEFAULT_BTREE_ORDER)
               : m_root(true, order), m_order(order), m_height(1), m_numKeys(0) {}

       BTree(const BTree&)            = delete;
       BTree& operator=(const BTree&) = delete;

       ~BTree() {
               unique_lock<shared_mutex> lock(m_mtx);
               m_root.clear();
       }

       // insert
       void insert(const value_type& key, Ref ref) {
               unique_lock<shared_mutex> lock(m_mtx);
               if( m_root.isFull() ) { m_root.splitRoot(); ++m_height; }
               m_root.insertNonFull(key, ref);
               ++m_numKeys;
       }

       // search
       tuple<value_type, Ref> search(const value_type& key) {
               shared_lock<shared_mutex> lock(m_mtx);
               return m_root.search(key);
       }

       // ForEach variadic
       template <typename Func, typename... Args>
       void ForEach(Func func, Args&&... args) {
               shared_lock<shared_mutex> lock(m_mtx);
               m_root.forEach(func, forward<Args>(args)...);
       }

       // FirstThat variadic
       template <typename Pred, typename... Args>
       tuple<value_type, Ref> FirstThat(Pred pred, Args&&... args) {
               shared_lock<shared_mutex> lock(m_mtx);
               KeyNode* p = m_root.firstThat(pred, forward<Args>(args)...);
               if( !p ) throw runtime_error("BTree::FirstThat: ninguna clave cumple");
               return { p->getData(), p->getRef() };
       }

       string toString() {
               shared_lock<shared_mutex> lock(m_mtx);
               ostringstream oss;
               m_root.appendToString(oss, 0);
               return oss.str();
       }

       size_t height() const { shared_lock<shared_mutex> lock(m_mtx); return m_height;  }
       size_t size()   const { shared_lock<shared_mutex> lock(m_mtx); return m_numKeys; }
       size_t order()  const { shared_lock<shared_mutex> lock(m_mtx); return m_order;   }

       friend ostream& operator<<(ostream& os, BTree& bt) {
               return os << bt.toString();
       }
};

#endif // BTREE_H
