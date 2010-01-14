/****************************************************************************/
/* Dynamic hierarchy management                                             */
/* Example code for dynamically disconnecting an element from its parent.   */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Benjamin DEPARDON (Benjamin.Depardon@ens-lyon.fr)                   */
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

using namespace std;

int
main (int argc, char** argv) {
  if (argc < 3) {
    cout << "Usage: " << argv[0] << " <SED|LA> <element name>" << endl;
    exit(1);
  }
  
  char * type = argv[1];
  char * name = argv[2];
  cout << "Element name: " << name << endl;
	

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
    if (CORBA::is_nil(obj.in()))
      cout << "Pb nameservice" << endl;

    //------------------------------------------------------------------------
    // Narrow this to the naming context (Narrowed reference to root context.)
    //------------------------------------------------------------------------
    CosNaming::NamingContext_var nc =
      CosNaming::NamingContext::_narrow(obj.in());
    if (CORBA::is_nil(nc.in()))
      cout << "pb narrow nameservice" << endl;
                                                                                
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

    if (!strcmp(type, "SED"))
      _corbaCosName[0].id="dietSeD";
    else
      _corbaCosName[0].id="dietAgent";


    //------------------------------------------------------------------------
    // Resolve "name text" identifier to an object reference.
    //------------------------------------------------------------------------
    CORBA::Object_var obj1;
    try {
      obj1 = nc->resolve(_corbaCosName);
    } catch(CosNaming::NamingContext::NotFound& ex) {
      cerr <<"context for " << name << " not found" << endl;
    } catch (CORBA::COMM_FAILURE& ex) {
      cerr << "system exception caught (COMM_FAILURE): unable to connect to "
	   << "the CORBA name server" << endl;
    } catch(omniORB::fatalException& ex) {
      throw;
    } catch (...) {
      cerr << "system exception caught while using the naming service" << endl;
    }

    if (!strcmp(type, "SED")) {
      SeD_ptr sed = SeD::_narrow(obj1.in());
      if (! sed->disconnect())
	cout << "** Disconnecting done **" << endl;
      else
	cout << "** Disconnecting failed **" << endl;
    } else {
      LocalAgent_ptr agent = LocalAgent::_narrow(obj1.in());
      if (! agent->disconnect())
	cout << "** Disconnecting done **" << endl;
      else
	cout << "** Disconnecting failed **" << endl;
    }
  }
  catch(CORBA::COMM_FAILURE& ex) {
    cerr << "Caught system exception COMM_FAILURE -- unable to contact the "
         << "object." << endl;
    cerr << "CORBA COMM_FAILURE" << endl;
    return 1;
  }
  catch(CORBA::SystemException& ) {
    cerr << "Caught a CORBA::SystemException." << endl;
    cerr << "CORBA Exception" << endl;
    return 1;
  }
  catch(CORBA::Exception& ) {
    cerr << "Caught CORBA::Exception." << endl;
    return 1;
  }  catch(omniORB::fatalException& fe) {
    cerr << "Caught omniORB::fatalException:" << endl;
    cerr << "  file: " << fe.file() << endl;
    cerr << "  line: " << fe.line() << endl;
    cerr << "  mesg: " << fe.errmsg() << endl;
    cerr << "CORBA Fatal Exception" << endl;
    return 1;
  }
  catch(...) {
    cerr << "Caught unknown exception." << endl;
    return 1;
  }

  return 0;
}
