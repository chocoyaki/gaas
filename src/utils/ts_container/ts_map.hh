#ifndef _TS_MAP_HH_
#define _TS_MAP_HH_

#include <map>
#include <omnithread.h>

/**
 * This is a thread safe version of the STL map. All the methods are
 * thread safe. All the method will be added when needed. Look at the
 * STL documentation for more information.
 *
 * a map is a sequence of (key,value) pairs that provides for fast
 * retrieval based on the key. At most one value is held for each key;
 * in other words, each key in a map is unique. A map porvides
 * bidirectional iterators.
 *
 * The map requires that a less-than operation exist for its key types
 * and keeps it elements sorted so that iteration over a map occurs in
 * order.
 *
 * @author Sylvain DAHAN : LIFC Besancon (France)
 */


template <class Key, class T, class CMP = less<Key>, class A = allocator<T> >
class ts_map : private map<Key, T, CMP, A> {

private :

  /**
   * This is the mutex that lock the access to the map to avoid that
   * to thread access to the map in the same time.
   */
  mutable omni_mutex locker ;

  /**
   * A type to avoid to type map<Key, T, CMP, A> each time.
   */
  typedef map<Key, T, CMP, A> MapType ;

public :

  /***<direct access>*********************************************************/

  /**
   * The subscript operator performs a lookup on the key given as an
   * index and returns the corresponding value. If the key isn't
   * found, an element with the key and the default value of the
   * mapped type is inserted into the map.
   */
  inline T& operator[] (const Key& k) {
    locker.lock() ;
    T& result = MapType::operator[](k) ;
    locker.unlock() ;
    return result ;
  }

  /**
   * return the size (number of elements) of the map.
   */
  inline size_type size() const {
    locker.lock() ;
    size_type result = MapType::size() ;
    locker.unlock() ;
    return result ;
  }

  /**
   * remove the element with the key \c k from the map.
   *
   * @return 1 if the element \c k was found and erased, 0 if not.
   */
  inline size_type erase(const Key& k) {
    locker.lock() ;
    size_type result = MapType::erase(k) ;
    locker.unlock() ;
    return result ;
  }

  /**
   * erase all the elements of the map. It can be a good idea to make
   * a \c clear which are thread safe before the call to the
   * destructor of the object.
   */
  inline void clear() {
    locker.lock() ;
    MapType::clear() ;
    locker.unlock() ;
  }
  
  /***<iterator>**************************************************************/

  /* NO ITERATOR FOR THE MOMENT */

  /**
   * The iterator for the ts_map type. It is the same that the
   * map::iterator but it free the lock on the map when it's
   * destroyed.
   */
  /*  class iterator : public MapType::iterator {
  private :
    omni_mutex* mapLocker ;
    int instanceNumber ;
    void setLocker(omni_mutex* locker) {
      mapLocker = locker ;
      instanceNumber = 1 ;
    }
  public :
    inline iterator(const iterator& i) {
      super(i) ;
      instanceNumber++ ;
      mapLocker = i.mapLocker ;
    }
    inline ~iterator() {
      instanceNumber-- ;
      if(!instanceNumber) {
	mapLocker->unlock() ;
      }
    }
  } ;

  friend class MapType::iterator ;*/

  /**************************************************************************/

} ;

#endif // _TS_MAP_HH_
