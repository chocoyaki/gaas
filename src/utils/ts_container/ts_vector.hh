// $Id$

#ifndef _TS_VECTOR_HH_
#define _TS_MAP_HH_

#include <vector>
#include <omnithread.h>
#include <assert.h>

/**
 * This is a thread safe version of the STL vector. All the methods are
 * thread safe. All the method will be added when needed. Look at the
 * STL documentation for more information.
 *
 * A vector is a Sequence that supports random access to elements,
 * constant time insertion and removal of elements at the end, and
 * linear time insertion and removal of elements at the beginning or
 * in the middle. The number of elements in a vector may vary
 * dynamically; memory management is automatic. Vector is the simplest
 * of the STL container classes, and in many cases the most efficient.
 *
 * @author Sylvain DAHAN : LIFC Besancon (France)
 */

/*
 * $Log$
 * Revision 1.2  2002/12/23 22:55:26  sdahan
 * removes the implicite stuff and define the default stuff. Now it compile
 * with gcc 3.1
 *
 * Revision 1.1  2002/12/20 14:46:57  sdahan
 * creation of a thread safe vector
 *
 */

template <class T, class A = std::allocator<T> >
class ts_vector : private std::vector<T, A> {

private :

  /**
   * This is the mutex that lock the access to the vector to avoid that
   * to thread access to the vector in the same time.
   */
  mutable omni_mutex locker ;

  /**
   * A type to avoid to type vector<T, A> each time.
   */
  typedef std::vector<T, A> VectorType ;

public :

  /**
   * the size_type type is the same as the vector::size_type
   */
  typedef typename VectorType::size_type size_type ;

public :

  /**
   * The subscript operator performs a lookup on the key given as an
   * index and returns the corresponding value. If the key isn't
   * found, an element with the key and the default value of the
   * mapped type is inserted into the map.
   *
   * @param k index of the element of type T. k must be lesser than
   * the size of the vector.
   */
  inline T& operator[] (size_type k) {
    assert(k < size()) ;
    locker.lock() ;
    T& result = VectorType::operator[](k) ;
    locker.unlock() ;
    return result ;
  }

  /**
   * Resize the vector to add or remove some elements at the end of
   * the vector.
   *
   * @param n new size of the vector.
   *
   * @param t is the value of the new created element.
   */
  inline void resize(size_type n, const T& t = T()) {
    locker.lock() ;
    VectorType::resize(n, t) ;
    locker.unlock() ;
  }

  /**
   * return the size (number of elements) of the vector.
   */
  inline size_type size() const {
    locker.lock() ;
    size_type result = VectorType::size() ;
    locker.unlock() ;
    return result ;
  }

  /**
   * erase all the elements of the vector. It can be a good idea to
   * make a \c clear which are thread safe before the call to the
   * destructor of the object.
   */
  inline void clear() {
    locker.lock() ;
    VectorType::clear() ;
    locker.unlock() ;
  }
  
} ;

#endif // _TS_VECTOR_HH_
