#include <iostream>
#include <sstream>
#include "../types.h"
#include "BTree.h"
using namespace std;

const TypeBTree * keys1 = "D1XJ2xTg8zKL9AhijOPQcEowRSp0NbW567BUfCqrs4FdtYZakHIuvGV3eMylmn";
const TypeBTree * keys2 = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
const TypeBTree * keys3 = "DYZakHIUwxVJ203ejOP9Qc8AdtuEop1XvTRghSNbW567BfiCqrs4FGMyzKLlmn";

const T1 BTreeSize = 3;

// Callback variadico para ForEach: imprime la clave en el ostream recibido
void ImprimirClave(tagObjectInfo< AscendingBTreeTrait<TypeBTree> >& info, Size nivel, ostream& os) {
    os << info.key << " ";
}

// Predicado variadico para FirstThat: true si la clave es vocal
Bool EsVocal(tagObjectInfo< AscendingBTreeTrait<TypeBTree> >& info, Size nivel) {
    TypeBTree k = info.key;
    return (k=='A'||k=='E'||k=='I'||k=='O'||k=='U' || k=='a'||k=='e'||k=='i'||k=='o'||k=='u');
}

void BTreeDemo(ostream& os) {
    BTree< AscendingBTreeTrait<TypeBTree> > bt(BTreeSize);

    // Insercion (ObjID = i*i)
    for (T1 i = 0; keys1[i]; i++)
        bt.Insert((TypeBTree)keys1[i], (Ref)(i * i));

    // operator<< (imprime via ForEach, indentado por nivel)
    os << "\n-- operator<< --" << endl;
    os << bt;

    // ForEach variadico (le paso os como argumento extra)
    os << "\n-- ForEach --" << endl;
    bt.ForEach(ImprimirClave, os);
    os << endl;

    // FirstThat variadico
    os << "\n-- FirstThat (primera vocal) --" << endl;
    auto* encontrado = bt.FirstThat(EsVocal);
    if (encontrado)
        os << "Vocal encontrada: " << encontrado->key << " (Ref: " << encontrado->ObjID << ")" << endl;

    // Forward iterator: recorrido ascendente (begin/end)
    os << "\n-- Forward iterator (begin/end) --" << endl << "  ";
    for (auto it = bt.begin(); it != bt.end(); ++it)
        os << it->key << " ";
    os << endl;

    // Backward iterator: recorrido descendente (rbegin/rend)
    os << "\n-- Backward iterator (rbegin/rend) --" << endl << "  ";
    for (auto it = bt.rbegin(); it != bt.rend(); ++it)
        os << it->key << " ";
    os << endl;

    // operator>> : serializo el arbol y lo reconstruyo en otro (round-trip)
    os << "\n-- operator>> (round-trip) --" << endl;
    stringstream ss;
    ss << bt;                                            // serializa con operator<<
    BTree< AscendingBTreeTrait<TypeBTree> > bt2(BTreeSize);
    ss >> bt2;                                           // reconstruye con operator>>
    os << "  original size=" << bt.size()
       << " | reconstruido size=" << bt2.size() << endl;
}
