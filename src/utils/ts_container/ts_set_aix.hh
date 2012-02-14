#ifndef _TS_SET_AIX_HH_
#define _TS_SET_AIX_HH_

#include <algorithm>
#include <vector>
#include <boost/thread.hpp>

namespace {
  template <class V, class C>
  class ts_set_cmp : public C {
    typedef typename C::first_argument_type value_type;
  public:
    ts_set_cmp() {}
    
    ts_set_cmp(const C& src) : C(src) {}

    bool
    operator()(const value_type& lhs,
	       const value_type& rhs) const {
      return C::operator()(lhs, rhs);
    }
  };

} /* namespace */ 

template <class V, class C = std::less<V>,
	  class A = std::allocator<V> >
class ts_set : private std::vector<V, A>, 
		private ts_set_cmp<V, C> {
  typedef std::vector<V, A> Base;
  typedef ts_set_cmp<V, C> Compare;
  typedef boost::unique_lock<boost::mutex> Lock;
public:
  typedef typename Base::value_type value_type;
  typedef typename Base::value_type key_type;
  typedef Compare key_compare;
  typedef Compare value_compare;
  typedef A allocator_type;

  typedef typename A::pointer pointer;
  typedef typename A::const_pointer const_pointer;
  typedef typename A::reference reference;
  typedef typename A::const_reference const_reference;

  typedef typename Base::iterator iterator;
  typedef typename Base::const_iterator const_iterator;
  typedef typename Base::reverse_iterator reverse_iterator;
  typedef typename Base::const_reverse_iterator const_reverse_iterator;
  typedef typename Base::size_type size_type;
  typedef typename Base::difference_type difference_type;

  // constructor
  ts_set(const key_compare& cmp = key_compare(), const A& alloc = A())
    : Base(alloc), Compare(cmp) {
  }

  template <class InputIterator>
  ts_set(InputIterator first, InputIterator last,
	 const key_compare& cmp = key_compare(),
	 const A& alloc = A())
    : Base(first, last, alloc), Compare(cmp) {
    Compare& me = *this;
    std::sort(begin(), end(), me);
  }

  ts_set&
  operator=(const ts_set& rhs) {
    ts_set(rhs).swap(*this);
    return *this;
  }

  void
  swap(ts_set& rhs) {
    Base::swap(rhs);
    Compare& me(*this);
    Compare& other(rhs);
    std::swap(me, other);
  }
  
  // iterators
  iterator
  begin() {
    Lock lock(m_);
    return Base::begin();
  }

  const_iterator
  cbegin() const {
    Lock lock(m_);
    return Base::begin();
  }

  iterator
  end() {
    Lock lock(m_);
    return Base::end();
  }

  const_iterator
  cend() const {
    Lock lock(m_);
    return Base::end();
  }
  
  // reverse iterators
  reverse_iterator
  rbegin() {
    Lock lock(m_);
    return Base::rbegin();
  }

  const_reverse_iterator
  rbegin() const {
    Lock lock(m_);
    return Base::rbegin();
  }

  reverse_iterator
  rend() {
    Lock lock(m_);
    return Base::rend();
  }

  const_reverse_iterator
  rend() const {
    Lock lock(m_);
    return Base::rend();
  }

  // capacity
  bool
  empty()const {
    Lock lock(m_);
    return Base::empty();
  }

  size_type
  size() const {
    Lock lock(m_);
    return Base::size();
  }
  
  size_type
  max_size() const {
    Lock lock(m_);
    return Base::max_size();
  }

  // modifiers
  std::pair<iterator, bool>
  insert(const value_type& val) {
    Lock lock(m_);
    bool not_found(false);
    iterator it(lower_bound(val));

    if(it != end() || this->operator()(val, *it)) {
      it = Base::insert(it, val);
      not_found = true;
      Compare& me = *this;
      std::sort(begin(), end(), me);
    }
    
    return std::make_pair(it, not_found);
  }
  
  iterator
  insert(iterator pos, const value_type& val) {
    if((pos == begin() || this->operator()(*(pos - 1), val)) &&
       (pos == end() || this->operator()(val, *pos))) {
      return Base::insert(pos, val);
    }
    // FIXME: not sorted
    return insert(val).first;
  }

  template <class InputIterator>
  void
  insert(InputIterator first, InputIterator last) {
    Lock lock(m_);
    for(; first != last; ++first) {
      insert(*first);
      Compare& me = *this;
      std::sort(begin(), end(), me);
    }
  }

  void
  erase(iterator pos) {
    Lock lock(m_);
    Base::erase(pos);
  }
  
  size_type
  erase(const key_type& k) {
    Lock lock(m_);
    iterator it(find(k));
    if (it == end()) {
      return 0;
    }
    Base::erase(it);
    return 1;
  }

  void
  erase(iterator first, iterator last) {
    Lock lock(m_);
    Base::erase(first, last);
  }

  void
  clear() {
    Lock lock(m_);
    Base::clear();
  }

  // set operations
  iterator
  find(const key_type& k) {
    Lock lock(m_);
    iterator it(lower_bound(k));

    if(it != cend() && this->operator()(k, *it)) {
      it = end();
    }
    
    return it;
  }

  size_type
  count(const key_type& k) const {
    Lock lock(m_);
    return find(k) != cend();
  }
  
  iterator
  lower_bound(const key_type& k) {
    Lock lock(m_);
    Compare& me(*this);

    return std::lower_bound(begin(), end(), k, me);
  }

  iterator
  upper_bound(const key_type& k) const {
    Lock lock(m_);
    Compare& me(*this);

    return std::upper_bound(cbegin(), cend(), k, me);
  }

  std::pair<iterator, iterator>
  equal_range(const key_type& k) const {
    Lock lock(m_);
    Compare& me(*this);

    return std::equal_range(cbegin(), cend(), k, me);
  }

  // specific lock-related operations from DIET original ts_set class
  void
  lock() const {
    m_.lock();
  }
  
  void
  unlock() const {
    m_.unlock();
  }
  

private:
  mutable boost::mutex m_;
};

#endif /* _TS_SET_AIX_HH_ */
