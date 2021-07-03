#pragma once

template <typename T>
struct vector {
    using iterator = T*;
    using const_iterator = T const*;

    vector() noexcept : data_(nullptr), size_(0), capacity_(0) {} // O(1) nothrow

    vector(vector const& v) : vector() {
        vector<T> tmp(v.data_, v.size_, v.size_);
        swap(tmp);
    } // O(N) strong

    vector& operator=(vector const& other) {
        if (&other != this) {
            vector vec(other);
            swap(vec);
        }
        return *this;
    } // O(N) strong

    ~vector() noexcept {
        clear();
        operator delete (data_);
    } // O(N) nothrow

    T& operator[](size_t i) noexcept {
        return data_[i];
    } // O(1) nothrow

    T const& operator[](size_t i) const noexcept {
        return data_[i];
    } // O(1) nothrow

    T* data() noexcept {
        return data_;
    } // O(1) nothrow

    T const* data() const noexcept {
        return data_;
    } // O(1) nothrow

    size_t size() const noexcept {
        return size_;
    } // O(1) nothrow

    T& front() noexcept {
        return *begin();
    } // O(1) nothrow

    T const& front() const noexcept {
        return *begin();
    } // O(1) nothrow

    T& back() noexcept {
        return *(end() - 1);
    } // O(1) nothrow

    T const& back() const noexcept {
        return *(end() - 1);
    } // O(1) nothrow

    void push_back(T const& el) {
        if (size_ == capacity_) {
            T tmp = el;
            realloc(get_new_capacity());
            new (end()) T(tmp);
            size_++;
            return;
        }
        new (end()) T(el);
        size_++;
    } // O(1)* strong


    void realloc(size_t new_capacity) {
        vector <T> tmp(data_, size_, new_capacity);
        swap(tmp);
    }


    void pop_back() noexcept {
        size_--;
        data_[size_].~T();
    } // O(1) nothrow

    bool empty() const {
      return size_ == 0;
    } // O(1) nothrow

    size_t capacity() const {
        return capacity_;
    } // O(1) nothrow

    void reserve(size_t capacity) noexcept {
        if (capacity > capacity_) {
            realloc(capacity);
        }
    } // O(N) strong

    void shrink_to_fit() {
        if (size_ < capacity_) {
            realloc(size_);
        }
    } // O(N) strong

    void clear() noexcept {
        while (!empty()) {
            pop_back();
        }
    } // O(N) nothrow

    void swap(vector& v) {
        std::swap(data_, v.data_);
        std::swap(size_, v.size_);
        std::swap(capacity_, v.capacity_);
    } // O(1) nothrow

    iterator begin() noexcept {
        return data_;
    } // O(1) nothrow

    iterator end() noexcept {
        return data_ + size_;
    }   // O(1) nothrow

    const_iterator begin() const noexcept {
        return data_;
    } // O(1) nothrow

    const_iterator end() const noexcept {
        return data_ + size_;
    }   // O(1) nothrow

    iterator insert(const_iterator pos, T const& el) {
        size_t it = pos - begin();
        push_back(el);
        for (size_t i = it + 1; i < size_; i++) {
            std::swap(data_[i], data_[it]);
        }
        return begin() + it;
    } // O(N) strong

    iterator erase(const_iterator pos) noexcept {
        return erase(pos, pos + 1);
    } // O(N) nothrow(swap)

    iterator erase(const_iterator first, const_iterator last) noexcept {
        size_t it = first - begin();
        size_t cnt = last - first;
        for (size_t i = it + cnt; i < size_; i++) {
            std::swap(data_[i], data_[i - cnt]);
        }
        while (cnt--) {
            pop_back();
        }
        return begin() + it;
    } // O(N) nothrow(swap)

private:
  vector(T const* data, size_t size, size_t capacity) : vector() {
    if (capacity == 0) {
      return;
    }
    T* new_data = static_cast<T*>(operator new(sizeof(T) * capacity));
      try {
        for (; size_ < size; size_++) {
          new (new_data + size_) T(data[size_]);
        }
      } catch (...) {
        while (size_) {
          new_data[--size_].~T();
        }
        operator delete(new_data);
        throw;
      }
      data_ = new_data;
      capacity_ = capacity;
    }

    T* data_;
    size_t size_;
    size_t capacity_;

    size_t get_new_capacity() const noexcept {
        return (capacity_ == 0) ? 1 : capacity_ * 2;
    }
};