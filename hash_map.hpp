#pragma once

#include <functional>
#include <memory>
#include <utility>
#include <type_traits>
#include <iostream>
#include <vector>
#include <cmath>
#include <limits>

namespace fefu
{
enum cellState {_empty, _busy, _freed};

template<typename T>
class allocator {
 public:
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using pointer = T*;
  using const_pointer = const T*;
  using reference = typename std::add_lvalue_reference<T>::type;
  using const_reference = typename std::add_lvalue_reference<const T>::type;
  using value_type = T;

  allocator() noexcept = default;

  allocator(const allocator&) noexcept = default;

  template <class U>
  explicit allocator(const allocator<U>&) noexcept;

  ~allocator() = default;

  pointer allocate(size_type n) {
    return static_cast<pointer>(:: operator new (n * sizeof(value_type)));
  }

  void deallocate(pointer p, size_type n) noexcept {
    ::operator delete(p);
  }
};

template<typename K, typename T,
    typename Hash = std::hash<K>,
    typename Pred = std::equal_to<K>,
    typename Alloc = allocator<std::pair<const K, T>>>
    class hash_map;


template<typename ValueType>
class hash_map_iterator {
 public:
  using iterator_category = std::forward_iterator_tag;
  using value_type = ValueType;
  using difference_type = std::ptrdiff_t;
  using reference = ValueType&;
  using pointer = ValueType*;

  template<typename K, typename T,
      typename Hash,
      typename Pred,
      typename Alloc>
  friend class hash_map;
  template<typename V>
  friend class hash_map_const_iterator;

  hash_map_iterator() noexcept = default;
  hash_map_iterator(const hash_map_iterator& other) noexcept:
    _x(other._x),
    _dataStates(other._dataStates),
    _xIndex(other._xIndex),
    _mapSize(other._mapSize) {}

  reference operator*() const {
    return *(_x + _xIndex);
  }
  pointer operator->() const {
    return _x + _xIndex;
  }

  // prefix ++
  hash_map_iterator& operator++() {
    for (size_t i = _xIndex + 1; i < _mapSize; i++)
      if (_dataStates[i] == _busy) {
        _xIndex = i;
        return *this;
      }

    _xIndex = _mapSize;
    return *this;
  }
  // postfix ++
  hash_map_iterator operator++(int) {
    auto tmp = hash_map_iterator(*this);
    operator++();
    return tmp;
  }

  friend bool operator==(const hash_map_iterator<ValueType>& a,const hash_map_iterator<ValueType>& b) {
    return (a._x + a._xIndex  == b._x + b._xIndex);
  }
  friend bool operator!=(const hash_map_iterator<ValueType>& a,const hash_map_iterator<ValueType>& b) {
    return !(a == b);
  }

 private:
  const pointer _x;
  const cellState* _dataStates;
  size_t _xIndex;
  size_t _mapSize;

  hash_map_iterator(
      const pointer x,
      size_t index,
      const cellState* dataStates,
      size_t mapSize):
      _x(x),
      _dataStates(dataStates),
      _xIndex(index),
      _mapSize(mapSize) {}
};

template<typename ValueType>
class hash_map_const_iterator {
// Shouldn't give non const references on value
 public:
  using iterator_category = std::forward_iterator_tag;
  using value_type = ValueType;
  using difference_type = std::ptrdiff_t;
  using reference = const ValueType&;
  using pointer = const ValueType*;

  template<typename K, typename T,
      typename Hash,
      typename Pred,
      typename Alloc>
  friend class hash_map;

  hash_map_const_iterator() noexcept = default;
  hash_map_const_iterator(const hash_map_const_iterator& other) noexcept :
      _x(other._x),
      _dataStates(other._dataStates),
      _xIndex(other._xIndex),
      _mapSize(other._mapSize) {}

  hash_map_const_iterator(const hash_map_iterator<ValueType>& other) noexcept :
      _x(other._x),
      _dataStates(other._dataStates),
      _xIndex(other._xIndex),
      _mapSize(other._mapSize) {}

  reference operator*() const {
    return *(_x + _xIndex);
  }
  pointer operator->() const {
    return _x + _xIndex;
  }

  // prefix ++
  hash_map_const_iterator& operator++() {
    for (size_t i = _xIndex + 1; i < _mapSize; i++)
      if (_dataStates[i] == _busy) {
        _xIndex = i;
        return *this;
      }

    _xIndex = _mapSize;
    return *this;
  }
  // postfix ++
  hash_map_const_iterator operator++(int) {
    auto tmp = hash_map_const_iterator(*this);
    operator++();
    return tmp;
  }

  friend bool operator==(const hash_map_const_iterator<ValueType>& a, const hash_map_const_iterator<ValueType>& b) {
    return (a._x + a._xIndex  == b._x + b._xIndex);
  }
  friend bool operator!=(const hash_map_const_iterator<ValueType>& a, const hash_map_const_iterator<ValueType>& b) {
    return !(a == b);
  }

 private:
  const pointer _x;
  const cellState* _dataStates;
  size_t _xIndex;
  size_t _mapSize;

  hash_map_const_iterator(
      const pointer x,
      size_t index,
      const cellState* dataStates,
      size_t mapSize):
      _x(x),
      _dataStates(dataStates),
      _xIndex(index),
      _mapSize(mapSize) {}
};

template<typename K, typename T,
    typename Hash,
    typename Pred,
    typename Alloc>
class hash_map
{
 public:
  using key_type = K;
  using mapped_type = T;
  using hasher = Hash;
  using key_equal = Pred;
  using allocator_type = Alloc;
  using value_type = std::pair<const key_type, mapped_type>;
  using reference = value_type&;
  using const_reference = const value_type&;
  using iterator = hash_map_iterator<value_type>;
  using const_iterator = hash_map_const_iterator<value_type>;
  using size_type = std::size_t;

 private:
  allocator_type _allocator = allocator_type();
  value_type* _data = _allocator.allocate(13);
  cellState* _cellsState = new cellState[13];
  float _loadFactor = 0.75;
  size_type _elementCount = 0;
  size_type _deletedElementCount = 0;
  size_type _bucketCount = 13;
  hasher _hash;
  key_equal _equal;

 public:
  /// Default constructor.
  hash_map() : hash_map(13) {}

  /**
   *  @brief  Default constructor creates no elements.
   *  @param n  Minimal initial number of buckets.
   */
  explicit hash_map(size_type n): hash_map(n, allocator_type()) {}

  /**
   *  @brief  Builds an %hash_map from a range.
   *  @param  first  An input iterator.
   *  @param  last  An input iterator.
   *  @param  n  Minimal initial number of buckets.
   *
   *  Create an %hash_map consisting of copies of the elements from
   *  [first,last).  This is linear in N (where N is
   *  distance(first,last)).
   */
  template<typename InputIterator>
  hash_map(InputIterator first, InputIterator last, size_type n = 0) : hash_map(n) {
    insert(first, last);
  }

  /// Copy constructor.
  hash_map(const hash_map& tmp) : hash_map(tmp, tmp.get_allocator()) {}

  /// Move constructor.
  hash_map(hash_map&& map) : hash_map() {
    swap(map);
  }

  /**
   *  @brief Creates an %hash_map with no elements.
   *  @param a An allocator object.
   */
  explicit hash_map(const allocator_type& a) : hash_map(13, a) {}

  /**
  *  @brief Copy constructor with allocator argument.
  *  @param  uset  Input %hash_map to copy.
  *  @param  a  An allocator object.
  */
  hash_map(const hash_map& umap,
           const allocator_type& a) : hash_map(umap.bucket_count(), a) {
    _hash = umap._hash;
    _equal = umap._equal;
    insert(umap.cbegin(), umap.cend());
  }

  /**
  *  @brief  Move constructor with allocator argument.
  *  @param  uset Input %hash_map to move.
  *  @param  a    An allocator object.
  */
  hash_map(hash_map&& umap,
           const allocator_type& a) : hash_map(umap, a) {}

  /**
   *  @brief  Builds an %hash_map from an initializer_list.
   *  @param  l  An initializer_list.
   *  @param n  Minimal initial number of buckets.
   *
   *  Create an %hash_map consisting of copies of the elements in the
   *  list. This is linear in N (where N is @a l.size()).
   */
  hash_map(std::initializer_list<value_type> l,
           size_type n = 0) : hash_map( (n != 0) ? n : (l.size() * 2) ) {
    insert(l);
  }

  /// Copy assignment operator.
  hash_map& operator=(const hash_map& map) {
    hash_map tmp(map);
    swap(tmp);
    return *this;
  }

  /// Move assignment operator.
  hash_map& operator=(hash_map&& map) {
    swap(map);
    return *this;
  }

  /**
   *  @brief  %hash_map list assignment operator.
   *  @param  l  An initializer_list.
   *
   *  This function fills an %hash_map with copies of the elements in
   *  the initializer list @a l.
   *
   *  Note that the assignment completely changes the %hash_map and
   *  that the resulting %hash_map's size is the same as the number
   *  of elements assigned.
   */
  hash_map& operator=(std::initializer_list<value_type> l) {
    hash_map tmp(l);
    swap(tmp);
    return *this;
  }

  ///  Returns the allocator object used by the %hash_map.
  allocator_type get_allocator() const noexcept {
    return _allocator;
  }

  // size and capacity:

  ///  Returns true if the %hash_map is empty.
  bool empty() const noexcept {
    return _elementCount <= 0;
  }

  ///  Returns the size of the %hash_map.
  size_type size() const noexcept {
    return _elementCount;
  }

  ///  Returns the maximum size of the %hash_map.
  size_type max_size() const noexcept {
    return std::numeric_limits<size_type>().max();
  }

  // iterators.

  /**
   *  Returns a read/write iterator that points to the first element in the
   *  %hash_map.
   */
  iterator begin() noexcept {
    size_type index = findFirstBusyCell();
    return hash_map_iterator<value_type>(_data, index, _cellsState, bucket_count());
  }

  //@{
  /**
   *  Returns a read-only (constant) iterator that points to the first
   *  element in the %hash_map.
   */
  const_iterator begin() const noexcept {
    return cbegin();
  }

  const_iterator cbegin() const noexcept {
    size_type index = findFirstBusyCell();
    return hash_map_const_iterator<value_type>(_data, index, _cellsState, bucket_count());
  }

  /**
   *  Returns a read/write iterator that points one past the last element in
   *  the %hash_map.
   */
  iterator end() noexcept {
    return hash_map_iterator<value_type>(_data, bucket_count(), _cellsState, bucket_count());
  }

  //@{
  /**
   *  Returns a read-only (constant) iterator that points one past the last
   *  element in the %hash_map.
   */
  const_iterator end() const noexcept {
    return cend();
  }

  const_iterator cend() const noexcept {
    return hash_map_const_iterator<value_type>(_data, bucket_count(), _cellsState, bucket_count());
  }



  /**
   *  @brief Erases elements according to the provided key.
   *  @param  x  Key of element to be erased.
   *  @return  The number of elements erased.
   *
   *  This function erases all the elements located by the given key from
   *  an %hash_map. For an %hash_map the result of this function
   *  can only be 0 (not present) or 1 (present).
   *  Note that this function only erases the element, and that if the
   *  element is itself a pointer, the pointed-to memory is not touched in
   *  any way.  Managing the pointer is the user's responsibility.
   */
  size_type erase(const key_type& x) {
    auto iterator = find(x);
    if (iterator != end()) {
      erase(iterator);
      return 1;
    }
    return 0;
  }


  void clear() noexcept {
    for (auto i = 0; i < bucket_count(); ++i) {
      if (_cellsState[i] == _busy) {
        destroy_at(_data + i);
      }
      _cellsState[i] = _empty;
    }

    _deletedElementCount = 0;
    _elementCount = 0;
  }

  /**
   *  @brief  Swaps data with another %hash_map.
   *  @param  x  An %hash_map of the same element and allocator
   *  types.
   *
   *  This exchanges the elements between two %hash_map in constant
   *  time.
   *  Note that the global std::swap() function is specialized such that
   *  std::swap(m1,m2) will feed to this function.
   */
  void swap(hash_map& x) {
    std::swap(_allocator, x._allocator);
    std::swap(_data, x._data);
    std::swap(_cellsState, x._cellsState);
    std::swap(_loadFactor, x._loadFactor);
    std::swap(_elementCount, x._elementCount);
    std::swap(_deletedElementCount, x._deletedElementCount);
    std::swap(_hash, x._hash);
    std::swap(_equal, x._equal);
    std::swap(_bucketCount, x._bucketCount);
  }

    std::pair<iterator, bool> insert(K key, T value) {
        if (load_factor() >= _loadFactor)
            rehash(_bucketCount * 2);
        bool state;
        int hash_index = hasher_(key) % _bucketCount;
        iterator it;
        it.hash_index = hash_index;
        it.p = _data;
        it.capacity = _bucketCount;
        it.status_ = _cellsState;
        it = it.next_free_space();

        if (it == end()) {
            it = end();
            state = false;
            return pair<iterator, bool>(it, state);
        } else {
            state = true;
            _elementCount++;
            _cellsState[hash_index] = _busy;
            new(_data + hash_index) value_type(key, value);
            return pair<iterator, bool>(hash_map_iterator<value_type>(_data,_bucketCount,_cellsState,hash_index),state);
        }
    }


  ///  Returns the hash functor object with which the %hash_map was
  ///  constructed.
  Hash hash_function() const {
    return _hash;
  }

  ///  Returns the key comparison object with which the %hash_map was
  ///  constructed.
  Pred key_eq() const {
    return _equal;
  }

  // lookup.

  //@{
  /**
   *  @brief Tries to locate an element in an %hash_map.
   *  @param  x  Key to be located.
   *  @return  Iterator pointing to sought-after element, or end() if not
   *           found.
   *
   *  This function takes a key and tries to locate the element with which
   *  the key matches.  If successful the function returns an iterator
   *  pointing to the sought after element.  If unsuccessful it returns the
   *  past-the-end ( @c end() ) iterator.
   */
  iterator find(const key_type& x) {
    size_type index = bucket(x);
    if (_cellsState[index] == _busy) {
      return hash_map_iterator<value_type>(_data, index, _cellsState, _bucketCount);
    } else {
      return end();
    }
  }

  const_iterator find(const key_type& x) const {
    size_type index = bucket(x);
    if (_cellsState[index] == _busy) {
      return hash_map_const_iterator<value_type>(_data, index, _cellsState, _bucketCount);
    } else {
      return cend();
    }
  }

  //@}

  /**
   *  @brief  Finds the number of elements.
   *  @param  x  Key to count.
   *  @return  Number of elements with specified key.
   *
   *  This function only makes sense for %unordered_multimap; for
   *  %hash_map the result will either be 0 (not present) or 1
   *  (present).
   */
  size_type count(const key_type& x) const {
    return contains(x) ? 1 : 0;
  }

  /**
   *  @brief  Finds whether an element with the given key exists.
   *  @param  x  Key of elements to be located.
   *  @return  True if there is any element with the specified key.
   */
  bool contains(const key_type& x) const {
    size_type index = bucket(x);
    return _cellsState[index] == _busy;
  }

  //@{
  /**
   *  @brief  Subscript ( @c [] ) access to %hash_map data.
   *  @param  k  The key for which data should be retrieved.
   *  @return  A reference to the data of the (key,data) %pair.
   *
   *  Allows for easy lookup with the subscript ( @c [] )operator.  Returns
   *  data associated with the key specified in subscript.  If the key does
   *  not exist, a pair with that key is created using default values, which
   *  is then returned.
   *
   *  Lookup requires constant time.
   */
  mapped_type& operator[](const key_type& k) {
    auto res = insert(value_type{k, mapped_type()});
    return res.first->second;
  }

  mapped_type& operator[](key_type&& k) {
    auto res = insert(value_type{std::move(k), mapped_type()});
    return res.first->second;
  }
  //@}

  //@{
  /**
   *  @brief  Access to %hash_map data.
   *  @param  k  The key for which data should be retrieved.
   *  @return  A reference to the data whose key is equal to @a k, if
   *           such a data is present in the %hash_map.
   *  @throw  std::out_of_range  If no such data is present.
   */
  mapped_type& at(const key_type& k) {
    return common_at(k);
  }

  const mapped_type& at(const key_type& k) const {
    return common_at(k);
  }
  //@}

  // bucket interface.at

  /// Returns the number of buckets of the %hash_map.
  size_type bucket_count() const noexcept {
    return _bucketCount;
  }

  /*
  * @brief  Returns the bucket index of a given element.
  * @param  _K  A key instance.
  * @return  The key bucket index.
  */
  size_type bucket(const key_type& _K) const {
    size_type index = hashFun(_K);

    while (!_equal(_K, _data[index].first) &&
    (_cellsState[index] == _busy || _cellsState[index] == _freed)) {
      index = (index + 1) % bucket_count();
    }

    return index;
  }

  // hash policy.

  /// Returns the average number of elements per bucket.
  float load_factor() const noexcept {
    return static_cast<float>(loadCells()) / static_cast<float>(bucket_count());
  }

  /// Returns a positive number that the %hash_map tries to keep the
  /// load factor less than or equal to.
  float max_load_factor() const noexcept {
    return _loadFactor;
  }

  /**
   *  @brief  Change the %hash_map maximum load factor.
   *  @param  z The new maximum load factor.
   */
  void max_load_factor(float z) {
    _loadFactor = z;
  }

  /**
   *  @brief  May rehash the %hash_map.
   *  @param  n The new number of buckets.
   *
   *  Rehash will occur only if the new number of buckets respect the
   *  %hash_map maximum load factor.
   */
  void rehash(size_type n) {
    if (static_cast<float>(loadCells()) / n > max_load_factor()) return;
    std::vector<value_type> tmp(begin(), end());
    destroy();
    _data = _allocator.allocate(n);
    _cellsState = new cellState[n];
    _bucketCount = n;
    insert(tmp.cbegin(), tmp.cend());
  }

  /**
   *  @brief  Prepare the %hash_map for a specified number of
   *          elements.
   *  @param  n Number of elements required.
   *
   *  Same as rehash(ceil(n / max_load_factor())).
   */
  void reserve(size_type n) {
    rehash(ceil(n / max_load_factor()));
  }

  bool operator==(const hash_map& other) const {
    if (size() != other.size()) return false;

    for(auto& i : other) {
      auto tmp = find(i.first);
      if (tmp == end()) return false;
      if (tmp->second != i.second) return false;
    }

    return true;
  }

  ~hash_map() {
    destroy();
  }

 private:
  hash_map(size_type n, const allocator_type& a) :
      _allocator(a),
      _data(_allocator.allocate(n)),
      _cellsState(new cellState[n]),
      _loadFactor(0.75),
      _elementCount(0),
      _deletedElementCount(0),
      _bucketCount(n) {}

  void destroy() {
    clear();
    _allocator.deallocate(_data, bucket_count());
    delete[]_cellsState;
  }

  mapped_type& common_at(const key_type& k) {
    iterator iter = find(k);
    if(iter == end()) {
      throw std::out_of_range("item not found");
    }
    return (iter->second);
  }

  template <typename... _Args>
  std::pair<iterator, bool> common_try_emplace(key_type&& k, _Args&&... args) {
    auto iter = find(k);
    if(iter != end()) return std::make_pair(iter, false);

    return emplace(std::piecewise_construct,
                   std::forward_as_tuple(std::move(k)),
                   std::forward_as_tuple(std::forward<_Args>(args)...));
  }

  template <typename _Obj>
  std::pair<iterator, bool> common_insert_or_assign(key_type&& k, _Obj&& obj){
    auto res = insert(value_type{std::move(k), obj});
    if (!res.second) {
      res.first->second = std::forward<_Obj>(obj);
    }
    return res;
  }

  std::pair<iterator, bool> common_insert(value_type&& x){
    auto iter = find(x.first);
    if (iter != end())
      return std::pair<iterator,bool>(iter, false);

    size_type index = bucketEmptyCell(x.first);
    if(_cellsState[index] == _freed)
      _deletedElementCount--;

    new(_data + index) value_type{std::move(x)};
    _cellsState[index] = _busy;
    _elementCount++;
    if (load_factor() > _loadFactor) {
      rehash(bucket_count() * 2);
      std::cout << "need to resize" << std::endl;
    }
    return std::pair<iterator,bool>(iterator(_data, index, _cellsState, _bucketCount), true);
  }

  size_type loadCells() const {
    return (_elementCount + _deletedElementCount);
  }

  size_type hashFun(const key_type& k) const {
    return (_hash(k) % bucket_count());
  }

  size_type findFirstBusyCell() const{
    for (size_type i = 0; i < bucket_count(); i++) {
      if(_cellsState[i] == _busy) return i;
    }
    return bucket_count();
  }

  size_type bucketEmptyCell(const key_type& _K) const{
    size_type index = hashFun(_K);

    while (!_equal(_K, _data[index].first) &&
        (_cellsState[index] == _busy)) {
      index = (index + 1) % bucket_count();
    }

    return index;
  }
};

} // namespace fefu