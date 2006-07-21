#include <iostream>
#include <string>
using namespace std;

#include "ORBMgr.hh"

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

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
 * Converts the string arguments to C-style arrays and invoke the
 * CORBA::ORB_init() through DIET::ORBMgr::init()
 * @OmniNamesHost Suggested hostname
 * @OmniNamesPort Suggested port
 */
bool ORBMgrInit( string OmniNamesHost, 
                 string OmniNamesPort )
{
  int    TempArgc = 2;
  char** TempArgv;
  TempArgv = (char**)malloc( TempArgc * sizeof(char*) );

  // First argument:
  TempArgv[ 0 ] = "-ORBendPoint";
  
  // Second argument:
  string EndPointArg = "giop:tcp:";
  if( OmniNamesHost.length() )
    EndPointArg += OmniNamesHost;
  EndPointArg += ":";

  if( OmniNamesPort.length() )
    EndPointArg += OmniNamesPort;
  TempArgv[ 1 ] = c_string( EndPointArg );
  std::cout << TempArgc << TempArgv[0] << TempArgv[ 1 ] <<"XXXXX\n"; 
  // Eventually try to initialize the ORB server:
  try
  {
    //std::cout << "HEERRERERER" 
    //    << CORBA::is_nil( CORBA::ORB_init( TempArgc, TempArgv, "omniORB4"))
    //    <<  endl;
//    if( CORBA::is_nil( CORBA::ORB_init( TempArgc, TempArgv, "omniORB4") ) )
    if( ORBMgr::init( TempArgc, TempArgv ) )
    {
      std::cout << "CORBA initialisation failed." <<  endl;
      return false;
    }
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
  std::cout << "Testing the existence of an ORB on localhost." << std::endl;
  std::cout << ORBMgrInit( "gdsdmi01.ens-lyon.fr", "" ) << std::endl;
  //std::cout << ORBMgrInit( "localhost", "11112809" ) << std::endl;

  return EXIT_SUCCESS;
}
