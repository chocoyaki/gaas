/****************************************************************************/
/* $Id$ */
/* DIET ORB Manager implementation                                          */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/*
 * $Log$
 * Revision 1.1  2003/02/04 09:58:13  pcombes
 * Unify ORBs interface with a manager class: ORBMgr
 *
 ****************************************************************************/

#include "ORBMgr.hh"

#include <iostream>
using namespace std;
//#include <stdio.h>


static CORBA::ORB_ptr          ORB;
static PortableServer::POA_var POA;


void
ORBMgr::traceLevel(int level)
{
#ifdef __OMNIORB4__
  omniORB::traceLevel = level;
#endif // __OMNIORB4__
}

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
       {"maxGIOPConnectionPerServer","50"},{"giopMaxMsgSize","33554432"},
       {0,0}};
  ORB = CORBA::ORB_init(argc, argv, "omniORB4", options);
#endif // __OMNIORB4__
#endif // __OMNIORB3__
  
  if (init_POA) {
    CORBA::Object_var obj = ORB->resolve_initial_references("RootPOA");
    POA = PortableServer::POA::_narrow(obj);
  }
  return 0;
}


int
ORBMgr::activate(PortableServer::ServantBase* obj)
{
  // Activate obj on the poa
  PortableServer::ObjectId_var mySeDId = POA->activate_object(obj);
  // Get the POAManager Ref and activate it
  PortableServer::POAManager_var pman = POA->the_POAManager();
  pman->activate();
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

  try {
    // Obtain a reference to the root context of the Name service:
    CORBA::Object_ptr initServ;
    initServ = ORB->resolve_initial_references("NameService");

    // Narrow the object returned by resolve_initial_references()
    // to a CosNaming::NamingContext object:
    rootContext = CosNaming::NamingContext::_narrow(initServ);
    if (CORBA::is_nil(rootContext)) 
      {
        cerr << "Failed to narrow naming context." << endl;
        return CORBA::Object::_nil();
      }
  }
  catch(CORBA::ORB::InvalidName& ex) {
    cerr << "Service required is invalid [does not exist]." << endl;
    return CORBA::Object::_nil();
  }

  // FIXME: is that relevant ???

  // Create a name object, containing the name test/context:
  CosNaming::Name name;
  name.length(2);

  name[0].id   = (const char*) "dietAgent";       // string copied
  name[0].kind = (const char*) ""; // string copied
  name[1].id   = (const char*) agentName;
  name[1].kind = (const char*) "";
  // Note on kind: The kind field is used to indicate the type
  // of the object. This is to avoid conventions such as that used
  // by files (name.type -- e.g. test.ps = postscript etc.)

  CORBA::Object_ptr obj;
  try {
    // Resolve the name to an object reference, and assign the reference 
    // returned to a CORBA::Object:
    obj = rootContext->resolve(name);
  }
  catch(CosNaming::NamingContext::NotFound& ex)
    {
      // This exception is thrown if any of the components of the
      // path [contexts or the object] aren't found:
      cerr << "Context not found." << endl;
      return CORBA::Object::_nil();
    }
  catch (CORBA::COMM_FAILURE& ex) {
    cerr << "Caught system exception COMM_FAILURE, unable to contact the "
         << "naming service." << endl;
    return CORBA::Object::_nil();
  }
  catch(omniORB::fatalException& ex) {
    throw;
  }
  catch (...) {
    cerr << "Caught a system exception while using the naming service."<< endl;
    return CORBA::Object::_nil();
  }

  return obj;
}

  
CORBA::Boolean
ORBMgr::bindAgentToName(CORBA::Object_ptr obj, const char* agentName)
{
  CosNaming::NamingContext_var rootContext;
  
  try {
    // Obtain a reference to the root context of the Name service:
    CORBA::Object_var initServ;
    initServ = ORB->resolve_initial_references("NameService");

    // Narrow the object returned by resolve_initial_references()
    // to a CosNaming::NamingContext object:
    rootContext = CosNaming::NamingContext::_narrow(initServ);
    if (CORBA::is_nil(rootContext))
      {
        cerr << "Failed to narrow naming context." << endl;
        return 0;
      }
  }
  catch(CORBA::ORB::InvalidName& ex) {
    cerr << "Service required is invalid [does not exist]." << endl;
    return 0;
  }


  try {
    // Bind a context called "test" to the root context:
    // FIXME: WHY WHY WHY ??????????????

    CosNaming::Name contextName;
    contextName.length(1);
    contextName[0].id   = (const char*) "dietAgent"; //"test";    // string copied
    contextName[0].kind = (const char*) ""; //"my_context"; // string copied    
    // Note on kind: The kind field is used to indicate the type
    // of the object. This is to avoid conventions such as that used
    // by files (name.type -- e.g. test.ps = postscript etc.)

    CosNaming::NamingContext_var testContext;
    try {
      // Bind the context to root, and assign testContext to it:
      testContext = rootContext->bind_new_context(contextName);
    }
    catch(CosNaming::NamingContext::AlreadyBound& ex) {
      // If the context already exists, this exception will be raised.
      // In this case, just resolve the name and assign testContext
      // to the object returned:
      CORBA::Object_var tmpobj;
      tmpobj = rootContext->resolve(contextName);
      testContext = CosNaming::NamingContext::_narrow(tmpobj);
      if (CORBA::is_nil(testContext)) {
        cerr << "Failed to narrow naming context." << endl;
        return 0;
      }
    } 

    // Bind the object (obj) to testContext, naming it Echo:
    // FIXME: Why not to the rootContext ???
    CosNaming::Name objectName;
    objectName.length(1);
    objectName[0].id   = (const char*) agentName;   // string copied
    objectName[0].kind = (const char*) ""; // string copied


    // Bind obj with name Echo to the testContext:
    try {
      testContext->bind(objectName,obj);
    }
    catch(CosNaming::NamingContext::AlreadyBound& ex) {
      testContext->rebind(objectName,obj);
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
  }
  catch (CORBA::COMM_FAILURE& ex) {
    cerr << "Caught system exception COMM_FAILURE, unable to contact the "
         << "naming service." << endl;
    return 0;
  }
  catch (omniORB::fatalException& ex) {
    throw;
  }
  catch (...) {
    cerr << "Caught a system exception while using the naming service."<< endl;
    return 0;
  }
  return 1;
}

CORBA::String_var
ORBMgr::getIORstring(CORBA::Object_ptr obj)
{
  return ORB->object_to_string(obj);  
}
