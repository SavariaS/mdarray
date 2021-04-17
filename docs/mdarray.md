## mdlib::mdarray
### Declaration

template<typename T, std::size_t... N> <br>
struct mdlib::mdarray;

### Member types

value_type = T<br>
size_type = std::size_t<br>
size_ilist const std::initializer_list\<size_type>&<br>
difference_type = std::ptrdiff_t<br>
reference = value_type&<br>
const_reference = const value_type&<br>
pointer = value_type*<br>
const_pointer = const value_type*<br>
iterator = value_type*<br>
const_iterator = const value_type*<br>
reverse_iterator       = std::reverse_iterator\<iterator><br>
const_reverse_iterator = std::reverse_iterator\<const_iterator><br>
subarray = mdlib::to_subarray_t<value_type, N...><br>
const_subarray  = const mdlib::to_subarray_t<const value_type, N...><br>
subspan = mdlib::to_subspan_t<value_type, N...><br>
const_subspan = mdlib::to_subspan_t<const value_type, N...><br>

### Member functions

#### Element access
constexpr reference at(id<size_type, N>...)<br>
constexpr const_reference at(id<size_type, N>...) const

constexpr subspan operator\[](size_type) <br>
constexpr const_subspan operator\[](size_type) const

constexpr reference front() <br>
constexpr const_reference front() const

constexpr reference back() <br>
constexpr const_reference back() const

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
[[nodiscard]] constexpr bool empty() const noexcept <br>
constexpr size_ilist size() const noexcept <br>
constexpr size_ilist max_size() const noexcept

#### Operations
constexpr void fill(const value_type&) <br>
constexpr void swap(mdarray<T, N...>&)


### Non-member functions
template<std::size_t... I, typename T, std::size_t... N> <br>
T& std::get(mdlib::mdarray<T, N...>&)

template<typename T, std::size_t... N> <br>
void std::swap(mdlib::mdarray<T, N...>&, mdlib::mdarray<T, N...>&)

template<typename T, std::size_t N> <br>
constexpr mdlib::from_array<std::remove_cv_t<T[N]>> std::to_mdarray(T (&)[N])

### Helper classes
template<typename T, std::size_t... N> <br>
struct std::rank<mdlib::mdarray<T, N...>>

template<typename T, std::size_t Head, std::size_t... Tail, unsigned N> <br>
struct std::extent<mdlib::mdarray<T, Head, Tail...>, N>