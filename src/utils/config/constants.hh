/****************************************************************************/
/* Configuration constants                                                  */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Haikel Guemar (haikel.guemar@sysfera.com)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.4  2011/03/07 15:34:55  hguemar
 * fix typo in mail address
 *
 * Revision 1.3  2011/03/03 11:55:52  bdepardo
 * Add missing headers
 *
 ****************************************************************************/


#ifndef _CONSTANTS_HH_
#define _CONSTANTS_HH_

#include <string>

namespace diet {
typedef enum {
  NONE_PARAMETER,
  BOOL_PARAMETER,
  INT_PARAMETER,
  ULONG_PARAMETER,
  STRING_PARAMETER,
  ADDRESS_PARAMETER,
  AGENT_PARAMETER, // like int
} c_type_t;

typedef enum {
  CONFIGFILE=0,
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
  ACKFILE,
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
  NB_PARAM_TYPE
} param_type_t;

struct param_t
{
  param_type_t key;
  const std::string value;
  // TODO: unused today but may help get rid of "simple_cast"
  c_type_t type;
};

extern param_t params[];
}

#endif /* _CONSTANTS_HH_ */
