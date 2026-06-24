#ifndef CBTreePage_H
#define CBTreePage_H

#include <iostream>
#include <sstream>
#include <cstddef>
#include <stdexcept>
#include <tuple>
#include <utility>
#include "../types.h"
#include "vector.h"
#include "traits.h"
using namespace std;

// BTreePage
template <typename Trait>
class BTreePage {
public:
       using value_type = typename Trait::value_type;
       using Comp       = typename Trait::Comp;
       using Page       = BTreePage<Trait>;
       using KeyVec     = Vector<VectorTrait<value_type>>;
       using ChildVec   = Vector<VectorTrait<Page*>>;
       using KeyNode    = typename KeyVec::Node; 

private:
       KeyVec   m_keys;       
       ChildVec m_children;   
       Comp     m_comp;      
       size_t   m_order;      
       bool     m_leaf;

       
       size_t lowerBound(const value_type& key) {
               size_t lo = 0, hi = m_keys.size();
               while( lo < hi ) {
                       size_t mid = (lo + hi) / 2;
                       if( m_comp(m_keys[mid], key) ) lo = mid + 1;
                       else                           hi = mid;
               }
               return lo;
       }

       
       bool isEqual(const value_type& a, const value_type& b) {
               return !m_comp(a, b) && !m_comp(b, a);
       }

       
       void insertKeyAt(size_t pos, const value_type& key, Ref ref) {
               m_keys.push_back(key, ref);
               for(size_t j = m_keys.size() - 1; j > pos; --j)
                       swap(m_keys.nodeAt(j), m_keys.nodeAt(j - 1));
       }
       void insertChildAt(size_t pos, Page* child) {
               m_children.push_back(child, Ref{});
               for(size_t j = m_children.size() - 1; j > pos; --j)
                       swap(m_children[j], m_children[j - 1]);
       }

       void splitChild(size_t i) {
               size_t t     = m_order;
               Page*  child = m_children[i];
               Page*  z     = new Page(child->m_leaf, t);

               
               for(size_t j = t; j < 2 * t - 1; ++j) {
                       KeyNode& kn = child->m_keys.nodeAt(j);
                       z->m_keys.push_back(kn.getData(), kn.getRef());
               }
               
               if( !child->m_leaf )
                       for(size_t j = t; j < 2 * t; ++j)
                               z->m_children.push_back(child->m_children[j], Ref{});

               
               KeyNode&   medNode = child->m_keys.nodeAt(t - 1);
               value_type medKey  = medNode.getData();
               Ref        medRef  = medNode.getRef();

               
               while( child->m_keys.size() > t - 1 ) child->m_keys.pop_back();
               if( !child->m_leaf )
                       while( child->m_children.size() > t ) child->m_children.pop_back();

               
               insertKeyAt(i, medKey, medRef);
               insertChildAt(i + 1, z);
       }

public:
       BTreePage(bool leaf, size_t order)
               : m_keys(2 * order), m_children(2 * order + 1), m_order(order), m_leaf(leaf) {}

       BTreePage(const BTreePage&)            = delete;
       BTreePage& operator=(const BTreePage&) = delete;

       ~BTreePage() {
               for(size_t i = 0; i < m_children.size(); ++i)
                       delete m_children[i];
       }

       bool isFull() { return m_keys.size() == 2 * m_order - 1; }

       
       void clear() {
               for(size_t i = 0; i < m_children.size(); ++i)
                       delete m_children[i];
               m_children = ChildVec(2 * m_order + 1);
               m_keys     = KeyVec(2 * m_order);
               m_leaf     = true;
       }

       
       void insertNonFull(const value_type& key, Ref ref) {
               if( m_leaf ) {
                       insertKeyAt(lowerBound(key), key, ref);
                       return;
               }
               size_t i = lowerBound(key);
               if( m_children[i]->isFull() ) {
                       splitChild(i);
                       
                       if( m_comp(m_keys[i], key) ) ++i;
               }
               m_children[i]->insertNonFull(key, ref);
       }

       
       void splitRoot() {
               size_t t     = m_order;
               Page*  left  = new Page(m_leaf, t);
               Page*  right = new Page(m_leaf, t);

               for(size_t j = 0; j < t - 1; ++j) {
                       KeyNode& kn = m_keys.nodeAt(j);
                       left->m_keys.push_back(kn.getData(), kn.getRef());
               }
               for(size_t j = t; j < 2 * t - 1; ++j) {
                       KeyNode& kn = m_keys.nodeAt(j);
                       right->m_keys.push_back(kn.getData(), kn.getRef());
               }
               if( !m_leaf ) {
                       for(size_t j = 0; j < t;     ++j)
                               left->m_children.push_back(m_children[j], Ref{});
                       for(size_t j = t; j < 2 * t; ++j)
                               right->m_children.push_back(m_children[j], Ref{});
               }

               KeyNode&   medNode = m_keys.nodeAt(t - 1);
               value_type medKey  = medNode.getData();
               Ref        medRef  = medNode.getRef();

               
               m_keys     = KeyVec(2 * t);
               m_children = ChildVec(2 * t + 1);
               m_leaf     = false;
               m_keys.push_back(medKey, medRef);
               m_children.push_back(left,  Ref{});
               m_children.push_back(right, Ref{});
       }

       
       tuple<value_type, Ref> search(const value_type& key) {
               size_t i = lowerBound(key);
               if( i < m_keys.size() && isEqual(m_keys[i], key) )
                       return { m_keys[i], m_keys.nodeAt(i).getRef() };
               if( m_leaf )
                       throw runtime_error("BTreePage::search: clave no encontrada");
               return m_children[i]->search(key);
       }

       
       template <typename Func, typename... Args>
       void forEach(Func func, Args&&... args) {
               size_t n = m_keys.size();
               for(size_t i = 0; i < n; ++i) {
                       if( !m_leaf ) m_children[i]->forEach(func, args...);
                       func(m_keys[i], args...);
               }
               if( !m_leaf ) m_children[n]->forEach(func, args...);
       }

       
       template <typename Pred, typename... Args>
       KeyNode* firstThat(Pred pred, Args&&... args) {
               size_t n = m_keys.size();
               for(size_t i = 0; i < n; ++i) {
                       if( !m_leaf ) {
                               KeyNode* r = m_children[i]->firstThat(pred, args...);
                               if( r ) return r;
                       }
                       if( pred(m_keys[i], args...) )
                               return &m_keys.nodeAt(i);
               }
               if( !m_leaf ) {
                       KeyNode* r = m_children[n]->firstThat(pred, args...);
                       if( r ) return r;
               }
               return nullptr;
       }

       
       void appendToString(ostringstream& oss, size_t level) {
               size_t n = m_keys.size();
               for(size_t i = 0; i < n; ++i) {
                       if( !m_leaf ) m_children[i]->appendToString(oss, level + 1);
                       for(size_t s = 0; s < level; ++s) oss << "\t";
                       oss << m_keys[i] << "->" << m_keys.nodeAt(i).getRef() << "\n";
               }
               if( !m_leaf ) m_children[n]->appendToString(oss, level + 1);
       }
};

#endif // CBTreePage_H
