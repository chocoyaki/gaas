/**
 * @file constants.hh
 *
 * @brief  Configuration constants
 *
 * @author  Haikel Guemar (haikel.guemar@sysfera.com)
 *
 * @section Licence
 *   |LICENSE|
 */

#ifndef _CONSTANTS_HH_
#define _CONSTANTS_HH_

#include <string>
#ifdef __WIN32__
#define DIET_API_LIB __declspec(dllexport)
#else
#define DIET_API_LIB
#endif
namespace diet {
typedef enum {
  NONE_PARAMETER,
  BOOL_PARAMETER,
  INT_PARAMETER,
  ULONG_PARAMETER,
  STRING_PARAMETER,
  ADDRESS_PARAMETER,
  AGENT_PARAMETER,  // like int
} c_type_t;

typedef enum {
  CONFIGFILE = 0,
  TRACELEVEL,
  MANAME,             // MA for client to attach to
  AGENTTYPE,          // Is this agent an LA or MA
  DIETPORT,           // End point definition
  DIETHOSTNAME,       // End point definition
  NAME,               // Agent name
  PARENTNAME,         // Parent to attach to
  FASTUSE,
  LDAPUSE, LDAPBASE, LDAPMASK,
  NWSUSE, NWSNAMESERVER, NWSFORECASTER,
  USELOGSERVICE, LSOUTBUFFERSIZE, LSFLUSHINTERVAL,
  NEIGHBOURS, MAXNEIGHBOURS, MINNEIGHBOURS, UPDATELINKPERIOD,
  BINDSERVICEPORT, USECONCJOBLIMIT, MAXCONCJOBS,
  LOCATIONID,         // For alternative transfer cost prediction
  MADAGNAME,
  USEWFLOGSERVICE,
  /* New : For user scheduler support. */
  MODULENAME,
  MODULECFG,
  BATCHNAME,
  BATCHQUEUE,
  PATHTONFS,
  PATHTOTMP,
  // RequestID begins with this value
  INITREQUESTID,
  // file to touch at the end of the initialization
  MAXMSGSIZE,
  MAXDISKSPACE,
  MAXMEMSPACE,
  CACHEALGORITHM,
  SHAREFILES,
  DATABACKUPFILE,
  RESTOREONSTART,
  STORAGEDIR,
  USE_SPECIFIC_SCHEDULING,
  INTERNOARQUEUENAME,
  CLIENT_MAX_NB_SED,
  CLOUDURL,
  EMINAME,
  ERINAME,
  EKINAME,
  KEYNAME,
  VMTYPE,
  VMMINCOUNT,
  VMMAXCOUNT,
  PATHTOCERT,
  PATHTOPK,
  PATHTOSSHKEY,
  INSTANTIATEVMS,
  SECURITYGROUP,
  USERNAME,
  FORCE_CLIENT_REBIND,
  NBRETRY,
  NB_PARAM_TYPE
} param_type_t;

struct param_t {
  param_type_t key;
  const std::string value;
  // TODO: unused today but may help get rid of "simple_cast"
  c_type_t type;
};

extern DIET_API_LIB param_t params[];
}

#endif /* _CONSTANTS_HH_ */
