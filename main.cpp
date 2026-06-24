#include <iostream>
#include "containers/vector.h"
#include "containers/linkedlist.h"
#include "containers/BinaryTree.h"
#include "containers/avl.h"
#include "containers/heap.h"
#include "containers/hashtable.h"
#include "containers/BTree.h"

void ListsDemo();
void BinaryTreeDemo();
void AVLDemo();
void HeapDemo();
void DemoHashTable();
void BTreeDemo(std::ostream& os);

int main(){
    ListsDemo();
    BinaryTreeDemo();
    AVLDemo();
    HeapDemo();
    DemoHashTable();
    BTreeDemo(std::cout);
    return 0;
}
