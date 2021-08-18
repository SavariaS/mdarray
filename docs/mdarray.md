
# mdlib::mdarray

  

```c++
template<typename  T, std::size_t... N>
struct  mdlib::mdarray;
```

## Typedefs
| Alias| Type |
| ---: | :---|
| `value_type` | `T`
| `size_type` | `std::size_t`|
| `size_ilist` | `const std::initializer_list<size_type>&`|
| `difference_type` | `std::ptrdiff_t` | 
| `reference` | `value_type&` |
| `const_reference` | `const value_type&` |
| `pointer` | `value_type*` |
| `const_pointer` | `const value_type*` |
| `iterator` | `value_type*` |
| `const_iterator` | `const value_type*` |
| `reverse_iterator` | `std::reverse_iterator<iterator>` |
| `const_reverse_iterator` | `std::reverse_iterator<const_iterator>` |
| `subarray` | `mdlib::to_subarray_t<value_type, N...>` |
| `const_subarray` | `const mdlib::to_subarray_t<const value_type, N...>` |
| `subspan` | `mdlib::to_subspan_t<value_type, N...>` |
| `const_subspan` | `mdlib::to_subspan_t<const value_type, N...>` |

## Member functions

  

#### Element access

| Name | Function |
| :---: | :--- |
| at | `constexpr reference at(id<size_type, N>...)` |
|  | `constexpr const_reference at(id<size_type, N>...) const` |
| operator[] | `constexpr subspan operator[](size_type)` |
| | `constexpr const_subspan operator[](size_type) const` |
| front | `constexpr reference front()` |
|  | `constexpr const_reference front() const` |
| back | `constexpr reference back()` |
|  | `constexpr const_reference back() const` |
| data | `constexpr pointer data() noexcept` |
|  | `constexpr const_pointer data() const noexcept` |

#### Iterators

| Name | Functions |
| :---: | :--- |
| begin | `constexpr iterator begin() noexcept` |
| cbegin | `constexpr const_iterator cbegin() const noexcept` |
| end | `constexpr iterator end() noexcept` |
| cend | `constexpr const_iterator cend() const noexcept` |
| rbegin | `constexpr reverse_iterator rbegin() noexcept` |
| crbegin | `constexpr const_reverse_iterator crbegin() const noexcept` |
| rend | `constexpr reverse_iterator rend() noexcept` |
| crend | `constexpr const_reverse_iterator crend() const noexcept` |

  

#### Capacity

| Name | Function |
| :---: | :--- |
| empty | `[[nodiscard]] constexpr bool empty() const noexcept` |
| size | `constexpr size_ilist size() const noexcept` |
| max_size | `constexpr size_ilist max_size() const noexcept` |

  

#### Operations

| Name | Function |
| :---: | :--- |
| fill | `constexpr void fill(const value_type&)` |
| swap | `constexpr void swap(mdarray<T,  N...>&)` |

  
  

## Non-member functions

std::get
```c++
template<std::size_t...  I,  typename  T,  std::size_t...  N>
T& std::get(mdlib::mdarray<T,  N...>&)
```

std::swap
```c++
template<typename  T,  std::size_t...  N>
void std::swap(mdlib::mdarray<T,  N...>&, mdlib::mdarray<T,  N...>&)
```

mdlib::from_array
```c++
template<typename  T,  std::size_t  N>
constexpr mdlib::from_array<std::remove_cv_t<T[N]>> std::to_mdarray(T (&)[N])
```

  

### Helper classes

std::rank
```c++
template<typename  T,  std::size_t...  N>
struct std::rank<mdlib::mdarray<T,  N...>>
```

std::extent
```c++
template<typename  T,  std::size_t  Head,  std::size_t...  Tail,  unsigned  N> 
struct std::extent<mdlib::mdarray<T,  Head,  Tail...>, N>
```
