// BTree.h

#ifndef BTREE_H
#define BTREE_H

#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include "../types.h"
#include "vector.h"    
#include "traits.h"
#include "BTreePage.h"
using namespace std;

#define DEFAULT_BTREE_ORDER 3

// BTree<Trait>
template <typename Trait>
class BTree
// this is the full version of the BTree
{
       using keyType   = typename Trait::value_type;
       using ObjIDType = Ref;
       typedef CBTreePage<Trait> BTNode;   // useful shorthand
       /*struct ObjectInfo
       {
               keyType first;
               long    second;
               ObjectInfo *&operator->() { return this; }
       };*/

public:
       //typedef ObjectInfo iterator;
       //typedef typename BTNode::lpfnForEach2    lpfnForEach2;
       //typedef typename BTNode::lpfnForEach3    lpfnForEach3;
       //typedef typename BTNode::lpfnFirstThat2  lpfnFirstThat2;
       //typedef typename BTNode::lpfnFirstThat3  lpfnFirstThat3;
       typedef typename BTNode::ObjectInfo ObjectInfo;

public:
       BTree(int order = DEFAULT_BTREE_ORDER, bool unique = true);
       ~BTree();
       //int           Open (char * name, int mode);
       //int           Create (char * name, int mode);
       //int           Close ();
       bool            Insert (const keyType key, const ObjIDType ObjID);
       bool            Remove (const keyType key, const ObjIDType ObjID);
       ObjIDType       Search (const keyType key);
       long            size()  { return m_NumKeys; }
       long            height() { return m_Height;      }
       long            GetOrder() { return m_Order;     }

       void            Print (ostream &os);

       // ForEach / FirstThat variadic
       template <typename Func, typename... Args>
       void            ForEach( Func func, Args&&... args );
       template <typename Func, typename... Args>
       ObjectInfo*     FirstThat( Func func, Args&&... args );
       //typedef               ObjectInfo iterator;

       // toString / operator<< reutilizan Print
       string toString();

protected:
       BTNode          m_Root;
       int             m_Height;  // height of tree
       int             m_Order;   // order of tree
       long            m_NumKeys; // number of keys
       bool            m_Unique;  // Accept the elements only once ?
};

const int MaxHeight = 5;
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
       ObjIDType ObjID = -1;
       m_Root.Search(key, ObjID);
       return ObjID;
}

template <typename Trait>
template <typename Func, typename... Args>
void BTree<Trait>::ForEach(Func func, Args&&... args)
{
       m_Root.ForEach(func, 0, std::forward<Args>(args)...);
}

template <typename Trait>
template <typename Func, typename... Args>
typename BTree<Trait>::ObjectInfo * BTree<Trait>::FirstThat(Func func, Args&&... args)
{
       return m_Root.FirstThat(func, 0, std::forward<Args>(args)...);
}

template <typename Trait>
void BTree<Trait>::Print(ostream &os)
{
       m_Root.Print(os);
}

template <typename Trait>
string BTree<Trait>::toString()
{
       ostringstream oss;
       Print(oss);
       return oss.str();
}

template <typename Trait>
ostream& operator<<(ostream& os, BTree<Trait>& bt)
{
       bt.Print(os);
       return os;
}

#endif
