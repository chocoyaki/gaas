/****************************************************************************/
/* Description for an node of the DIET hierarchy                            */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Sylvain DAHAN (Sylvain.Dahan@lifc.univ-fcomte.fr)                   */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.5  2004/06/11 15:45:39  ctedesch
 * add DIET/JXTA
 *
 * Revision 1.4  2003/09/22 21:08:07  pcombes
 * Still some problems with Coding Standards ...
 *
 * Revision 1.3  2003/05/22 12:20:25  sdahan
 * Now the NodeDescriptor completly manages its own memory by itself.
 * The -> operator is defined. You can do :
 * descriptor->ping();
 * instead of :
 * LocalAgent_ptr la = localAgent::_duplicate(descriptor.getIor());
 * la->ping();
 * CORBA::release(la);
 *
 * Revision 1.2  2003/05/05 14:32:29  pcombes
 * assert in NodeDescription(T_ptr ior, const char* hostName) was buggy.
 *
 * Revision 1.1  2003/04/10 12:58:35  pcombes
 * Replace AgentDescription.hh. Fix bugs on memory management.
 ****************************************************************************/

#ifndef _NODEDESCRIPTION_HH_
#define _NODEDESCRIPTION_HH_

#include "ms_function.hh"
#include <assert.h>
#include "debug.hh"

/**
 * The NodeDescription manages the hostname and the IOR of an
 * node. The node can be an Agent or a SeD. The T_ptr must be the
 * pointer of the node.  ex : NodeDescription<SeD_ptr>
 *
 * All the methods make a copy of the ior and hostname given in
 * argument. The memory is release when the object is destroyed or
 * when a new value of the ior and of the hostname is set.
 */

template<class T, class T_ptr> class NodeDescription {
private :
  /** the IOR of the node */
  T_ptr ior; 

  /**
   * the hostname of the node. It is \c NULL if the description is not
   * defined
   */
  char* hostName;

#if HAVE_JXTA
  /**
   * the uuid of JXTA SeD
   */
  char* uuid;
#endif // HAVE_JXTA

  /**
   * free the memory of the node description
   */
  inline void
  freeMemory() {
    if(defined()) {
      ms_strfree(hostName);
      hostName = NULL;
#if HAVE_JXTA
      ms_strfree(uuid);
      uuid = NULL;
#endif // HAVE_JXTA
      CORBA::release(ior);
    }
  }

  /**
   * copy the argument into the attribut of the node
   */
  inline void
  copyMemory(T_ptr ior, const char* hostName) {
    if (hostName != NULL) {
      this->ior = T::_duplicate(ior);
      this->hostName = ms_strdup(hostName);
    } else {
      this->ior = T::_nil();
      this->hostName = NULL;
    }
  }

#if HAVE_JXTA
/**
   * copy the argument into the attribut of the node
   */
  inline void
  copyMemory(T_ptr ior, const char* hostName, const char* uuid) {
    if (hostName != NULL) {
      this->ior = T::_duplicate(ior);
      this->hostName = ms_strdup(hostName);
      this->uuid = ms_strdup(uuid);
    } else {
      this->ior = T::_nil();
      this->hostName = NULL;
    }
  }
#endif // HAVE_JXTA

public :
  /** return true if the node is define, false if not. */
  inline bool
  defined() const { return hostName; }

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
  NodeDescription(T_ptr ior, const char* hostName) {
    assert(hostName != NULL);
    copyMemory(ior, hostName);
  }

#if HAVE_JXTA
/**
   * creates a new defined NodeDescription with the IOR \c ior, 
   * the hostName \c hostName and the uuid \c uuid.
   *
   * @param ior the IOR of the node. the ior must be defined.
   * @param hostName the hostName of the node. It must be not \c NULL. A
   * copy of the hostName is made.
   */
  NodeDescription(T_ptr ior, const char* hostName, const char* uuid) {
    assert(hostName != NULL);
    copyMemory(ior, hostName, uuid);
  }
#endif // HAVE_JXTA

  /**
   * Creates a new NodeDescription which is a clone of \c
   * nodeDescription.
   *
   * @param nodeDescription the NodeDescription which are
   * duplicated.
   */
  NodeDescription(const NodeDescription& nodeDescription) {
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
  NodeDescription<T, T_ptr>&
  operator=(const NodeDescription<T, T_ptr>& nodeDescription) {
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
  set(const T_ptr ior, const char* hostName) {
    freeMemory();
    copyMemory(ior, hostName);
  }
  
#if HAVE_JXTA
  /**
   * Set fields if this NodeDescription has been built with default constructor.
   * @param ior      deep copied
   * @param hostName deep copied
   * @param uuid     deep copied
   */
  void
  set(const T_ptr ior, const char* hostName, const char* uuid) {
    freeMemory() ;
    copyMemory(ior, hostName, uuid) ;
  }
#endif // HAVE_JXTA
 

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
  const char*
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
};

#endif // _NODE_DESCRIPTION_HH_
