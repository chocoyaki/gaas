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
 * Revision 1.15  2004/09/29 13:35:31  sdahan
 * Add the Multi-MAs feature.
 *
 * Revision 1.14  2004/03/01 18:40:40  rbolze
 * remove functions getIOD() and setIOD(..)
 * change in wait() : wait CRTL+D to exit
 * change in activate(...)
 * this change are provide by cpontvieux
 *
 * Revision 1.13  2003/10/06 10:04:13  cpontvie
 * Moving the interruption manager here
 * The current interruption is mapped on SIGINT (Ctrl+C)
 * The 'wait' function now return after the SIGINT
 *
 * Revision 1.12  2003/09/22 21:06:12  pcombes
 * Generalize the bind/unbindAgentToName and getAgentReference methods.
 *
 * Revision 1.9  2003/08/28 16:54:08  cpontvie
 * Add functions deactivate, unbindAgent, get_orb, get_poa, get_poa_bidir, 
 * get_oid, set_oid
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
 * Revision 1.1  2003/02/04 09:58:13  pcombes
 * Unify ORBs interface with a manager class: ORBMgr
 ****************************************************************************/


using namespace std;
#include <iostream>
#include <signal.h>
#include <setjmp.h>

#include "ORBMgr.hh"
#include "debug.hh"

/** The trace level. */
extern unsigned int TRACE_LEVEL;

/* Initialize private static members. */
CORBA::ORB_ptr ORBMgr::ORB = CORBA::ORB::_nil();
PortableServer::POA_var ORBMgr::POA = PortableServer::POA::_nil();
PortableServer::POA_var ORBMgr::POA_BIDIR = PortableServer::POA::_nil();
const char* ORBMgr::CONTEXTS[] =
  {"dietAgent", "dietDataMgr", "dietLocMgr", "LogService", "dietSeD"};

#if INTERRUPTION_MGR
sigjmp_buf ORBMgr::buff_int;
#endif

int
ORBMgr::init(int argc, char** argv, bool init_POA)
{
#ifdef __OMNIORB3__
  ORB = CORBA::ORB_init(argc, argv, "omniORB3");
  omniORB::scanGranularity(0);
#elif defined (__OMNIORB4__)
  const char* options[][2]
    = {{"inConScanPeriod","0"},{"outConScanPeriod","0"},
       {"maxGIOPConnectionPerServer","50"},
       //{"giopMaxMsgSize","33554432"},
       {0,0}};
  ORB = CORBA::ORB_init(argc, argv, "omniORB4", options);
#else
#error "This version of omniORB is not supported !!!"
#endif

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

    // omniORB3 does not know the bidirectional policy
#ifndef __OMNIORB3__
    // Create a POA with the Bidirectional policy
    pl.length(1);
    a <<= BiDirPolicy::BOTH;
    pl[0] =
      ORBMgr::ORB->create_policy(BiDirPolicy::BIDIRECTIONAL_POLICY_TYPE, a);
    ORBMgr::POA_BIDIR = ORBMgr::POA->create_POA("bidir", pman, pl);
#endif

    pman->activate();
  }
  return 0;

}

void
ORBMgr::destroy()
{
  try {
#if INTERRUPTION_MGR
    signal(SIGINT, SIG_DFL);
#endif // INTERRUPTION_MGR
    ORB->shutdown(true);
    ORB->destroy();
  } catch (...) {};
}


int
ORBMgr::activate(PortableServer::ServantBase* obj,
                 PortableServer::ObjectId_var* idVar_ptr)
{
  PortableServer::ObjectId_var objId;
  try {
    if (!CORBA::is_nil(ORBMgr::POA_BIDIR)) {
      objId = ORBMgr::POA_BIDIR->activate_object(obj);
    } else {
      objId = ORBMgr::POA->activate_object(obj);
    }
    obj->_remove_ref();
  } catch (...) {
    INTERNAL_ERROR("exception caught in ORBMgr::" << __FUNCTION__, -1);
  }

  if (idVar_ptr != NULL) {
    // copy the objectId if user needs it
    *idVar_ptr = objId;
  }
  return 0;
}


#if INTERRUPTION_MGR
int
ORBMgr::wait()
{
  signal(SIGINT, ORBMgr::SigIntHandler);
  if (! sigsetjmp(buff_int, 1)) {
    try {
      char c;
      // This loop replaces ORB->run()
      // it might not be the official way but it works very well
      printf("Press CRTL-D to exit\n");
      while(cin.get(c)) {
        if (c==10) printf("Press CRTL-D to exit\n");
      }
    } catch (...) {
      ERROR("exception caught in ORBMgr::" << __FUNCTION__, true);
    }
  }
  return 0;
}
#else // INTERRUPTION_MGR
int
ORBMgr::wait()
{
  ORB->run();
  return 0;
}
#endif // INTERRUPTION_MGR


int
ORBMgr::bindObjToName(CORBA::Object_ptr obj, ORBMgr::object_type_t type,
		      const char* name)
{
  CosNaming::NamingContext_var rootContext, context;
  CosNaming::Name cosName;

  rootContext = ORBMgr::getRootContext();
  if (CORBA::is_nil(rootContext))
    return 1;
  
  cosName.length(1);
  cosName[0].id   = CONTEXTS[type];   // string copied
  cosName[0].kind = (const char*) ""; // string copied    
  // Note on kind: The kind field is used to indicate the type
  // of the object. This is to avoid conventions such as that used
  // by files (name.type -- e.g. test.ps = postscript etc.)

  try {
    try {
      context = rootContext->bind_new_context(cosName);
    } catch(CosNaming::NamingContext::AlreadyBound& ex) {
      // If the context already exists, this exception will be raised.
      // In this case, just resolve the name and assign testContext
      // to the object returned:
      CORBA::Object_var tmpobj;
      tmpobj = rootContext->resolve(cosName);
      context = CosNaming::NamingContext::_narrow(tmpobj);
      if (CORBA::is_nil(context)) {
	INTERNAL_ERROR("failed to narrow the naming context for "
		       << CONTEXTS[type], 1);
      }
    }

    /* Bind the object (obj) to testContext */
    cosName[0].id   = (const char*) name; // string copied
    cosName[0].kind = (const char*) "";   // string copied

    // Bind obj with name Echo to the testContext:
    try {
      context->bind(cosName,obj);
    } catch(CosNaming::NamingContext::AlreadyBound& ex) {
      context->rebind(cosName,obj);
    }

    // Note: Using rebind() will overwrite any Object previously bound
    //       to /test/Echo with obj.
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
  }
  catch (omniORB::fatalException& ex) {
    throw;
  }
  catch (...) {
    INTERNAL_ERROR("system exception caught while using the naming service", 1);
  }
  return 0;
} // bindObjToName(...)


int
ORBMgr::unbindObj(object_type_t type, const char* name)
{
  CosNaming::NamingContext_var rootContext, context;
  CosNaming::Name cosName;

  rootContext = ORBMgr::getRootContext();
  if (CORBA::is_nil(rootContext))
    return 1;

  cosName.length(1);

  cosName[0].id = CONTEXTS[type];     // string copied
  cosName[0].kind = (const char*) ""; // string copied
  // Note on kind: The kind field is used to indicate the type
  // of the object. This is to avoid conventions such as that used
  // by files (name.type -- e.g. test.ps = postscript etc.)

  try {
    try {
      // Resolve the name and assign testContext to the object returned:
      CORBA::Object_var tmpobj;
      tmpobj = rootContext->resolve(cosName);
      context = CosNaming::NamingContext::_narrow(tmpobj);
      if (CORBA::is_nil(context)) {
        INTERNAL_ERROR("failed to narrow the naming context for "
		       << CONTEXTS[type], 1);
      }
    } catch(CosNaming::NamingContext::NotFound& ex) {
      ERROR("root context for dietAgent not found", 1);
    } catch (...) {
      INTERNAL_ERROR("system exception caught while using the naming service",
                     1);
    }

    /* Unbind obj to the agent context */
    cosName[0].id   = (const char*) name; // string copied
    cosName[0].kind = (const char*) "";   // string copied

    // Unbind obj with name name to the context:
    context->unbind(cosName);
  } catch (CORBA::COMM_FAILURE& ex) {
    ERROR("system exception caught (COMM_FAILURE), unable to connect to "
	  << "the CORBA name server", 1);
  } catch (omniORB::fatalException& ex) {
    throw;
  } catch (...) {
    INTERNAL_ERROR("system exception caught while using the naming service", 1);
  }
  return 0;
} // unbindObj(...)


CORBA::Object_ptr
ORBMgr::getObjReference(ORBMgr::object_type_t type, const char* name)
{
  CosNaming::NamingContext_var rootContext;
  CORBA::Object_ptr obj;
  CosNaming::Name cosName;
  
  rootContext = ORBMgr::getRootContext();
  if (CORBA::is_nil(rootContext))
    return CORBA::Object::_nil();

  // Create a name object, containing the name CONTEXT[type]/name:
  cosName.length(2);
  cosName[0].id   = CONTEXTS[type];     // string copied
  cosName[0].kind = (const char*) "";   // string copied
  cosName[1].id   = (const char*) name; // string copied
  cosName[1].kind = (const char*) "";   // string copied
  // Note on kind: The kind field is used to indicate the type
  // of the object. This is to avoid conventions such as that used
  // by files (name.type -- e.g. test.ps = postscript etc.)

  try {
    // Resolve the name to an object reference, and assign the reference 
    // returned to a CORBA::Object:
    obj = rootContext->resolve(cosName);
  } catch(CosNaming::NamingContext::NotFound& ex) {
    // This exception is thrown if any of the components of the
    // path [contexts or the object] aren't found:
    ERROR("context for " << name << " not found", CORBA::Object::_nil());
  } catch (CORBA::COMM_FAILURE& ex) {
    ERROR("system exception caught (COMM_FAILURE): unable to connect to "
	  << "the CORBA name server", CORBA::Object::_nil());
  } catch(omniORB::fatalException& ex) {
    throw;
  } catch (...) {
    INTERNAL_ERROR("system exception caught while using the naming service", 1);
  }
  return obj;

} // getObjReference(...)


char*
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
    ERROR("Bad format for stringified reference: "<< IOR,
	  CORBA::Object::_nil());
  }
  return CORBA::Object::_nil();
}

CORBA::ORB_ptr
ORBMgr::getORB()
{
  return ORB;
}

PortableServer::POA_var
ORBMgr::getPOA()
{
  return POA;
}


PortableServer::POA_var
ORBMgr::getPOA_BIDIR()
{
  return POA_BIDIR;
}


#if INTERRUPTION_MGR
/**
 * The SIGINT handler function
 */
void
ORBMgr::SigIntHandler(int sig)
{
  /* Prevent from raising a new SIGINT handler */
  signal(SIGINT, SIG_IGN);
  siglongjmp(buff_int, 1);
}
#endif // INTERRUPTION_MGR


CosNaming::NamingContext_var
ORBMgr::getRootContext()
{
  CosNaming::NamingContext_var rootContext;
  try {
    // Obtain a reference to the root context of the Name service:
    CORBA::Object_ptr initServ;
    initServ = ORB->resolve_initial_references("NameService");

    // Narrow the object returned by resolve_initial_references()
    // to a CosNaming::NamingContext object:
    rootContext = CosNaming::NamingContext::_narrow(initServ);
    if (CORBA::is_nil(rootContext)) 
      {
	INTERNAL_ERROR("failed to narrow the root naming context", 1);
      }
  } catch(CORBA::ORB::InvalidName& ex) {
    INTERNAL_ERROR("service required is invalid [does not exist]", 1);
  } catch (CORBA::Exception& e) {
    CORBA::Any tmp;
    tmp <<= e;
    CORBA::TypeCode_var tc = tmp.type();
    ERROR("exception caught (" << tc->name() << ") while attempting to "
	  << "connect to the CORBA name server",
	  CosNaming::NamingContext::_nil());
  }
  return rootContext;
} // getRootContext()
