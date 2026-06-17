#include <iostream>
#include <fstream>
#include "../types.h"
#include "hashtable.h"
using namespace std;

void DemoHashTable() {
    cout << "\n=== PRUEBAS HASHTABLE ===" << endl;

    HashTable<HashTrait<Param1, Param2>> m;

    // operator[] (set y get)
    cout << "\n operator[] (set):" << endl;
    m[10] = "Diez";
    m[25] = "Veinticinco";
    m[3]  = "Tres";
    m[10] = "DiezModificado";
    cout << "  m[10] = " << m[10] << endl;
    cout << "  m[3]  = " << m[3]  << endl;
    cout << "  size  = " << m.size() << endl;

    // for-range con structured bindings
    cout << "\n for-range con structured bindings:" << endl;
    for (const auto& [k, v] : m)
        cout << "  " << k << " -> " << v << endl;

    // toString (vista compacta tipo diccionario)
    cout << "\n toString (vista de diccionario):" << endl;
    cout << "  " << m.toString() << endl;

    // operator<< (consola + archivo) -- delega al AVL
    cout << "\n operator<< (consola + archivo):" << endl;
    cout << "  " << m << endl;
    { ofstream os("hashtable.txt"); os << m; }

    // operator>> (REAPROVECHA operator>> del AVL)
    cout << "\n operator>> (lee hashtable.txt en m2, reaprovecha operator>> del AVL):" << endl;
    HashTable<HashTrait<Param1, Param2>> m2;
    { ifstream is("hashtable.txt"); is >> m2; }
    cout << "  m2 = " << m2 << endl;
    cout << "  m2.toString = " << m2.toString() << endl;

    // Constructor copia
    cout << "\n Constructor copia (independiente):" << endl;
    HashTable<HashTrait<Param1, Param2>> copia(m);
    copia[99] = "NoventaYNueve";
    cout << "  original tiene 99? " << (m.contains(99) ? "si" : "no") << endl;
    cout << "  copia tiene 99?    " << (copia.contains(99) ? "si" : "no") << endl;
    cout << "  original size = " << m.size() << " | copia size = " << copia.size() << endl;

    // Move constructor
    cout << "\n Move constructor:" << endl;
    HashTable<HashTrait<Param1, Param2>> movido(std::move(copia));
    cout << "  movido size  = " << movido.size() << endl;
    cout << "  copia vacia? " << (copia.size() == 0 ? "si" : "no") << endl;

    // contains y at
    cout << "\n contains y at:" << endl;
    cout << "  contains(10) = " << (m.contains(10) ? "true" : "false") << endl;
    cout << "  contains(77) = " << (m.contains(77) ? "true" : "false") << endl;
    cout << "  m.at(10) = " << m.at(10) << endl;

    // Inserciones masivas (HashTable<long, long>)
    cout << "\n Inserciones masivas (30 elementos):" << endl;
    HashTable<HashTrait<Param1, Param1>> mn;
    for (long i = 0; i < 30; ++i) mn[i] = i * 100;
    cout << "  size esperado 30 -> " << mn.size() << endl;
    cout << "  mn.at(15) = " << mn.at(15) << endl;
}
