#ifndef BITS_ARRAY_HPP
#define BITS_ARRAY_HPP

#include "bits_utils.hpp"

#include <cstdint>
#include <type_traits>
#include <stdexcept>
#include <iterator>
#include <cassert>


template<typename T>
using allowed_for_bits_container_type = std::enable_if_t<std::is_unsigned_v<T>>;

template<typename InputIt>
using has_iterator_type = std::enable_if_t<
    std::is_same_v<
    typename std::iterator_traits<InputIt>::iterator_category,
    typename std::iterator_traits<InputIt>::iterator_category
    >
>;

template<typename T, typename = allowed_for_bits_container_type<T>>
class bits_array {
    class reference_impl;

    class pointer_impl;
    class const_pointer_impl;

    class iterator_impl;
    class const_iterator_impl;

    friend class iterator_impl;
    friend class const_iterator_impl;

public:
    using bits_container_type = T;
    using value_type = bool;
    using size_type = std::uint8_t;
    using difference_type = int;
    using reference = reference_impl;
    using const_reference = bool;
    using pointer = pointer_impl;
    using const_pointer = const_pointer_impl;

    using iterator = iterator_impl;
    using const_iterator = const_iterator_impl;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

public:
    static constexpr auto max_size = 8 * sizeof(T);

    explicit bits_array() = default;
    explicit bits_array(size_type sz) : size_{ sz } { check_overflow(sz); }
    explicit bits_array(size_type sz, bool val) : size_{ sz }
    {
        check_overflow(sz);
        if (!val) return;

        bits_ |= (~(0 << sz)) << (max_size - sz);
    }

    template<class It, typename = has_iterator_type<It>>
    explicit bits_array(It first, It last) { std::copy(first, last, std::back_inserter(*this)); }

    constexpr reference operator[](std::size_t index) { return reference{ bits_, index }; }
    constexpr bool operator[](std::size_t index) const { return get_bit(bits_, index); }
    constexpr reference at(std::size_t index) { check_index(index); return (*this)[index]; }
    constexpr bool at(std::size_t index) const { check_index(index); return (*this)[index]; }
    constexpr bool empty() const { return size_ == 0; }

    constexpr reference front() { empty_check(); return *(begin()); }
    constexpr bool front() const { empty_check(); return *(cbegin()); }

    constexpr reference back() { empty_check(); return *(end() - 1); }
    constexpr bool back() const { empty_check(); return *(cend() - 1); }

    constexpr iterator insert(const_iterator it, size_type count, bool value)
    {
        check_iterator(it);

        const auto new_size = size_ + count;
        check_overflow(new_size);

        const auto index = it - cbegin();
        bits_ = insert_bits(bits_, index, count, value);
        size_ = new_size;
        return iterator{ *this, index };
    }
    constexpr iterator insert(const_iterator it, bool value) { return insert(it, 1, value); }
    template<typename InputIt, typename = has_iterator_type<InputIt>>
    constexpr iterator insert(const_iterator it, InputIt first, InputIt last)
    {
        assert(first <= last);
        check_iterator(it);

        const auto itIndex = it - cbegin();
        for (; first != last; ++first, ++it) {
            insert(it, 1, bool(*first));
        }
        return iterator{ *this, itIndex };
    }

    constexpr iterator erase(const_iterator first, const_iterator last)
    {
        check_iterators_range(first, last);
        if (first == last) {
            return iterator{ *this, last - cbegin() };
        }

        const auto indexFirst = first - cbegin();
        const auto indexLast = last - cbegin();
        const auto count = indexLast - indexFirst;

        bits_ = erase_bits(bits_, indexFirst, count);
        size_ -= count;
        return iterator{ *this, indexFirst };
    }
    constexpr iterator erase(const_iterator it)
    {
        if (it < cbegin() || it >= cend()) throw std::out_of_range{ "iterator is out of range" };
        const auto index = it - cbegin();
        bits_ = erase_bits(bits_, index, 1);
        --size_;
        return iterator{ *this, index };
    }

    constexpr void push_back(bool value) { insert(cend(), 1, value); }
    constexpr void pop_back() { erase(cend() - 1); }

    constexpr void resize(size_type count, bool value)
    {
        if (size_ == count) return;
        check_overflow(count);

        bits_ = (size_ > count)
            ? erase_bits(bits_, count, size_ - count)
            : insert_bits(bits_, size_, count - size_, value);

        size_ = count;
    }
    constexpr void resize(size_type count) { resize(count, 0); }

    constexpr size_type size() const { return size_; }
    constexpr void clear() { bits_ = 0; size_ = 0; }

    constexpr iterator begin() { return iterator{ *this, 0 }; }
    constexpr iterator end() { return iterator{ *this, size_ }; }

    constexpr const_iterator cbegin() const { return const_iterator{ *this, 0 }; }
    constexpr const_iterator cend() const { return const_iterator{ *this, size_ }; }

    constexpr const_iterator begin() const { return cbegin(); }
    constexpr const_iterator end() const { return cend(); }

private: // reference implementation
    class reference_impl {
        friend class bits_array<T>;
        friend class iterator_impl;
    private:
        explicit constexpr reference_impl(bits_container_type& bits_cont, size_type index)
            : bits_cont_{ &bits_cont }, index_{ index } {}
    public:
        constexpr reference_impl& operator=(bool value)
        {
            *bits_cont_ = set_bit(*bits_cont_, index_, value);
            return *this;
        }
        constexpr operator bool() { assert(bits_cont_ != nullptr); return get_bit(*bits_cont_, index_); }
        constexpr friend void swap(reference_impl left, reference_impl right)
        {
            const bool tmp = bool(left);
            left = bool(right);
            right = tmp;
        }

    private:
        bits_container_type* bits_cont_ = nullptr;
        size_type index_ = 0;
    };

private: // pointers implementation
    class pointer_impl {
        friend class bits_array<T>;
        friend class iterator_impl;
    private:
        explicit constexpr pointer_impl(bits_container_type& bits_cont, size_type index)
            : bits_cont_{ &bits_cont }, index_{ index } {}

    public:
        constexpr reference_impl operator*() { return reference_impl{ *bits_cont_, index_ }; }
        constexpr reference_impl operator->() { return reference_impl{ *bits_cont_, index_ }; }

        constexpr operator bool() const { return bits_cont_ != nullptr; }

        constexpr bool operator==(decltype(nullptr)) const { return bits_cont_ == nullptr; }
        constexpr bool operator!=(decltype(nullptr)) const { return bits_cont_ != nullptr; }

    private:
        bits_container_type* bits_cont_ = nullptr;
        size_type index_ = 0;
    };

    class const_pointer_impl {
        friend class bits_array<T>;
        friend class const_iterator_impl;
    private:
        explicit constexpr const_pointer_impl(const bits_container_type& bits_cont, size_type index)
            : bits_cont_{ &bits_cont }, index_{ index } {}

    public:
        constexpr bool operator*() { return get_bit(*bits_cont_, index_); }
        constexpr bool operator->() { return get_bit(*bits_cont_, index_); }

        constexpr operator bool() const { return bits_cont_ != nullptr; }

        constexpr bool operator==(decltype(nullptr)) const { return bits_cont_ == nullptr; }
        constexpr bool operator!=(decltype(nullptr)) const { return bits_cont_ != nullptr; }

    private:
        const bits_container_type* bits_cont_ = nullptr;
        size_type index_ = 0;
    };

private: // iterators
    class iterator_impl : public std::iterator<std::random_access_iterator_tag, bool, int, pointer_impl, reference_impl> {
        friend class bits_array<T>;
        friend class const_iterator_impl;

        explicit constexpr iterator_impl(bits_array& context, int index)
            : context_{ &context }, index_{ index } {}

    public:
        explicit constexpr iterator_impl() = default;

        constexpr iterator_impl& operator++() { ++index_; return *this; }
        constexpr iterator_impl operator++(int) { auto result = *this; ++(*this); return result; }

        constexpr iterator_impl& operator--() { --index_; return *this; }
        constexpr iterator_impl operator--(int) { auto result = *this; --(*this); return result; }

        constexpr iterator_impl& operator+=(difference_type shift) { index_ += shift; return *this; }
        constexpr iterator_impl operator+(difference_type shift) const { auto result = *this; result += shift; return result; }

        constexpr iterator_impl& operator-=(difference_type shift) { index_ -= shift; return *this; }
        constexpr iterator_impl operator-(difference_type shift) const { auto result = *this; result -= shift; return result; }

        constexpr difference_type operator-(iterator_impl other) const { return index_ - other.index_; }

        constexpr reference_impl operator*() const
        {
            assert(context_ != nullptr);
            assert((index_ >= 0 && index_ < context_->size_));
            return reference_impl{ context_->bits_, static_cast<size_type>(index_) };
        }
        constexpr pointer_impl operator->() const { return pointer_impl{ context_->bits_, index_ }; }
        constexpr reference_impl operator[](difference_type n) const { return *(*this + n); }

        constexpr bool operator<(iterator_impl other) { return (*this - other) < 0; }
        constexpr bool operator>(iterator_impl other) { return (*this - other) > 0; }

        constexpr bool operator==(iterator_impl other) { return (*this - other) == 0; }
        constexpr bool operator!=(iterator_impl other) { return !(*this == other); }

        constexpr bool operator<=(iterator_impl other) { return !(*this > other); }
        constexpr bool operator>=(iterator_impl other) { return !(*this < other); }

    private:
        bits_array* context_ = nullptr;
        difference_type index_ = 0;
    };

    class const_iterator_impl : public std::iterator<std::random_access_iterator_tag, bool, int, const_pointer_impl, bool> {
        friend class bits_array<T>;

    private:
        explicit constexpr const_iterator_impl(const bits_array& context, difference_type index)
            : context_{ &context }, index_{ index } {}

    public:
        explicit constexpr const_iterator_impl() = default;
        ~const_iterator_impl() = default;
        explicit constexpr const_iterator_impl(const iterator_impl& other)
            : context_{ other.context_ }, index_{ other.index_ } {}

        constexpr const_iterator_impl& operator=(const iterator_impl& other)
        {
            context_ = other.context_;
            index_ = other.index_;
            return *this;
        }

        constexpr const_iterator_impl(const const_iterator_impl&) noexcept = default;
        constexpr const_iterator_impl& operator=(const const_iterator_impl&) noexcept = default;

        constexpr const_iterator_impl(const_iterator_impl&&) noexcept = default;
        constexpr const_iterator_impl& operator=(const_iterator_impl&&) noexcept = default;

        constexpr const_iterator_impl& operator++() { ++index_; return *this; }
        constexpr const_iterator_impl operator++(int) { auto result = *this; ++(*this); return result; }

        constexpr const_iterator_impl& operator--() { --index_; return *this; }
        constexpr const_iterator_impl operator--(int) { auto result = *this; --(*this); return result; }

        constexpr const_iterator_impl& operator+=(difference_type shift) { index_ += shift; return *this; }
        constexpr const_iterator_impl operator+(difference_type shift) const { auto result = *this; result += shift; return result; }

        constexpr const_iterator_impl& operator-=(difference_type shift) { index_ -= shift; return *this; }
        constexpr const_iterator_impl operator-(difference_type shift) const { auto result = *this; result -= shift; return result; }

        constexpr difference_type operator-(const_iterator_impl other) const { return index_ - other.index_; }

        constexpr bool operator*() const
        {
            assert(context_ != nullptr);
            assert((index_ >= 0 && index_ < context_->size_));
            return get_bit(context_->bits_, index_);
        }
        constexpr const_pointer_impl operator->() const { return const_pointer_impl{ context_->bits_, index_ }; }
        constexpr bool operator[](difference_type n) const { return *(*this + n); }

        constexpr bool operator<(const_iterator_impl other) { return (*this - other) < 0; }
        constexpr bool operator>(const_iterator_impl other) { return (*this - other) > 0; }

        constexpr bool operator==(const_iterator_impl other) { return (*this - other) == 0; }
        constexpr bool operator!=(const_iterator_impl other) { return !(*this == other); }

        constexpr bool operator<=(const_iterator_impl other) { return !(*this > other); }
        constexpr bool operator>=(const_iterator_impl other) { return !(*this < other); }

    private:
        const bits_array* context_ = nullptr;
        difference_type index_ = 0;
    };

private:
    void check_index(size_type index) const { if (index >= size_) throw std::out_of_range{ "index is out of range" }; }
    void check_overflow(size_type sz) const { if (sz > max_size) throw std::out_of_range{ "size is greater than maximum allowed" }; }
    void empty_check() const { if (empty()) throw std::out_of_range{ "container is empty" }; }
    void check_iterator(const_iterator it) const { if (it < cbegin() || it > cend()) throw std::out_of_range{ "iterator is out of range" }; }
    void check_iterators_range(const_iterator first, const_iterator last) const
    {
        if (first > last || first < cbegin() || last > cend()) throw std::out_of_range{ "invalid iterators range" };
    }

private:
    size_type size_{ 0 };
    bits_container_type bits_{ 0 };
};

#endif // !BITS_ARRAY_HPP
