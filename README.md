# mdlib::mdarray (C++ 20)

**mdlib::mdarray** is a multidimensional array container using the same public interface as the standard library's `std::array`. It requires C++20 and is licensed under the MIT license.

## Documentation

[mdarray](https://github.com/SavariaS/mdarray/docs/mdarray.md) <br>
[mdspan](https://github.com/SavariaS/mdarray/docs/mdspan.md) <br>

## Related proposals

### [P009R10](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p0009r10.html): mdpsan

### [P2128R3](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2128r3.pdf): Multidimensional subscript operator

## To-Do List

* Format documentation
* Write documentation for individual functions
* Add the following operators:
    * `mdlib::mdarray::operator<=>(mdlib::mdarray)`
    * `mdlib:mdspan::operator<=>(mdlib::mdspan)`
* Add the following helper classes:
    * `std::tuple_size<mdlib::mdarray>`
    * `std::tuple_element<mdlib::mdarray>`
    * `std::tuple_size<mdlib::mdspan>`?
    * `std::tuple_element<mdlib::mdspan>`?