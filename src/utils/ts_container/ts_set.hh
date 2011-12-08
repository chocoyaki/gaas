/**
* @file ts_set.hh
* 
* @brief  Thread safe STL set header 
* 
* @author  Sylvain DAHAN (Sylvain.Dahan@lifc.univ-fcomte.fr)
* 
* @section Licence
*   |LICENCE|                                                                
*/


#ifndef _TS_SET_HH_
#define _TS_SET_HH_

#include <cassert>
#include <set>
#include <omniconfig.h>
#include <omnithread.h>

/**
 * This is a thread safe version of the STL set. Some methods are
 * thread safe, some other are not. The methods which are used with
 * the iterator are not thread safe. All the method will be added when
 * needed. Look at the STL documentation for more information.
 *
 * @author Sylvain DAHAN : LIFC Besancon (France)
 */


template <class Key, class CMP = std::less<Key>,
          class A = std::allocator<Key> >
class ts_set : private std::set<Key, CMP, A> {
private:
#ifndef NDEBUG
  /**
   * used bye the assertion to check if the \c lock() methods is
   * called before the not thread safe methods are called.
   */
  mutable bool accessLocked;
#endif  // NDEBUG

  /**
   * This is the mutex that lock the access to the set to avoid that
   * to thread access to the set in the same time.
   */
  mutable omni_mutex locker;

  /**
   * A type to avoid to type set<Key, T, CMP, A> each time.
   */
  typedef std::set<Key, CMP, A> SetType;

public:

  /**
   * the size_type type is the same as the set::size_type
   */
  typedef typename SetType::size_type size_type;

  /**
   * the iterator type
   */
  typedef typename SetType::iterator iterator;

public:

  /***<direct access>*********************************************************/

  /**
   * creates a new ts_set.
   */
#ifndef NDEBUG // only used by the assert
  ts_set() : accessLocked(false) {}
#else
  ts_set() {}
#endif

  /**
   * return the size (number of elements) of the set. (thread safe)
   */
  inline size_type size() const {
    locker.lock();
    size_type result = SetType::size();
    locker.unlock();
    return result;
  }

  /**
   * remove the element with the key \c k from the set. (thread safe)
   *
   * @return 1 if the element \c k was found and erased, 0 if not.
   */
  inline size_type erase(const Key& k) {
    locker.lock();
    size_type result = SetType::erase(k);
    locker.unlock();
    return result;
  }

  /**
   * erase all the elements of the set. It can be a good idea to make
   * a \c clear which are thread safe before the call to the
   * destructor of the object. (thread safe)
   */
  inline void clear() {
    locker.lock();
    SetType::clear();
    locker.unlock();
  }

  /**
   * Inserts x into the set. Inserts x into the set if and only if the
   * set does not already contain an element whose key is the same as
   * the key of x. The return value is a pair P. P.first is an
   * iterator pointing to the element whose key is the same as the key
   * of x. P.second is a bool: it is true if x was actually inserted
   * into the set, and false if x was not inserted into the set,
   * i.e. if the set already contained an element with the same key as
   * x. (thread safe)
   *
   * @param x the key added into the set
   */
  std::pair<iterator, bool> insert(const Key & x) {
    locker.lock();
    std::pair<iterator, bool> result = SetType::insert(x);
    locker.unlock();
    return result;
  }

  /***<iterator>**************************************************************/

  /**
   * locks the access to the container
   */
  inline void lock() const {
    locker.lock();
#ifndef NDEBUG // only used by the assert
    assert(!accessLocked);
    accessLocked = true;
#endif  // NDEBUG
  }

  /**
   * unlocks the access to the container
   */
  inline void unlock() const {
    locker.unlock();
#ifndef NDEBUG // only used by the assert
    assert(accessLocked);
    accessLocked = false;
#endif  // NDEBUG
  }

  /**
   * Returns an iterator pointing to the beginning of the set.
   */
  inline iterator begin() const {
#ifndef NDEBUG // only used by the assert
    assert(accessLocked);
#endif  // NDEBUG
    return begin();
  }

  /**
   * Returns an iterator pointing to the end of the set.
   */
  inline iterator end() const {
#ifndef NDEBUG // only used by the assert
    assert(accessLocked);
#endif  // NDEBUG
    return end();
  }

  /**
   * returns an iterator pointing onto the x elements.
   */
  inline iterator find(const Key & x) {
#ifndef NDEBUG // only used by the assert
    assert(accessLocked);
#endif  // NDEBUG
    return SetType::find(x);
  }

  /**
   * Insert a element x using posas a hint to where it will be
   * inserted. The set must be lock to use it.
   */
  inline iterator insert(iterator pos, const Key & x) {
#ifndef NDEBUG // only used by the assert
    assert(accessLocked);
#endif  // NDEBUG
    return SetType::insert(pos, x);
  }

  /**************************************************************************/

};

#endif  // _TS_SET_HH_
