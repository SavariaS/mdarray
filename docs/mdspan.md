## mdlib::mdspan
### Declaration

template<typename T, std::size_t... N> <br>
struct mdlib::mdspan;

### Member types

value_type = T<br>
size_type = std::size_t<br>
size_ilist const std::initializer_list\<size_type>&<br>
difference_type = std::ptrdiff_t<br>
pointer = value_type*<br>
const_pointer = const value_type*<br>
iterator = value_type*<br>
const_iterator = const value_type*<br>
reverse_iterator       = std::reverse_iterator\<iterator><br>
const_reverse_iterator = std::reverse_iterator\<const_iterator><br>
subspan = mdlib::to_subspan_t<value_type, N...><br>
const_subspan = mdlib::to_subspan_t<const value_type, N...><br>
array = mdlib::mdarray<T, N...>

### Member functions

#### Element access
constexpr subspan operator\[](size_type) <br>
constexpr const_subspan operator\[](size_type) const

constexpr pointer data() noexcept <br>
constexpr const_pointer data() const noexcept

#### Iterators
constexpr iterator begin() noexcept <br>
constexpr const_iterator cbegin() const noexcept

constexpr iterator end() noexcept <br>
constexpr const_iterator cend() const noexcept

constexpr reverse_iterator rbegin() noexcept <br>
constexpr const_reverse_iterator crbegin() const noexcept

constexpr reverse_iterator rend() noexcept
constexpr const_reverse_iterator crend() const noexcept

#### Capacity
constexpr size_ilist size() const noexcept <br>

### Helper classes
template<typename T, std::size_t... N> <br>
struct std::rank<mdlib::mdspan<T, N...>>

template<typename T, std::size_t Head, std::size_t... Tail, unsigned N> <br>
struct std::extent<mdlib::mdspan<T, Head, Tail...>, N>