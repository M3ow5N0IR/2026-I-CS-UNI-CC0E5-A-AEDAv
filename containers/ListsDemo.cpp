#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <thread>

#include "../types.h"
#include "linkedlist.h"
#include "doublelinkedlist.h"
#include "circularlinkedlist.h"
#include "circulardoublelinkedlist.h"

using namespace std;

// Prueba genérica: Funciona para cualquier contenedor que tenga insert, operator<< y operator>>
template <typename Container>
void DemoList(Container& list, string fileName){
    list.insert(28, 15);
    list.insert(17, 25);
    list.insert(8, 35);
    list.insert(4, 45);
    list.insert(35, 55);
    cout << "  Original:      " << list << endl;
    // Grabar la lista en un archivo
    ofstream os(fileName);
    os << list << endl;
    os.close();
    // Leer la lista desde un archivo en una lista NUEVA
    Container listFromFile;
    ifstream is(fileName);
    is >> listFromFile;
    cout << "  Leida archivo: " << listFromFile << endl;
}

// ==============================
// DEMO: LinkedList
// ==============================
void LinkedListDemo(){
    // 1. Inserción ordenada + archivos
    cout << "1. INSERCION ORDENADA + ARCHIVOS" << endl;
    LinkedList<AscendingLinkedListTrait<T1>> list1;
    DemoList(list1, "AscLL.txt");
    LinkedList<DescendingLinkedListTrait<T1>> list2;
    DemoList(list2, "DescLL.txt");

    // 2. Push front / Push back
    cout << "\n2. PUSH FRONT / PUSH BACK" << endl;
    LinkedList<AscendingLinkedListTrait<T1>> pushList;
    pushList.push_back(20, 2); pushList.push_back(30, 3);
    pushList.push_front(10, 1); pushList.push_front(5, 0);
    cout << "  Luego de pushes: " << pushList << endl;

    // 3. Pop front / Pop back
    cout << "\n3. POP FRONT / POP BACK" << endl;
    auto [d1, r1] = pushList.pop_front();
    cout << "  pop_front -> (" << d1 << "," << r1 << ") | lista: " << pushList << endl;
    auto [d2, r2] = pushList.pop_back();
    cout << "  pop_back  -> (" << d2 << "," << r2 << ") | lista: " << pushList << endl;

    // 4. Iterador Forward
    cout << "\n4. ITERADOR FORWARD" << endl;
    LinkedList<AscendingLinkedListTrait<T1>> iterList;
    iterList.insert(1, 10); iterList.insert(2, 20); iterList.insert(3, 30);
    iterList.insert(4, 40); iterList.insert(5, 50);
    cout << "  ranged-for: ";
    for (auto &v : iterList) cout << v << " ";
    cout << endl;

    // 5. ForEach
    cout << "\n5. FOREACH" << endl;
    cout << "  ForEach: ";
    iterList.ForEach([](T1 &v){ cout << v << " "; });
    cout << endl;

    // 6. Operator[]
    cout << "\n6. OPERATOR[]" << endl;
    cout << "  [0]=" << iterList[0] << " [2]=" << iterList[2]
         << " [4]=" << iterList[4] << endl;

    // 7. Copy / Move Constructors
    cout << "\n7. COPY / MOVE CONSTRUCTORS" << endl;
    LinkedList<AscendingLinkedListTrait<T1>> copied(iterList);
    cout << "  Original: " << iterList << endl;
    cout << "  Copiada:  " << copied << endl;
    LinkedList<AscendingLinkedListTrait<T1>> moved(std::move(copied));
    cout << "  Moved:    " << moved << endl;
    cout << "  Original tras move (size=0): " << copied.size() << endl;

    // 8. Operator>> desde stream
    cout << "\n8. OPERATOR>> DESDE STREAM" << endl;
    LinkedList<AscendingLinkedListTrait<T1>> streamList;
    stringstream ss("[(30, 3), (10, 1), (20, 2)]");
    ss >> streamList;
    cout << "  Stream desordenado -> lista: " << streamList << endl;

    // 9. Concurrencia
    cout << "\n9. CONCURRENCIA (5 hilos x 1000 push_front)" << endl;
    LinkedList<AscendingLinkedListTrait<T1>> concList;
    auto worker = [&concList](int id){
        for (int i = 0; i < 1000; i++) concList.push_front(i, id);
    };
    thread t1(worker,1), t2(worker,2), t3(worker,3), t4(worker,4), t5(worker,5);
    t1.join(); t2.join(); t3.join(); t4.join(); t5.join();
    cout << "  Tamano esperado 5000: " << concList.size() << endl;
    cout << "  ESTADO: " << (concList.size()==5000 ? "EXITO" : "FALLO") << endl;
}

// ==============================
// DEMO: DoubleLinkedList
// ==============================
void DoubleLinkedListDemo(){
    // 1. Inserción ordenada
    cout << "1. INSERCION ORDENADA" << endl;
    DoubleLinkedList<AscendingDLLTrait<T1>> asc;
    asc.insert(3,30); asc.insert(1,10); asc.insert(5,50);
    asc.insert(2,20); asc.insert(4,40);
    cout << "  Asc:  " << asc << endl;
    DoubleLinkedList<DescendingDLLTrait<T1>> desc;
    desc.insert(3,30); desc.insert(1,10); desc.insert(5,50);
    desc.insert(2,20); desc.insert(4,40);
    cout << "  Desc: " << desc << endl;

    // 2. Push front / Push back
    cout << "\n2. PUSH FRONT / PUSH BACK" << endl;
    DoubleLinkedList<AscendingDLLTrait<T1>> pushList;
    pushList.push_back(20, 2); pushList.push_back(30, 3);
    pushList.push_front(10, 1); pushList.push_front(5, 0);
    cout << "  Luego de pushes: " << pushList << endl;

    // 3. Pop front / Pop back
    cout << "\n3. POP FRONT / POP BACK" << endl;
    auto [d1, r1] = pushList.pop_front();
    cout << "  pop_front -> (" << d1 << "," << r1 << ") | lista: " << pushList << endl;
    auto [d2, r2] = pushList.pop_back();
    cout << "  pop_back  -> (" << d2 << "," << r2 << ") | lista: " << pushList << endl;

    // 4. Iterador Forward
    cout << "\n4. ITERADOR FORWARD" << endl;
    DoubleLinkedList<AscendingDLLTrait<T1>> iterList;
    iterList.insert(1,10); iterList.insert(2,20); iterList.insert(3,30);
    iterList.insert(4,40); iterList.insert(5,50);
    cout << "  fwd: ";
    for (auto &v : iterList) cout << v << " ";
    cout << endl;

    // 5. Iterador Backward
    cout << "\n5. ITERADOR BACKWARD" << endl;
    cout << "  bwd: ";
    for (auto it = iterList.rbegin(); it != iterList.rend(); ++it)
        cout << *it << " ";
    cout << endl;

    // 6. ForEach
    cout << "\n6. FOREACH " << endl;
    cout << "  ForEach fwd:        ";
    iterList.ForEach([](T1 &v){ cout << v << " "; });
    cout << endl;
    

    // 7. Operator[]
    cout << "\n7. OPERATOR[]" << endl;
    cout << "  [0]=" << iterList[0] << " [2]=" << iterList[2]
         << " [4]=" << iterList[4] << endl;

    // 8. Copy / Move Constructors
    cout << "\n8. COPY / MOVE CONSTRUCTORS" << endl;
    DoubleLinkedList<AscendingDLLTrait<T1>> copied(iterList);
    cout << "  Original: " << iterList << endl;
    cout << "  Copiada:  " << copied << endl;
    DoubleLinkedList<AscendingDLLTrait<T1>> moved(std::move(copied));
    cout << "  Moved:    " << moved << endl;
    cout << "  Original tras move (size=0): " << copied.size() << endl;

    // 9. Archivos
    cout << "\n9. ESCRITURA / LECTURA ARCHIVOS" << endl;
    DoubleLinkedList<AscendingDLLTrait<T1>> fileAsc;
    DemoList(fileAsc, "AscDLL.txt");
    DoubleLinkedList<DescendingDLLTrait<T1>> fileDesc;
    DemoList(fileDesc, "DescDLL.txt");

    // 10. Operator>> desde stream
    cout << "\n10. OPERATOR>> DESDE STREAM" << endl;
    DoubleLinkedList<AscendingDLLTrait<T1>> streamList;
    stringstream ss("[(30, 3), (10, 1), (20, 2)]");
    ss >> streamList;
    cout << "  Stream desordenado -> lista: " << streamList << endl;

    // 11. Concurrencia
    cout << "\n11. CONCURRENCIA (5 hilos x 1000 push_front)" << endl;
    DoubleLinkedList<AscendingDLLTrait<T1>> concList;
    auto worker = [&concList](int id){
        for (int i = 0; i < 1000; i++) concList.push_front(i, id);
    };
    thread t1(worker,1), t2(worker,2), t3(worker,3), t4(worker,4), t5(worker,5);
    t1.join(); t2.join(); t3.join(); t4.join(); t5.join();
    cout << "  Tamano esperado 5000: " << concList.size() << endl;
    cout << "  ESTADO: " << (concList.size()==5000 ? "EXITO" : "FALLO") << endl;
}

// ==============================
// DEMO: CircularLinkedList
// ==============================
void CircularLinkedListDemo(){
    // 1. Inserción ordenada
    cout << "1. INSERCION ORDENADA" << endl;
    CircularLinkedList<AscendingCLLTrait<T1>> asc;
    asc.insert(3,30); asc.insert(1,10); asc.insert(5,50);
    asc.insert(2,20); asc.insert(4,40);
    cout << "  Asc:  " << asc << endl;
    CircularLinkedList<DescendingCLLTrait<T1>> desc;
    desc.insert(3,30); desc.insert(1,10); desc.insert(5,50);
    desc.insert(2,20); desc.insert(4,40);
    cout << "  Desc: " << desc << endl;

    // 2. Push front / Push back / Pop front / Pop back
    cout << "\n2. PUSH FRONT / PUSH BACK / POP FRONT / POP BACK" << endl;
    CircularLinkedList<AscendingCLLTrait<T1>> pushList;
    pushList.push_back(20,2); pushList.push_back(30,3);
    pushList.push_front(10,1); pushList.push_front(5,0);
    cout << "  Despues de pushes: " << pushList << endl;
    auto [d1, r1] = pushList.pop_front();
    cout << "  pop_front -> (" << d1 << "," << r1 << ") | lista: " << pushList << endl;
    auto [d2, r2] = pushList.pop_back();
    cout << "  pop_back  -> (" << d2 << "," << r2 << ") | lista: " << pushList << endl;

    // 3. Naturaleza circular: circularForEach x2 vueltas
    cout << "\n3. NATURALEZA CIRCULAR circularForEach x2 vueltas" << endl;
    CircularLinkedList<AscendingCLLTrait<T1>> circList;
    circList.insert(1,10); circList.insert(2,20); circList.insert(3,30);
    cout << "  Lista: " << circList << endl;
    cout << "  x2 vueltas: ";
    circList.circularForEach(2, [](T1 &v){ cout << v << " "; });
    cout << endl;

    // 4. Iterador Forward (1 vuelta exacta)
    cout << "\n4. ITERADOR FORWARD" << endl;
    CircularLinkedList<AscendingCLLTrait<T1>> iterList;
    iterList.insert(1,10); iterList.insert(2,20); iterList.insert(3,30);
    iterList.insert(4,40); iterList.insert(5,50);
    cout << "  ranged-for (1 vuelta exacta): ";
    for (auto &v : iterList) cout << v << " ";
    cout << endl;

    // 5. ForEach
    cout << "\n5. FOREACH" << endl;
    cout << "  ForEach: ";
    iterList.ForEach([](T1 &v){ cout << v << " "; });
    cout << endl;

    // 6. Copy / Move Constructors
    cout << "\n6. COPY / MOVE CONSTRUCTORS" << endl;
    CircularLinkedList<AscendingCLLTrait<T1>> orig;
    orig.insert(10,1); orig.insert(20,2); orig.insert(30,3);
    CircularLinkedList<AscendingCLLTrait<T1>> copied(orig);
    cout << "  Orig:   " << orig   << endl;
    cout << "  Copied: " << copied << endl;
    cout << "  Copied x2 vueltas: ";
    copied.circularForEach(2, [](T1 &v){ cout << v << " "; });
    cout << endl;
    CircularLinkedList<AscendingCLLTrait<T1>> moved(std::move(orig));
    cout << "  Moved:  " << moved << endl;
    cout << "  Orig tras move (size=0): " << orig.size() << endl;

    // 7. Archivos
    cout << "\n7. ESCRITURA / LECTURA ARCHIVOS" << endl;
    CircularLinkedList<AscendingCLLTrait<T1>> fileAsc;
    DemoList(fileAsc, "AscCLL.txt");
    CircularLinkedList<DescendingCLLTrait<T1>> fileDesc;
    DemoList(fileDesc, "DescCLL.txt");

    // 8. Concurrencia
    cout << "\n8. CONCURRENCIA (5 hilos x 1000 push_front)" << endl;
    CircularLinkedList<AscendingCLLTrait<T1>> concList;
    auto worker = [&concList](int id){
        for (int i = 0; i < 1000; i++) concList.push_front(i, id);
    };
    thread t1(worker,1), t2(worker,2), t3(worker,3), t4(worker,4), t5(worker,5);
    t1.join(); t2.join(); t3.join(); t4.join(); t5.join();
    cout << "  Tamano esperado 5000: " << concList.size() << endl;
    cout << "  ESTADO: " << (concList.size()==5000 ? "EXITO" : "FALLO") << endl;
}

// ==============================
// DEMO: CircularDoubleLinkedList
// ==============================
void CircularDoubleLinkedListDemo(){
    // 1. Inserción ordenada
    cout << "1. INSERCION ORDENADA" << endl;
    CircularDoubleLinkedList<AscendingCDLLTrait<T1>> asc;
    asc.insert(3,30); asc.insert(1,10); asc.insert(5,50);
    asc.insert(2,20); asc.insert(4,40);
    cout << "  Asc:  " << asc << endl;
    CircularDoubleLinkedList<DescendingCDLLTrait<T1>> desc;
    desc.insert(3,30); desc.insert(1,10); desc.insert(5,50);
    desc.insert(2,20); desc.insert(4,40);
    cout << "  Desc: " << desc << endl;

    // 2. Push front / Push back / Pop front / Pop back
    cout << "\n2. PUSH FRONT / PUSH BACK / POP FRONT / POP BACK" << endl;
    CircularDoubleLinkedList<AscendingCDLLTrait<T1>> pushList;
    pushList.push_back(20,2); pushList.push_back(30,3);
    pushList.push_front(10,1); pushList.push_front(5,0);
    cout << "  Despues de pushes: " << pushList << endl;
    auto [d1, r1] = pushList.pop_front();
    cout << "  pop_front -> (" << d1 << "," << r1 << ") | lista: " << pushList << endl;
    auto [d2, r2] = pushList.pop_back();
    cout << "  pop_back  -> (" << d2 << "," << r2 << ") | lista: " << pushList << endl;

    // 3. Naturaleza circular doble (fwd y bwd x2 vueltas)
    cout << "\n3. NATURALEZA CIRCULAR DOBLE (fwd y bwd x2 vueltas)" << endl;
    CircularDoubleLinkedList<AscendingCDLLTrait<T1>> circList;
    circList.insert(1,10); circList.insert(2,20); circList.insert(3,30);
    cout << "  Lista: " << circList << endl;
    cout << "  fwd x2: ";
    circList.circularForEach(2, [](T1 &v){ cout << v << " "; });
    cout << endl;

    // 4. Iteradores Forward y Backward
    cout << "\n4. ITERADORES FORWARD Y BACKWARD (ranged-for)" << endl;
    CircularDoubleLinkedList<AscendingCDLLTrait<T1>> iterList;
    iterList.insert(1,10); iterList.insert(2,20); iterList.insert(3,30);
    iterList.insert(4,40); iterList.insert(5,50);
    cout << "  ranged-for fwd: ";
    for (auto &v : iterList) cout << v << " ";
    cout << endl;
    cout << "  ranged-for bwd: ";
    for (auto it = iterList.rbegin(); it != iterList.rend(); ++it)
        cout << *it << " ";
    cout << endl;

    // 5. ForEach / ReverseForEach
    cout << "\n5. FOREACH / REVERSEFOREACH" << endl;
    cout << "  ForEach fwd:        ";
    iterList.ForEach([](T1 &v){ cout << v << " "; });
    cout << endl;
    cout << "  ReverseForEach bwd: ";
    iterList.ReverseForEach([](T1 &v){ cout << v << " "; });
    cout << endl;

    // 6. Copy / Move Constructors
    cout << "\n6. COPY / MOVE CONSTRUCTORS" << endl;
    CircularDoubleLinkedList<AscendingCDLLTrait<T1>> orig;
    orig.insert(10,1); orig.insert(20,2); orig.insert(30,3);
    CircularDoubleLinkedList<AscendingCDLLTrait<T1>> copied(orig);
    cout << "  Orig:   " << orig   << endl;
    cout << "  Copied: " << copied << endl;
    cout << "  Copied fwd x2: ";
    copied.circularForEach(2, [](T1 &v){ cout << v << " "; });
    cout << endl;
    CircularDoubleLinkedList<AscendingCDLLTrait<T1>> moved(std::move(orig));
    cout << "  Moved:  " << moved << endl;
    cout << "  Orig tras move (size=0): " << orig.size() << endl;

    // 7. Archivos
    cout << "\n7. ESCRITURA / LECTURA ARCHIVOS" << endl;
    CircularDoubleLinkedList<AscendingCDLLTrait<T1>> fileAsc;
    DemoList(fileAsc, "AscCDLL.txt");
    CircularDoubleLinkedList<DescendingCDLLTrait<T1>> fileDesc;
    DemoList(fileDesc, "DescCDLL.txt");

    // 8. Concurrencia
    cout << "\n8. CONCURRENCIA (5 hilos x 1000 push_front)" << endl;
    CircularDoubleLinkedList<AscendingCDLLTrait<T1>> concList;
    auto worker = [&concList](int id){
        for (int i = 0; i < 1000; i++) concList.push_front(i, id);
    };
    thread t1(worker,1), t2(worker,2), t3(worker,3), t4(worker,4), t5(worker,5);
    t1.join(); t2.join(); t3.join(); t4.join(); t5.join();
    cout << "  Tamano esperado 5000: " << concList.size() << endl;
    cout << "  ESTADO: " << (concList.size()==5000 ? "EXITO" : "FALLO") << endl;
}

// ==============================
// PUNTO DE ENTRADA
// ==============================
void ListsDemo(){
    cout << "========== PRUEBAS LINKEDLIST ==========" << endl;
    LinkedListDemo();
    cout << "\n-----------------------------\n" << endl;
    cout << "========== PRUEBAS DOUBLELINKEDLIST ==========" << endl;
    DoubleLinkedListDemo();
    cout << "\n-----------------------------\n" << endl;
    cout << "========== PRUEBAS CIRCULARLINKEDLIST ==========" << endl;
    CircularLinkedListDemo();
    cout << "\n-----------------------------\n" << endl;
    cout << "========== PRUEBAS CIRCULARDOUBLELINKEDLIST ==========" << endl;
    CircularDoubleLinkedListDemo();
    cout << "\nFIN DE LAS PRUEBAS" << endl;
}