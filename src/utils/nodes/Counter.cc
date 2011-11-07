/**
 * @file Counter.cc
 *
 * @brief  Thread safe counter implementation
 *
 * @author  Sylvain DAHAN (Sylvain.Dahan@lifc.univ-fcomte.fr)
 *
 * @section Licence
 *   |LICENCE|
 */


#include "Counter.hh"
#include <cassert>


Counter::Counter(const Counter &aCounter) {
  value = static_cast<CORBA::ULong>(aCounter);
}

Counter
Counter::operator++(int i) {
  valueMutex.lock();
  assert(value < value + 1);  // check for overflow
  Counter oldValue;
  oldValue.value = value;
  value++;
  valueMutex.unlock();
  return oldValue;
}

Counter
Counter::operator--(int i) {
  valueMutex.lock();
  assert(value > 0);
  Counter oldValue;
  oldValue.value = value;
  value--;
  valueMutex.unlock();
  return oldValue;
}

Counter &
Counter::operator=(const Counter &aCounter) {
  value = static_cast<CORBA::ULong>(aCounter);
  return *this;
}


Counter::operator CORBA::ULong() const {
  CORBA::ULong valueBuf;
  valueMutex.lock();
  valueBuf = value;
  valueMutex.unlock();
  return valueBuf;
}

Counter &
Counter::operator--() {
  assert(value > 0);
  valueMutex.lock();
  value--;
  valueMutex.unlock();
  return *this;
}

Counter &
Counter::operator++() {
  assert(value < value + 1);
  valueMutex.lock();
  value++;
  valueMutex.unlock();
  return *this;
}

Counter &
Counter::operator-=(const Counter &aCounter) {
  assert(value > 0);
  valueMutex.lock();
  value = value - static_cast<CORBA::ULong>(aCounter);
  valueMutex.unlock();
  return *this;
}

Counter &
Counter::operator+=(const Counter &aCounter) {
  assert(value < value + 1);
  valueMutex.lock();
  value = value + static_cast<CORBA::ULong>(aCounter);
  valueMutex.unlock();
  return *this;
}
