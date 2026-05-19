#include <iostream>
#include <fstream>
#include <thread>
#include "../types.h"
#include "BinaryTree.h"
using namespace std;

using AscTree = BinaryTree<AscendingTrait<BinaryTreeNode<T1>>>;

void BinaryTreeDemo() {
    cout << "=== PRUEBAS BINARYTREE ===" << endl;

    AscTree t;
    t.insert(5, 50); t.insert(3, 30); t.insert(7, 70);
    t.insert(1, 10); t.insert(4, 40); t.insert(6, 60); t.insert(8, 80);

    // ToString
    cout << "\n--- ToString ---" << endl;
    cout << t.toString() << endl;

    // operator<<
    cout << "\n--- operator<< (consola y archivo) ---" << endl;
    cout << t << endl;
    { ofstream os("tree.txt"); os << t; }

    // operator>>
    cout << "\n--- operator>> ---" << endl;
    AscTree t2;
    { ifstream is("tree.txt"); is >> t2; }
    cout << "escrito: " << t  << endl;
    cout << "leido:   " << t2 << endl;

    // Copy constructor
    cout << "\n--- Copy constructor ---" << endl;
    AscTree tCopy(t);
    cout << "original: " << t     << endl;
    cout << "copia:    " << tCopy << endl;

    // Move constructor
    cout << "\n--- Move constructor ---" << endl;
    AscTree tMove(std::move(tCopy));
    cout << "movido:  " << tMove << endl;
    cout << "vaciado: " << tCopy << endl;

    // Destructor seguro
    cout << "\n--- Destructor seguro ---" << endl;
    { AscTree temp; temp.insert(9, 90); temp.insert(2, 20); }
    cout << "destructor OK" << endl;

    // Forward/Backward iterator inorder
    cout << "\n--- Inorder ---" << endl;
    cout << "fwd: ";
    for (auto& val : t.inorder()) cout << val << " ";
    cout << endl;
    cout << "bwd: ";
    for (auto it = t.inorder().rbegin(); it != t.inorder().rend(); ++it)
        cout << *it << " ";
    cout << endl;

    // Range-based for nativo
    cout << "\n--- foreach nativo ---" << endl;
    cout << "for(auto& v : t): ";
    for (auto& val : t) cout << val << " ";
    cout << endl;

    // Forward/Backward iterator preorder
    cout << "\n--- Preorder ---" << endl;
    cout << "fwd: ";
    t.preorder().forEach([](T1& v) { cout << v << " "; });
    cout << endl;
    cout << "bwd: ";
    t.preorder().rForEach([](T1& v) { cout << v << " "; });
    cout << endl;

    // Forward/Backward iterator postorder
    cout << "\n--- Postorder ---" << endl;
    cout << "fwd: ";
    t.postorder().forEach([](T1& v) { cout << v << " "; });
    cout << endl;
    cout << "bwd: ";
    t.postorder().rForEach([](T1& v) { cout << v << " "; });
    cout << endl;

    // search devuelve tuple
    cout << "\n--- Search (tuple) ---" << endl;
    auto [val, ref] = t.search(4);
    cout << "search(4) -> dato: " << val << " ref: " << ref << endl;

    // printTree
    cout << "\n--- printTree ---" << endl;
    t.printTree();

    // Concurrencia
    cout << "\n--- Concurrencia ---" << endl;
    AscTree tConc;
    auto worker = [&tConc](int id) {
        for (int i = 0; i < 200; i++) tConc.insert(i * id, id);
    };
    thread th1(worker, 1), th2(worker, 2), th3(worker, 3),
           th4(worker, 4), th5(worker, 5);
    th1.join(); th2.join(); th3.join(); th4.join(); th5.join();
    cout << "esperado <=1000 (sin duplicados): " << tConc.size() << endl;

    cout << "\n=== FIN BINARYTREE ===" << endl;
}
