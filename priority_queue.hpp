#pragma once
#ifndef PRIORITY_QUEUE_HPP
#define PRIORITY_QUEUE_HPP

#include "memory_facilities.hpp"

#include <memory>
#include <stdexcept>
#include <cassert>
#include <algorithm>
#include <functional>


namespace priority_queue_impl {

    template<typename T, typename Comp>
    T* copy_and_get_place_to_insertion(T* first, T* last, T* dest, const T& value, Comp pred) {
        assert(first != nullptr);
        assert(last != nullptr);
        assert(first <= last);

        // searching for place to insert
        T* after_current = std::upper_bound(first, last, value, pred);
        mem_facilities::uninitialized_move_if_noexcept(first, after_current, dest);

        T* current = dest + std::distance(first, after_current);
        if (after_current == last) {
            return current;
        }

        // get place to insert in dest
        mem_facilities::uninitialized_move_if_noexcept(after_current, last, current + 1);

        assert(current != nullptr);
        return current;
    }


    template<typename T, typename Comp>
    T* get_place_to_insertion(T* first, T* last, const T& value, Comp pred) {
        assert(first != nullptr);
        assert(last != nullptr);
        assert(first <= last);

        // searching for place to insert
        T* after_current = std::upper_bound(first, last, value, pred);
        if (after_current == last) {
            return after_current;
        }

        auto before_last = last - 1;
        ::new (last) T(std::move_if_noexcept(*before_last));
        mem_facilities::move_backward_if_noexcept(after_current, before_last, last);

        assert(after_current != nullptr);
        (void)after_current->~T();
        return after_current;
    }

}


template<typename T, typename Comp = std::less<>, typename Alloc = std::allocator<T>>
struct priority_queue {
    using value_type = T;
    using size_type = std::size_t;
    using allocator_type = typename std::allocator_traits<Alloc>::template rebind_alloc<T>;
    using reference = T&;
    using const_reference = const T&;
    using const_iterator = const T*;
    using difference_type = std::ptrdiff_t;

    priority_queue(Comp comp = Comp()) : comp_{comp} {}
    priority_queue(size_type capacity, Comp comp = Comp()) : comp_{ comp } {
        begin_capacity_ = allocator_type().allocate(capacity);
        end_capacity_ = begin_capacity_ + capacity;

        front_ = begin_capacity_;
        back_ = front_;
    }
    template<typename Iter, typename = typename std::iterator_traits<Iter>::iterator_category>
    priority_queue(Iter first, Iter last, Comp comp = Comp()) : priority_queue(std::distance(first, last), comp) {
        std::uninitialized_copy(first, last, front_);
        back_ = front_ + std::distance(first, last);
        std::stable_sort(front_, back_, comp);
    }

    priority_queue(const priority_queue& other) : priority_queue(other.size(), other.comp_) {
        std::uninitialized_copy(other.front_, other.back_, front_);
        back_ = front_ + size();
    }
    priority_queue& operator=(const priority_queue& other) {
        if (this == &other) {
            return *this;
        }

        const auto current_size = other.size();
        if (capacity() <= current_size) {
            comp_ = other.comp_;
            if (size() == current_size) {
                std::copy(other.front_, other.back_, front_);
            }
            else {
                std::destroy(front_, back_);

                front_ = begin_capacity_;
                back_ = front_ + current_size;
                std::uninitialized_copy(other.front_, other.back_, front_);
            }

            return *this;
        }

        priority_queue tmp(current_size, other.comp_);
        std::uninitialized_copy(other.front_, other.back_, tmp.front_);
        tmp.back_ = tmp.front_ + current_size;

        this->swap(tmp);
        return *this;
    }

    priority_queue(priority_queue&& other) : comp_{ other.comp } { this->swap(other); }
    priority_queue& operator=(priority_queue&& other) {
        if (this == &other) {
            return *this;
        }

        this->swap(other);
        return *this;
    }

    ~priority_queue() { clear(); }

    T& front() { check_empty(); return *front_; }
    const T& front() const { check_empty(); return *front_; }

    T& top() { return front(); }
    const T& top() const { return front(); }

    T& back() { check_empty(); return *(back_ - 1); }
    const T& back() const { check_empty(); return *(back_ - 1); }

    inline bool empty() const noexcept { return (front_ == back_); }
    inline size_type size() const noexcept { return back_ - front_; }
    inline size_type capacity() const noexcept { return end_capacity_ - begin_capacity_; }

    void pop() { check_empty(); (void) front_->~T(); ++front_; }
    void push(const T& value) { ::new (prepare_place_to_push(value)) T(value); }
    void push(T&& value) { ::new (prepare_place_to_push(value)) T(std::move(value)); }

    template<typename... Args>
    void emplace(Args&&... args) {
        T value(std::forward<Args>(args)...);
        ::new (prepare_place_to_push(value)) T(std::move(value));
    }

    void swap(priority_queue& other) noexcept {
        std::swap(begin_capacity_, other.begin_capacity_);
        std::swap(end_capacity_, other.end_capacity_);
        std::swap(front_, other.front_);
        std::swap(back_, other.back_);
    }

    void clear() {
        if (begin_capacity_ == nullptr) {
            assert(end_capacity_ == nullptr);
            assert(front_ == nullptr);
            assert(back_ == nullptr);
            return;
        }

        assert(end_capacity_ != nullptr);
        assert(front_ != nullptr);
        assert(back_ != nullptr);

        std::destroy(front_, back_);
        allocator_type().deallocate(begin_capacity_, capacity());

        begin_capacity_ = nullptr;
        end_capacity_ = nullptr;
        front_ = nullptr;
        back_ = nullptr;
    }

    inline const_iterator begin() const noexcept { return front_; }
    inline const_iterator end() const noexcept { return back_; }

    inline const_iterator cbegin() const noexcept { return begin(); }
    inline const_iterator cend() const noexcept { return end(); }

private:
    static constexpr auto DEFAULT_SIZE = 8;
    static constexpr auto FACTOR = 2;

    void check_empty() const { if (front_ == back_) throw std::out_of_range{ "queue was empty" }; }


    T* prepare_place_to_push(const T& value) {

        // if queue has enough capacity - shift by one element to right and get place to insert new element
        if (begin_capacity_ != nullptr && back_ != end_capacity_) {
            T* place_to_insert = priority_queue_impl::get_place_to_insertion(front_, back_, value, comp_);
            ++back_;
            return place_to_insert;
        }

        // if queue hasn't capacity (was in empty state) - allocate storage
        if (begin_capacity_ == nullptr) {
            priority_queue tmp(DEFAULT_SIZE);
            this->swap(tmp);
            T* place_to_insert = back_;
            ++back_;
            return place_to_insert;
        }

        // if back_ is out of bounds
        assert(back_ == end_capacity_);
        const auto curr_size = size();


        T* place_to_insert = nullptr;

        // if capacity is exceeded - reallocate storage
        if (front_ == begin_capacity_) {
            priority_queue tmp(curr_size * FACTOR);
            tmp.back_ = tmp.front_ + curr_size + 1;

            place_to_insert = priority_queue_impl::copy_and_get_place_to_insertion(front_, back_, tmp.front_, value, comp_);
            this->swap(tmp);
        }
        else { // if not - shift front_ to the begin_capacity_
            place_to_insert = priority_queue_impl::copy_and_get_place_to_insertion(front_, back_, begin_capacity_, value, comp_);
            std::destroy(front_, back_);

            front_ = begin_capacity_;
            back_ = front_ + curr_size + 1;
        }

        assert(place_to_insert != nullptr);
        return place_to_insert;
    }

private:
    T* begin_capacity_ = nullptr;
    T* end_capacity_ = nullptr;
    T* front_ = nullptr;
    T* back_ = nullptr;
    Comp comp_;
};


#endif // !PRIORITY_QUEUE_HPP
