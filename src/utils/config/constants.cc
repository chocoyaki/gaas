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
 * Revision 1.5  2011/03/07 15:34:55  hguemar
 * fix typo in mail address
 *
 * Revision 1.4  2011/03/03 11:55:52  bdepardo
 * Add missing headers
 *
 ****************************************************************************/


#include "constants.hh"

namespace diet {

  // !!! None of these keywords can be a prefix for another !!!
  param_t params[] =
    {
      {CONFIGFILE, "configFile", STRING_PARAMETER},
      /* [0]  */ {TRACELEVEL, "traceLevel", INT_PARAMETER},
      /* [1]  */ {MANAME, "MAName", STRING_PARAMETER},
      /* [2]  */ {AGENTTYPE, "agentType", AGENT_PARAMETER},
      /* [3]  */ {DIETPORT, "dietPort", INT_PARAMETER},
      /* [4]  */ {DIETHOSTNAME, "dietHostname", STRING_PARAMETER},
      /* [5]  */ {NAME, "name", STRING_PARAMETER},
      /* [6]  */ {PARENTNAME, "parentName", STRING_PARAMETER},
      /* [7]  */ {FASTUSE, "fastUse", BOOL_PARAMETER},
      /* [8]  */ {LDAPUSE, "ldapUse", BOOL_PARAMETER},
      /* [9]  */ {LDAPBASE, "ldapBase", ADDRESS_PARAMETER},
      /* [10] */ {LDAPMASK, "ldapMask", STRING_PARAMETER},
      /* [11] */ {NWSUSE, "nwsUse", BOOL_PARAMETER},
      /* [12] */ {NWSNAMESERVER, "nwsNameserver", ADDRESS_PARAMETER},
      /* [13] */ {NWSFORECASTER, "nwsForecaster", ADDRESS_PARAMETER},
      /* [14] */ {USELOGSERVICE, "useLogService", BOOL_PARAMETER},
      /* [15] */ {LSOUTBUFFERSIZE, "lsOutbuffersize", INT_PARAMETER},
      /* [16] */ {LSFLUSHINTERVAL, "lsFlushinterval", INT_PARAMETER},
      /* [17] */ {NEIGHBOURS, "neighbours", STRING_PARAMETER},
      /* [18] */ {MAXNEIGHBOURS, "maximumNeighbours", INT_PARAMETER},
      /* [19] */ {MINNEIGHBOURS, "minimumNeighbours", INT_PARAMETER},
      /* [20] */ {UPDATELINKPERIOD, "updateLinkPeriod", INT_PARAMETER},
      /* [21] */ {BINDSERVICEPORT, "bindServicePort", INT_PARAMETER},
      /* [22] */ {USECONCJOBLIMIT, "useConcJobLimit", BOOL_PARAMETER},
      /* [23] */ {MAXCONCJOBS, "maxConcJobs", INT_PARAMETER},
      /* [24] */ {LOCATIONID, "locationID", STRING_PARAMETER},
      /* [25] */ {MADAGNAME, "MADAGNAME", STRING_PARAMETER},
      /* [26] */ {USEWFLOGSERVICE, "USEWFLOGSERVICE", BOOL_PARAMETER},
      /* New : For user scheduler support. */
      /*       schedulerModule  : The path to the scheduler library file. */
      /*       moduleConfigFile : Optionnal configuration file for the module. */
      /* [27] */ {MODULENAME, "schedulerModule", STRING_PARAMETER},
      /* [28] */ {MODULECFG, "moduleConfigFile", STRING_PARAMETER},
      /* [29] */ {BATCHNAME, "batchName", STRING_PARAMETER},
      /* [30] */ {BATCHQUEUE, "batchQueue", STRING_PARAMETER},
      /* [31] */ {PATHTONFS, "pathToNFS", STRING_PARAMETER},
      /* [32] */ {PATHTOTMP, "pathToTmp", STRING_PARAMETER},
      /* [33] */ {INITREQUESTID, "initRequestID", INT_PARAMETER},
      /* [34] */ {ACKFILE, "ackFile", STRING_PARAMETER},
      /* [35] */ {MAXMSGSIZE, "maxMsgSize",ULONG_PARAMETER},
      /* [36] */ {MAXDISKSPACE, "maxDiskSpace", ULONG_PARAMETER},
      /* [37] */ {MAXMEMSPACE, "maxMemSpace", ULONG_PARAMETER},
      /* [38] */ {CACHEALGORITHM, "cacheAlgorithm", STRING_PARAMETER},
      /* [39] */ {SHAREFILES, "shareFiles", INT_PARAMETER},
      /* [40] */ {DATABACKUPFILE, "dataBackupFile", STRING_PARAMETER},
      /* [41] */ {RESTOREONSTART, "restoreOnStart", BOOL_PARAMETER},
      /* [42] */ {STORAGEDIR, "storageDirectory", STRING_PARAMETER},
      /* [43] */ {USE_SPECIFIC_SCHEDULING, "USE_SPECIFIC_SCHEDULING", STRING_PARAMETER},
      /* [44] */ {INTERNOARQUEUENAME, "internOARbatchQueueName", STRING_PARAMETER},
      /* [45] */ {CLIENT_MAX_NB_SED, "clientMaxNbSeD", ULONG_PARAMETER},
      /* [46] */ {CLOUDURL, "cloudURL", STRING_PARAMETER},
      /* [47] */ {EMINAME, "emiName", STRING_PARAMETER},
      /* [48] */ {ERINAME, "eriName", STRING_PARAMETER},
      /* [49] */ {EKINAME, "ekiName", STRING_PARAMETER},
      /* [50] */ {KEYNAME, "keyName", STRING_PARAMETER},
      /* [51] */ {VMTYPE, "vmType", STRING_PARAMETER},
      /* [52] */ {VMMINCOUNT, "vmMinCount", INT_PARAMETER},
      /* [53] */ {VMMAXCOUNT, "vmMaxCount", INT_PARAMETER},
      /* [54] */ {PATHTOCERT,"pathToCert", STRING_PARAMETER},
      /* [55] */ {PATHTOPK, "pathToPK", STRING_PARAMETER},
      /* [56] */ {PATHTOSSHKEY, "pathToSSHKey", STRING_PARAMETER},
      /* [57] */ {INSTANTIATEVMS, "instantiateVMs", INT_PARAMETER},
      /* [58] */ {SECURITYGROUP, "securityGroup", STRING_PARAMETER},
      /* [59] */ {USERNAME, "userName", STRING_PARAMETER}
    };
}

