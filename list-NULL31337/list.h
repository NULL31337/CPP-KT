#pragma once
#include <cassert>
#include <iterator>

template <typename T>
class list {
public:
  template <typename V>
  struct basic_iterator;

  using iterator = basic_iterator<T>;
  using const_iterator = basic_iterator<T const>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  struct Node {
    Node() : prev(this), next(this) {}
    Node(Node* p, Node* n) : prev(p), next(n) {}
    T& value() {
      return static_cast<VNode*>(this)->value;
    }
    void swap(Node& other) {
      std::swap(other.prev, prev);

      std::swap(other.next, next);
    }

  private:
    friend class list;
    Node* prev = nullptr;
    Node* next = nullptr;
  };

  struct VNode : Node {
    VNode(T val) : Node(), value(val) {}
    VNode(Node* p, Node* n, T val) : Node(p, n), value(val) {}

  private:
    friend class list;
    T value;
  };

  // O(1)
  list() noexcept {};

  // O(n), strong
  list(list const& other) : list() {
    if (other.empty()) {
      return;
    }
    Node* tmp = other.node.next;
    while (tmp != other.node.prev->next) {
      push_back(tmp->value());
      tmp = tmp->next;
    }
  }

  // O(n), strong
  list& operator=(list const& other) {
    if (this != &other) {
      list copy(other);
      swap(copy);
    }
    return *this;
  }

  // O(n)
  ~list() {
    clear();
  }

  // O(1)
  bool empty() const noexcept {
    return node.prev == &node || node.prev->next == node.next;
  }

  // O(1)
  T& front() noexcept {
    return node.next->value();
  }

  // O(1)
  T const& front() const noexcept {
    return node.next->value();
  }

  // O(1), strong
  void push_front(T const& val) {
    node.next->prev = new VNode(&node, node.next, val);
    node.next = node.next->prev;
  }

  // O(1)
  void pop_front() noexcept {
    Node* tmp = node.next;
    if (node.prev == node.next) {
      node.prev = &node;
      node.next = &node;
    } else {
      node.next = node.next->next;
      node.next->prev = &node;
    }
    delete static_cast<VNode*>(tmp);
  }

  // O(1)
  T& back() noexcept {
    return node.prev->value();
  }

  // O(1)
  T const& back() const noexcept {
    return node.prev->value();
  }

  // O(1), strong
  void push_back(T const& val) {
    node.prev->next = new VNode(node.prev, &node, val);
    node.prev = node.prev->next;
  }

  // O(1)
  void pop_back() noexcept {
    Node* tmp = node.prev;
    if (node.prev == node.next) {
      node.prev = &node;
      node.next = &node;
    } else {
      node.prev = node.prev->prev;
      node.prev->next = &node;
    }
    delete static_cast<VNode*>(tmp);
  }

  // O(1)
  iterator begin() noexcept {
    return iterator(node.next);
  }

  // O(1)
  const_iterator begin() const noexcept {
    return const_iterator(node.next);
  }

  // O(1)
  iterator end() noexcept {
    return iterator(&node);
  }

  // O(1)
  const_iterator end() const noexcept {
    return const_iterator(&node);
  }

  // O(1)
  reverse_iterator rbegin() noexcept {
    return reverse_iterator(end());
  }

  // O(1)
  const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator(end());
  }

  // O(1)
  reverse_iterator rend() noexcept {
    return reverse_iterator(begin());
  }

  // O(1)
  const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator(begin());
  }

  // O(n)
  void clear() noexcept {
    while (!empty()) {
      pop_back();
    }
  }

  // O(1), strong
  iterator insert(const_iterator pos, T const& val);
  // O(1)
  iterator erase(const_iterator pos) noexcept;
  // O(n)
  iterator erase(const_iterator first, const_iterator last) noexcept;
  // O(1)
  void splice(const_iterator pos, list& other, const_iterator first,
              const_iterator last) noexcept;

  void swap(list& other) {
    node.swap(other.node);
    if (node.prev != &other.node) {
      node.prev->next = &node;
      node.next->prev = &node;
    } else {
      node.prev = &node;
      node.next = &node;
    }
    if (other.node.prev != &node) {
      other.node.prev->next = &other.node;
      other.node.next->prev = &other.node;
    } else {
      other.node.prev = &other.node;
      other.node.next = &other.node;
    }
  }

  friend void swap(list& a, list& b) noexcept {
    a.swap(b);
  }

private:
  mutable Node node;
};

template <typename T>
template <typename U>
struct list<T>::basic_iterator {
  using iterator_category = std::bidirectional_iterator_tag;
  using value_type = U;
  using difference_type = std::ptrdiff_t;
  using pointer = U*;
  using reference = U&;

  basic_iterator() = default;
  basic_iterator(basic_iterator const&) = default;

  template <typename V, typename = std::enable_if_t<std::is_const_v<U> &&
                                                    !std::is_const_v<V>>>
  basic_iterator(basic_iterator<V> const& other) : p(other.p) {}

  U& operator*() const {
    return static_cast<VNode*>(p)->value;
  }
  U* operator->() const {
    return &static_cast<VNode*>(p)->value;
  }

  basic_iterator& operator++() & {
    p = p->next;
    return *this;
  }
  basic_iterator operator++(int) & {
    basic_iterator res(*this);
    ++*this;
    return res;
  }

  basic_iterator& operator--() & {
    p = p->prev;
    return *this;
  }

  basic_iterator operator--(int) & {
    basic_iterator res(*this);
    --*this;
    return res;
  }

  friend bool operator==(basic_iterator const& lhs, basic_iterator const& rhs) {
    return lhs.p == rhs.p;
  }

  friend bool operator!=(basic_iterator const& lhs, basic_iterator const& rhs) {
    return !(lhs == rhs);
  }

private:
  explicit basic_iterator(Node* pp) : p(pp) {}

private:
  Node* p;

  friend struct list<T>;
};

// O(1), strong
template <typename T>
typename list<T>::iterator list<T>::insert(const_iterator pos, T const& val) {

  Node* it = pos.p;

  it->prev = new VNode(it->prev, it, val);
  it->prev->prev->next = it->prev;

  return iterator(it->prev);
}

// O(1)
template <typename T>
typename list<T>::iterator list<T>::erase(const_iterator pos) noexcept {
  Node* old = pos.p;
  Node* next = old->next;
  old->prev->next = old->next;
  old->next->prev = old->prev;
  delete static_cast<VNode*>(old);
  return iterator(next);
}

// O(n)
template <typename T>
typename list<T>::iterator list<T>::erase(const_iterator first,
                                          const_iterator last) noexcept {
  while (first != last)
    first = erase(first);

  return iterator(last.p);
}

// O(1)
template <typename T>
void list<T>::splice(const_iterator p, list& other, const_iterator f,
                     const_iterator l) noexcept {
  if (f == l) {
    return;
  }
  --l;
  Node* first = f.p;
  Node* pos = p.p;
  Node* last = l.p;
  first->prev->next = last->next;
  last->next->prev = first->prev;
  pos->prev->next = first;
  first->prev = pos->prev;
  pos->prev = last;
  last->next = pos;
}
