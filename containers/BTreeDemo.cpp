#include <iostream>
#include "../types.h"
#include "BTree.h"
using namespace std;

void BTreeDemo(ostream& os) {
    os << "\n>>> PRUEBAS BTREE <<<" << endl;

    BTree<AscendingBTreeTrait<T1>> bt(3);

    // Insert (ref = clave * 10), conjunto desordenado
    T1 claves[] = {50, 20, 70, 10, 30, 60, 80, 5, 15, 25, 35, 55, 65, 75, 85};
    for (T1 k : claves)
        bt.insert(k, (Ref)(k * 10));
    os << "altura=" << bt.height()
       << " | claves=" << bt.size()
       << " | orden=" << bt.order() << endl;

    // toString / operator<< (inorder, con sangria por nivel)
    os << "\n-- toString / operator<< --" << endl;
    os << bt;

    // ForEach con captura: imprime las claves en orden
    os << "\n-- ForEach (inorder) --" << endl << "  ";
    bt.ForEach([&os](T1 key) { os << key << " "; });
    os << endl;

    // ForEach variadico con argumento extra (perfect forwarding): suma de claves
    long suma = 0;
    bt.ForEach([](T1 key, long& acc) { acc += key; }, suma);
    os << "suma de claves (arg extra) = " << suma << endl;

    // FirstThat: primera clave que cumple el predicado
    os << "\n-- FirstThat --" << endl;
    auto [k1, r1] = bt.FirstThat([](T1 key) { return key > 50; });
    os << "FirstThat(>50) -> clave=" << k1 << " ref=" << r1 << endl;

    // FirstThat variadico con argumento extra (umbral)
    auto [k2, r2] = bt.FirstThat([](T1 key, T1 umbral) { return key > umbral; }, (T1)70);
    os << "FirstThat(>umbral=70) -> clave=" << k2 << " ref=" << r2 << endl;

    // search
    os << "\n-- search --" << endl;
    auto [sk, sr] = bt.search(35);
    os << "search(35) -> clave=" << sk << " ref=" << sr << endl;

    os << "\n>>> FIN BTREE <<<" << endl;
}
