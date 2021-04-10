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
    //
    //--- Forward declaration ---//
    //
    template<typename T, std::size_t... N>
    struct mdarray;
    //
    //--- END --//
    //
    // 

    //
    //--- Helper metafunctions ---//
    //
    template<typename T, std::size_t D>
    using id = T;

    // mdize returns the volume of a mdarray
    // Forward declaration of mdsize
    template<typename T, std::size_t... N>
    struct mdsize;

    template<typename T, std::size_t... N>
    struct mdsize<mdarray<T, N...>>
    {
        static constexpr std::size_t value = (1 * ... * N);
    };

    // subarray is the type of mdarray's subarray. Shaves 1 dimension from the mdarray
    // Forward declaration of subarray
    template<typename T, std::size_t... N>
    struct mdsubarray;
    template<typename T, std::size_t... N>
    using mdsubarray_T = typename mdsubarray<T, N...>::type;

    // Base case of subarray
    template<typename T, std::size_t Head>
    struct mdsubarray<T, Head>
    {
        using type = T;
    };

    // Recursive case of subarray
    template<typename T, std::size_t Head, std::size_t... Tail>
    struct mdsubarray<T, Head, Tail...>
    {
        using type = mdarray<mdsubarray_T<T, Tail...>, Tail...>;
    };

    // index_sequence is a parameter pack containing indices [0, N)
    // Forward declaration of index_sequence
    template<std::size_t... N>
    struct index_sequence {};

    // Forward declaration of next_index_sequence
    template<typename T>
    struct next_index_sequence;

    // Append the next index at the end of an index sequence
    template<std::size_t... N>
    struct next_index_sequence<index_sequence<N...>>
    {
        using type = index_sequence<N..., sizeof...(N)>;
    };

    // Forward declaration of make_index_seq_impl
    template<std::size_t I, std::size_t N>
    struct make_index_seq_impl;
    // Make an index_sequence containing indices [0, N)
    template<std::size_t N>
    using make_index_sequence = make_index_seq_impl<0, N>::type;

    // Recursive case
    template<std::size_t I, std::size_t N>
    struct make_index_seq_impl
    {
        using type = next_index_sequence<typename make_index_seq_impl<I+1, N>::type>::type;
    };

    // Base case (when I reaches the size of the futur index_sequence)
    template<std::size_t N>
    struct make_index_seq_impl<N, N>
    {
        using type = index_sequence<>;
    };

    // mdarray_storage is the data member type. T[] when owning, T* when referencing
    // Forward declaration of mdarray_storage
    template<bool Owning, typename T, std::size_t... N>
    struct mdarray_storage;
    template<bool Owning, typename T, std::size_t... N>
    using mdarray_storage_T = mdarray_storage<Owning, T, N...>::type;

    // Case where mdarray_storage is owning
    template<typename T, std::size_t... N>
    struct mdarray_storage<true, T, N...>
    {
        using type = T[(1*...*N)];
    };

    // Case where mdarray_storage is referencing
    template<typename T, std::size_t... N>
    struct mdarray_storage<false, T, N...>
    {
        using type = T* const;
    };

    // from_array_impl defines a mdarray type from the corresponding built-in type
    // Forward declaration of from_array_impl
    template <typename T, typename Seq>
    struct from_array_impl;

    // Define mdarray from a number of dimensions (rank)
    template <typename T, size_t... Is>
    struct from_array_impl<T, index_sequence<Is...>> 
    {
        //                   Get the type                  Get the extent size (extent) of each dimension
        using type = mdarray<std::remove_all_extents_t<T>, std::extent_v<T, Is>...>;
    };

    // Deduce the number of dimensions (rank) of a built-in array and return the corresponding mdarray
    template <typename T>
    using from_array = from_array_impl<T, make_index_sequence<std::rank_v<T>>>::type;
    //
    //--- END --//
    // 

    //
    //--- Helper functions --//
    //
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
    //--- END --//
    //

    //
    //--- mdarray ---//
    //
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

            using subarray               = mdsubarray_T<value_type&, N...>;
            using const_subarray         = const mdsubarray_T<const value_type&, N...>;

            //--- Data member ---//
            // value_type[] when owning
            // value_type* const when referencing
            mdarray_storage_T<!std::is_reference<T>::value, value_type, N...> _m_arr;
            
            //--- Conversion operator ---//
            operator mdarray<value_type, N...> () const
            {
                return [this]<std::size_t... Seq>(index_sequence<Seq...>) -> mdarray<value_type, N...>
                       {
                           return { _m_arr[Seq]...};
                       }(make_index_sequence<(1 * ... * N)>());
            }

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

            constexpr subarray operator[](size_type pos)
            {
                if constexpr (sizeof...(N) == 1) // If array has 1 dimension, return the value
                    return _m_arr[pos];
                else                             // Else, return a subarray
                    return {&_m_arr[pos * (1 * ... * N) / *s_size_ilist.begin()]};
            }

            constexpr const_subarray operator[](size_type pos) const
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
    //
    //--- END --//
    //
}

//
//--- Specialization of standard library functions ---//
//
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
        // Copy the built-in array into the mdarray
        mdlib::from_array<std::remove_cv_t<T[N]>> temp{};
        std::memcpy(temp.data(), arr, sizeof(temp));
        return temp;
    }
}
//
//--- END --//
//

#endif