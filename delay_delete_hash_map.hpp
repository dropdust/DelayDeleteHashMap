#ifndef UTILS_DELAY_DELETED_HASH_MAP_HPP_
#define UTILS_DELAY_DELETED_HASH_MAP_HPP_

#include <utility>
#include <type_traits>
#include <initializer_list>
#include "delay_delete_allocator.hpp"
#include "delay_delete_table.hpp"

namespace utils {

template <class Key, class Val,
          class Alloc = DelayDeleteAllocator<std::pair<const Key, Val> >,
          class Equal = std::equal_to<Key>,
          class Hash = std::hash<Key> >
class DelayDeleteHashMap {
 private:
  typedef DelayDeleteHashtable<Key, std::pair<const Key, Val>, Alloc,
            std::_Select1st<std::pair<const Key, Val>>, Equal, Hash> HashTable; 
  HashTable ht_;

 public:
  typedef Val data_type; 
  typedef Val mapped_type;
  typedef typename HashTable::key_type key_type;
  typedef typename HashTable::value_type value_type;
  typedef typename HashTable::size_type size_type;
  typedef typename HashTable::difference_type difference_type;
  typedef typename HashTable::pointer pointer;
  typedef typename HashTable::const_pointer const_pointer;
  typedef typename HashTable::iterator iterator;
  typedef typename HashTable::const_iterator const_iterator;

 public:
  DelayDeleteHashMap() {}
  DelayDeleteHashMap(const DelayDeleteHashMap& other) : ht_(other.ht_) {}
  DelayDeleteHashMap(const DelayDeleteHashMap&& other) : ht_(other.ht_) {}
  DelayDeleteHashMap& operator = (const DelayDeleteHashMap& other) { ht_ = other.ht_;}
  DelayDeleteHashMap& operator = (const DelayDeleteHashMap&& other) { ht_ = other.ht_;}
  int init(size_type n) { return ht_.init(n); }
  size_type size() const { return ht_.size(); }
  size_type resize_count() const { return ht_.resize_count(); }
  bool empty() const { return ht_.empty(); }
  iterator begin() { return ht_.begin(); }
  iterator end() { return ht_.end(); }
  const_iterator begin() const { return ht_.begin(); }
  const_iterator end() const { return ht_.end(); }

  std::pair<iterator, bool> insert(
                              const value_type& obj,
                              bool is_resize = true, bool is_replace = true) {
    return ht_.insert_unique(obj, is_resize, is_replace);
  }

  std::pair<iterator, bool> insert(const Key& k, Val&& v, bool is_resize = true, bool is_replace = true) {
     return ht_.insert_unique(value_type(k, v), is_resize, is_replace); 
  }

  iterator find(const key_type& key) {
    return ht_.find(key);
  }
  const_iterator find(const key_type& key) const {
    return ht_.find(key);
  }

  size_type count(const key_type& key) { return ht_.count(key); }
  void erase(const key_type& key) { ht_.erase(key); }
  void erase(iterator it) { ht_.erase(it); } 
  void erase(iterator f, iterator l) { ht_.erase(f, l); }

  void clear() { ht_.clear(); }
  size_type bucket_count() {return ht_.bucket_count(); }

  void garbage_collect() { ht_.garbage_collect(); } 
};

template <class Key, class Val,
          class Alloc = DelayDeleteAllocator<std::pair<const Key, Val> >,
          class Equal = std::equal_to<Key>,
          class Hash = std::hash<Key> >
class DelayDeleteMultiHashMap {
 private:
  typedef DelayDeleteHashtable<Key, std::pair<const Key, Val>, Alloc,
            std::_Select1st<std::pair<const Key, Val>>, Equal, Hash> HashTable; 
  HashTable ht_;

 public:
  typedef Val data_type; 
  typedef Val mapped_type;
  typedef typename HashTable::key_type key_type;
  typedef typename HashTable::value_type value_type;
  typedef typename HashTable::size_type size_type;
  typedef typename HashTable::difference_type difference_type;
  typedef typename HashTable::pointer pointer;
  typedef typename HashTable::const_pointer const_pointer;
  typedef typename HashTable::iterator iterator;
  typedef typename HashTable::const_iterator const_iterator;
  typedef typename HashTable::value_cmp value_cmp;
  typedef typename HashTable::value_equal value_equal;

 public:
  DelayDeleteMultiHashMap() {}
  DelayDeleteMultiHashMap(const DelayDeleteMultiHashMap& other) : ht_(other.ht_) {}
  DelayDeleteMultiHashMap(const DelayDeleteMultiHashMap&& other) : ht_(other.ht_) {}
  DelayDeleteMultiHashMap& operator = (const DelayDeleteMultiHashMap& other) { ht_ = other.ht_;}
  DelayDeleteMultiHashMap& operator = (const DelayDeleteMultiHashMap&& other) { ht_ = other.ht_;}
  int init(size_type n) { return ht_.init(n); }
  size_type resize_count() const { return ht_.resize_count(); }
  size_type size() const { return ht_.size(); }
  bool empty() const { return ht_.empty(); }
  iterator begin() { return ht_.begin(); }
  iterator end() { return ht_.end(); }
  const_iterator begin() const { return ht_.begin(); }
  const_iterator end() const { return ht_.end(); }
  
  iterator insert_with_value_cmp(const Key& k, const Val& v, value_cmp cmp, bool is_resize = true, bool is_replace = true) {
    //相同key的value值不能重复
    return ht_.insert_equal_with_value_cmp(value_type(k, v), cmp, is_resize, is_replace);
  }
  
  iterator insert_with_value_cmp(const value_type& obj, value_cmp cmp, bool is_resize = true, bool is_replace = true) {
    //相同key的value值不能重复
    return ht_.insert_equal_with_value_cmp(obj, cmp, is_resize, is_replace);
  }

  iterator insert(const Key& k, Val&& v, bool is_resize = true) {
    return ht_.insert_equal(value_type(k, v), is_resize);
  }
  iterator insert(const value_type& obj, bool is_resize = true) {
    return ht_.insert_equal(obj, is_resize);
  }
  iterator find(const key_type& key) {
    return ht_.find(key);
  }
  const_iterator find (const key_type& key) const {
    return ht_.find(key);
  }
  std::pair<iterator, iterator> equal_range(const key_type& key) {
    return ht_.equal_range(key);
  }
  size_type count(const key_type& key) { return ht_.count(key); }
  void erase(const key_type& key) { ht_.erase(key); }
  void erase(iterator it) { ht_.erase(it); } 
  void erase(iterator f, iterator l) { ht_.erase(f, l); }
  void erase(const key_type& key, value_equal value_equal_fun) { ht_.erase(key, value_equal_fun); }

  void clear() { ht_.clear(); }
  size_type bucket_count() {return ht_.bucket_count(); }

  void garbage_collect() { ht_.garbage_collect(); } 
};


}

#endif
