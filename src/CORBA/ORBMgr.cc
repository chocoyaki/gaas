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
 * Revision 1.6  2003/06/02 09:20:19  cpera
 * Delete debug trace.
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

extern unsigned int TRACE_LEVEL;

/* Initialize private static members. */
CORBA::ORB_ptr ORBMgr::ORB = CORBA::ORB::_nil();
PortableServer::POA_var ORBMgr::POA = PortableServer::POA::_nil();
int ORBMgr::DIET_ct = 1;
PortableServer::POA_var ORBMgr::POA_BIDIR = PortableServer::POA::_nil();

void
ORBMgr::setTraceLevel()
{
#ifdef __OMNIORB4__
  omniORB::traceLevel =
    (TRACE_LEVEL < TRACE_MAX_VALUE) ? 0 : (TRACE_LEVEL - TRACE_MAX_VALUE);
#endif // __OMNIORB4__
  return;
}

int
ORBMgr::init(int argc, char** argv, bool init_POA, int DIET_ct)
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
    CORBA::Object_var obj = ORB->resolve_initial_references("RootPOA");
    POA = PortableServer::POA::_narrow(obj);
    // Get the POAManager Ref and activate it
    PortableServer::POAManager_var pman = POA->the_POAManager();
    if (ORBMgr::DIET_ct == 1) {
    // Create a POA with the Bidirectional policy
      CORBA::PolicyList pl;
      pl.length(1);
      CORBA::Any a;
      a <<= BiDirPolicy::BOTH;
      pl[0] = ORBMgr::ORB->create_policy(BiDirPolicy::BIDIRECTIONAL_POLICY_TYPE, a);
      ORBMgr::POA_BIDIR = ORBMgr::POA->create_POA("bidir", pman, pl);
    }
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
  try {
    if (ORBMgr::DIET_ct == 1) PortableServer::ObjectId_var mySeDId = ORBMgr::POA_BIDIR->activate_object(obj);
    else { 
      PortableServer::ObjectId_var mySeDId = ORBMgr::POA->activate_object(obj);
      // Activate obj on the poa
      // Get the POAManager Ref and activate it
      // PortableServer::POAManager_var pman = POA->the_POAManager();
      // pman->activate();
    }
  }
  catch (...){
    return -1;
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

  try {
    // Obtain a reference to the root context of the Name service:
    CORBA::Object_ptr initServ;
    initServ = ORB->resolve_initial_references("NameService");

    // Narrow the object returned by resolve_initial_references()
    // to a CosNaming::NamingContext object:
    rootContext = CosNaming::NamingContext::_narrow(initServ);
    if (CORBA::is_nil(rootContext)) {
      cerr << "Failed to narrow naming context." << endl;
      return CORBA::Object::_nil();
    }
  } catch(CORBA::ORB::InvalidName& ex) {
    cerr << "Service required is invalid [does not exist]." << endl;
    return CORBA::Object::_nil();
  } catch (CORBA::Exception& e) {
    CORBA::Any tmp;
    tmp <<= e;
    CORBA::TypeCode_var tc = tmp.type();
    cerr << "Caught exception " << tc->name()
	 << " while attempting to connect to the CORBA name server.\n";
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

  try {
    // Resolve the name to an object reference, and assign the reference 
    // returned to a CORBA::Object:
    obj = rootContext->resolve(name);
  } catch(CosNaming::NamingContext::NotFound& ex) {
    // This exception is thrown if any of the components of the
    // path [contexts or the object] aren't found:
    cerr << "Context not found." << endl;
    return CORBA::Object::_nil();
  } catch (CORBA::COMM_FAILURE& ex) {
    cerr << "Caught system exception COMM_FAILURE, unable to contact the "
         << "naming service." << endl;
    return CORBA::Object::_nil();
  } catch(omniORB::fatalException& ex) {
    throw;
  } catch (...) {
    cerr << "Caught a system exception while using the naming service."<< endl;
    return CORBA::Object::_nil();
  }

  return obj;
}

  
int
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
    if (CORBA::is_nil(rootContext)) {
      cerr << "Failed to narrow naming context." << endl;
      return 1;
    }
  } catch(CORBA::ORB::InvalidName& ex) {
    cerr << "Service required is invalid [does not exist]." << endl;
    return 1;
  } catch (CORBA::Exception& e) {
    CORBA::Any tmp;
    tmp <<= e;
    CORBA::TypeCode_var tc = tmp.type();
    cerr << "Caught exception " << tc->name()
	 << " while attempting to register as " << agentName
	 << " to the CORBA Name server.\n";
    return 1;
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
        return 1;
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
    return 1;
  }
  catch (omniORB::fatalException& ex) {
    throw;
  }
  catch (...) {
    cerr << "Caught a system exception while using the naming service." << endl;
    return 1;
  }
  return 0;
}

char *
ORBMgr::getIORString(CORBA::Object_ptr obj)
{
  try {
    return ORB->object_to_string(obj);
  }
  catch(...){
  }
  return NULL;  
}


CORBA::Object_ptr
ORBMgr::stringToObject(const char* IOR)
{
  return ORB->string_to_object(IOR);
}
