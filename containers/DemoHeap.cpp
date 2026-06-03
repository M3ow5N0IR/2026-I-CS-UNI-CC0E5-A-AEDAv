#include <iostream>
#include <fstream>
#include <thread>
#include "../types.h"
#include "heap.h"
using namespace std;

using MinHeapT1 = Heap<MinHeapTrait<T1>>;
using MaxHeapT1 = Heap<MaxHeapTrait<T1>>;

static const initializer_list<T1> kValores = {10, 4, 15, 1, 7, 12, 3};

void DemoMinHeap() {
    cout << "\n--- MinHeap (raiz = minimo) ---" << endl;
    MinHeapT1 h;

    cout << "Inserciones (heapifyUp):" << endl;
    for (T1 v : kValores) {
        h.insert(v, (Ref)(v * 10));
        cout << "  insert(" << v << ") size=" << h.size() << endl;
    }

    cout << "\nEstructura final:\n" << h.toString() << endl;

    auto [vMin, rMin] = h.peek();
    cout << "peek (esperado minimo=1): " << vMin << " ref=" << rMin << endl;

    cout << "\noperator<< (consola + archivo minheap.txt):" << endl;
    cout << "  " << h << endl;
    { ofstream os("minheap.txt"); os << h; }

    cout << "\noperator>> (lee minheap.txt en h2):" << endl;
    MinHeapT1 h2;
    { ifstream is("minheap.txt"); is >> h2; }
    cout << "  h2.peek = " << get<0>(h2.peek()) << endl;

    cout << "\nExtract sucesivo (debe salir en orden ascendente):" << endl;
    cout << "  ";
    while (!h.isEmpty()) {
        auto [v, r] = h.extract();
        cout << v << " ";
    }
    cout << endl;
}

void DemoMaxHeap() {
    cout << "\n--- MaxHeap (raiz = maximo) ---" << endl;
    MaxHeapT1 h;

    cout << "Inserciones (heapifyUp):" << endl;
    for (T1 v : kValores) {
        h.insert(v, (Ref)(v * 10));
        cout << "  insert(" << v << ") size=" << h.size() << endl;
    }

    cout << "\nEstructura final:\n" << h.toString() << endl;

    auto [vMax, rMax] = h.peek();
    cout << "peek (esperado maximo=15): " << vMax << " ref=" << rMax << endl;

    cout << "\nExtract sucesivo (debe salir en orden descendente):" << endl;
    cout << "  ";
    while (!h.isEmpty()) {
        auto [v, r] = h.extract();
        cout << v << " ";
    }
    cout << endl;
}

void DemoHeapConcurrencia() {
    cout << "\n--- Concurrencia (5 hilos x 100 valores disjuntos -> 500 nodos) ---" << endl;
    MinHeapT1 h;
    auto worker = [&h](size_t inicio, Ref id) {
        for (size_t i = 0; i < 100; ++i)
            h.insert((T1)(inicio + i), id);
    };
    thread t1(worker,   0, 1);
    thread t2(worker, 100, 2);
    thread t3(worker, 200, 3);
    thread t4(worker, 300, 4);
    thread t5(worker, 400, 5);
    t1.join(); t2.join(); t3.join(); t4.join(); t5.join();
    cout << "  size esperado 500 -> " << h.size() << endl;
    auto [v, r] = h.peek();
    cout << "  peek esperado 0 -> " << v << endl;
}

void HeapDemo() {
    cout << "\n=== PRUEBAS HEAP ===" << endl;
    DemoMinHeap();
    DemoMaxHeap();
    DemoHeapConcurrencia();
    cout << "\n=== FIN HEAP ===" << endl;
}
