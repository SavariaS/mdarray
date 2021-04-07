#ifndef MDARRAY_HPP
#define MDARRAY_HPP

#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <cstring>

#include <iostream>

namespace temp
{
    /* Forward declaration */
    template<typename T, std::size_t... N>
    struct mdarray;

    /* Helper metafunctions */
    template<typename T, std::size_t D>
    using id = T;

    // Forward declaration of subarray
    template<typename T, std::size_t... N>
    struct md_subarray;
    template<typename T, std::size_t... N>
    using md_subarray_t = typename md_subarray<T, N...>::type;

    // Base case of subarray
    template<typename T, std::size_t Head>
    struct md_subarray<T, Head>
    {
        using type = T;
    };

    // Recursive case of subarray
    template<typename T, std::size_t Head, std::size_t... Tail>
    struct md_subarray<T, Head, Tail...>
    {
        using type = mdarray<md_subarray_t<T, Tail...>, Tail...>;
    };

    // Index sequence used for subarrays
    template<std::size_t... N>
    struct index_sequence {};

    // Recursive function to make index sequence
    template<std::size_t Head, std::size_t... Tail>
    auto make_index_sequence_impl()
    {
        // Base case
        if constexpr (Head == 0)
        {
            return index_sequence<Tail...>();
        }
        // Recursive case
        else
        {
            return make_index_sequence_impl<Head-1, Head-1, Tail...>();
        }
    }

    template<std::size_t Head, std::size_t... Tail>
    auto make_index_sequence()
    {
        return make_index_sequence_impl<(1*...*Tail)>();
    }

    template<typename T, std::size_t... N>
    struct mdarray
    {
        public:
            /* Type definitions */
            using value_type = T;
            using size_type              = std::size_t;
            using size_ilist             = const std::initializer_list<size_type>&;
            using difference_type        = std::ptrdiff_t;

            using reference              = value_type&;
            using const_reference        = const value_type&;
            using pointer                = value_type*;
            using const_pointer          = const value_type*;
            using iterator               = value_type*;
            using const_iterator         = const value_type*;
            using reverse_iterator       = std::reverse_iterator<iterator>;
            using const_reverse_iterator = std::reverse_iterator<const_iterator>;

            using subarray               = md_subarray_t<value_type, N...>;
            using const_subarray         = const md_subarray_t<value_type, N...>;

            /* Data member */
            value_type _m_arr[(1 * ... * N)];

            /* Elements access */
            constexpr reference at(id<size_type, N>... pos)
            {
                size_type indices[sizeof...(N)] = {pos...};

                size_type index = 0;
                size_type position = 0;
                size_type weight = (1 * ... * N);

                for(size_type dim : s_sizes)
                {
                    if(indices[index] >= dim)
                    {
                        throw std::out_of_range("mdarray::at: pos[" + std::to_string(index) + "] (which is " + std::to_string(indices[index]) + ") >= size[" + std::to_string(index) + "] (which is " + std::to_string(dim) + ")");
                    }

                    weight /= dim;
                    position += indices[index] * weight;
                    ++index;
                }

                return _m_arr[position];
            }

            constexpr const_reference at(id<size_type, N>... pos) const
            {
                size_type indices[sizeof...(N)] = {pos...};

                size_type index = 0;
                size_type position = 0;
                size_type weight = (1 * ... * N);

                for(size_type dim : s_sizes)
                {
                    if(indices[index] >= dim)
                    {
                        throw std::out_of_range("mdarray::at: pos[" + std::to_string(index) + "] (which is " + std::to_string(indices[index]) + ") >= size[" + std::to_string(index) + "] (which is " + std::to_string(dim) + ")");
                    }

                    weight /= dim;
                    position += indices[index] * weight;
                    ++index;
                }

                return _m_arr[position];
            }

            constexpr subarray operator[](size_type pos)
            {
                // Use a lambda to capture index_sequence's parameter pack
                return [pos, this]<std::size_t... Seq>(index_sequence<Seq...>) -> subarray
                       {
                           return { _m_arr[pos * sizeof...(Seq) + Seq]...};
                       }(make_index_sequence<N...>());
            }

            constexpr const_subarray operator[](size_type pos) const
            {
                // Use a lambda to capture index_sequence's parameter pack
                return [pos, this]<std::size_t... Seq>(index_sequence<Seq...>) -> const_subarray
                       {
                           return { _m_arr[pos * sizeof...(Seq) + Seq]...};
                       }(make_index_sequence<N...>());
            }

            constexpr reference front() { return _m_arr[0]; }
            constexpr reference back()  { return _m_arr[(1*...*N)]; }
            constexpr const_reference front() const { return _m_arr[0]; }
            constexpr const_reference back()  const { return _m_arr[(1*...*N)]; }

            constexpr pointer data() noexcept { return _m_arr; }
            constexpr const_pointer data() const noexcept { return _m_arr; }

            /* Iterators */
            constexpr iterator begin() noexcept { return iterator(data()); }
            constexpr iterator end()   noexcept { return iterator(data() + (1*...*N)); }
            constexpr reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
            constexpr reverse_iterator rend()   noexcept { return reverse_iterator(begin()); }

            constexpr const_iterator begin() const noexcept { return const_iterator(data()); }
            constexpr const_iterator end()   const noexcept { return const_iterator(data() + (1*...*N)); }
            constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
            constexpr const_reverse_iterator rend()   const noexcept { return const_reverse_iterator(begin()); }

            constexpr const_iterator cbegin() const noexcept { return const_iterator(data()); }
            constexpr const_iterator cend()   const noexcept { return const_iterator(data() + (1*...*N)); }
            constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(cend()); }
            constexpr const_reverse_iterator crend()   const noexcept { return const_reverse_iterator(cbegin()); }

            /* Capacity */
            [[nodiscard]] constexpr bool empty() const noexcept { return ((1 * ... * N) == 0) ? true : false; }
            constexpr size_ilist size() const noexcept { return s_sizes; }
            constexpr size_ilist max_size() const noexcept { return s_sizes; }

            /* Operations */
            constexpr void fill(const value_type& value)
            {
                if(std::is_trivially_copyable<value_type>::value)
                {
                    for(iterator itr = begin(); itr != end(); ++itr)
                    {
                        std::memcpy(itr, &value, sizeof(value_type));
                    }
                }
                else
                {
                    for(iterator itr = begin(); itr != end(); ++itr)
                    {
                        *itr = value;
                    }
                }
            }

            constexpr void swap(mdarray<T, N...>& other)
            {
                mdarray<T, N...> temp{};

                if(std::is_trivially_copyable<value_type>::value)
                {
                    std::memcpy(temp.data(), this->data(), sizeof(mdarray<T, N...>));

                    std::memcpy(this->data(), other.data(), sizeof(mdarray<T, N...>));
                    std::memcpy(other.data(), temp.data(), sizeof(mdarray<T, N...>));
                }
                else
                {
                    iterator src;
                    iterator dest;

                    src = this->begin();
                    dest = temp.begin();
                    while(src != this->end())
                    {
                        *dest = *src;
                        ++src;
                        ++dest;
                    }

                    src = other.begin();
                    dest = this->begin();
                    while(src != other.end())
                    {
                        *dest = *src;
                        ++src;
                        ++dest;
                    }

                    src = temp.begin();
                    dest = other.begin();
                    while(src != temp.end())
                    {
                        *dest = *src;
                        ++src;
                        ++dest;
                    }
                }
            }

        private:
            static constexpr size_ilist s_sizes = {N...};
    };
}

#endif