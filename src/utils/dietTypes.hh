/****************************************************************************/
/* DIET types specification                                                 */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*    - Frederic LOMBARD (Frederic.Lombard@lifc.univ-fcomte.fr)             */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.12  2003/09/19 16:07:09  bdelfabr
 * removing dlist from the diettype files. THe removing is not done on DataManager,
 * but now you can compile the others stuff
 *
 * Revision 1.11  2003/09/18 09:47:19  bdelfabr
 * adding data persistence
 *
 * Revision 1.10  2003/04/10 12:49:27  pcombes
 * Apply Coding Standards, manage data ID, and remove all dlist types.
 *
 * Revision 1.9  2003/02/04 09:59:01  pcombes
 * Apply Coding Standards (still much work)
 *
 * Revision 1.8  2003/01/22 17:06:43  pcombes
 * API 0.6.4 : istrans -> order (row- or column-major)
 *
 * Revision 1.7  2003/01/17 18:08:43  pcombes
 * New API (0.6.3): structures are not hidden, but the user can ignore them.
 *
 * Revision 1.6  2002/12/03 19:08:24  pcombes
 * Update configure, update to FAST 0.3.15, clean CVS logs in files.
 * Put main Makefile in root directory.
 *
 * Revision 1.2  2002/08/30 16:50:16  pcombes
 * This version works as well as the alpha version from the user point of view,
 * but the API is now the one imposed by the latest specifications (GridRPC API
 * in its sequential part, config file for all parts of the platform, agent
 * algorithm, etc.)
 *  - Reduce marshalling by using CORBA types internally
 *  - Creation of a class ServiceTable that is to be replaced
 *    by an LDAP DB for the MA
 *  - No copy for client/SeD data transfers
 *  - ...
 ****************************************************************************/


#ifndef _DIETTYPES_HH_
#define _DIETTYPES_HH_

#include <omnithread.h>

#include "common_types.hh"
#include "DIET_config.h"
#include "DIET_server.h"

#include "SeD.hh"
#include "Agent.hh"
#include "dataMgr.hh"
#if 0

/****************************************************
 * All slimfast API types are included in DIET types
 ****************************************************/
#if HAVE_FAST
#include "slimfast_api.h"
#else  // HAVE_FAST
#include "slimfast_api_local.h"
#endif // HAVE_FAST


/************/
/* DECISION */
/************/

/*------------------------------------------------------------*/
/* Request and response structures :                          */
/* These are the standard structures for communication        */
/* between agents and SeDs during ressource localization      */
/* and performances evaluation,                               */
/* The response structure is not sockets compliant for now.   */
/* SeD IORs should be replaced by generic communicators.      */
/*------------------------------------------------------------*/

/*------------------------------------------------------*/
/* Rem: The role field of the sf_pb_desc_t structure    */
/* is not used in this context. It may be used to store */
/* the data unique identifier (for the localization     */
/* service. Using an int as identifier should speed up  */
/* the lookups in the localization table.               */
/*------------------------------------------------------*/

typedef struct {
  int reqId;
  sf_inst_desc_t pb_profile;
} diet_request_t;

struct diet_data_loc_ {
  SeD_var localization;
  char* hostname;
  int port;

  double timeToMe;
};

typedef struct diet_data_loc_ diet_data_loc_t;

struct diet_server_comp_ {
  SeD_var myRef;
  char* hostname;
  int port;

  char* implName;
  double tComp;
  double* tComm;
};

typedef struct diet_server_comp_ diet_server_comp_t;

struct diet_response_ {
  int reqId;

  int myId;
  int nbIn;
  int nbServers;

  diet_data_loc_t* data;
  diet_server_comp_t* comp;
};

typedef struct diet_response_ diet_response_t;


/*------------------------------------------------------------*/
/* Request log structure :                                    */
/* This structure describe the state of a request that is     */
/* beeing processed. All responses are included in the log    */
/* of a request. The log structure includes a mutex that      */
/* allow different threads to access it. A condition variable */
/* allows to wake up the main thread of this request when all */
/* responses are gathered.                                    */
/* This structure is not meant to be sent to another process. */
/* It thus has no Corba equivalent.                           */
/*------------------------------------------------------------*/

typedef struct {
  int reqId;           /* Request ID */

  corba_request_t req;  /* A copy of the request */

  int nbSons;          /* how many sons where contacted */

  omni_mutex respMutex; /* responses gathering mutual exclusion */

  omni_condition *gatheringEnded;

  int nbResp;          /* how many respomnses where gathered for now */

  corba_response_t* responses; /* this is where responses are gathered */

} diet_request_log_t;


/*------------------------------------------------------------*/
/* These structures are used by a server to keep a list of    */
/* the datas it owns. The diet_server_data_desc_t just        */
/* contains the data ID for now. It should be extended later  */
/* to include informations about he data type, size and       */
/* distribution.                                              */
/*------------------------------------------------------------*/

struct diet_server_data_desc_
{
  long id;
  diet_data_t data;
};

typedef struct diet_server_data_desc_ diet_server_data_desc_t;

#endif // 0

/*------------------------------------------------------------*/
/* These structures are used by a server to keep a list of    */
/* the datas it owns. The diet_server_data_desc_t just        */
/* contains the data ID for now. It should be extended later  */
/* to include informations about he data type, size and       */
/* distribution.                                              */
/*------------------------------------------------------------*/

struct dietDataDesc
{
  corba_dataMgr_desc_t dataDesc;
};

typedef struct dietDataDesc dietDataDescT;



/***********************************************************************
 * These structure are used by the Data object to store the reference 
 * of the data hold by the server
 *
 ***********************************************************************/

struct dietDataId
{
   corba_data_id_t dataId;
};

typedef struct dietDataId dietDataIdT;


/* structure useful for transfert management */

struct dietDataIdLock
{
  char *id;
  omni_mutex lockMutex;
};

typedef struct dietDataIdLock dietDataIdLockT;



/****************************************************************************/
/* Useful functions for data descriptors manipulation                       */
/* <implemented in DIET_data.cc>                                            */
/****************************************************************************/

/* Compute size in bytes of data from its descriptor. */
size_t
data_sizeof(const diet_data_desc_t* desc);
size_t
data_sizeof(const corba_data_desc_t* desc);
size_t
data_sizeof(const corba_dataMgr_desc_t* desc);


/* There should be no use of allocating and freeing functions */

/*----[ Descriptors altering ]----------------------------------------------*/
/* Each -1 (or NULL for pointers) argument does not alter the corresponding
   field. */

int
scalar_set_desc(diet_data_desc_t* desc, char* const id,
		const diet_persistence_mode_t mode,
		const diet_base_type_t base_type, void* const value);
int
vector_set_desc(diet_data_desc_t* desc, char* const id,
		const diet_persistence_mode_t mode,
		const diet_base_type_t base_type, const size_t size);
int
matrix_set_desc(diet_data_desc_t* desc, char* const id,
		const diet_persistence_mode_t mode,
		const diet_base_type_t base_type, const size_t nb_r,
		const size_t nb_c, const diet_matrix_order_t order);
int
string_set_desc(diet_data_desc_t* desc, char* const id,
		const diet_persistence_mode_t mode, const size_t length);
/* Computes the file size */
int
file_set_desc(diet_data_desc_t* desc, char* const id,
	      const diet_persistence_mode_t mode, char* const path);



/****************************************************************************/
/* Useful functions for profile manipulation                                */
/* <implemented in DIET_data.cc>                                            */
/****************************************************************************/

/* Profile descriptor comparison: as strcmp, return 0 if equal. */
int
profile_desc_cmp(const corba_profile_desc_t* p1,
		 const corba_profile_desc_t* p2);

/* Return true if p1 is exactly identical to p2. */
int
profile_desc_match(const corba_profile_desc_t* p1,
		   const corba_profile_desc_t* p2);

/* Return true if sv_profile describes a service that matches the problem
   that pb_desc describes. */
int
profile_match(const corba_profile_desc_t* sv_profile,
	      const corba_pb_desc_t*      pb_desc);

/* Return true if sv_profile describes a service that matches the problem
   that pb and path describe. */
int
profile_match(const corba_profile_desc_t* sv_profile,
	      const char* path, const corba_profile_t* pb);

#endif // _DIETTYPES_HH_
