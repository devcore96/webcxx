#include <concepts>
#include <functional>
#include <vector>

template<class T>
concept two_way_iterable = requires(T arr) { arr.begin(); arr.rbegin(); arr.end(); arr.rend(); };

template<class T>
concept iterable = requires(T arr) { arr.begin(); arr.end(); };

template<std::equality_comparable T, iterable ArrT = std::vector<T>>
inline bool contains(const ArrT arr, T val) { for(T v : arr) if(v == val) return true; return false; }



template<class T>
concept reverse_iterable = requires(T arr) { arr.rbegin(); arr.rend(); };

template<reverse_iterable T>
struct reverse_impl {
    T& data;
    auto begin() { return data.rbegin(); }
    auto end()   { return data.rend(); }
};

template<reverse_iterable T>
reverse_impl<T> reverse(T&& iterable) { return { iterable }; }



class numeric_iterator {
private:
              size_t it = 0;
    const     size_t size = 0;

public:
    constexpr        numeric_iterator() = default;
    constexpr        numeric_iterator(size_t s) : it(0), size(s)           { }
    constexpr        numeric_iterator(size_t i, size_t s) : it(i), size(s) { }

    constexpr size_t operator* ()                                          { return it; }
    constexpr auto&  operator++()                                          { ++it; return *this; }
    constexpr bool   operator==(numeric_iterator& y)                       { return it == y.it; }
};

class reverse_numeric_iterator {
private:
              size_t it = 0;
    const     size_t size = 0;

public:
    constexpr        reverse_numeric_iterator() = default;
    constexpr        reverse_numeric_iterator(size_t s) : it(s - 1), size(s)               { }
    constexpr        reverse_numeric_iterator(size_t i, size_t s) : it(s - 1 - i), size(s) { }

    constexpr size_t operator* ()                                                          { return it; }
    constexpr auto&  operator++()                                                          { --it; return *this; }
    constexpr bool   operator==(reverse_numeric_iterator& y)                               { return it == y.it; }
};

class iterate_impl {
public:
    using iterator         = numeric_iterator;
    using reverse_iterator = reverse_numeric_iterator;
    using value_type       = size_t;

private:
    iterator         _begin;
    iterator         _end;
    reverse_iterator _rbegin;
    reverse_iterator _rend;

public:
                     iterate_impl(size_t s) : _begin(s), _end(s, s), _rbegin(s), _rend(s, s)                 { }
                     iterate_impl(size_t b, size_t s) : _begin(b, s), _end(s, s), _rbegin(b, s), _rend(s, s) { }
    iterator         begin()                                                                                 { return _begin; }
    iterator         end()                                                                                   { return _end; }
    reverse_iterator rbegin()                                                                                { return _rbegin; }
    reverse_iterator rend()                                                                                  { return _rend; }
};

template<std::convertible_to<size_t> T>
iterate_impl iterate(T size) { return iterate_impl(size); }

template<std::convertible_to<size_t> T, std::convertible_to<size_t> U>
iterate_impl iterate(T begin, U size) { return iterate_impl(begin, size); }


template<two_way_iterable T>
struct offset_impl {
    T& data;
    size_t offset = 0;
    auto  begin() { return data. begin() + offset; }
    auto rbegin() { return data.rbegin() + offset; }
    auto    end() { return data.   end(); }
    auto   rend() { return data.  rend(); }
};

template<two_way_iterable T, std::convertible_to<size_t> U>
offset_impl<T> iterate(T arr, U offset) { return { arr, offset }; }

template<iterable T>
class sized_iterator {
private:
    using reference   = T::reference;
    using pointer     = T::pointer;
          T&     data;
          size_t it   = 0;
    const size_t size = 0;
    const size_t N    = 1;

public:
    constexpr         sized_iterator() = default;
    constexpr         sized_iterator(T& val, size_t i, size_t s, size_t n) :
                                    data(val), N(n), it(i), size(s)    { }
    reference         operator*()                                      { return data[it]; }
    pointer           operator->()                                     { return &data[it]; }
    constexpr auto&   operator++()                                     { it += N; return *this; }
    constexpr bool    operator==(sized_iterator<T>& y)                 { return it >= y.it; }
};

template<reverse_iterable T>
class reverse_sized_iterator {
private:
    using reference   = T::reference;
    using pointer     = T::pointer;
          T&      data;
          size_t  it   = 0;
    const size_t  size = 0;
    const size_t  N    = 1;

public:
    constexpr         reverse_sized_iterator() = default;
    constexpr         reverse_sized_iterator(T& val, size_t i, size_t s, size_t n) :
                                            data(val), N(n), it(s - 1 - i), size(s) { }
    reference         operator*()                                                   { return data[it]; }
    pointer           operator->()                                                  { return &data[it]; }
    constexpr auto&   operator++()                                                  { it -= N; return *this; }
    constexpr bool    operator==(reverse_sized_iterator<T>& y)                      { return it <= y.it; }
};

template<two_way_iterable T>
class sized_iterator_impl {
public:
    using iterator         = sized_iterator<T>;
    using reverse_iterator = reverse_sized_iterator<T>;
    using value_type       = T::value_type;
    using reference        = T::reference;

private:
    T&               data;
    iterator         _begin;
    iterator         _end;
    reverse_iterator _rbegin;
    reverse_iterator _rend;

public:
         sized_iterator_impl(T& val, size_t N, size_t O) : data(val),
                                                           _begin(val, O, val.size(), N),
                                                           _end(val, val.size(), val.size(), N),
                                                           _rbegin(val, O, val.size(), N),
                                                           _rend(val, val.size(), val.size(), N) { }
    auto begin()                                                                                 { return _begin; }
    auto rbegin()                                                                                { return _rbegin; }
    auto end()                                                                                   { return _end; }
    auto rend()                                                                                  { return _rend; }
};

template<two_way_iterable T>
sized_iterator_impl<T> iterate(T& arr, size_t N, size_t O) { return sized_iterator_impl<T>(arr, N, O); }



template<
    iterable ContainerType,
    typename    SourceType,
    typename      DestType
>
std::vector<DestType> map_to(ContainerType source,
                             DestType (*callback)(SourceType)) {
    std::vector<DestType> destination;

    for (auto& value : source)
        destination.push_back(callback(value));

    return destination;
}
