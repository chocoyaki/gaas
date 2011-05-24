/****************************************************************************/
/* Remove a diet component                                                  */
/* Function code for dynamically killing an element and (potentially) its   */
/* underlying hierachy.                                                     */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Benjamin DEPARDON (Benjamin.Depardon@ens-lyon.fr)                   */
/*    - Kevin COULOMB (kevin.coulomb@-sysfera.fr)                           */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.2  2010/01/14 11:06:37  bdepardo
 * Compiles with gcc 4.4
 *
 * Revision 1.1  2009/10/26 09:23:51  bdepardo
 * Examples to dynamically manage the hierarchy.
 *
 *****************************************************************************/

#include <iostream>
#include <fstream>
#include <cstdlib>

#include <omniORB4/CORBA.h>

#include "SeD.hh"
#include "LocalAgent.hh"
#include "ORBMgr.hh"

#include "admin.hh"


int
removeFromHierarchy(DynamicType type, char *name, int recursive) {
  int argc = 1;
  char **argv = NULL;
  try {
    //------------------------------------------------------------------------
    // Initialize ORB object.
    //------------------------------------------------------------------------
    CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);
                                                                                
    //------------------------------------------------------------------------
    // Bind ORB object to name service object.
    // (Reference to Name service root context.)
    //------------------------------------------------------------------------
    CORBA::Object_var obj = orb->resolve_initial_references("NameService");
    if (CORBA::is_nil(obj.in())) {
      return DIET_NAMING;
    }

    //------------------------------------------------------------------------
    // Narrow this to the naming context (Narrowed reference to root context.)
    //------------------------------------------------------------------------
    CosNaming::NamingContext_var nc =
      CosNaming::NamingContext::_narrow(obj.in());
    if (CORBA::is_nil(nc.in())) {
      return DIET_NARROW;
    }
                                                                                
    //------------------------------------------------------------------------
    // The "name text" put forth by CORBA server in name service.
    // This same name ("DataServiceName1") is used by the CORBA server when
    // binding to the name server (CosNaming::Name).
    //------------------------------------------------------------------------
    CosNaming::Name _corbaCosName;
    _corbaCosName.length(2);
    _corbaCosName[0].kind=(const char*)"";
    _corbaCosName[1].id=CORBA::string_dup(name);
    _corbaCosName[1].kind=(const char*)"";

    switch (type) {
    case SED :
      _corbaCosName[0].id="dietSeD";
      break;
    case AGENT :
    default :
      _corbaCosName[0].id="dietAgent";
      break;
    }

    //------------------------------------------------------------------------
    // Resolve "name text" identifier to an object reference.
    //------------------------------------------------------------------------
    CORBA::Object_var obj1;
    try {
      obj1 = nc->resolve(_corbaCosName);
    } catch(CosNaming::NamingContext::NotFound& ex) {
      return DIET_NAMING;
    } catch (CORBA::COMM_FAILURE& ex) {
      return DIET_COMM_FAILURE;
    } catch(omniORB::fatalException& ex) {
      return DIET_OMNIFATAL;
    } catch (...) {
      return DIET_UNKNOWN;
    }

    // Depending on the type to remove, calling the remove call
    if (type==SED) { // No switch because cannot create objet in a switch
      SeD_ptr sed = SeD::_narrow(obj1.in());
      // If error
      if (sed->removeElement()) {
	return DIET_SED;
      }
    }
    if (type== AGENT) {
      Agent_ptr agent = Agent::_narrow(obj1.in());
      // If error
      if (agent->removeElement(recursive)) {
	return DIET_AGENT;
      }
    }
  }
  catch(CORBA::COMM_FAILURE& ex) {
    return DIET_COMM_FAILURE;
  }
  catch(CORBA::SystemException& ) {
    return DIET_SYSTEM;
  }
  catch(CORBA::Exception& ) {
    return DIET_SYSTEM;
  }  catch(omniORB::fatalException& fe) {
    return DIET_OMNIFATAL;
  }
  catch(...) {
    return DIET_UNKNOWN;
  }
  return DIET_SUCCESS;
}
