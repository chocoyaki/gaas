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
 * Revision 1.2  2003/05/05 14:32:29  pcombes
 * assert in NodeDescription(T_ptr ior, const char* hostName) was buggy.
 *
 * Revision 1.1  2003/04/10 12:58:35  pcombes
 * Replace AgentDescription.hh. Fix bugs on memory management.
 ****************************************************************************/

#ifndef _NODE_DESCRIPTION_HH_
#define _NODE_DESCRIPTION_HH_

#include "ms_function.hh"
#include <assert.h>
#include "debug.hh"

/**
 * The NodeDescription manages the hostname and the IOR of an node. The node can
 * be an Agent or a SeD. The T_ptr must be the pointer of the node.
 * ex : NodeDescription<SeD_ptr>
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

public :
  /** return true if the node is define, false if not. */
  inline bool defined() const { return hostName;};

  /**
   * creates a new undefined NodeDescription.
   */
  NodeDescription() {
    this->hostName = NULL;
    this->ior = T::_nil();
  }

  /**
   * creates a new defined NodeDescription with the IOR \c ior and
   * the hostName \c hostName.
   *
   * @param ior the IOR of the node. the ior must be defined.
   *
   * @param hostName the hostName of the node. It must be not \c NULL. A
   * copy of the hostName is made.
   */
  NodeDescription(T_ptr ior, const char* hostName) {
    assert(hostName != NULL);
    this->hostName = ms_strdup(hostName);
    this->ior = T::_duplicate(ior);
  }

  /**
   * Creates a new NodeDescription which is a clone of \c
   * nodeDescription.
   *
   * @param nodeDescription the NodeDescription which are
   * duplicated.
   */
  NodeDescription(const NodeDescription& nodeDescription) {
    if(nodeDescription.defined()) {
      this->hostName = ms_strdup(nodeDescription.hostName);
      this->ior = T::_duplicate(nodeDescription.ior);
    } else {
      this->hostName = NULL;
      this->ior = T::_nil();
    }
  }

  /**
   * Destructor of the NodeDescription.
   */
  ~NodeDescription() {
    if (this->hostName)
      ms_strfree(this->hostName);
    if (! CORBA::is_nil(this->ior))
      CORBA::release(this->ior);
  }

  /**
   * The NodeDescription become a copy of \c nodeDescription.
   *
   * @param nodeDescription a NodeDescription that must be copied.
   */
  NodeDescription<T, T_ptr>& operator=
  (const NodeDescription<T, T_ptr>& nodeDescription)  {
    if (hostName)
      ms_strfree(hostName);
    if (! CORBA::is_nil(this->ior))
      CORBA::release(this->ior);
	
    if(nodeDescription.defined()) {
      this->ior = T::_duplicate(nodeDescription.ior);
      this->hostName = ms_strdup(nodeDescription.hostName);
    }
    return *this;
  }

  /**
   * Set fields if this NodeDescription has been built with default constructor.
   * @param ior      deep copied
   * @param hostName deep copied
   */
  void
  set(const T_ptr ior, const char* hostName) {
    if (! CORBA::is_nil(this->ior))
      CORBA::release(this->ior);
    if (this->hostName)
      ms_strfree(this->hostName);
    this->ior = T::_duplicate(ior);
    this->hostName = ms_strdup(hostName);
  }
   

  /**
   * returns the IOR of the node. The NodeDescription must be defined.
   *
   * @return the IOR of the node.
   */
  T_ptr getIor() const {
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
  const char* getHostName() const {
    assert(defined());
    return this->hostName;
  }
};

#endif // _NODE_DESCRIPTION_HH_
