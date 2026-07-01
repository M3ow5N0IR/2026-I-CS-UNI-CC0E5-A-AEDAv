// BTree.h

#ifndef BTREE_H
#define BTREE_H

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <utility>
#include <mutex>
#include <shared_mutex>
#include "../types.h"
#include "vector.h"
#include "traits.h"
#include "BTreePage.h"
using namespace std;

#define DEFAULT_BTREE_ORDER 3

// BTree<Trait>
template <typename Trait>
class BTree
{
       using keyType   = typename Trait::value_type;
       using ObjIDType = Ref;
       typedef CBTreePage<Trait> BTNode;

public:
       typedef typename BTNode::ObjectInfo ObjectInfo;   // tagObjectInfo (key + ObjID)

       //Forward Iterator
       class forward_iterator {
              shared_ptr<vector<ObjectInfo>> m_snap;
              size_t                         m_pos;
              static constexpr size_t        npos = static_cast<size_t>(-1);
       public:
              forward_iterator() : m_snap(nullptr), m_pos(npos) {}
              forward_iterator(shared_ptr<vector<ObjectInfo>> snap, size_t pos)
                     : m_snap(std::move(snap)), m_pos(pos) {}

              const ObjectInfo& operator*()  const { return (*m_snap)[m_pos]; }
              const ObjectInfo* operator->() const { return &(*m_snap)[m_pos]; }

              forward_iterator& operator++() {          // avanza hacia el final
                     if (++m_pos >= m_snap->size()) { m_snap = nullptr; m_pos = npos; }
                     return *this;
              }
              bool atEnd() const { return !m_snap || m_pos >= m_snap->size(); }
              friend bool operator==(const forward_iterator& a, const forward_iterator& b) {
                     if (a.atEnd() && b.atEnd()) return true;
                     return a.m_snap == b.m_snap && a.m_pos == b.m_pos;
              }
              friend bool operator!=(const forward_iterator& a, const forward_iterator& b) { return !(a == b); }
       };

       // Backward Iterator
       class backward_iterator {
              shared_ptr<vector<ObjectInfo>> m_snap;
              size_t                         m_pos;
              static constexpr size_t        npos = static_cast<size_t>(-1);
       public:
              backward_iterator() : m_snap(nullptr), m_pos(npos) {}
              backward_iterator(shared_ptr<vector<ObjectInfo>> snap, size_t pos)
                     : m_snap(std::move(snap)), m_pos(pos) {}

              const ObjectInfo& operator*()  const { return (*m_snap)[m_pos]; }
              const ObjectInfo* operator->() const { return &(*m_snap)[m_pos]; }

              backward_iterator& operator++() {         // avanza hacia el inicio
                     if (m_pos == 0) { m_snap = nullptr; m_pos = npos; }
                     else --m_pos;
                     return *this;
              }
              bool atEnd() const { return !m_snap || m_pos == npos; }
              friend bool operator==(const backward_iterator& a, const backward_iterator& b) {
                     if (a.atEnd() && b.atEnd()) return true;
                     return a.m_snap == b.m_snap && a.m_pos == b.m_pos;
              }
              friend bool operator!=(const backward_iterator& a, const backward_iterator& b) { return !(a == b); }
       };

       BTree(int order = DEFAULT_BTREE_ORDER, bool unique = true);
       ~BTree();

       bool        Insert (const keyType key, const ObjIDType ObjID);
       bool        Remove (const keyType key, const ObjIDType ObjID);
       ObjIDType   Search (const keyType key);
       long        size();
       long        height();
       long        GetOrder();

       void        Print (ostream &os);
       string      toString();

       // ForEach / FirstThat variadic
       template <typename Func, typename... Args>
       void        ForEach(Func func, Args&&... args);
       template <typename Func, typename... Args>
       ObjectInfo* FirstThat(Func func, Args&&... args);

       // Iteradores: begin/end (forward) y rbegin/rend (backward)
       forward_iterator  begin();
       forward_iterator  end();
       backward_iterator rbegin();
       backward_iterator rend();

protected:
       BTNode               m_Root;
       int                  m_Height;
       int                  m_Order;
       long                 m_NumKeys;
       bool                 m_Unique;
       mutable shared_mutex m_mtx;   // Concurrencia
};

template <typename Trait>
BTree<Trait>::BTree(int order, bool unique)
                               : m_Root(2 * order + 1, unique),
                                 m_Height(1),
                                 m_Order(order),
                                 m_NumKeys(0),
                                 m_Unique(unique)
{
       m_Root.SetMaxKeysForChilds(order);
}

template <typename Trait>
BTree<Trait>::~BTree()
{
}

template <typename Trait>
bool BTree<Trait>::Insert(const keyType key, const ObjIDType ObjID)
{
       unique_lock<shared_mutex> lock(m_mtx);
       bt_ErrorCode error = m_Root.Insert(key, ObjID);
       if( error == bt_duplicate )
               return false;
       m_NumKeys++;
       if( error == bt_overflow )
       {
               m_Root.SplitRoot();
               m_Height++;
       }
       return true;
}

template <typename Trait>
bool BTree<Trait>::Remove (const keyType key, const ObjIDType ObjID)
{
       unique_lock<shared_mutex> lock(m_mtx);
       bt_ErrorCode error = m_Root.Remove(key, ObjID);
       if( error == bt_duplicate || error == bt_nofound )
               return false;
       m_NumKeys--;
       if( error == bt_rootmerged )
               m_Height--;
       return true;
}

template <typename Trait>
typename BTree<Trait>::ObjIDType BTree<Trait>::Search (const keyType key)
{
       shared_lock<shared_mutex> lock(m_mtx);
       ObjIDType ObjID = -1;
       m_Root.Search(key, ObjID);
       return ObjID;
}

template <typename Trait>
long BTree<Trait>::size()
{
       shared_lock<shared_mutex> lock(m_mtx);
       return m_NumKeys;
}

template <typename Trait>
long BTree<Trait>::height()
{
       shared_lock<shared_mutex> lock(m_mtx);
       return m_Height;
}

template <typename Trait>
long BTree<Trait>::GetOrder()
{
       shared_lock<shared_mutex> lock(m_mtx);
       return m_Order;
}

template <typename Trait>
template <typename Func, typename... Args>
void BTree<Trait>::ForEach(Func func, Args&&... args)
{
       shared_lock<shared_mutex> lock(m_mtx);
       m_Root.ForEach(func, 0, std::forward<Args>(args)...);
}

template <typename Trait>
template <typename Func, typename... Args>
typename BTree<Trait>::ObjectInfo * BTree<Trait>::FirstThat(Func func, Args&&... args)
{
       shared_lock<shared_mutex> lock(m_mtx);
       return m_Root.FirstThat(func, 0, std::forward<Args>(args)...);
}

// Print: recorren con ForEach (ya no hay Print/PrintNodeHelper aparte).
template <typename Trait>
void BTree<Trait>::Print(ostream &os)
{
       shared_lock<shared_mutex> lock(m_mtx);
       m_Root.ForEach([&os](ObjectInfo &info, int level) {
               for (int i = 0; i < level; i++) os << "\t";
               os << "(" << info.key << "," << info.ObjID << ")\n";
       }, 0);
}

template <typename Trait>
string BTree<Trait>::toString()
{
       shared_lock<shared_mutex> lock(m_mtx);
       ostringstream oss;
       m_Root.ForEach([&oss](ObjectInfo &info, int level) {
               for (int i = 0; i < level; i++) oss << "\t";
               oss << "(" << info.key << "," << info.ObjID << ")\n";
       }, 0);
       return oss.str();
}

// Iteradores
template <typename Trait>
typename BTree<Trait>::forward_iterator BTree<Trait>::begin()
{
       auto snap = make_shared<vector<ObjectInfo>>();
       ForEach([snap](ObjectInfo &info, int) { snap->push_back(info); });
       return forward_iterator(snap, 0);
}

template <typename Trait>
typename BTree<Trait>::forward_iterator BTree<Trait>::end()
{
       return forward_iterator();
}

template <typename Trait>
typename BTree<Trait>::backward_iterator BTree<Trait>::rbegin()
{
       auto snap = make_shared<vector<ObjectInfo>>();
       ForEach([snap](ObjectInfo &info, int) { snap->push_back(info); });
       if (snap->empty()) return backward_iterator();
       return backward_iterator(snap, snap->size() - 1);
}

template <typename Trait>
typename BTree<Trait>::backward_iterator BTree<Trait>::rend()
{
       return backward_iterator();
}

// operator<<: imprime via Print (que ya recorre con ForEach).
template <typename Trait>
ostream& operator<<(ostream& os, BTree<Trait>& bt)
{
       bt.Print(os);
       return os;
}

// operator>>: lee nodos con formato (clave,ObjID) y los inserta.
template <typename Trait>
istream& operator>>(istream& is, BTree<Trait>& bt)
{
       using keyType   = typename Trait::value_type;
       keyType key;
       Ref     id;
       char    ch;
       while (is >> ch) {
               if (ch != '(') break;      // fin de datos o formato invalido
               is >> key >> ch;           // clave y la coma
               is >> id  >> ch;           // ObjID y el parentesis de cierre
               if (!is) break;
               bt.Insert(key, id);
       }
       return is;
}

#endif
