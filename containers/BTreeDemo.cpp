#include <iostream>
#include "../types.h"
#include "BTree.h"
using namespace std;

const TypeBTree * keys1 = "D1XJ2xTg8zKL9AhijOPQcEowRSp0NbW567BUfCqrs4FdtYZakHIuvGV3eMylmn";
const TypeBTree * keys2 = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
const TypeBTree * keys3 = "DYZakHIUwxVJ203ejOP9Qc8AdtuEop1XvTRghSNbW567BfiCqrs4FGMyzKLlmn";

const T1 BTreeSize = 3;

// Callback variadico para ForEach: imprime la clave en el ostream recibido
void ImprimirClave(tagObjectInfo< AscendingBTreeTrait<TypeBTree> >& info, int nivel, ostream& os) {
    os << info.key << " ";
}

// Predicado variadico para FirstThat: true si la clave es vocal
bool EsVocal(tagObjectInfo< AscendingBTreeTrait<TypeBTree> >& info, int nivel) {
    TypeBTree k = info.key;
    return (k=='A'||k=='E'||k=='I'||k=='O'||k=='U' || k=='a'||k=='e'||k=='i'||k=='o'||k=='u');
}

void BTreeDemo(ostream& os) {
    BTree< AscendingBTreeTrait<TypeBTree> > bt(BTreeSize);

    // Insercion (ObjID = i*i)
    for (T1 i = 0; keys1[i]; i++)
        bt.Insert((TypeBTree)keys1[i], (Ref)(i * i));
    bt.Print(os);

    // ForEach variadico (le paso os como argumento extra)
    os << "\nForEach:" << endl;
    bt.ForEach(ImprimirClave, os);
    os << endl;

    // FirstThat variadico
    os << "\nFirstThat (primera vocal):" << endl;
    auto* encontrado = bt.FirstThat(EsVocal);
    if (encontrado)
        os << "Vocal encontrada: " << encontrado->key << " (Ref: " << encontrado->ObjID << ")" << endl;
    else
        os << "No se encontraron vocales." << endl;
}
