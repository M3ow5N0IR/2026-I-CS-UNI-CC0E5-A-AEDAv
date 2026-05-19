#include <iostream>
#include <fstream>
#include <string>
#include <thread>

#include "../types.h"
#include "linkedlist.h"
#include "doublelinkedlist.h"
#include "circularlinkedlist.h"
#include "circulardoublelinkedlist.h"

using namespace std;

// operator<< y operator>>
template <typename Container>
void DemoFileIO(Container &list, const string &fileName) {
    ofstream os(fileName);
    os << list;
    os.close();

    Container listFromFile;
    ifstream is(fileName);
    is >> listFromFile;

    cout << "  escritura: " << list << endl;
    cout << "  lectura:   " << listFromFile << endl;
}

// ---------------------------------------------------------------------------
// LinkedList demo
// ---------------------------------------------------------------------------
void LinkedListDemo() {
    cout << "\n--- LinkedList ---" << endl;

    LinkedList<AscendingTrait<LLNode<T1>>> list;
    list.insert(3, 30); list.insert(1, 10); list.insert(2, 20);

    cout << "operator<<: " << list << endl;
    DemoFileIO(list, "LL.txt");

    // Reutilizacion de iteradores
    cout << "forward:   ";
    for (auto it = list.begin(); it != list.end(); ++it) cout << *it << " ";
    cout << endl;

    // Concurrencia: 5 hilos x 1000 push_front, esperamos 5000.
    LinkedList<AscendingTrait<LLNode<T1>>> lc;
    auto worker = [&lc](int id) { for (int i = 0; i < 1000; i++) lc.push_front(i, id); };
    thread t1(worker, 1), t2(worker, 2), t3(worker, 3), t4(worker, 4), t5(worker, 5);
    t1.join(); t2.join(); t3.join(); t4.join(); t5.join();
    cout << "concurrencia (esperado 5000): " << lc.size() << endl;
}

// ---------------------------------------------------------------------------
// DoubleLinkedList demo
// ---------------------------------------------------------------------------
void DoubleLinkedListDemo() {
    cout << "\n--- DoubleLinkedList ---" << endl;

    DoubleLinkedList<AscendingTrait<DLLNode<T1>>> list;
    list.insert(3, 30); list.insert(1, 10); list.insert(5, 50);
    list.insert(2, 20); list.insert(4, 40);

    cout << "operator<<: " << list << endl;
    DemoFileIO(list, "DLL.txt");

    // Reutilizacion de iteradores
    cout << "rbegin/rend: ";
    for (auto it = list.rbegin(); it != list.rend(); ++it) cout << *it << " ";
    cout << endl;
}

// ---------------------------------------------------------------------------
// CircularLinkedList demo
// ---------------------------------------------------------------------------
void CircularLinkedListDemo() {
    cout << "\n--- CircularLinkedList ---" << endl;

    CircularLinkedList<AscendingTrait<LLNode<T1>>> list;
    list.insert(3, 30); list.insert(1, 10); list.insert(5, 50);
    list.insert(2, 20); list.insert(4, 40);

    cout << "operator<<: " << list << endl;
    DemoFileIO(list, "CLL.txt");

    // Reutilizacion de iteradores
    cout << "cbegin/cend: ";
    for (auto it = list.cbegin(); it != list.cend(); ++it) cout << *it << " ";
    cout << endl;

    // circularForEach: dos vueltas completas.
    CircularLinkedList<AscendingTrait<LLNode<T1>>> c;
    c.insert(1, 10); c.insert(2, 20); c.insert(3, 30);
    cout << "circularForEach x2: ";
    c.circularForEach(2, [](T1 &v) { cout << v << " "; });
    cout << endl;
}

// ---------------------------------------------------------------------------
// CircularDoubleLinkedList demo
// ---------------------------------------------------------------------------
void CircularDoubleLinkedListDemo() {
    cout << "\n--- CircularDoubleLinkedList ---" << endl;

    CircularDoubleLinkedList<AscendingTrait<DLLNode<T1>>> list;
    list.insert(3, 30); list.insert(1, 10); list.insert(5, 50);
    list.insert(2, 20); list.insert(4, 40);

    cout << "operator<<: " << list << endl;
    DemoFileIO(list, "CDLL.txt");

    // Reutilizacion de iteradores
    cout << "cbegin/cend:   ";
    for (auto it = list.cbegin();  it != list.cend();  ++it) cout << *it << " ";
    cout << endl;
    cout << "crbegin/crend: ";
    for (auto it = list.crbegin(); it != list.crend(); ++it) cout << *it << " ";
    cout << endl;

    CircularDoubleLinkedList<AscendingTrait<DLLNode<T1>>> c;
    c.insert(1, 10); c.insert(2, 20); c.insert(3, 30);
    cout << "circularForEach fwd x2: ";
    c.circularForEach(2,  1, [](T1 &v) { cout << v << " "; });
    cout << endl;
    cout << "circularForEach bwd x2: ";
    c.circularForEach(2, -1, [](T1 &v) { cout << v << " "; });
    cout << endl;
}

void ListsDemo() {
    cout << "=== PRUEBAS LINKEDLIST ===";
    LinkedListDemo();
    cout << "\n=== PRUEBAS DOUBLELINKEDLIST ===";
    DoubleLinkedListDemo();
    cout << "\n=== PRUEBAS CIRCULARLINKEDLIST ===";
    CircularLinkedListDemo();
    cout << "\n=== PRUEBAS CIRCULARDOUBLELINKEDLIST ===";
    CircularDoubleLinkedListDemo();
    cout << "\n=== FIN ===" << endl;
}
