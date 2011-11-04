#ifndef __OPERATION_HPP__
#define __OPERATION_HPP__

#include <DIET_client.h>
#include <DIET_grpc.h>
#include <omniORB4/CORBA.h>

#include "SeD.hh"
#include "LocalAgent.hh"
#include "ORBMgr.hh"

using namespace std;

enum operation {DISCONNECT, CONNECT, REMOVE};

int
dynamicOperation(const std::string &type,
                 const std::string &name,
                 enum operation op,
                 const std::string &parentName = "",
                 bool recursive = false) {
  try {
    // ------------------------------------------------------------------------
    // Initialize ORB object.
    // ------------------------------------------------------------------------
    int argc = 0;
    char **argv = NULL;
    CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);

    // ------------------------------------------------------------------------
    // Bind ORB object to name service object.
    // (Reference to Name service root context.)
    // ------------------------------------------------------------------------
    CORBA::Object_var obj = orb->resolve_initial_references("NameService");
    BOOST_CHECK_EQUAL(0, CORBA::is_nil(obj.in()));

    // ------------------------------------------------------------------------
    // Narrow this to the naming context (Narrowed reference to root context.)
    // ------------------------------------------------------------------------
    CosNaming::NamingContext_var nc =
      CosNaming::NamingContext::_narrow(obj.in());
    BOOST_CHECK_EQUAL(0, CORBA::is_nil(nc.in()));

    // ------------------------------------------------------------------------
    // The "name text" put forth by CORBA server in name service.
    // This same name ("DataServiceName1") is used by the CORBA server when
    // binding to the name server (CosNaming::Name).
    // ------------------------------------------------------------------------
    CosNaming::Name _corbaCosName;
    _corbaCosName.length(2);
    _corbaCosName[0].kind = (const char *) "";
    _corbaCosName[1].id = CORBA::string_dup(name.c_str());
    _corbaCosName[1].kind = (const char *) "";

    if (type == "SED") {
      _corbaCosName[0].id = "dietSeD";
    } else {
      _corbaCosName[0].id = "dietAgent";
    }


    // ------------------------------------------------------------------------
    // Resolve "name text" identifier to an object reference.
    // ------------------------------------------------------------------------
    CORBA::Object_var obj1;
    try {
      obj1 = nc->resolve(_corbaCosName);
    } catch (CosNaming::NamingContext::NotFound &ex) {
      cerr << "context for " << name << " not found" << endl;
    } catch (CORBA::COMM_FAILURE &ex) {
      cerr << "system exception caught (COMM_FAILURE): unable to connect to "
           << "the CORBA name server" << endl;
    } catch (omniORB::fatalException &ex) {
      throw;
    } catch (...) {
      cerr << "system exception caught while using the naming service" << endl;
    }

    switch (op) {
    case CONNECT:
      if (type == "SED") {
        SeD_ptr sed = SeD::_narrow(obj1.in());
        if (sed->bindParent(parentName.c_str())) {
          return 1;
        }
      } else {
        LocalAgent_ptr agent = LocalAgent::_narrow(obj1.in());
        if (agent->bindParent(parentName.c_str())) {
          return 1;
        }
      }
      break;

    case DISCONNECT:
      if (type == "SED") {
        SeD_ptr sed = SeD::_narrow(obj1.in());
        if (sed->disconnect()) {
          return 1;
        }
      } else {
        LocalAgent_ptr agent = LocalAgent::_narrow(obj1.in());
        if (agent->disconnect()) {
          return 1;
        }
      }
      break;

    case REMOVE:
      if (type == "SED") {
        SeD_ptr sed = SeD::_narrow(obj1.in());
        if (sed->removeElement()) {
          return 1;
        }
      } else {
        Agent_ptr agent = Agent::_narrow(obj1.in());
        if (agent->removeElement(recursive)) {
          return 1;
        }
      }
      break;
    }   // end switch

    orb->destroy();
  } catch (CORBA::COMM_FAILURE &ex) {
    cerr << "Caught system exception COMM_FAILURE -- unable to contact the "
         << "object." << endl;
    cerr << "CORBA COMM_FAILURE" << endl;
    return 1;
  } catch (CORBA::SystemException &) {
    cerr << "Caught a CORBA::SystemException." << endl;
    cerr << "CORBA Exception" << endl;
    return 1;
  } catch (CORBA::Exception &) {
    cerr << "Caught CORBA::Exception." << endl;
    return 1;
  } catch (omniORB::fatalException &fe) {
    cerr << "Caught omniORB::fatalException:" << endl;
    cerr << "  file: " << fe.file() << endl;
    cerr << "  line: " << fe.line() << endl;
    cerr << "  mesg: " << fe.errmsg() << endl;
    cerr << "CORBA Fatal Exception" << endl;
    return 1;
  } catch (...) {
    cerr << "Caught unknown exception." << endl;
    return 1;
  }

  return 0;
} // dynamicOperation


#endif  // __OPERATION_HPP__
