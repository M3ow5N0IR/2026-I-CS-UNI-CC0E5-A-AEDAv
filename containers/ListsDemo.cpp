#include "linkedlist.h"

void LinkedListDemo() {
  cout << "\n========== DEMOSTRACION LINKEDLIST ==========" << endl;
  LinkedList<DescendingLinkedListTrait<T1>> list;

  cout << "1. Operaciones Base (insert descendente):" << endl;
  list.insert(10, 0);
  list.insert(50, 0);
  list.insert(30, 0);
  cout << "Lista inicial: " << list << " Size: " << list.size() << endl;

  cout << "\n2. push_front() y push_back():" << endl;
  list.push_front(99, 0);
  list.push_back(1, 0);
  cout << "Lista alterada: " << list << endl;

  cout << "\n3. pop_front() y pop_back():" << endl;
  list.pop_front();
  list.pop_back();
  cout << "Tras extraccion en bordes: " << list << endl;

  cout << "\n4. operator[] (Leyendo indice 1):" << endl;
  cout << "Valor en ind 1 (O(n)): " << list[1] << endl;

  cout << "\n5. Copy Constructor (Clonando):" << endl;
  LinkedList<DescendingLinkedListTrait<T1>> clon = list;
  cout << "Mi Clon profundo: " << clon << endl;

  cout << "\n6. Move Constructor (Robando):" << endl;
  LinkedList<DescendingLinkedListTrait<T1>> ladrona = std::move(clon);
  cout << "Lista ladrona: " << ladrona << endl;
  cout << "La lista base robada quedo asi: " << clon << endl;

  cout << "\n7. Deserializacion con operator>> (Trait Ascendente):" << endl;
  LinkedList<AscendingLinkedListTrait<T1>> listaParser;
  istringstream texto("[8,2,6,1,9]");
  texto >> listaParser;
  cout << "Parseado desde '[8,2,6,1,9]': " << listaParser << endl;

  cout << "\n8. ForEach Concurrente (Duplicando a la lista de arriba):" << endl;
  listaParser.ForEach([](T1 &n) { n *= 2; });
  cout << "Al multiplicar * 2: " << listaParser << endl;
  cout << "=============================================\n" << endl;
}

void ListsDemo() { LinkedListDemo(); }
