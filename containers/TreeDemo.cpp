#include <iostream>
#include <fstream>
#include <thread>
#include "../types.h"
#include "BinaryTree.h"
using namespace std;

using ArbolAsc = BinaryTree<AscendingTrait<BinaryTreeNode<T1>>>;

void BinaryTreeDemo() {
    cout << "\n>>> PRUEBAS BINARYTREE <<<" << endl;

    ArbolAsc arbol;
    arbol.insert(5, 50); arbol.insert(3, 30); arbol.insert(7, 70);
    arbol.insert(1, 10); arbol.insert(4, 40); arbol.insert(6, 60); arbol.insert(8, 80);

    // ToString muestra los 3 recorridos
    cout << "\n toString (los 3 recorridos):" << endl;
    cout << arbol.toString() << endl;

    // operator<< a consola y archivo
    cout << "\n operator<< (consola + archivo tree.txt):" << endl;
    cout << "  consola: " << arbol << endl;
    { ofstream archivo("tree.txt"); archivo << arbol; }

    // operator>> reconstruye desde archivo
    cout << "\n operator>> (lee tree.txt en arbol2):" << endl;
    ArbolAsc arbol2;
    { ifstream archivo("tree.txt"); archivo >> arbol2; }
    cout << "  original: " << arbol  << endl;
    cout << "  leido:    " << arbol2 << endl;

    // Copy constructor produce una copia independiente
    cout << "\n Copy constructor (modificar copia NO afecta original):" << endl;
    ArbolAsc copia(arbol);
    copia.insert(9, 90);
    cout << "  original: " << arbol << " (sin 9)" << endl;
    cout << "  copia:    " << copia << " (con 9)" << endl;

    // Move constructor
    cout << "\n Move constructor:" << endl;
    ArbolAsc movido(std::move(copia));
    cout << "  movido:  " << movido << endl;
    cout << "  vaciado: " << copia  << " (debe ser [])" << endl;

    // Destructor seguro (scope cerrado)
    cout << "\n Destructor seguro (scope artificial):" << endl;
    { ArbolAsc local; local.insert(2, 20); local.insert(11, 110); }
    cout << "  scope cerrado sin errores" << endl;

    // Inorder forward y backward
    cout << "\n Inorder forward / backward:" << endl;
    cout << "  fwd: ";
    for (auto& v : arbol.inorder()) cout << v << " ";
    cout << endl << "  bwd: ";
    for (auto it = arbol.inorder().rbegin(); it != arbol.inorder().rend(); ++it)
        cout << *it << " ";
    cout << endl;

    // Range-based for nativo (usa inorder)
    cout << "\n Range-based for (default = inorder):" << endl;
    for (auto& v : arbol) cout << v << " ";
    cout << endl;

    // Preorder forward y backward
    cout << "\n Preorder forward / backward (via forEach):" << endl;
    cout << "  fwd: ";
    arbol.preorder().forEach([](T1& v) { cout << v << " "; });
    cout << endl << "  bwd: ";
    arbol.preorder().rForEach([](T1& v) { cout << v << " "; });
    cout << endl;

    // Postorder forward y backward
    cout << "\n Postorder forward / backward (via forEach):" << endl;
    cout << "  fwd: ";
    arbol.postorder().forEach([](T1& v) { cout << v << " "; });
    cout << endl << "  bwd: ";
    arbol.postorder().rForEach([](T1& v) { cout << v << " "; });
    cout << endl;

    // search devuelve tuple
    cout << "\n search (devuelve tuple<value,ref>):" << endl;
    auto [valor, refer] = arbol.search(6);
    cout << "  search(6) -> dato=" << valor << " ref=" << refer << endl;

    // contains complementario
    cout << "\n contains (booleano sin excepcion):" << endl;
    cout << "  contains(4)  = " << (arbol.contains(4)  ? "true" : "false") << endl;
    cout << "  contains(99) = " << (arbol.contains(99) ? "true" : "false") << endl;

    // printTree por niveles
    cout << "\n printTree (BFS por niveles):" << endl;
    arbol.printTree();

    // Concurrencia: cada hilo cubre un rango distinto (sin colisiones)
    cout << "\n Concurrencia (5 hilos, rangos disjuntos -> 1000 nodos):" << endl;
    ArbolAsc arbolConc;
    auto worker = [&arbolConc](size_t inicio, size_t fin, Ref id) {
        for (size_t v = inicio; v < fin; ++v) arbolConc.insert((T1)v, id);
    };
    thread h1(worker,    0,  200, 1);
    thread h2(worker,  200,  400, 2);
    thread h3(worker,  400,  600, 3);
    thread h4(worker,  600,  800, 4);
    thread h5(worker,  800, 1000, 5);
    h1.join(); h2.join(); h3.join(); h4.join(); h5.join();
    cout << "  size esperado 1000 -> " << arbolConc.size() << endl;
}
