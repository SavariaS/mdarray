#ifndef MDARRAY_HPP
#define MDARRAY_HPP

#include <cstddef> // std::size_t, std::ptrdiff
#include <iterator> // std::reverse_iterator
#include <initializer_list> // std::initializer_list
#include <stdexcept> // std::out_of_range
#include <type_traits> // General type traits
#include <cstring> // std::memcpy
#include <utility> // std::index_sequence, std::make_index_sequence
#include <algorithm> // std::copy
#include <tuple> // std::make_tuple

//--- Forward declaration ---//
namespace xcontainer
{
    template<typename T, std::size_t... N>
    struct mdarray;
    template<typename T, std::size_t... N>
    struct mdspan;
}

//--- Helper metafunctions ---//
namespace detail
{
    //-- to_subspan ---//
    // Returns a subspan of mdspan. The first dimension (Head) is replaced by Count (N)
    template<typename T, std::size_t N, std::size_t Head, std::size_t... Tail>
    struct to_subspan
    {
        using type = xcontainer::mdspan<T, N, Tail...>;
    };

    //-- to_subview --//
    // Returns a view to a subarray. The first dimension (Head) is dropped
    template<typename T, std::size_t... N>
    struct to_subview;
    template<typename T, std::size_t... N>
    using to_subview_t = typename to_subview<T, N...>::type;

    template<typename T, std::size_t Head>
    struct to_subview<T, Head>
    {
        using type = T&;
    };

    template<typename T, std::size_t Head, std::size_t... Tail>
    struct to_subview<T, Head, Tail...>
    {
        using type = xcontainer::mdspan<T, Tail...>;
    };

    //-- from_array --//
    // Returns the mdarray type equivalent to a built-in multidimensional array
    template <typename T, typename Seq>
    struct from_array_impl;

    template <typename T, size_t... Is>
    struct from_array_impl<T, std::index_sequence<Is...>> 
    {
        using type = xcontainer::mdarray<std::remove_all_extents_t<T>, std::extent_v<T, Is>...>;
    };

    template <typename T>
    using from_array = from_array_impl<T, std::make_index_sequence<std::rank_v<T>>>::type; // Deduce the number of dimensions (rank) of a built-in array and return the corresponding mdarray

    //-- to_array --//
    // Returns the type of a built-in multidimensional array equivalent to a mdspan
    template<typename T, std::size_t... N>
    struct to_array;
    template<typename T, std::size_t... N>
    using to_array_t = typename to_array<T, N...>::type;

    template<typename T, std::size_t Head>
    struct to_array<T, Head>
    {
        using type = T[Head];
    };

    template<typename T, std::size_t Head, std::size_t... Tail>
    struct to_array<T, Head, Tail...>
    {
        using U = typename to_array<T, Tail...>::type;
        using type = U[Head];
    };

    //-- to_bytes --//
    // Returns a span of bytes equivalent to another span of T
    template <auto N>
    struct constant { static constexpr auto value = N; };

    template <typename B, typename T, typename Tuple, std::size_t... Is>
    auto to_bytes_impl(Tuple, std::index_sequence<Is...>) 
    -> xcontainer::mdspan<
        B,
        std::tuple_element_t<Is, Tuple>::value...,
        std::tuple_element_t<sizeof...(Is), Tuple>::value * sizeof(T)>;
    
    template <typename T, std::size_t... N>
    using to_bytes_t = decltype(to_bytes_impl<const std::byte, T>( std::make_tuple(constant<N>{}...), std::make_index_sequence<sizeof...(N) - 1>{} ));

    template <typename T, std::size_t... N>
    using to_writable_bytes_t = decltype(to_bytes_impl<std::byte, T>( std::make_tuple(constant<N>{}...), std::make_index_sequence<sizeof...(N) - 1>{} ));
}

namespace xcontainer
{
    //--- mdspan ---//
    template<typename T, std::size_t... Extents>
    struct mdspan
    {
        public:
            //--- Type definitions ---//
            using value_type             = T;
            using size_type              = std::size_t;
            using size_ilist             = const std::initializer_list<size_type>&;
            using difference_type        = std::ptrdiff_t;

            using reference              = detail::to_subview_t<value_type, Extents...>;
            using pointer                = value_type*;

            using iterator               = value_type*;
            using const_iterator         = const value_type*;
            using reverse_iterator       = std::reverse_iterator<iterator>;
            using const_reverse_iterator = std::reverse_iterator<const_iterator>;

            template<std::size_t Count>
            using subspan_t              = typename detail::to_subspan<T, Count, Extents...>::type;

            //--- Constructors and conversion operators ---//
            constexpr mdspan() noexcept : _m_data(nullptr) {}

            constexpr mdspan(detail::to_array_t<T, Extents...> arr) : _m_data((pointer)arr) {}

            template<typename Itr, std::enable_if_t<!std::is_array_v<Itr> &&                                 // Not an array
                                                    !std::is_same_v<Itr, mdarray<T, Extents...>> &&          // Not mdarray
                                                    !std::is_same_v<Itr, mdspan<T, Extents...>>>* = nullptr> // Not mdspan
            constexpr mdspan(Itr first) : _m_data(std::to_address(first)) {}

            constexpr mdspan(mdarray<T, Extents...>& arr) noexcept : _m_data(arr.data()) {}

            constexpr mdspan(const mdarray<T, Extents...>& arr) noexcept : _m_data(arr.data()) {}

            constexpr mdspan(const mdspan& other) noexcept = default;

            constexpr mdspan& operator=(const mdspan& other) noexcept = default;

            operator mdarray<T, Extents...>() const; // Implicit conversion back to a mdarray

            //--- Elements access ---//
            constexpr reference operator[](size_type pos) const
            {
                if constexpr (sizeof...(Extents) == 1) // If array has 1 dimension, return the value
                {
                    return _m_data[pos];
                }
                else // Else return a view 
                {
                    reference span;
                    span = reference(&_m_data[pos * span.size()]);

                    return span;
                } 
            }

            constexpr reference front() const
            {
                return (*this)[0];
            }

            constexpr reference back() const
            {
                return (*this)[*(s_size_ilist.end() - 1) - 1];
            }

            constexpr pointer data() const noexcept { return _m_data; }

            //--- Iterators ---// 
            constexpr iterator begin() noexcept { return iterator(data()); }
            constexpr iterator end()   noexcept { return iterator(data() + (1*...*Extents)); }
            constexpr reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
            constexpr reverse_iterator rend()   noexcept { return reverse_iterator(begin()); }

            constexpr const_iterator begin() const noexcept { return const_iterator(data()); }
            constexpr const_iterator end()   const noexcept { return const_iterator(data() + (1*...*Extents)); }
            constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
            constexpr const_reverse_iterator rend()   const noexcept { return const_reverse_iterator(begin()); }

            constexpr const_iterator cbegin() const noexcept { return const_iterator(data()); }
            constexpr const_iterator cend()   const noexcept { return const_iterator(data() + (1*...*Extents)); }
            constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(cend()); }
            constexpr const_reverse_iterator crend()   const noexcept { return const_reverse_iterator(cbegin()); }

            //--- Observers ---//
            constexpr size_type size() const noexcept { return (1 * ... * Extents); }
            constexpr size_type size_bytes() const noexcept { return size() * sizeof(T); }
            constexpr size_ilist size_list() const noexcept { return s_size_ilist; }
            [[nodiscard]] constexpr bool empty() const noexcept { return (size() == 0) ? true : false; }

            // Subviews
            template<std::size_t Count>
            constexpr subspan_t<Count> first()
            {
                subspan_t<Count> span = subspan_t<Count>(_m_data);

                return span;
            }

            template<std::size_t Count>
            constexpr subspan_t<Count> last()
            {
                subspan_t<Count> subspan;
                subspan = subspan_t<Count>(end() - subspan.size());
                
                return subspan;
            }

            template<std::size_t Offset, std::size_t Count>
            constexpr subspan_t<Count> subspan()
            {
                subspan_t<Count> subspan;

                size_type elements = subspan.size() / Count;
                subspan = subspan_t<Count>(_m_data + (elements * Offset));
                
                return subspan;
            }

        private:
            //--- Data member ---//
            pointer _m_data;

            //--- Static size initialize list ---//
            static constexpr size_ilist s_size_ilist = {Extents...};
    };

    //--- mdarray ---//
    template<typename T, std::size_t... N>
    struct mdarray
    {
        public:
            //--- Type definitions ---//
            using value_type             = T;
            using size_type              = std::size_t;
            using size_ilist             = const std::initializer_list<size_type>&;
            using difference_type        = std::ptrdiff_t;

            using reference              = value_type&;
            using const_reference        = const value_type&;
            using pointer                = value_type*;
            using const_pointer          = const value_type*;
            using subview                = detail::to_subview_t<value_type, N...>;
            using const_subview          = detail::to_subview_t<const value_type, N...>;

            using iterator               = value_type*;
            using const_iterator         = const value_type*;
            using reverse_iterator       = std::reverse_iterator<iterator>;
            using const_reverse_iterator = std::reverse_iterator<const_iterator>;

            //--- Data member ---//
            value_type _m_arr[(1 * ... * N)];

            //--- Elements access ---//
            constexpr subview at(size_type pos)
            {
                size_type dim = *size_list().begin();
                if(pos >= dim)
                {
                    throw std::out_of_range("mdarray::at: pos (which is " + std::to_string(pos) + ") >= dim (which is " + std::to_string(dim) + ")");
                }

                if constexpr (sizeof...(N) == 1) // If array has 1 dimension, return the value
                {
                    return _m_arr[pos];
                }
                else                             // Else, return a subview
                {
                    mdspan<T, N...> view = *this;
                    return view[pos];
                }
            }

            constexpr const_subview at(size_type pos) const
            {
                size_type dim = *size_list().begin();
                if(pos >= dim)
                {
                    throw std::out_of_range("mdarray::at: pos (which is " + std::to_string(pos) + ") >= dim (which is " + std::to_string(dim) + ")");
                }

                if constexpr (sizeof...(N) == 1) // If array has 1 dimension, return the value
                {
                    return _m_arr[pos];
                }
                else                             // Else, return a subview
                {
                    mdspan<T, N...> view = *this;
                    return view[pos];
                }
            }

            constexpr subview operator[](size_type pos)
            {
                if constexpr (sizeof...(N) == 1) // If array has 1 dimension, return the value
                {
                    return _m_arr[pos];
                }
                else                             // Else, return a subview
                {
                    mdspan<T, N...> view = *this;
                    return view[pos];
                }
            }

            constexpr const_subview operator[](size_type pos) const
            {
                if constexpr (sizeof...(N) == 1) // If array has 1 dimension, return the value
                {
                    return _m_arr[pos];
                }
                else                             // Else, return a subview
                {
                    mdspan<T, N...> view = *this;
                    return view[pos];
                }
            }

            constexpr reference front() { return _m_arr[0]; }
            constexpr reference back()  { return _m_arr[(1*...*N) - 1]; }
            constexpr const_reference front() const { return _m_arr[0]; }
            constexpr const_reference back()  const { return _m_arr[(1*...*N) - 1]; }

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
            constexpr size_type size() const noexcept { return (1 * ... * N); }
            constexpr size_type max_size() const noexcept { return (1 * ... * N); }
            constexpr size_ilist size_list() const noexcept { return s_size_ilist; }

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
                mdarray<T, N...> atemp{};

                copy(this->begin(), this->end(), atemp.begin());
                copy(other.begin(), other.end(), this->begin());
                copy(atemp.begin(), atemp.end(), other.begin());
            }

        private:
            //--- Static size initialize list ---//
            static constexpr size_ilist s_size_ilist = {N...};
    };

    //-- Non-member functions --//
    template<typename T, std::size_t... N>
    bool operator<=>(const mdspan<T, N...>& rhs, const mdspan<T, N...>& lhs)
    {
        auto a = rhs.begin();
        auto b = lhs.begin();

        while(a != rhs.end())
        {
            if(*a != *b)
                return 1;

            ++a;
            ++b;
        }

        return 0;
    }

    template<typename T, std::size_t... N>
    bool operator<=>(const mdarray<T, N...>& rhs, const mdarray<T, N...>& lhs)
    {
        auto a = rhs.begin();
        auto b = lhs.begin();

        while(a != rhs.end())
        {
            if(*a != *b)
                return 1;

            ++a;
            ++b;
        }

        return 0;
    }
    
    template<typename T, std::size_t... N>
    mdspan<T, N...>::operator mdarray<T, N...>() const
    {
        return [this]<std::size_t... Seq>(std::index_sequence<Seq...>) -> mdarray<value_type, N...>
               {
                   return { _m_data[Seq]...};
               }(std::make_index_sequence<(1 * ... * N)>());
    }

    template<typename T, std::size_t... N>
    detail::to_bytes_t<T, N...> as_bytes(mdspan<T, N...> span) noexcept
    {
        detail::to_bytes_t<T, N...> bytes_span = reinterpret_cast<const std::byte*>(span.data());
        return bytes_span;
    }

    template<typename T, std::size_t... N, std::enable_if_t<!std::is_const_v<T>>* = nullptr>
    detail::to_writable_bytes_t<T, N...> as_writable_bytes(mdspan<T, N...> span) noexcept
    {
        detail::to_writable_bytes_t<T, N...> bytes_span = reinterpret_cast<std::byte*>(span.data());
        return bytes_span;
    }
}

//--- Specialization of standard library functions ---//
namespace std
{
    template<std::size_t... I, typename T, std::size_t... N>
    T& get(xcontainer::mdarray<T, N...>& arr)
    {
        return arr.at(I...);
    }

    template<typename T, std::size_t... N>
    void swap(xcontainer::mdarray<T, N...>& lhs, xcontainer::mdarray<T, N...>& rhs)
    {
        lhs.swap(rhs);
    }

    template<typename T, std::size_t N>
    constexpr detail::from_array<std::remove_cv_t<T[N]>> to_mdarray(T (&arr)[N])
    {
        detail::from_array<std::remove_cv_t<T[N]>> temp{};
        std::memcpy(temp.data(), arr, sizeof(temp));
        return temp;
    }

    template<typename T, std::size_t... N>
    struct rank<xcontainer::mdarray<T, N...>> : public std::integral_constant<std::size_t, sizeof...(N)> {};

    template<typename T, std::size_t... N>
    struct rank<xcontainer::mdspan<T, N...>> : public std::integral_constant<std::size_t, sizeof...(N)> {};

    template<typename T, std::size_t Head, std::size_t... Tail>
    struct extent<xcontainer::mdarray<T, Head, Tail...>, 0> : std::integral_constant<std::size_t, Head> {};

    template<typename T, std::size_t Head, std::size_t... Tail, unsigned N>
    struct extent<xcontainer::mdarray<T, Head, Tail...>, N> : std::extent<xcontainer::mdarray<T, Tail...>, N-1> {};

    template<typename T, std::size_t Head, std::size_t... Tail>
    struct extent<xcontainer::mdspan<T, Head, Tail...>, 0> : std::integral_constant<std::size_t, Head> {};

    template<typename T, std::size_t Head, std::size_t... Tail, unsigned N>
    struct extent<xcontainer::mdspan<T, Head, Tail...>, N> : std::extent<xcontainer::mdspan<T, Tail...>, N-1> {};
}

#endif