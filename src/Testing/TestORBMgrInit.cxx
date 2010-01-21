// This test doesn't test DIET per se. Instead it checks that the context
// for testing DIET is properly established i.e. it checks that one has
// proper access to an OMG's COS Naming Service Specification. 
// Hence the code of this test doesn't rely on DIET but only on the one
// of omniORB.
//
// Some links to CORBA documentation (which were mandatory to write this test):
// 1/ CORBA C++ Programming Reference:
//    http://edocs.bea.com/wle/cref/index.htm
// 2/ a CORBA tutorial by Mario Konrad with three different implementations
//    illustrating how to connect and initialize client ORB:
//        http://www.mario-konrad.ch/index.php?page=20026
//    i.e. result of a google search on "CORBA tutorial" "connecting 3 ORBs"
// 3/ the article "Interoperable Naming Service" by Fintan Bolton:
//    http://www.informit.com/articles/article.asp?p=23266&seqNum=4&rl=1
// 4/ another CORBA Programmer's Reference C++ (about Orbix)
//    http://www.iona.com/support/docs/orbix/6.3/reference/corba_pref/cpp/pref_cpp.pdf
//
// Note: the java equivalent of this test is only a couple lines long. See
//       e.g. the function boolean testConnection() as taken from the cvs
//       repository diet-contrib/VizDIET/src/Controler/LogControler.java.

#include <iostream>
#include <string>
#include <cstdlib>
#include "ORBMgr.hh"

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif

#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif

#define DIET_CTEST_INDENT " Diet: "

/**
 * Small conversion utility directly taken from Stroustrup's
 * "The C++ programming language", third edition chapter 20.3.7
 */
char* c_string( const std::string & s)
{
  char* p = new char[ s.length() + 1 ];
  s.copy( p, std::string::npos );
  p[ s.length() ] = 0;
  return p;
}


/**
 * Converts the string arguments to C-style arrays and invoke the
 * CORBA::ORB_init()
 * @OmniNamesHost omniNames host to contact
 * @OmniNamesPort associated port on given host
 */
bool ORBMgrInit( std::string OmniNamesHost, 
                 std::string OmniNamesPort )
{
   /////// Prepare the arguments CORBA::ORB_init( ) invocation
   int    TempArgc = 2;
   char** TempArgv;
   TempArgv = (char**)malloc( TempArgc * sizeof(char*) );
 
   // First argument:
   TempArgv[ 0 ] = (char*)"-ORBInitRef";
   
   // Second argument is of the form NameService=corbaname::<host>:<port>:
   std::string EndPointArg = "NameService=corbaloc::";
   if( OmniNamesHost.length() )
     EndPointArg += OmniNamesHost;
   EndPointArg += ":";

   if( OmniNamesPort.length() )
     EndPointArg += OmniNamesPort;
   EndPointArg += "/NameService";
   TempArgv[ 1 ] = c_string( EndPointArg );

   try
   {
      bool ExitFlag = false;
      /////////////////////////////////////////////////////////////////////////
      // Try to retrieve a reference to the CORBA::ORB object:
      CORBA::ORB_ptr orb;
      const char* options[][2] =
      {
         { "inConScanPeriod", "0" },
         { "outConScanPeriod", "0" },
         { "maxGIOPConnectionPerServer", "50" },
         { "giopMaxMsgSize", "33554432" },
      // Uncomment following line for debugging this test:
      // { "traceLevel", "40" },
         { 0, 0 }
      };

      try
      {
         orb = CORBA::ORB_init( TempArgc, TempArgv, "omniORB4", options );
      }
      catch( CORBA::INITIALIZE& ex )
      {
         std::cerr << DIET_CTEST_INDENT
                   << "caught CORBA::INITIALIZE on ORB_init()."
                   << std::endl;
         // Just to illustrate omniORB's syntax of minor handling:
         if (ex.minor() == omni::INITIALIZE_TransportError )
           std::cerr << "CORBA exception TransportError." << std::endl;
         else if (ex.minor() == omni::INITIALIZE_InvalidORBInitArgs )
           std::cerr << "CORBA exception InvalidORBInitArgs." << std::endl;
         else if (ex.minor() == omni::INITIALIZE_FailedBOAInit )
           std::cerr << "CORBA exception FailedBOAInit." << std::endl;
         else if (ex.minor() == omni::INITIALIZE_FailedORBInit )
           std::cerr << "CORBA exception FailedORBInit." << std::endl;
         else if (ex.minor() == omni::INITIALIZE_FailedPOAInit )
           std::cerr << "CORBA exception FailedPOAInit." << std::endl;
         else
            std::cerr << "CORBA exception generic Initialize." << std::endl;
         ExitFlag = true;
      }
      catch( CORBA::Exception& )
      {
         std::cerr << DIET_CTEST_INDENT
                   << "caught CORBA::Exception on ORB_init()."
                   << std::endl;
         ExitFlag = true;
      }
      if( CORBA::is_nil( orb ) )
      {
         std::cerr << DIET_CTEST_INDENT 
                   << "CORBA::ORB object is nil."
                   << std::endl;
         ExitFlag = true;
      }
      if( ExitFlag )
      {
         std::cerr << DIET_CTEST_INDENT
                   << "Could not retrieve a CORBA::ORB object: exiting"
                   << std::endl;
         CORBA::release( orb );
         return false;
      }
      std::cout <<  ".1";

      /////////////////////////////////////////////////////////////////////////
      // Try to retrieve a reference to the name service:
      CORBA::Object_var NameServiceObject;
      try
      {
         NameServiceObject = orb->resolve_initial_references("NameService");
      }
      catch ( CORBA::ORB::InvalidName& )
      {
         std::cerr << DIET_CTEST_INDENT
                   << "Cannot retrieve Name Service: Invalid Name exception"
                   << std::endl;
         ExitFlag = true;
      }
      if ( CORBA::is_nil(NameServiceObject) )
      {
         std::cerr << DIET_CTEST_INDENT
                   << "retrieved Name Service was nil."
                   << std::endl;
         ExitFlag = true;
      }
      if( ExitFlag )
      {
         std::cerr << DIET_CTEST_INDENT
                   << "Could not retrieve a CORBA::ORB object: exiting"
                   << std::endl;
         CORBA::release( orb );
         return false;
      }
      std::cout <<  ".2";
  
      /////////////////////////////////////////////////////////////////////////
      // Try to retrieve the CosNaming::NamingContext:
      // Note: interstingly enough this stage is critical before tempting
      //       any further orb->list_initial_services(). If ones forgets the
      //       present stage (essentially a call to NamingContext::_narrow()
      //       then any call to list_initial_services() will still return
      //       a result but an invalid one...
      CosNaming::NamingContext_ptr NamingContext;
      try
      {
         NamingContext = CosNaming::NamingContext::_narrow( NameServiceObject );
      }
      catch( CORBA::SystemException& )
      {
         std::cerr << DIET_CTEST_INDENT
                   << "Cannot retrieve Naming Context: System exception."
                   << std::endl;
         ExitFlag = true;
      }
      catch( CosNaming::NamingContext::NotFound & )
      {
         std::cerr << DIET_CTEST_INDENT
                   << "caught a NotFound exception for Naming Context."
                   << std::endl;
         ExitFlag = true;
      }
      catch( CosNaming::NamingContext::InvalidName &)
      {
         std::cerr << DIET_CTEST_INDENT
                   << "caught an InvalidName exception for Naming Context."
                   << std::endl;
         ExitFlag = true;
      }
      catch( CosNaming::NamingContext::CannotProceed &)
      {
         std::cerr << DIET_CTEST_INDENT
                   << "caught a CannotProceed exception for Naming Context."
                   << std::endl;
         ExitFlag = true;
      }
      if ( !ExitFlag && CORBA::is_nil( NamingContext ) )
      {
         std::cerr << DIET_CTEST_INDENT
                   << "retrieved Naming Context was nil."
                   << std::endl;
         ExitFlag = true;
      }
      if( ExitFlag )
      {
         std::cerr << DIET_CTEST_INDENT
                   << "could not retrieve a Naming Context: exiting."
                   << std::endl;
         // CORBA::release( NameServiceObject );
         CORBA::release( orb );
         return false;
      }
      std::cout <<  ".3";
  
      /////////////////////////////////////////////////////////////////////////
      // Go paranoid and make sure the orb is really valid by listing the
      // initial services:
      CORBA::ORB::ObjectIdList* idlist;
      try
      {
         idlist = orb->list_initial_services();
         if ( !idlist )
         {
            std::cerr << "empty list of initial services." << std::endl;
            ExitFlag = true;
         }
         else
         {
            std::cerr <<  DIET_CTEST_INDENT
                      << "number of inital services: "
                      << idlist->length()
                      << std::endl;
            std::cerr <<  DIET_CTEST_INDENT
                      << "items of the inital services: "
                      << std::endl;
            for( unsigned int i = 0; i < idlist->length(); i++ )
            {
               std::cerr <<  DIET_CTEST_INDENT
                         << "    service: "
                         <<  (*idlist)[i]
                         << ", object: "
                         << orb->resolve_initial_references( (*idlist)[i] )
                         << std::endl;
            }
         }
      }
      catch( CORBA::Exception& ex )
      {
         std::cerr << DIET_CTEST_INDENT
                   << "caught CORBA::Exception on list_initial_services()" 
                   << std::endl;
         ExitFlag = true;
      }
      if( ExitFlag )
      {
         std::cerr << DIET_CTEST_INDENT
                   << "was right to get paranoid on list_initial_services()"
                   << std::endl;
         CORBA::release( orb );
         return false;
      }
      std::cout <<  ".4.";
      return true;
   }
   catch( CORBA::Exception& ex )
   {
      std::cerr << DIET_CTEST_INDENT
                << "uncaught CORBA::Exception ! " << std::endl;
      return false;
   }
   return true;
}

int TestORBMgrInit( int argc, char* argv[] )
{
   std::string OmniNamesHost = DIET_CTEST_FQDN_HOSTNAME;
   std::string OmniNamesPort = DIET_CTEST_OMNIORB_NAMESERVICE_PORT;
   std::cout << DIET_CTEST_INDENT
             << "Naming Service on host " 
             << OmniNamesHost
             << ":" << OmniNamesPort << " ...";
   bool orbMgrSuccess = ORBMgrInit( OmniNamesHost, OmniNamesPort );
   if( ! orbMgrSuccess )
   {
      std::cout << ".FAILED." << std::endl;
      return EXIT_FAILURE;
   }
   std::cout << "OK." << std::endl;
   return EXIT_SUCCESS;
}
