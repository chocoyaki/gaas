#include <iostream>
#include <string>
using namespace std;

#include "ORBMgr.hh"

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

///    export PATH=/usr/local/j2sdk1.4.2_12/bin:$PATH

/* The following crap was massively "inspired" by:
 * - int diet_SeD(char* config_file_name, int argc, char* argv[])
 *   as taken from diet/src/SeD/DIET_server.cc
 * - boolean testConnection()
 *   as taken from diet-contrib/VizDIET/src/Controler/LogControler.java
 */

// Small conversion utility directly taken from Stroustrup's
// "The C++ programming language", third edition chapter 20.3.7
char* c_string( const string & s)
{
  char* p = new char[ s.length() + 1 ];
  s.copy( p, string::npos );
  p[ s.length() ] = 0;
  return p;
}


/*
 * CORBA::ORB_init() through DIET::ORBMgr::init()
 * @OmniNamesHost Suggested hostname
 * @OmniNamesPort Suggested port
 */
bool ORBMgrInit( string OmniNamesHost, 
                 string OmniNamesPort )
{
   /////// Prepare the arguments CORBA::ORB_init( ) invocation
   int    TempArgc = 2;
   char** TempArgv;
   TempArgv = (char**)malloc( TempArgc * sizeof(char*) );
 
   // First argument:
   TempArgv[ 0 ] = "-ORBInitRef";
   
   // Second argument is of the form NameService=corbaname::<host>:<port>:
   string EndPointArg = "NameService=corbaname::";
   if( OmniNamesHost.length() )
     EndPointArg += OmniNamesHost;
   EndPointArg += ":";
   if( OmniNamesPort.length() )
     EndPointArg += OmniNamesPort;
   TempArgv[ 1 ] = c_string( EndPointArg );

   const char* options[][2] =
   {
      { "traceLevel", "10" },
      { "inConScanPeriod", "0" },
      { "outConScanPeriod", "0" },
      { "maxGIOPConnectionPerServer", "50" },
      { "giopMaxMsgSize","33554432"},
      { 0, 0 }
   }; 

   //////// Try to initialize the ORB server:
   try
   {

      CORBA::ORB_ptr orb =  CORBA::ORB_init( TempArgc, TempArgv,
                                             "omniORB4", options );
  
      if( CORBA::is_nil( orb ) ) {
         std::cout << "Return orb is nil ? :" << std::endl;
         return false;
      }
      std::cout << "Return orb is valid." << std::endl;
  
      // Just make sure the orb is valid by listing initial services:
      CORBA::ORB::ObjectIdList *idlist = orb->list_initial_services();
      if ( !idlist )
      {
         std::cout << "Empty list of initial services." << std::endl;
         CORBA::release( orb );
         return false;
      }

      // Display the collected information to make sur orb is OK:
      std::cout << "List of initial services:" << std::endl;
      for( unsigned int i = 0; i < idlist->length(); i++ )
      {
         std::cout << "    "  << (*idlist)[i] << std::endl;
      }
      
      // The orb looks ok indeed !
      CORBA::release( orb );
      return true;
   }
   catch( CORBA::INITIALIZE& ex )
   {
      // Just to illustrate the syntax of minor handling:
      if (ex.minor() == omni::INITIALIZE_TransportError )
        std::cout << "CORBA exception TransportError." <<  endl;
      else if (ex.minor() == omni::INITIALIZE_InvalidORBInitArgs )
        std::cout << "CORBA exception InvalidORBInitArgs." <<  endl;
      else if (ex.minor() == omni::INITIALIZE_FailedBOAInit )
        std::cout << "CORBA exception FailedBOAInit." <<  endl;
      else if (ex.minor() == omni::INITIALIZE_FailedORBInit )
        std::cout << "CORBA exception FailedORBInit." <<  endl;
      else if (ex.minor() == omni::INITIALIZE_FailedPOAInit )
        std::cout << "CORBA exception FailedPOAInit." <<  endl;
      else
         std::cout << "CORBA exception generic Initialize." <<  endl;
      return false;
   }
   catch( CORBA::Exception& )
   {
      std::cout << "Caught CORBA::Exception." << endl;
      return false;
   }
   return false;
}

int TestORBMgrInit( int argc, char* argv[] )
{
   string OmniNamesHost = DIET_CTEST_FQDN_HOSTNAME;
   string OmniNamesPort = DIET_CTEST_OMNIORB_MAPPER_PORT;
   std::cout << "Testing the presence of an ORB server on "
             << OmniNamesHost << " on port " << OmniNamesPort   
             << std::endl;
   bool orbMgrSuccess = ORBMgrInit( OmniNamesHost, OmniNamesPort );
   if( ! orbMgrSuccess )
   {
      return EXIT_FAILURE;
   }
   return EXIT_SUCCESS;
}
