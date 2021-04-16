#ifndef MDARRAY_HPP
#define MDARRAY_HPP

// Types 
#include <cstddef>
#include <iterator>
#include <initializer_list>
// Exceptions 
#include <stdexcept>
// Type traits
#include <type_traits>
// Memcpy
#include <cstring>


namespace mdlib
{
    //--- Forward declaration ---//
    template<typename T, std::size_t... N>
    struct mdarray;
    template<typename T, std::size_t... N>
    struct mdspan;

    //--- Helper metafunctions ---//
    template<typename T, std::size_t D>
    using id = T;

    // Forward declaration of subarray
    template<typename T, std::size_t... N>
    struct to_subarray;
    template<typename T, std::size_t... N>
    using to_subarray_T = typename to_subarray<T, N...>::type;

    template<typename T, std::size_t Head>
    struct to_subarray<T, Head>
    {
        using type = T&;
    };

    template<typename T, std::size_t Head, std::size_t... Tail>
    struct to_subarray<T, Head, Tail...>
    {
        using type = mdarray<T, Tail...>;
    };

    // Forward declaration of subarray
    template<typename T, std::size_t... N>
    struct to_subspan;
    template<typename T, std::size_t... N>
    using to_subspan_T = typename to_subspan<T, N...>::type;

    template<typename T, std::size_t Head>
    struct to_subspan<T, Head>
    {
        using type = T&;
    };

    template<typename T, std::size_t Head, std::size_t... Tail>
    struct to_subspan<T, Head, Tail...>
    {
        using type = mdspan<T, Tail...>;
    };

    // Forward declaration of index_sequence
    template<std::size_t... N>
    struct index_sequence {};

    // Forward declaration of next_index_sequence
    template<typename T>
    struct next_index_sequence;

    template<std::size_t... N>
    struct next_index_sequence<index_sequence<N...>>
    {
        using type = index_sequence<N..., sizeof...(N)>;
    };

    // Forward declaration of make_index_seq_impl
    template<std::size_t I, std::size_t N>
    struct make_index_seq_impl;
    template<std::size_t N>
    using make_index_sequence = make_index_seq_impl<0, N>::type;

    template<std::size_t I, std::size_t N>
    struct make_index_seq_impl
    {
        using type = next_index_sequence<typename make_index_seq_impl<I+1, N>::type>::type;
    };

    template<std::size_t N>
    struct make_index_seq_impl<N, N>
    {
        using type = index_sequence<>;
    };

    // Forward declaration of from_array_impl
    template <typename T, typename Seq>
    struct from_array_impl;

    template <typename T, size_t... Is>
    struct from_array_impl<T, index_sequence<Is...>> 
    {
        using type = mdarray<std::remove_all_extents_t<T>, std::extent_v<T, Is>...>;
    };

    template <typename T>
    using from_array = from_array_impl<T, make_index_sequence<std::rank_v<T>>>::type; // Deduce the number of dimensions (rank) of a built-in array and return the corresponding mdarray

    // Copies range
    template<class InputItr>
    void copy(InputItr first, InputItr last, InputItr dest)
    {
        while(first != last)
        {
            *dest = *first;

            ++first; 
            ++dest;
        }
    }

    //--- mdspan ---//
    template<typename T, std::size_t... N>
    struct mdspan
    {
        public:
            //--- Type definitions ---//
            using value_type = std::remove_reference_t<T>;
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

            using subspan                = to_subspan_T<value_type, N...>;
            using const_subspan          = to_subspan_T<const value_type, N...>;
            using array                  = mdarray<T, N...>;

            //--- Data member ---//
            pointer _m_ptr;

            //--- Constructors and conversion operators ---//
            mdspan() : _m_ptr(nullptr) {}
            mdspan(pointer ptr) : _m_ptr(ptr) {}

            operator array() const; // Implicit conversion back to a mdarray

            //--- Elements access ---//
            constexpr subspan operator[](size_type pos)
            {
                if constexpr (sizeof...(N) == 1) // If array has 1 dimension, return the value
                    return _m_ptr[pos];
                else                             // Else, return a subspan
                    return {&_m_ptr[pos * (1 * ... * N) / *s_size_ilist.begin()]};
            }

            constexpr const_subspan operator[](size_type pos) const
            {
                if constexpr (sizeof...(N) == 1) // If array has 1 dimension, return the value
                    return _m_ptr[pos];
                else                             // Else, return a subspan
                    return {&_m_ptr[pos * (1 * ... * N) / *s_size_ilist.begin()]};
            }

            constexpr pointer data() noexcept { return _m_ptr; }
            constexpr const_pointer data() const noexcept { return _m_ptr; }

            //--- Iterators ---// 
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

            //--- Capacity ---//
            constexpr size_ilist size() const noexcept { return s_size_ilist; }

        private:
            //--- Static size initialize list ---//
            static constexpr size_ilist s_size_ilist = {N...};
    };

    //--- mdarray ---//
    template<typename T, std::size_t... N>
    struct mdarray
    {
        public:
            //--- Type definitions ---//
            using value_type = std::remove_reference_t<T>;
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

            using subarray               = to_subarray_T<value_type&, N...>;
            using const_subarray         = const to_subarray_T<const value_type&, N...>;
            using subspan                = to_subspan_T<value_type, N...>;
            using const_subspan          = to_subspan_T<const value_type, N...>;

            //--- Data member ---//
            value_type _m_arr[(1 * ... * N)];

            //--- Elements access ---//
            constexpr reference at(id<size_type, N>... pos)
            {
                size_type index = 0;
                size_type weight = (1 * ... * N);

                (([&index, &weight](size_type pos, size_type dim) // Glorified loop
                {
                    if(pos >= dim)
                        throw std::out_of_range("mdarray::at: pos (which is "+std::to_string(pos)+") >= dim (which is "+std::to_string(dim)+")");

                    weight /= dim;
                    index += pos * weight;
                }(pos, N)),...); // For each index and its corresponding dimension...

                return _m_arr[index];
            }

            constexpr const_reference at(id<size_type, N>... pos) const
            {
                size_type index = 0;
                size_type weight = (1 * ... * N);

                (([&index, &weight](size_type pos, size_type dim) // Glorified loop
                {
                    if(pos >= dim)
                        throw std::out_of_range("mdarray::at: pos (which is "+std::to_string(pos)+") >= dim (which is "+std::to_string(dim)+")");

                    weight /= dim;
                    index += pos * weight;
                }(pos, N)),...); // For each index and its corresponding dimension...

                return _m_arr[index];
            }

            constexpr subspan operator[](size_type pos)
            {
                if constexpr (sizeof...(N) == 1) // If array has 1 dimension, return the value
                    return _m_arr[pos];
                else                             // Else, return a subarray
                    return {&_m_arr[pos * (1 * ... * N) / *s_size_ilist.begin()]};
            }

            constexpr const_subspan operator[](size_type pos) const
            {
                if constexpr (sizeof...(N) == 1) // If array has 1 dimension, return the value
                    return _m_arr[pos];
                else                             // Else, return a subarray
                    return {&_m_arr[pos * (1 * ... * N) / *s_size_ilist.begin()]};
            }

            constexpr reference front() { return _m_arr[0]; }
            constexpr reference back()  { return _m_arr[(1*...*N)]; }
            constexpr const_reference front() const { return _m_arr[0]; }
            constexpr const_reference back()  const { return _m_arr[(1*...*N)]; }

            constexpr pointer data() noexcept { return _m_arr; }
            constexpr const_pointer data() const noexcept { return _m_arr; }

            //--- Iterators ---// 
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

            //--- Capacity ---//
            [[nodiscard]] constexpr bool empty() const noexcept { return ((1 * ... * N) == 0) ? true : false; }
            constexpr size_ilist size() const noexcept { return s_size_ilist; }
            constexpr size_ilist max_size() const noexcept { return s_size_ilist; }

            //--- Operations ---//
            constexpr void fill(const value_type& value)
            {
                for(iterator itr = begin(); itr != end(); ++itr)
                {
                    *itr = value;
                }
            }

            constexpr void swap(mdarray<T, N...>& other)
            {
                mdarray<T, N...> awooo{};

                copy(this->begin(), this->end(), awooo.begin());
                copy(other.begin(), other.end(), this->begin());
                copy(awooo.begin(), awooo.end(), other.begin());
            }

        private:
            //--- Static size initialize list ---//
            static constexpr size_ilist s_size_ilist = {N...};
    };
    
    // Conversion operator from mdpsan to mdarray
    template<typename T, std::size_t... N>
    mdspan<T, N...>::operator mdarray<T, N...>() const
    {
        return [this]<std::size_t... Seq>(index_sequence<Seq...>) -> mdarray<value_type, N...>
               {
                   return { _m_ptr[Seq]...};
               }(make_index_sequence<(1 * ... * N)>());
    }
}

//--- Specialization of standard library functions ---//
namespace std
{
    template<std::size_t... I, typename T, std::size_t... N>
    T& get(mdlib::mdarray<T, N...>& arr)
    {
        return arr.at(I...);
    }

    template<typename T, std::size_t... N>
    void swap(mdlib::mdarray<T, N...>& lhs, mdlib::mdarray<T, N...>& rhs)
    {
        lhs.swap(rhs);
    }

    template<typename T, std::size_t N>
    constexpr mdlib::from_array<std::remove_cv_t<T[N]>> to_mdarray(T (&arr)[N])
    {
        mdlib::from_array<std::remove_cv_t<T[N]>> temp{};
        std::memcpy(temp.data(), arr, sizeof(temp));
        return temp;
    }

    template<typename T, std::size_t... N>
    struct rank<mdlib::mdarray<T, N...>> : public std::integral_constant<std::size_t, sizeof...(N)> {};

    template<typename T, std::size_t... N>
    struct rank<mdlib::mdspan<T, N...>> : public std::integral_constant<std::size_t, sizeof...(N)> {};

    template<typename T, std::size_t Head, std::size_t... Tail>
    struct extent<mdlib::mdarray<T, Head, Tail...>, 0> : std::integral_constant<std::size_t, Head> {};

    template<typename T, std::size_t Head, std::size_t... Tail, unsigned N>
    struct extent<mdlib::mdarray<T, Head, Tail...>, N> : std::extent<mdlib::mdarray<T, Tail...>, N-1> {};

    template<typename T, std::size_t Head, std::size_t... Tail>
    struct extent<mdlib::mdspan<T, Head, Tail...>, 0> : std::integral_constant<std::size_t, Head> {};

    template<typename T, std::size_t Head, std::size_t... Tail, unsigned N>
    struct extent<mdlib::mdspan<T, Head, Tail...>, N> : std::extent<mdlib::mdspan<T, Tail...>, N-1> {};
}

#endif