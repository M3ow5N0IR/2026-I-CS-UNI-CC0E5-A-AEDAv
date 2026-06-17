#ifndef __TRAITS_H__
#define __TRAITS_H__
#include <functional>
#include "../types.h"
using namespace std;

template <typename _Node, typename _Comp>
struct BaseTrait {
    using Node       = _Node;
    using value_type = typename _Node::value_type;
    using Comp       = _Comp;
};

template <typename _Node>
struct AscendingTrait : public BaseTrait<_Node, less<typename _Node::value_type>> {};

template <typename _Node>
struct DescendingTrait : public BaseTrait<_Node, greater<typename _Node::value_type>> {};

// Forward declarations para los traits de Vector/Heap
template <typename T> class VectorNode;

// VectorTrait: para usar Vector con un tipo simple (sin orden)
template <typename T>
struct VectorTrait : public BaseTrait<VectorNode<T>, less<T>> {};

// MinHeapTrait: Heap mantiene minimo en la raiz
template <typename T>
struct MinHeapTrait : public BaseTrait<VectorNode<T>, less<T>> {};

// MaxHeapTrait: Heap mantiene maximo en la raiz
template <typename T>
struct MaxHeapTrait : public BaseTrait<VectorNode<T>, greater<T>> {};

// HashTrait: define el par (Key, Value) que va dentro de cada nodo de la HashTable
template <typename _Key, typename _Value>
struct HashTrait {
    using Key   = _Key;
    using Value = _Value;
};

#endif // __TRAITS_H__
