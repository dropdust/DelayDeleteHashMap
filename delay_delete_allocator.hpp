#ifndef _UTILS_DELAY_DELETE_ALLOCATOR_HPP_
#define _UTILS_DELAY_DELETE_ALLOCATOR_HPP_

#include <cstddef>
#include <deque>
#include <memory>

namespace utils {

//对象的add 和 del 在同一个线程操作  
//对象析构时，先将对象放入回收列表。然后定期调用garbage_collect函数进行对象析构。
//其中class U 同class T 
template <class T>
class DelayDeleteAllocator {
 public:
  typedef size_t size_type; 
  typedef ptrdiff_t difference_type;
  typedef T value_type;
  typedef T* pointer;
  typedef const T* const_pointer;
  typedef T& reference;
  typedef const T& const_reference;

  template <class U>
  struct rebind {
    typedef DelayDeleteAllocator<U> other;
  };

  DelayDeleteAllocator() {
  } 
  ~DelayDeleteAllocator() {
    garbage_collect();
  }
  pointer allocate(size_type n, const void* hint = 0) {
    if (n > this->max_size()) {
      return nullptr;
    }
    return static_cast<T*>(::operator new(n * sizeof(T))); 
  }
  void construct(pointer p, const T& value) {
    ::new((void*) p) T(value);
  }
  template <class U, typename... Args>
  void construct(U* p, Args&&... args) {
    ::new((void*) p) U(std::forward<Args>(args)...);
  }

  void deallocate(pointer p, size_type n) {
    //不进行删除，在garbage_collect中删除
    return;
  }

  void destroy(pointer p) {
    dirty_list_.push_back(p);
    return;
  }

  template <class U>
  void destroy(U* p) {
    dirty_list_.push_back(static_cast<T*>(p));
    return;
  }

  size_type max_size() const {
    return size_t(-1)/sizeof(T);
  };
  pointer address(reference x) {
    return std::addressof(x);
  }
  const_pointer const_address(const_reference x) {
    return std::addressof(x);
  }
  void garbage_collect() {
    while(!dirty_list_.empty()) {
      delete dirty_list_.front();
      dirty_list_.pop_front();
    }
  }

 private:
  DelayDeleteAllocator(const DelayDeleteAllocator&) = delete;
  DelayDeleteAllocator& operator = (const DelayDeleteAllocator&) = delete;
  std::deque<pointer> dirty_list_;     //未destroy对象列表
}; 

}

#endif
