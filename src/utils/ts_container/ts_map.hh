/**
* @file ts_map.hh
* 
* @brief  Thread safe STL map header 
* 
* @author  Sylvain DAHAN (Sylvain.Dahan@lifc.univ-fcomte.fr)
* 
* @section Licence
*   |LICENSE|                                                                
*/



#ifndef _TS_MAP_HH_
#define _TS_MAP_HH_

#include <cassert>
#include <functional>
#include <map>
#include <memory>
#include <omniconfig.h>
#include <omnithread.h>

/**
 * This is a thread safe version of the STL map. All the methods are
 * thread safe. All the method will be added when needed. Look at the
 * STL documentation for more information.
 *
 * a map is a sequence of (key, value) pairs that provides for fast
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

#define LIMIT_MAP_TEMPLATE

#ifdef LIMIT_MAP_TEMPLATE
template <class Key, class T, class CMP = std::less<Key> >
class ts_map : private std::map<Key, T, CMP> {
#else /* LIMIT_MAP_TEMPLATE */
  /*
  ** the following definition is specific to gcc-4.0, and it may
  ** not work in older versions of GCC!
  */
  template <class Key, class T, class CMP = std::less<Key>,
            class A = std::allocator<std::pair<const Key, T> > >
  class ts_map : private std::map<Key, T, CMP, A> {
#endif /* LIMIT_MAP_TEMPLATE */

  private:

#ifndef NDEBUG
    /**
     * used bye the assertion to check if the \c lock() methods is
     * called before the not thread safe methods are called.
     */
    mutable bool accessLocked;
#endif  // NDEBUG

    /**
     * This is the mutex that lock the access to the map to avoid that
     * to thread access to the map in the same time.
     */
    mutable omni_mutex locker;

    /**
     * A type to avoid to type map<Key, T, CMP, A> each time.
     */
#ifdef LIMIT_MAP_TEMPLATE
    typedef std::map<Key, T, CMP> MapType;
#else
    typedef std::map<Key, T, CMP, A> MapType;
#endif /* LIMIT_MAP_TEMPLATE */

  public:

    /**
     * the size_type type is the same as the map::size_type
     */
    typedef typename MapType::size_type size_type;

    /**
     * the iterator type
     */
    typedef typename MapType::iterator iterator;

    typedef typename MapType::value_type value_type;

  public:

    /***<direct access>*********************************************************/

    /**
     * The subscript operator performs a lookup on the key given as an
     * index and returns the corresponding value. If the key isn't
     * found, an element with the key and the default value of the
     * mapped type is inserted into the map.
     */
    inline T& operator[] (const Key& k) {
      locker.lock();
      T& result = MapType::operator[](k);
      locker.unlock();
      return result;
    }

    /**
     * return the size (number of elements) of the map.
     */
    inline size_type size() const {
      locker.lock();
      size_type result = MapType::size();
      locker.unlock();
      return result;
    }

    /**
     * remove the element with the key \c k from the map.
     *
     * @return 1 if the element \c k was found and erased, 0 if not.
     */
    inline size_type erase(const Key& k) {
      locker.lock();
      size_type result = MapType::erase(k);
      locker.unlock();
      return result;
    }

    /**
     * erase all the elements of the map. It can be a good idea to make
     * a \c clear which are thread safe before the call to the
     * destructor of the object.
     */
    inline void clear() {
      locker.lock();
      MapType::clear();
      locker.unlock();
    }

    /***<iterator>**************************************************************/

    /**
     * locks the access to the container
     */
    inline void lock() const {
      locker.lock();
#ifndef NDEBUG // only used by the assert
      accessLocked = true;
#endif  // NDEBUG
    }

    /**
     * unlocks the access to the container
     */
    inline void unlock() const {
      assert(accessLocked);
#ifndef NDEBUG //only used by the assert
      accessLocked = false;
#endif  // NDEBUG
      locker.unlock();
    }

    inline iterator find(const Key& x) {
      assert(accessLocked);
      return MapType::find(x);
    }

    inline void erase(iterator pos) {
      assert(accessLocked);
      MapType::erase(pos);
    }

    inline iterator insert(iterator pos, const value_type& x) {
      assert(accessLocked);
      return MapType::insert(pos, x);
    }

    /**
     * Returns an iterator pointing to the beginning of the map.
     */
    inline iterator begin() {
      assert(accessLocked);
      return MapType::begin();
    }

    /**
     * Returns an iterator pointing to the end of the map.
     */
    inline iterator end() {
      assert(accessLocked);
      return MapType::end();
    }

    /**************************************************************************/

  };

#endif  // _TS_MAP_HH_
