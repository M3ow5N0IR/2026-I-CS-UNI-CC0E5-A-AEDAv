#ifndef __CIRCULARLINKEDLIST_H__
#define __CIRCULARLINKEDLIST_H__

#include "linkedlist.h"

// T12: Ascending y Descending Traits para CLL
template <typename T>
struct AscendingCLLTrait : BaseTrait<T, less<T>, LLNode<T>> {};

template <typename T>
struct DescendingCLLTrait : BaseTrait<T, greater<T>, LLNode<T>> {};

// T13: Forward Iterator con lógica de centinela (condición de parada)
template <typename Container>
class CLLForwardIterator
    : public general_iterator<Container, CLLForwardIterator<Container>> {
public:
  using MySelf = CLLForwardIterator<Container>;
  using Parent = general_iterator<Container, MySelf>;
  using Parent::Parent;

  MySelf operator++() {
    if (this->m_pNode) {
      this->m_pNode = this->m_pNode->getNext();
      // Magia de la parada: Si dimos la vuelta completa y chocamos con el root
      // original Destruimos el iterador (nullptr) para que el bucle 'for' crea
      // que es el end() y se detenga.
      if (this->m_pContainer && this->m_pNode == this->m_pContainer->m_pRoot) {
        this->m_pNode = nullptr;
      }
    }
    return *this;
  }
};

template <typename Trait> class CircularLinkedList : public LinkedList<Trait> {
public:
  using MySelf = CircularLinkedList<Trait>;
  using forward_iterator = CLLForwardIterator<MySelf>;
  friend forward_iterator;

  // Enganches de iteración
  forward_iterator begin() { return forward_iterator(this, this->m_pRoot); }
  forward_iterator end() { return forward_iterator(this, nullptr); }

  CircularLinkedList() : LinkedList<Trait>() {}

  // T14: Copy Constructor con bucle do-while para anillo
  CircularLinkedList(const CircularLinkedList &other) : LinkedList<Trait>() {
    shared_lock<shared_mutex> lock(other.m_mtx);
    if (other.m_size == 0)
      return;

    auto *curr = other.m_pRoot;
    do {
      this->push_back(curr->getData(), curr->getRef());
      curr = curr->getNext();
    } while (curr != other.m_pRoot);
  }

  // T14: Move Constructor
  CircularLinkedList(CircularLinkedList &&other) noexcept
      : LinkedList<Trait>(std::move(other)) {}

  // T14: Copy Assignment
  CircularLinkedList &operator=(const CircularLinkedList &other) {
    if (this != &other) {
      this->clear();
      shared_lock<shared_mutex> lock(other.m_mtx);
      if (other.m_size > 0) {
        auto *curr = other.m_pRoot;
        do {
          this->push_back(curr->getData(), curr->getRef());
          curr = curr->getNext();
        } while (curr != other.m_pRoot);
      }
    }
    return *this;
  }

  // T14: Move Assignment
  CircularLinkedList &operator=(CircularLinkedList &&other) noexcept {
    if (this != &other) {
      this->clear();
      LinkedList<Trait>::operator=(std::move(other));
    }
    return *this;
  }

  // T14: Destructor Seguro (rompe el ciclo antes de borrar)
  virtual ~CircularLinkedList() { clear(); }

  void clear() {
    unique_lock<shared_mutex> lock(this->m_mtx);
    if (this->m_size == 0)
      return;

    // CRÍTICO: Romper el ciclo circular para evitar bucle infinito en la
    // eliminación
    this->m_tail->setNext(nullptr);

    auto *current = this->m_pRoot;
    while (current) {
      auto *next = current->getNext();
      delete current;
      current = next;
    }
    this->m_pRoot = nullptr;
    this->m_tail = nullptr;
    this->m_size = 0;
  }

  // T15: push_front circular
  void push_front(typename Trait::value_type value, Ref ref) override {
    unique_lock<shared_mutex> lock(this->m_mtx);
    auto *newNode = new typename Trait::Node(value, ref, this->m_pRoot);
    this->m_pRoot = newNode;
    if (this->m_size == 0) {
      this->m_tail = newNode;
    }
    this->m_tail->setNext(this->m_pRoot); // Reparar anillo
    this->m_size++;
  }

  // T15: push_back circular
  void push_back(typename Trait::value_type value, Ref ref) override {
    unique_lock<shared_mutex> lock(this->m_mtx);
    auto *newNode = new typename Trait::Node(value, ref);
    if (this->m_size == 0) {
      this->m_pRoot = newNode;
      this->m_tail = newNode;
    } else {
      this->m_tail->setNext(newNode);
      this->m_tail = newNode;
    }
    this->m_tail->setNext(this->m_pRoot); // Reparar anillo
    this->m_size++;
  }

  // T16: pop_front circular
  std::tuple<typename Trait::value_type, Ref> pop_front() override {
    unique_lock<shared_mutex> lock(this->m_mtx);
    if (!this->m_pRoot)
      throw runtime_error("La lista esta vacia");

    auto *temp = this->m_pRoot;
    auto result = std::make_tuple(temp->getData(), temp->getRef());

    if (this->m_size == 1) {
      this->m_pRoot = nullptr;
      this->m_tail = nullptr;
    } else {
      this->m_pRoot = this->m_pRoot->getNext();
      this->m_tail->setNext(this->m_pRoot); // Reparar anillo
    }

    delete temp;
    this->m_size--;
    return result;
  }

  // T16: pop_back circular (Vuelve a ser O(N) por usar LLNode de un enlace)
  std::tuple<typename Trait::value_type, Ref> pop_back() override {
    unique_lock<shared_mutex> lock(this->m_mtx);
    if (!this->m_pRoot)
      throw runtime_error("La lista esta vacia");

    auto result =
        std::make_tuple(this->m_tail->getData(), this->m_tail->getRef());

    if (this->m_size == 1) {
      delete this->m_pRoot;
      this->m_pRoot = nullptr;
      this->m_tail = nullptr;
    } else {
      auto *act = this->m_pRoot;
      while (act->getNext() != this->m_tail) {
        act = act->getNext();
      }
      delete this->m_tail;
      this->m_tail = act;
      this->m_tail->setNext(this->m_pRoot); // Reparar anillo
    }

    this->m_size--;
    return result;
  }

  // T17: insert circular ordenado
  void insert(const typename Trait::value_type &value, Ref ref) override {
    unique_lock<shared_mutex> lock(this->m_mtx);

    if (this->m_size == 0) {
      auto *newNode = new typename Trait::Node(value, ref);
      this->m_pRoot = newNode;
      this->m_tail = newNode;
      this->m_tail->setNext(this->m_pRoot);
      this->m_size++;
      return;
    }

    // Si va a la cabeza
    if (this->m_comp(value, this->m_pRoot->getDataRef())) {
      auto *newNode = new typename Trait::Node(value, ref, this->m_pRoot);
      this->m_pRoot = newNode;
      this->m_tail->setNext(this->m_pRoot);
      this->m_size++;
      return;
    }

    // Búsqueda a lo largo del anillo (sin nullptrs)
    auto *prev = this->m_pRoot;
    auto *curr = this->m_pRoot->getNext();

    while (curr != this->m_pRoot && !this->m_comp(value, curr->getDataRef())) {
      prev = curr;
      curr = curr->getNext();
    }

    auto *newNode = new typename Trait::Node(value, ref, curr);
    prev->setNext(newNode);

    if (curr == this->m_pRoot) { // Si dimos toda la vuelta, se insertó al final
      this->m_tail = newNode;
    }

    this->m_size++;
  }

  // ForEach
  template <typename Func, typename... Args>
  void ForEach(Func func, Args &&...args) {
    unique_lock<shared_mutex> lock(this->m_mtx);
    if (this->m_size == 0)
      return;
    for (auto &item : *this) {
      func(item, std::forward<Args>(args)...);
    }
  }

  // T18: Función especial circularForEach
  template <typename Func, typename... Args>
  void circularForEach(size_t vueltas, Func func, Args &&...args) {
    unique_lock<shared_mutex> lock(this->m_mtx);
    if (this->m_size == 0 || vueltas == 0)
      return;

    auto *curr = this->m_pRoot;
    size_t iteracionesTotales = this->m_size * vueltas;

    for (size_t i = 0; i < iteracionesTotales; ++i) {
      func(curr->getDataRef(), std::forward<Args>(args)...);
      curr = curr->getNext();
    }
  }

  // Reparación Crítica: operator<< específico para CLL para evitar Bucle
  // Infinito
  friend ostream &operator<<(ostream &os, const CircularLinkedList &list) {
    shared_lock<shared_mutex> lock(list.m_mtx);
    os << "[";
    if (list.m_size > 0) {
      auto *act = list.m_pRoot;
      do {
        os << "(" << act->getData() << "," << act->getRef() << ")";
        act = act->getNext();
        if (act != list.m_pRoot)
          os << ",";
      } while (act != list.m_pRoot);
      os << "] ->root(" << list.m_pRoot->getData() << ")";
    } else {
      os << "]";
    }
    return os;
  }
};

#endif // __CIRCULARLINKEDLIST_H__
