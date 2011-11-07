/**
 * @file  NodeDescription.hh
 *
 * @brief  Description for an node of the DIET hierarchy
 *
 * @author  Sylvain DAHAN (Sylvain.Dahan@lifc.univ-fcomte.fr)
 *
 * @section Licence
 *   |LICENCE|
 */


#ifndef _NODEDESCRIPTION_HH_
#define _NODEDESCRIPTION_HH_

#include <cassert>
#include "debug.hh"
#include "ms_function.hh"



template<class T, class T_ptr> class NodeDescription {
public:
  /** return true if the node is define, false if not. */
  inline bool
  defined() const {
    return hostName;
  }

  /**
   * creates a new undefined NodeDescription.
   */
  NodeDescription() {
    copyMemory(T::_nil(), NULL);
  }

  /**
   * creates a new defined NodeDescription with the IOR \c ior and
   * the hostName \c hostName.
   *
   * @param ior the IOR of the node. the ior must be defined.
   * @param hostName the hostName of the node. It must be not \c NULL. A
   * copy of the hostName is made.
   */
  NodeDescription(T_ptr ior, const char *hostName) {
    assert(hostName != NULL);
    copyMemory(ior, hostName);
  }

  /**
   * Creates a new NodeDescription which is a clone of \c
   * nodeDescription.
   *
   * @param nodeDescription the NodeDescription which are
   * duplicated.
   */
  NodeDescription(const NodeDescription &nodeDescription) {
    copyMemory(nodeDescription.ior, nodeDescription.hostName);
  }

  /**
   * Destructor of the NodeDescription.
   */
  ~NodeDescription() {
    freeMemory();
  }

  /**
   * The NodeDescription become a copy of \c nodeDescription.
   *
   * @param nodeDescription a NodeDescription that must be copied.
   */
  NodeDescription<T, T_ptr> &
  operator = (const NodeDescription<T, T_ptr>&nodeDescription) {
    freeMemory();
    copyMemory(nodeDescription.ior, nodeDescription.hostName);
    return *this;
  }

  /**
   * Set fields if this NodeDescription has been built with default constructor.
   * @param ior      deep copied
   * @param hostName deep copied
   */
  void
  set(const T_ptr ior, const char *hostName) {
    freeMemory();
    copyMemory(ior, hostName);
  }


  /**
   * returns the IOR of the node. The NodeDescription must be defined.
   *
   * @return the IOR of the node.
   */
  T_ptr
  getIor() const {
    assert(defined());
    return this->ior;
  }

  /**
   * Returns the hostName of the node. The NodeDescription must be
   * defined. The returned string memory is owned by the
   * NodeDescrition. The string must not be changes and it is
   * destroyed at the same time as the NodeDescription.
   *
   * @return the hostName of the node.
   */
  const char *
  getHostName() const {
    assert(defined());
    return this->hostName;
  }

  /**
   * accessor to the ior.
   */
  inline T_ptr
  operator->() const {
    return ior;
  }
private:
  /** the IOR of the node */
  T_ptr ior;

  /**
   * the hostname of the node. It is \c NULL if the description is not
   * defined
   */
  char *hostName;

  /**
   * free the memory of the node description
   */
  inline void
  freeMemory() {
    if (defined()) {
      ms_strfree(hostName);
      hostName = NULL;
      CORBA::release(ior);
    }
  }

  /**
   * copy the argument into the attribut of the node
   */
  inline void
  copyMemory(T_ptr ior, const char *hostName) {
    if (hostName != NULL) {
      this->ior = T::_duplicate(ior);
      this->hostName = ms_strdup(hostName);
    } else {
      this->ior = T::_nil();
      this->hostName = NULL;
    }
  }
};

#endif  // _NODE_DESCRIPTION_HH_
