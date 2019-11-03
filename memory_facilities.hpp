#pragma once
#ifndef MEMORY_FACILITIES_HPP

#include <memory>
#include <iterator>
#include <type_traits>


namespace mem_facilities {
    namespace details {
        struct need_to_move {};
        struct need_to_copy {};

        template<typename InputIt, typename ForwardIt>
        void uninitialized_move_if_noexcept_impl(InputIt first, InputIt last, ForwardIt dest, details::need_to_move) {
            std::uninitialized_move(first, last, dest);
        }

        template<typename InputIt, typename ForwardIt>
        void uninitialized_move_if_noexcept_impl(InputIt first, InputIt last, ForwardIt dest, details::need_to_copy) {
            std::uninitialized_copy(first, last, dest);
        }


        template<typename InputIt, typename ForwardIt>
        void move_range_if_noexcept_impl(InputIt first, InputIt last, ForwardIt dest, details::need_to_move) {
            std::move(first, last, dest);
        }

        template<typename InputIt, typename ForwardIt>
        void move_range_if_noexcept_impl(InputIt first, InputIt last, ForwardIt dest, details::need_to_copy) {
            std::copy(first, last, dest);
        }


        template<typename InputIt, typename ForwardIt>
        void move_backward_if_noexcept_impl(InputIt first, InputIt last, ForwardIt dest, details::need_to_move) {
            std::move_backward(first, last, dest);
        }

        template<typename InputIt, typename ForwardIt>
        void move_backward_if_noexcept_impl(InputIt first, InputIt last, ForwardIt dest, details::need_to_copy) {
            std::copy_backward(first, last, dest);
        }
    }

    template<typename InputIt, typename ForwardIt>
    void uninitialized_move_if_noexcept(InputIt first, InputIt last, ForwardIt dest) {
        using ValueType = typename std::iterator_traits<ForwardIt>::value_type;
        details::uninitialized_move_if_noexcept_impl(first, last, dest,
            std::conditional_t<std::is_nothrow_move_constructible_v<ValueType> || !std::is_copy_constructible_v<ValueType>,
            details::need_to_move,
            details::need_to_copy>());
    }

    template<typename InputIt, typename ForwardIt>
    void move_range_if_noexcept(InputIt first, InputIt last, ForwardIt dest) {
        using ValueType = typename std::iterator_traits<ForwardIt>::value_type;
        details::move_range_if_noexcept_impl(first, last, dest,
            std::conditional_t<std::is_nothrow_move_assignable_v<ValueType> || !std::is_copy_assignable_v<ValueType>,
            details::need_to_move,
            details::need_to_copy>());
    }

    template<typename InputIt, typename ForwardIt>
    void move_backward_if_noexcept(InputIt first, InputIt last, ForwardIt dest) {
        using ValueType = typename std::iterator_traits<ForwardIt>::value_type;
        details::move_backward_if_noexcept_impl(first, last, dest,
            std::conditional_t<std::is_nothrow_move_assignable_v<ValueType> || !std::is_copy_assignable_v<ValueType>,
            details::need_to_move,
            details::need_to_copy>());
    }
}


#endif // !MEMORY_FACILITIES_HPP
