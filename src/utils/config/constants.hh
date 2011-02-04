#ifndef _CONSTANTS_HH_
#define _CONSTANTS_HH_

#include <string>

namespace diet {
    typedef enum {
	NONE_PARAMETER,
	INT_PARAMETER,
	STRING_PARAMETER,
	ADDRESS_PARAMETER,
	AGENT_PARAMETER, // like int
	ULONG_PARAMETER
    } c_type_t ;

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
#ifdef USERSCHED
	MODULENAME,
	MODULECFG,
#endif /* USERSCHED */
#ifdef HAVE_ALT_BATCH
	BATCHNAME,
	BATCHQUEUE,
	PATHTONFS,
	PATHTOTMP,
#endif /* HAVE_ALT_BATCH */
	// RequestID begins with this value
	INITREQUESTID,
#ifdef HAVE_ACKFILE
	// file to touch at the end of the initialization
	ACKFILE,
#endif /* HAVE_ACKFILE */
#if HAVE_DAGDA
	MAXMSGSIZE,
	MAXDISKSPACE,
	MAXMEMSPACE,
	CACHEALGORITHM,
	SHAREFILES,
	DATABACKUPFILE,
	RESTOREONSTART,
#endif /* HAVE_DAGDA */
#if HAVE_DAGDA || HAVE_ALT_BATCH
	STORAGEDIR,
#endif /* HAVE_DAGDA || HAVE_DAGDA */
#ifdef HAVE_CCS
	USE_SPECIFIC_SCHEDULING,
#endif /* HAVE_CCS */
#ifdef HAVE_ALT_BATCH
	INTERNOARQUEUENAME,
#endif /* HAVE_ALT_BATCH */
	CLIENT_MAX_NB_SED,
#ifdef HAVE_CLOUD
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
#endif /* HAVE_CLOUD */
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
