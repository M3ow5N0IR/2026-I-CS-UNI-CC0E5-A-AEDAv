#include <iostream>
#include <thread>
#include "../types.h"
#include "avl.h"
using namespace std;

using AVLAsc = AVLTree<AscendingAVLTrait<T1>>;

// imprimir el estado del arbol tras una insercion
static void mostrarEstado(AVLAsc& a, T1 valor) {
    cout << "  insert(" << valor << ") | h=" << a.height()
         << " | bf=" << a.balance() << " | n=" << a.size() << endl;
    a.printTree();
}

void AVLDemo() {
    cout << "\n>>> PRUEBAS AVL <<<" << endl;

    // Ascendente:  rotaciones izquierda (RR)
    cout << "\n Insercion ascendente (rotaciones izquierda)" << endl;
    AVLAsc arbolA;
    for (size_t i = 1; i <= 7; ++i) {
        arbolA.insert((T1)i, (Ref)(i * 10));
        mostrarEstado(arbolA, (T1)i);
    }

    // Descendente:  rotaciones derecha (LL)
    cout << "\n Insercion descendente (rotaciones derecha)" << endl;
    AVLAsc arbolB;
    for (size_t i = 7; i > 0; --i) {
        arbolB.insert((T1)i, (Ref)(i * 10));
        mostrarEstado(arbolB, (T1)i);
    }

    // Mixto: LL, RR, LR, RL
    cout << "\n Insercion mixta (los 4 casos)" << endl;
    AVLAsc arbolC;
    for (T1 v : {10, 20, 30, 5, 4, 15, 25, 8, 12}) {
        arbolC.insert(v, (Ref)(v * 10));
        mostrarEstado(arbolC, v);
    }

    // Copy + modificacion: la copia debe ser independiente
    cout << "\n Copy constructor + insert independiente" << endl;
    AVLAsc copia(arbolA);
    cout << "  altura origen=" << arbolA.height()
         << " copia=" << copia.height() << " (deben ser iguales)" << endl;
    copia.insert((T1)100, 1000);
    cout << "  tras insertar 100 en copia: origen.size=" << arbolA.size()
         << " copia.size=" << copia.size() << endl;

    // Concurrencia: 5 hilos con rangos disjuntos
    cout << "\n Concurrencia (5 hilos x 100 valores disjuntos -> 500 nodos)" << endl;
    AVLAsc arbolConc;
    auto worker = [&arbolConc](size_t inicio, Ref id) {
        for (size_t i = 0; i < 100; ++i)
            arbolConc.insert((T1)(inicio + i), id);
    };
    thread h1(worker,   0, 1);
    thread h2(worker, 100, 2);
    thread h3(worker, 200, 3);
    thread h4(worker, 300, 4);
    thread h5(worker, 400, 5);
    h1.join(); h2.join(); h3.join(); h4.join(); h5.join();
    cout << "  size esperado 500 -> "         << arbolConc.size()    << endl;
    cout << "  altura aprox log2(500) ~= 9 -> " << arbolConc.height()  << endl;
    cout << "  balance raiz en [-1,1] -> "     << arbolConc.balance() << endl;

    cout << "\n>>> FIN AVL <<<" << endl;
}
