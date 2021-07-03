#pragma once
#include <cassert>
#include <cstdlib>
#include <iterator>
#include <utility>

template <typename T>
struct circular_buffer {

  template <typename V>
  struct basic_iterator;

  using iterator = basic_iterator<T>;
  using const_iterator = basic_iterator<T const>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  circular_buffer() noexcept
      : data_(nullptr), head(0), size_(0), capacity_(0){}; // O(1)

  circular_buffer(circular_buffer const& v)
      : circular_buffer(v.data_, v.size_, v.capacity_, v.capacity_, v.head) {
  } // O(n), strong

  ~circular_buffer() {
    clear();
    operator delete(data_);
  } // O(n)

  circular_buffer& operator=(circular_buffer const& other) {
    if (&other != this) {
      circular_buffer cb(other);
      swap(cb);
    }
    return *this;
  } // O(n), strong

  size_t size() const noexcept {
    return size_;
  } // O(1)

  T& operator[](size_t index) noexcept {
    return data_[get_i(index)];
  } // O(1)

  T const& operator[](size_t index) const noexcept {
    return data_[get_i(index)];
  } // O(1)

  bool empty() const noexcept {
    return size_ == 0;
  } // O(1), nothrow

  void clear() noexcept {
    while (!empty()) {
      pop_back();
    }
  } // O(n), nothrow

  void push_back(T const& val) {
    if (size_ == capacity_) {
      T tmp = val;
      realloc(get_new_capacity());
      new (data_ + size_) T(tmp);
    } else {
      new (data_ + get_i(size_)) T(val);
    }
    size_++;
  } // O(1), strong

  void pop_back() noexcept {
    assert(!empty());
    size_--;
    data_[get_i(size_)].~T();
  } // O(1)

  T& back() noexcept {
    assert(!empty());
    return data_[get_i(size_ - 1)];
  } // O(1)

  T const& back() const noexcept {
    assert(!empty());
    return data_[get_i(size_ - 1)];
  } // O(1)

  void push_front(T const& val) {
    assert(!empty());
    if (size_ == capacity_) {
      T tmp = val;
      realloc(get_new_capacity());
      new (data_ + capacity_ - 1) T(tmp);
      head = capacity_ - 1;
    } else {
      head = get_i(size_ - 1 + capacity_);
      new (data_ + head) T(val);
    }
    size_++;
  } // O(1), strong

  void pop_front() noexcept {
    assert(!empty());
    data_[head].~T();
    head++;
    head %= capacity_;
    size_--;
  } // O(1)

  T& front() noexcept {
    assert(!empty());
    return *(begin());
  } // O(1)
  T const& front() const noexcept {
    assert(!empty());
    return *(begin());
  } // O(1)

  void realloc(size_t new_capacity) {
    circular_buffer<T> tmp(data_, size_, capacity_, new_capacity, head);
    swap(tmp);
  }

  void reserve(size_t desired_capacity) {
    if (desired_capacity > capacity_) {
      realloc(desired_capacity);
    }
  } // O(n), strong

  size_t capacity() const noexcept {
    return capacity_;
  } // O(1)

  iterator begin() noexcept {
    return iterator(this, 0, empty());
  } // O(1)

  const_iterator begin() const noexcept {
    return const_iterator(this, 0, empty());
  } // O(1)

  iterator end() noexcept {
    return iterator(this, (size_ == 0 ? size_ : size_ - 1), true);
  } // O(1)

  const_iterator end() const noexcept {
    return const_iterator(this, (size_ == 0 ? size_ : size_ - 1), true);
  } // O(1)

  reverse_iterator rbegin() noexcept {
    return reverse_iterator(end());
  } // O(1)

  const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator(end());
  } // O(1)

  reverse_iterator rend() noexcept {
    return reverse_iterator(begin());
  } // O(1)

  const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator(begin());
  } // O(1)

  iterator insert(const_iterator pos, T const& val) {
    size_t save_pos = pos.pos;
    if (pos.is_end) {
      if (!empty())
        save_pos++;
      push_back(val);
    } else {
      if (save_pos * 2 >= size_) {
        push_back(val);
        for (size_t i = save_pos; i != size_ - 1; i++) {
          std::swap(data_[get_i(i)], data_[get_i(size_ - 1)]);
        }
      } else {
        push_front(val);
        for (size_t i = 0; i != save_pos; i++) {
          std::swap(data_[get_i(i)], data_[get_i(save_pos)]);
        }
      }
    }
    return iterator(this, (save_pos), false);
  } // O(n), basic

  iterator erase(const_iterator pos) {
    return erase(pos, pos + 1);
  } // O(n), basic

  iterator erase(const_iterator first, const_iterator last) {
    ptrdiff_t cnt = last - first;
    size_t ans = first.pos;
    if (first.pos < size_ - last.pos) {
      for (size_t i = 0; i < first.pos; i++) {
        std::swap(data_[get_i(i)], data_[get_i(cnt + i)]);
      }
      for (size_t i = 0; i < cnt; i++) {
        pop_front();
      }
    } else {
      for (size_t i = first.pos + cnt; i < size_; i++) {
        std::swap(data_[get_i(i)], data_[get_i(i - cnt)]);
      }
      for (size_t i = 0; i < cnt; i++) {
        pop_back();
      }
    }
    if (empty()) {
      return iterator(this, 0, true);
    }
    return iterator(this, ans - (ans == size_), ans >= size_);
  } // O(n), basic

  void swap(circular_buffer& other) noexcept {
    std::swap(data_, other.data_);
    std::swap(head, other.head);
    std::swap(size_, other.size_);
    std::swap(capacity_, other.capacity_);
  } // O(1)

private:
  T* data_;
  size_t head;
  size_t size_;
  size_t capacity_;
  circular_buffer(T const* data, size_t size, size_t capacity,
                  size_t new_capacity, size_t head_)
      : circular_buffer() {
    T* new_data = static_cast<T*>(operator new(sizeof(T) * new_capacity));
    if (capacity != 0) {
      try {
        for (; size_ < size; size_++) {
          new (new_data + size_) T(data[(head_ + size_) % capacity]);
        }
      } catch (...) {
        while (size_) {
          new_data[--size_].~T();
        }
        operator delete(new_data);
        throw;
      }
    }
    head = 0;
    capacity_ = new_capacity;
    data_ = new_data;
  }
  size_t get_new_capacity() const noexcept {
    return (capacity_ == 0) ? 1 : capacity_ * 2;
  }
  size_t get_i(size_t i) const noexcept {
    return (capacity_ == 0 ? 0 : (head + i) % capacity_);
  }
  size_t i_get(size_t i) const noexcept {
    return (head + i) % capacity_;
  }
};

template <typename T>
template <typename U>
struct circular_buffer<T>::basic_iterator {
  using iterator_category = std::bidirectional_iterator_tag;
  using value_type = U;
  using difference_type = std::ptrdiff_t;
  using pointer = U*;
  using reference = U&;

  basic_iterator() = default;
  basic_iterator(basic_iterator const&) = default;
  template <typename V, typename = std::enable_if_t<std::is_const_v<U> &&
                                                    !std::is_const_v<V>>>
  basic_iterator(basic_iterator<V> const&);
  U& operator*() const {
    return p->data_[(p->head + pos) % p->capacity_];
  }
  U* operator->() const {
    return &(p->data_[(p->head + pos) % p->capacity_]);
  };

  basic_iterator& operator++() &;
  basic_iterator operator++(int) &;

  basic_iterator& operator--() &;
  basic_iterator operator--(int) &;

  friend ptrdiff_t operator-(basic_iterator l, basic_iterator r) {
    return l.pos - r.pos + l.is_end - r.is_end;
  }
  friend basic_iterator operator-(basic_iterator l, int r) {
    if (l.is_end && r > 0) {
      r--;
      l.is_end = false;
    }
    l.pos -= r;
    return l;
  }

  basic_iterator& operator+=(size_t shift) & {
    pos += shift;
    if (pos == p->size_) {
      pos--;
      is_end = true;
    }
    return *this;
  }
  friend basic_iterator operator+(basic_iterator l, size_t shift) {
    l += shift;
    return l;
  }

  friend basic_iterator operator+(size_t shift, basic_iterator l) {
    l += shift;
    return l;
  }

  basic_iterator& operator-=(size_t shift) & {
    if (is_end && shift > 0) {
      is_end = false;
      shift--;
    }
    pos -= shift;
    return *this;
  }

  U& operator[](size_t shift) {
    return p->data_[(pos + shift + p->head) % p->capacity_];
  }

  friend bool operator<(basic_iterator const& l, basic_iterator const& r) {
    return l.pos < r.pos;
  }

  friend bool operator>(basic_iterator const& l, basic_iterator const& r) {
    return r < l;
  }
  friend bool operator>=(basic_iterator const& l, basic_iterator const& r) {
    return !(l < r);
  }
  friend bool operator<=(basic_iterator const& l, basic_iterator const& r) {
    return !(r < l);
  }

  friend bool operator==(basic_iterator const& lhs, basic_iterator const& rhs) {
    return lhs.is_end == rhs.is_end && lhs.pos == rhs.pos && lhs.p == rhs.p;
  }

  friend bool operator==(const int& lhs, basic_iterator const& rhs) {
    return lhs == rhs.pos;
  }

  friend bool operator!=(basic_iterator const& lhs, basic_iterator const& rhs) {
    return !(lhs == rhs);
  }

private:
  explicit basic_iterator(circular_buffer const* p_, size_t pos_, bool is_end_)
      : p(p_), pos(pos_), is_end(is_end_){};

private:
  circular_buffer const* p;
  size_t pos;
  bool is_end;
  friend struct circular_buffer<T>;
};

template <typename T>
template <typename U>
template <typename V, typename>
circular_buffer<T>::basic_iterator<U>::basic_iterator(
    basic_iterator<V> const& other)
    : p(other.p), pos(other.pos), is_end(other.is_end) {}

template <typename T>
template <typename U>
typename circular_buffer<T>::template basic_iterator<U>&
circular_buffer<T>::basic_iterator<U>::operator++() & {
  if (pos == p->size_ - 1 || p->size_ == 0) {
    is_end = true;
  } else {
    pos++;
  }
  return *this;
}

template <typename T>
template <typename U>
typename circular_buffer<T>::template basic_iterator<U>
circular_buffer<T>::basic_iterator<U>::operator++(int) & {
  basic_iterator res(*this);
  ++*this;
  return res;
}

template <typename T>
template <typename U>
typename circular_buffer<T>::template basic_iterator<U>&
circular_buffer<T>::basic_iterator<U>::operator--() & {
  if (is_end) {
    is_end = false;
  } else {
    pos--;
  }
  return *this;
}

template <typename T>
template <typename U>
typename circular_buffer<T>::template basic_iterator<U>
circular_buffer<T>::basic_iterator<U>::operator--(int) & {
  basic_iterator res(*this);
  --*this;
  return res;
}