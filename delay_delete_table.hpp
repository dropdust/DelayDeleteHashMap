#ifndef UTILS_DELAY_DELETE_TABLE_HPP_ 
#define UTILS_DELAY_DELETE_TABLE_HPP_ 

#include <memory.h>
#include <iterator>
#include <new>
#include <functional>
#include <iostream>

namespace utils {

inline size_t find_near_prime (size_t n_bucket) {
    // Following prime sequence is copied from STL
    static const unsigned long __stl_prime_list[] = {
        53ul,         97ul,         193ul,       389ul,       769ul,
        1543ul,       3079ul,       6151ul,      12289ul,     24593ul,
        49157ul,      98317ul,      196613ul,    393241ul,    786433ul,
        1572869ul,    3145739ul,    6291469ul,   12582917ul,  25165843ul,
        50331653ul,   100663319ul,  201326611ul, 402653189ul, 805306457ul,
        1610612741ul, 3221225473ul, 4294967291ul
    };
    const size_t n_prime =
        sizeof(__stl_prime_list)/sizeof(__stl_prime_list[0]);
    
    for (size_t i = 0; i < n_prime; i++) {
        if (__stl_prime_list[i] >= n_bucket) {
            return __stl_prime_list[i];
        }
    }
    return n_bucket;
}

template <class Val>
struct HashTableNode {
  HashTableNode* p_next; 
  Val* p_value;
};

template <typename Key, typename Val, typename Alloc, typename ExtractKey,
         typename Equal, typename Hash>
class DelayDeleteHashtable;

template <typename Key, typename Val, typename Alloc, typename ExtractKey,
         typename Equal, typename Hash>
struct DelayDeleteHashtableIterator {
  typedef DelayDeleteHashtable<Key, Val, Alloc, ExtractKey, Equal, Hash> hashtable;
  typedef DelayDeleteHashtableIterator<Key, Val, Alloc, ExtractKey, Equal, Hash> iterator;
  typedef DelayDeleteHashtableIterator<Key, Val, Alloc, ExtractKey, Equal, Hash> const_iterator;
  typedef HashTableNode<Val> Node;
  typedef std::forward_iterator_tag iterator_category;
  typedef Val value_type;
  typedef ptrdiff_t difference_type;
  typedef size_t size_type;
  typedef Val& reference;
  typedef Val* pointer;
  Node* cur_ {nullptr};
  Node** ht_ {nullptr};
  size_type ht_sz_ {0};
  ExtractKey extract_key_;
  Hash hash_func_;

  DelayDeleteHashtableIterator() {}
  DelayDeleteHashtableIterator(Node* n, Node** tab, size_type sz) :
    cur_(n), ht_(tab), ht_sz_(sz) {
  }
  DelayDeleteHashtableIterator(const DelayDeleteHashtableIterator& other) {
    cur_ = other.cur_;
    ht_ = other.ht_;
    ht_sz_ = other.ht_sz_;
  }
  DelayDeleteHashtableIterator(const DelayDeleteHashtableIterator&& other) {
    cur_ = other.cur_;
    ht_ = other.ht_;
    ht_sz_ = other.ht_sz_;
  }
  DelayDeleteHashtableIterator& operator = (const DelayDeleteHashtableIterator& other) {
    cur_ = other.cur_;
    ht_ = other.ht_;
    ht_sz_ = other.ht_sz_;
    return *this;
  }
  DelayDeleteHashtableIterator& operator = (const DelayDeleteHashtableIterator&& other) {
    cur_ = other.cur_;
    ht_ = other.ht_;
    ht_sz_ = other.ht_sz_;
    return *this;
  }
  
  explicit operator bool() const noexcept {
    return nullptr != cur_;
  }
  reference operator* () const { return *(cur_->p_value); }
  pointer operator-> () const { return cur_->p_value; }
  bool operator== (const iterator& it) const {
    return cur_ == it.cur_ && ht_ == it.ht_ && ht_sz_ == it.ht_sz_;
  }
  bool operator!= (const iterator& it) const {
    return cur_ != it.cur_ || ht_ != it.ht_ || ht_sz_ != it.ht_sz_;
  }

  iterator& operator++() {
    const Node* old = cur_;
    cur_ = cur_->p_next;
    if (!cur_) {
      size_type bucket_num = hash_func_(extract_key_(*old->p_value)) % ht_sz_;
      while (!cur_ && ++bucket_num < ht_sz_) {
        cur_ = ht_[bucket_num];
      }
    }
    return *this;
  }

  iterator operator++(int) {
    iterator tmp = *this;
    ++ *this;
    return tmp;
  }
};

template <typename Key, typename Val, typename Alloc, typename ExtractKey,
         typename Equal, typename Hash>
class DelayDeleteHashtable {
 public:
  typedef Key key_type;
  typedef Val value_type;
  typedef Alloc allocator_type;
  typedef Equal key_equal;
  typedef std::function<int(const Val&, const Val&)> value_cmp;
  typedef std::function<bool(const Val&)> value_equal;
  
  typedef typename Alloc::pointer pointer;
  typedef typename Alloc::const_pointer const_pointer;
  typedef typename Alloc::reference reference;
  typedef typename Alloc::const_reference const_reference;
  typedef std::size_t size_type;
  typedef std::ptrdiff_t difference_type;

  typedef DelayDeleteHashtableIterator<Key, Val, Alloc, ExtractKey,
          Equal, Hash> iterator;
  typedef DelayDeleteHashtableIterator<Key, Val, Alloc, ExtractKey,
          Equal, Hash> const_iterator;

  typedef HashTableNode<Val> Node;
  typedef Alloc value_allocator;
  typedef DelayDeleteAllocator<Node> node_allocator;
  
  DelayDeleteHashtable() {}
  DelayDeleteHashtable(const DelayDeleteHashtable& other) {
    make_copy(other);
  }
  DelayDeleteHashtable(const DelayDeleteHashtable&& other) {
    make_move(other);
  }
  DelayDeleteHashtable& operator = (const DelayDeleteHashtable& other) {
    clear();
    make_copy(other);
    return *this;
  }
  DelayDeleteHashtable& operator = (const DelayDeleteHashtable&& other) {
    clear();
    make_move(other);
    return *this;
  }
  void make_copy(const DelayDeleteHashtable& other) {
    max_load_factor_ = other.max_load_factor_;
    n_item_ = other.n_item_;
    init(n_item_);
    size_type other_current = other.current_;
    deep_cp_bucket(other.bucket_[other_current], other.nbucket_[other_current], 
        bucket_[current_], nbucket_[current_]);
  }
  void make_move(const DelayDeleteHashtable&& other) {
    max_load_factor_ = other.max_load_factor_;
    n_item_ = other.n_item_;
    nbucket_[0] = other.nbucket_[0];
    nbucket_[1] = other.nbucket_[1];
    bucket_[0] = other.bucket_[0];
    bucket_[1] = other.bucket_[1];
    current_ = other.current_;
    resize_count_ = other.resize_count_;
    other.n_item_ = 0;
    other.bucket_[0] = nullptr; 
    other.bucket_[1] = nullptr; 
    other.nbucket_[0] = 0;
    other.nbucket_[1] = 0;
    other.resize_count_ = 0;
  }

  ~DelayDeleteHashtable() {
    clear();
  }

  int init(size_t n) {
    size_t nbucket = find_near_prime(n);  
    Node** bkt = new_bucket(nbucket);
    if (!bkt) {
      std::cerr << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__<< "init no memory " << nbucket << std::endl;
      return -1;
    }
    bucket_[current_] = bkt;
    nbucket_[current_] = nbucket;
    return 0;
  }
  size_type resize_count() const {
    return resize_count_;
  }
  size_type size() const {
    return n_item_;
  }
  bool empty() const {
    return n_item_ ? true : false;
  }
  size_type bucket_size(size_type n) const {
    return bucket_size(n, bucket_[current_], nbucket_[current_]);
  }

  size_type bucket_size(size_type n, Node** bkt, size_type bkt_sz) const { 
    size_type sz = 0;
    if (n >= bkt_sz) {
      return sz;
    }
    Node* cur = bkt[n];
    while (cur) {
      ++sz;
      cur = cur->p_next;
    }
    return sz;
  }
  size_type bucket_count() {
    return nbucket_[current_];
  }
  size_type bucket_num(const value_type& obj) {
    return bucket_num(obj, nbucket_[current_]);
  }
  size_type bucket_num(const value_type& obj, size_type n) {
    if (0 == n) {
      return 0;
    }
    return hash_func_(extract_key_(obj)) % n;
  }

  Node* new_node(const value_type& obj) {
    Node* n = node_alloc_.allocate(1); 
    pointer v = value_alloc_.allocate(1);
    value_alloc_.construct(v, obj);
    n->p_next = nullptr;
    n->p_value = v;
    return n;
  }
  Node* new_node(value_type* obj) {
    Node* n = node_alloc_.allocate(1);
    n->p_next = nullptr;
    n->p_value = obj;
  }

  void delete_node(Node* n, bool with_value = false) {
    if (with_value) {
      value_alloc_.destroy(n->p_value);
    }
    node_alloc_.destroy(n);
  }

  void garbage_collect() {
    delete_bucket(bucket_[1 - current_], nbucket_[1 - current_]);
    bucket_[1 - current_] = nullptr;
    nbucket_[1 - current_] = 0;
    value_alloc_.garbage_collect();
    node_alloc_.garbage_collect();
  }

  void resize() {
    if (nbucket_[current_] && n_item_ / (double)nbucket_[current_] <= max_load_factor_) {
      return;
    }
    ++resize_count_;
    //获取下个大素数，并且切换buffer，重新hash。
    size_t nbucket = find_near_prime(nbucket_[current_] + 1);
    Node** bkt = new_bucket(nbucket);
    if (!bkt) {
      //无内存空间
      std::cerr << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__<< "delay_delete_table resize no memory" << std::endl;
      return;
    }
    //将current内元素拷入新桶内
    cp_bucket(bucket_[current_], nbucket_[current_], bkt, nbucket);
    if (bucket_[1 - current_]) {
      delete_bucket(bucket_[1 - current_], nbucket_[1 - current_]);
      bucket_[1 - current_] = nullptr;
      nbucket_[1 - current_] = 0;
    }
    bucket_[1 - current_] = bkt;
    nbucket_[1 - current_] = nbucket;
    //切换current
    current_ = 1 - current_; 
    return;
  }

  std::pair<iterator, bool> insert_unique(const value_type& obj, Node** bkt, size_type sz, bool is_replace) {
    size_type bkt_num = bucket_num(obj, sz);
    Node* bkt_first =  bkt[bkt_num];
    for (Node* pre = nullptr, * cur = bkt_first; cur; pre = cur, cur = cur->p_next) {
      if (equals_(extract_key_(obj), extract_key_(*cur->p_value))) {
        if (is_replace) {
          Node* tmp = new_node(obj);
          tmp->p_next = cur->p_next;
          if (pre) {
            pre->p_next = tmp;
          } else {
            bkt[bkt_num] = tmp;
          }
          delete_node(cur, true);
          return std::pair<iterator, bool> (iterator(tmp, bkt, sz), true);
        } else {
          return std::pair<iterator, bool> (iterator(cur, bkt, sz), false);
        }
      }
    }
    //创建新节点,插入头部
    Node* tmp = new_node(obj);
    tmp->p_next = bkt_first;
    bkt[bkt_num] = tmp;
    ++n_item_;
    return std::pair<iterator, bool> (iterator(tmp, bkt, sz), true);
  }

  std::pair<iterator, bool> insert_unique(const value_type& obj, bool is_resize = true, bool is_replace = false) {
    if (is_resize) {
      resize(); 
    }
    return insert_unique(obj, bucket_[current_], nbucket_[current_], is_replace);
  }

  iterator insert_equal(const value_type& obj, Node** bkt, size_type sz) {
    size_type bkt_num = bucket_num(obj, sz);
    Node* bkt_first = bkt[bkt_num];
    for (Node *pre = nullptr, *cur = bkt_first; cur; pre = cur, cur = cur->p_next) {
      if (equals_(extract_key_(obj), extract_key_(*cur->p_value))) {
        Node* tmp = new_node(obj);
        tmp->p_next = cur;
        if (pre) {
          pre->p_next = tmp;
        } else {
          bkt[bkt_num] = tmp;
        }
        ++n_item_;
        return iterator(tmp, bkt, sz);
      }
    }
    //没有找到相等节点，插入链表头部
    Node* tmp = new_node(obj);
    tmp->p_next = bkt_first;
    bkt[bkt_num] = tmp;
    ++n_item_;
    return iterator(tmp, bkt, sz);
  }

  iterator insert_equal(const value_type& obj, bool is_resize = true) {
    if (is_resize) {
      resize();
    }
    return insert_equal(obj, bucket_[current_], nbucket_[current_]);
  }

  //当cmp == 0  时不插入，
  // 查找 < obj < 位置,进行插入
  iterator insert_equal_with_value_cmp(const value_type& obj,
                                       value_cmp cmp,
                                       Node** bkt,
                                       size_type sz,
                                       bool is_replace) {
    //插入equal头部
    size_type bkt_num = bucket_num(obj, sz);
    Node* bkt_first = bkt[bkt_num];
    for (Node *pre = nullptr, *cur = bkt_first; cur; pre = cur, cur = cur->p_next) {
      if (equals_(extract_key_(obj), extract_key_(*cur->p_value))) {
        for (; cur && equals_(extract_key_(obj), extract_key_(*cur->p_value));  pre = cur, cur = cur->p_next) {
          int cmp_res = cmp(obj, *cur->p_value);
          if (0 == cmp_res) {
            if (is_replace) {
              Node* tmp = new_node(obj);
              tmp->p_next = cur->p_next;
              if (pre) {
                pre->p_next = tmp;
              } else {
                bkt[bkt_num] = tmp;
              }
              delete_node(cur, true);
              return iterator(tmp, bkt, sz);
            } else {
              return end();
            }
          } else if (cmp_res < 0) {
            Node* tmp = new_node(obj);
            tmp->p_next = cur;
            if (pre) {
              pre->p_next = tmp;
            } else {
              bkt[bkt_num] = tmp;
            }
            ++n_item_;
            return iterator(tmp, bkt, sz);
          }
        }
        //没有找到< 或者 = ,一定 > 插入尾部
        Node* tmp = new_node(obj);
        ++n_item_;
        tmp->p_next = pre->p_next;
        pre->p_next = tmp;
        return iterator(tmp, bkt, sz);
      }
    }
    //没有找到相等节点，插入链表头部
    Node* tmp = new_node(obj);
    tmp->p_next = bkt_first;
    bkt[bkt_num] = tmp;
    ++n_item_;
    return iterator(tmp, bkt, sz);
  }

  iterator insert_equal_with_value_cmp(const value_type& obj,
                                       value_cmp cmp,
                                       bool is_resize = true,
                                       bool is_replace = false) {
    if (is_resize) {
      resize();
    }
    return insert_equal_with_value_cmp(obj,
                                       cmp,
                                       bucket_[current_],
                                       nbucket_[current_],
                                       is_replace);
  }

  void clear() {
    int current = current_;
    delete_bucket(bucket_[current], nbucket_[current], true);
    bucket_[current] = nullptr;
    nbucket_[current] = 0;
    delete_bucket(bucket_[1 - current], nbucket_[1 - current]);
    bucket_[1 - current] = nullptr;
    nbucket_[1 - current] = 0;
    garbage_collect();
    n_item_ = 0;
  }


  iterator find(const key_type& key, Node** bkt, size_type sz) {
    size_type bkt_num = hash_func_(key)% sz;
    Node* cur = bkt[bkt_num];
    while (cur) {
      if (equals_(key, extract_key_(*cur->p_value))) {
        return iterator(cur, bkt, sz);
      }
      cur = cur->p_next;
    }
    return end();
  }

  iterator find(const key_type& key) {
    int current = current_;
    return find(key, bucket_[current], nbucket_[current]); 
  }
  std::pair<iterator, iterator> equal_range(const key_type& key,
                                            Node** bkt, size_type sz) {
    size_type bkt_num = hash_func_(key)% sz; 
    Node* cur = bkt[bkt_num];
    Node* p_first = nullptr;
    Node* p_end = nullptr;
    while (cur) {
      if (equals_(key, extract_key_(*cur->p_value))) {
        p_first = cur;
        while (cur) {
          if (!equals_(key, extract_key_(*cur->p_value))) {
            p_end = cur;
            break;
          }
          cur = cur->p_next;
        }
        break;
      }
      cur = cur->p_next;
    }
    return std::pair<iterator, iterator> (iterator(p_first, bkt, sz), iterator(p_end, bkt, sz));
  }

  std::pair<iterator, iterator> equal_range(const key_type& key) {
    int current = current_;
    return equal_range(key, bucket_[current], nbucket_[current]);
  }
  
  size_type count(const key_type& key, Node** bkt, size_type sz) {
    size_type bkt_num = hash_func_(key)% sz; 
    Node* cur = bkt[bkt_num];
    size_type cnt = 0;
    while(cur) {
      if (equals_(key, extract_key_(*cur->p_value))) {
        ++cnt;
      }
      cur = cur->p_next;
    }
    return cnt;
  }
  size_type count(const key_type& key) {
    int current = current_;
    return count(key, bucket_[current], nbucket_[current]);
  }

  void erase(const key_type& key, Node** bkt, size_type sz) {
    size_type bkt_num = hash_func_(key)% sz; 
    Node* bkt_first =  bkt[bkt_num];
    for (Node* pre = nullptr, * cur = bkt_first; cur; pre = cur, cur = cur->p_next) {
      if (equals_(key, extract_key_(*cur->p_value))) {
        if (pre) {
          pre->p_next = cur->p_next;
        } else {
          bkt[bkt_num] = cur->p_next;
        }
        delete_node(cur, true);
         --n_item_;
        return;
      }
    }
    return;
  }

  void erase(const key_type& key) {
    return erase(key, bucket_[current_], nbucket_[current_]);
  }

  void erase(const_iterator first, const_iterator last, Node** bkt, size_type sz) {
    size_type bkt_num = bucket_num(*first, sz);
    size_type last_bkt_num = last == end() ? sz : bucket_num(*last, sz);
    if (bkt_num > last_bkt_num) {
      std::cerr << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ <<" last before first" << std::endl;
      return;
    }
    Node* first_node = first.cur;
    Node* last_node = last.cur;
    Node* cur = bkt[bkt_num];
    Node* pre_cur = nullptr;
    Node* pre_first = nullptr;
    Node* p_first = nullptr;
    for( ; cur && cur != first_node; pre_cur = cur, cur = cur->p_next);

    if (!cur) {
      std::cerr << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ <<"no find iterator in erase" << std::endl;
      return;
    }
    pre_first = pre_cur;
    p_first = cur;

    if (bkt_num == last_bkt_num) {
      if (pre_first) {
        pre_first->p_next = last_node;
      } else {
        bkt[bkt_num] = last_node;
      }
      n_item_ -= delete_chain(p_first, last_node, true);
      return;
    }

    if (pre_first) {
      pre_first->p_next = nullptr;
    } else {
      bkt[bkt_num] = nullptr;
    }
    n_item_ -= delete_chain(p_first, nullptr, true);

    for (size_type i = bkt_num + 1; i < last_bkt_num; ++i) {
      Node* tmp = bkt[i];
      bkt[i] = nullptr;
      n_item_ -= delete_chain(tmp, nullptr, true);
    }
    if (last_bkt_num == sz) {
      return;
    }
    cur = bkt[last_bkt_num];
    if (cur == last_node) {
      return;
    }
    bkt[last_bkt_num] = last_node;
    n_item_ -= delete_chain(cur, last_node, true);
    return; 
  }

  void erase(const_iterator first, const_iterator last) {
    return erase(first, last, bucket_[current_], nbucket_[current_]);
  }
  void erase(const_iterator position) {
    const_iterator end = position;
    ++end;
    return erase(position, ++end, bucket_[current_], nbucket_[current_]);
  }

  void erase(const key_type& key, value_equal value_equal_fun, Node** bkt, size_type sz) {
    size_type bkt_num = hash_func_(key)% sz; 
    Node* bkt_first = bkt[bkt_num];
    for (Node* pre = nullptr, * cur = bkt_first; cur; pre = cur, cur = cur->p_next) {
      if (equals_(key, extract_key_(*cur->p_value)) && 
          value_equal_fun(*cur->p_value)) {
        if (pre) {
          pre->p_next = cur->p_next;
        } else {
          bkt[bkt_num] = cur->p_next;
        }
        delete_node(cur, true);
        --n_item_;
        return;
      }
    }
    return;
  }

  void erase(const key_type& key, value_equal value_equal_fun) {
    return erase(key, value_equal_fun, bucket_[current_], nbucket_[current_]);
  }
  
  iterator begin() { 
    int current = current_;
    Node** bkt = bucket_[current];
    size_type bkt_sz = nbucket_[current];
    for (size_type idx = 0; idx < bkt_sz; ++idx) {
      if (bkt[idx]) {
        return iterator(bkt[idx], bkt, bkt_sz);
      }
    } 
    return  iterator(nullptr, bkt, bkt_sz);
  }
  iterator end() { 
    int current = current_;
    return iterator(nullptr, bucket_[current], nbucket_[current]);
  }

  const_iterator begin() const {
    int current = current_;
    Node** bkt = bucket_[current];
    size_type bkt_sz = nbucket_[current];
    for (size_type idx = 0; idx < bkt_sz; ++idx) {
      if (bkt[idx]) {
        return iterator(bkt[idx], bkt, bkt_sz);
      }
    } 
    return  iterator(nullptr, bkt, bkt_sz);
  }

  const_iterator end() const { 
    int current = current_;
    return iterator(nullptr, bucket_[current], nbucket_[current]);
  }
 private:
  inline void insert_value_to_bucket(value_type* p_node, Node** bkt, size_type bkt_sz) {
    //cp_bucket时使用,尾部插入。保证原来单链表顺序
    Node* tmp = new_node(p_node);
    size_type bkt_num = bucket_num(*p_node, bkt_sz);
    Node* cur = bkt[bkt_num];
    if (nullptr == cur) {
      bkt[bkt_num] = tmp;
      return;
    }
    while (cur->p_next) {
      cur = cur->p_next;
    }
    cur->p_next = tmp;
    return;
  }

  inline void cp_bucket(Node** sbkt, size_type sbkt_sz, Node** dbkt, size_type dbkt_sz) {
    //在rehash时使用，将一个同中节点，复制到另外一个桶内
    if (nullptr == sbkt || nullptr == dbkt) {
      return;
    }
    for (size_type i = 0; i < sbkt_sz; ++i) {
      Node* cur = sbkt[i];
      while (cur) {
        insert_value_to_bucket(cur->p_value, dbkt, dbkt_sz);
        cur = cur->p_next;
      }
    }
  }
  
  inline void deep_insert_value_to_bucket(value_type* p_node, Node** bkt, size_type bkt_sz) {
    Node* tmp = new_node(*p_node);
    size_type bkt_num = bucket_num(*p_node, bkt_sz);
    Node* cur = bkt[bkt_num];
    if (nullptr == cur) {
      bkt[bkt_num] = tmp;
      return;
    }
    while (cur->p_next) {
      cur = cur->p_next;
    }
    cur->p_next = tmp;
    return;
  }
  
  inline void deep_cp_bucket(Node** sbkt, size_type sbkt_sz, Node** dbkt, size_type dbkt_sz) {
    //在rehash时使用，将一个同中节点，复制到另外一个桶内
    if (nullptr == sbkt || nullptr == dbkt) {
      return;
    }
    for (size_type i = 0; i < sbkt_sz; ++i) {
      Node* cur = sbkt[i];
      while (cur) {
        deep_insert_value_to_bucket(cur->p_value, dbkt, dbkt_sz);
        cur = cur->p_next;
      }
    }
  }

  int delete_chain(Node* p_begin, Node* p_end, bool with_value = false) {
    int cnt = 0;
    while (p_begin != p_end) {
      Node* p_node = p_begin;
      p_begin = p_begin->p_next;
      ++cnt;
      delete_node(p_node, with_value);
    }
    return cnt;
  }

  void delete_bucket(Node** bkt, size_t sz, bool with_value = false) {
    if (nullptr == bkt) {
      return;
    }
    //删除开链中的每个元素
    for (int i = 0; i < sz; ++i) {
      if (nullptr != bkt[i]) {
        delete_chain(bkt[i], nullptr, with_value);
      }
    }
    delete [] bkt;
  }

  Node** new_bucket(size_t sz) {
    Node** bkt = new (std::nothrow) Node* [sz];
    if (!bkt) {
      std::cerr << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__<< "new_bucket no memory " << sz << std::endl;
      return nullptr;
    }
    memset(bkt, 0, sizeof(Node*) * sz);
    return bkt;
  }

  value_allocator value_alloc_;
  node_allocator node_alloc_;
  key_equal equals_;
  ExtractKey extract_key_; 
  Hash hash_func_;
  double max_load_factor_ {1.0};  // max  n_item / nbucket_;
  size_t n_item_ {0};    //元素数量
  size_t nbucket_[2] {0, 0};   //桶的数量
  Node** bucket_[2] {nullptr, nullptr};
  int current_ {0};
  int resize_count_{0};
};

}


#endif

