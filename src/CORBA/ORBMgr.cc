/****************************************************************************/
/* DIET ORB Manager implementation                                          */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*    - Frederic LOMBARD (Frederic.Lombard@lifc.univ-fcomte.fr)             */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.7  2003/07/04 09:47:54  pcombes
 * Rm setTraceLevel and DIET_ct. Use new ERROR and WARNING macros.
 *
 * Revision 1.5  2003/06/02 08:53:16  cpera
 * Update api for asynchronize calls, manage bidir poa.
 *
 * Revision 1.4  2003/05/13 17:20:06  pcombes
 * Catch all CORBA exceptions in registers and requests to name server.
 *
 * Revision 1.3  2003/05/05 14:10:55  pcombes
 * Add destroy and stringToObject methods.
 *
 * Revision 1.2  2003/04/10 12:43:56  pcombes
 * Use the TRACE_LEVEL of the debug module. Uniformize return codes.
 *
 * Revision 1.1  2003/02/04 09:58:13  pcombes
 * Unify ORBs interface with a manager class: ORBMgr
 ****************************************************************************/

#include <iostream>
using namespace std;

#include "ORBMgr.hh"
#include "debug.hh"

/** The trace level. */
extern unsigned int TRACE_LEVEL;

/* Initialize private static members. */
CORBA::ORB_ptr ORBMgr::ORB = CORBA::ORB::_nil();
PortableServer::POA_var ORBMgr::POA = PortableServer::POA::_nil();
PortableServer::POA_var ORBMgr::POA_BIDIR = PortableServer::POA::_nil();

int
ORBMgr::init(int argc, char** argv, bool init_POA)
{
#ifdef __OMNIORB3__
  ORB = CORBA::ORB_init(argc, argv, "omniORB3");
  omniORB::scanGranularity(0);
#else
#ifdef __OMNIORB4__
  const char* options[][2]
    = {{"inConScanPeriod","0"},{"outConScanPeriod","0"},
       {"maxGIOPConnectionPerServer","50"},
       //{"giopMaxMsgSize","33554432"}, // 32MB
       {0,0}};
  ORB = CORBA::ORB_init(argc, argv, "omniORB4", options);
#else  // __OMNIORB4__
#error "No omniORB version defined !!!"
#endif // __OMNIORB4__
#endif // __OMNIORB3__

  if (CORBA::is_nil(ORB))
    return 1;
  
  if (init_POA) {
    CORBA::Object_var obj;
    PortableServer::POAManager_var pman;
    CORBA::PolicyList pl;
    CORBA::Any a;

    obj = ORB->resolve_initial_references("RootPOA");
    POA = PortableServer::POA::_narrow(obj);
    // Get the POAManager Ref and activate it
    pman = POA->the_POAManager();
    
    // Create a POA with the Bidirectional policy
    pl.length(1);
    a <<= BiDirPolicy::BOTH;
    pl[0] =
      ORBMgr::ORB->create_policy(BiDirPolicy::BIDIRECTIONAL_POLICY_TYPE, a);
    ORBMgr::POA_BIDIR = ORBMgr::POA->create_POA("bidir", pman, pl);

    pman->activate();
  }
  return 0;
}


void
ORBMgr::destroy()
{
  ORB->destroy();
}


int
ORBMgr::activate(PortableServer::ServantBase* obj)
{
  PortableServer::ObjectId_var objId;
  try {
    if (!CORBA::is_nil(ORBMgr::POA_BIDIR))
      objId = ORBMgr::POA_BIDIR->activate_object(obj);
    else 
      objId = ORBMgr::POA->activate_object(obj);
  } catch (...) {
    INTERNAL_ERROR("exception caught in ORBMgr::" << __FUNCTION__, -1);
  }
  return 0;
}

void
ORBMgr::wait()
{
  ORB->run();
  ORB->destroy();
}


CORBA::Object_ptr
ORBMgr::getAgentReference(const char* agentName)
{
  CosNaming::NamingContext_var rootContext;
  CORBA::Object_ptr obj;
  CosNaming::Name CosName;

  try {
    // Obtain a reference to the root context of the Name service:
    CORBA::Object_ptr initServ;
    initServ = ORB->resolve_initial_references("NameService");

    // Narrow the object returned by resolve_initial_references()
    // to a CosNaming::NamingContext object:
    rootContext = CosNaming::NamingContext::_narrow(initServ);
    if (CORBA::is_nil(rootContext)) {
      INTERNAL_ERROR("failed to narrow the root naming context", 1);
    }
  } catch(CORBA::ORB::InvalidName& ex) {
    INTERNAL_ERROR("service required is invalid [does not exist]", 1);
  } catch (CORBA::Exception& e) {
    CORBA::Any tmp;
    tmp <<= e;
    CORBA::TypeCode_var tc = tmp.type();
    ERROR("exception caught (" << tc->name() << ") while attempting to "
	  << "connect to the CORBA name server", CORBA::Object::_nil());
  }

  // Create a name object, containing the agentName and its context:

  CosName.length(2);

  CosName[0].id   = (const char*) "dietAgent"; // string copied
  CosName[0].kind = (const char*) "";          // string copied
  CosName[1].id   = (const char*) agentName;
  CosName[1].kind = (const char*) "";
  // Note on kind: The kind field is used to indicate the type
  // of the object. This is to avoid conventions such as that used
  // by files (CosName.type -- e.g. test.ps = postscript etc.)

  try {
    // Resolve the name to an object reference, and assign the reference 
    // returned to a CORBA::Object:
    obj = rootContext->resolve(CosName);
  } catch (CosNaming::NamingContext::NotFound& ex) {
    // This exception is thrown if any of the components of the
    // path [contexts or the object] aren't found:
    ERROR("context for " << agentName << " not found", CORBA::Object::_nil());
  } catch (CORBA::COMM_FAILURE& ex) {
    ERROR("system exception caught (COMM_FAILURE): unable to connect to "
	  << "the CORBA name server", CORBA::Object::_nil());
  } catch (omniORB::fatalException& ex) {
    throw;
  } catch (...) {
    INTERNAL_ERROR("system exception caught while using the naming service", 1);
  }

  return obj;
}

  
int
ORBMgr::bindAgentToName(CORBA::Object_ptr obj, const char* agentName)
{
  CosNaming::NamingContext_var rootContext, agtContext;
  CosNaming::Name cosName;
  
  try {
    // Obtain a reference to the root context of the Name service:
    CORBA::Object_var initServ;
    initServ = ORB->resolve_initial_references("NameService");

    // Narrow the object returned by resolve_initial_references()
    // to a CosNaming::NamingContext object:
    rootContext = CosNaming::NamingContext::_narrow(initServ);
    if (CORBA::is_nil(rootContext)) {
      INTERNAL_ERROR("failed to narrow the root naming context.\n", 1);
    }
  } catch (CORBA::ORB::InvalidName& ex) {
    INTERNAL_ERROR("service required is invalid [does not exist]", 1);
  } catch (CORBA::Exception& e) {
    CORBA::Any tmp;
    tmp <<= e;
    CORBA::TypeCode_var tc = tmp.type();
    ERROR("exception caught (" << tc->name() << ") while attempting to "
	  << "register as " << agentName << " to the CORBA name server",1);
  }

  cosName.length(1);
  
  /* Bind a context called "dietAgent" to the root context */
  
  cosName[0].id = (const char*) "dietAgent"; // string copied
  cosName[0].kind = (const char*) "";        // string copied    
  // Note on kind: The kind field is used to indicate the type
  // of the object. This is to avoid conventions such as that used
  // by files (name.type -- e.g. test.ps = postscript etc.)
  
  try {
    try {
      // Bind the context to root, and assign testContext to it:
      agtContext = rootContext->bind_new_context(cosName);
    } catch(CosNaming::NamingContext::AlreadyBound& ex) {
      // If the context already exists, this exception will be raised.
      // In this case, just resolve the name and assign testContext
      // to the object returned:
      CORBA::Object_var tmpobj;
      tmpobj = rootContext->resolve(cosName);
      agtContext = CosNaming::NamingContext::_narrow(tmpobj);
      if (CORBA::is_nil(agtContext)) {
        INTERNAL_ERROR("failed to narrow the agent naming context", 1);
      }
    } 

    /* Bind obj to the agent context */
    
    cosName[0].id = (const char*) agentName; // string copied
    cosName[0].kind = (const char*) "";      // string copied

    // Bind obj with name agentName to the agtContext:
    try {
      agtContext->bind(cosName,obj);
    } catch(CosNaming::NamingContext::AlreadyBound& ex) {
      agtContext->rebind(cosName,obj);
    }

    // Note: Using rebind() will overwrite any Object previously bound 
    //       to /dietAgent/agentName with obj.
    //       Alternatively, bind() can be used, which will raise a
    //       CosNaming::NamingContext::AlreadyBound exception if the name
    //       supplied is already bound to an object.

    // Amendment: When using OrbixNames, it is necessary to first try bind
    // and then rebind, as rebind on it's own will throw a NotFoundexception if
    // the Name has not already been bound. [This is incorrect behaviour -
    // it should just bind].

  } catch (CORBA::COMM_FAILURE& ex) {
    ERROR("system exception caught (COMM_FAILURE), unable to connect to "
	  << "the CORBA name server", 1);
  } catch (omniORB::fatalException& ex) {
    throw;
  } catch (...) {
    INTERNAL_ERROR("system exception caught while using the naming service", 1);
  }
  return 0;
}

char *
ORBMgr::getIORString(CORBA::Object_ptr obj)
{
  try {
    return ORB->object_to_string(obj);
  } catch (...) {}
  return NULL;  
}


CORBA::Object_ptr
ORBMgr::stringToObject(const char* IOR)
{
  try {
    return ORB->string_to_object(IOR);
  } catch (...) {
  }
  return CORBA::Object::_nil();
}
