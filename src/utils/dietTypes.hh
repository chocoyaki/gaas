/****************************************************************************/
/* $Id$ */
/* DIET types specification                                                 */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Frederic LOMBARD          - LIFC Besançon (France)                  */
/*    - Philippe COMBES           - LIP ENS-Lyon (France)                   */
/*                                                                          */
/*  This is part of DIET software.                                          */
/*  Copyright (C) 2002 ReMaP/INRIA                                          */
/*                                                                          */
/****************************************************************************/
/*
 * $Log$
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
 *
 ****************************************************************************/


#ifndef _DIETTYPES_HH_
#define _DIETTYPES_HH_

#include <omnithread.h>

#include "dlist.hh"
#include "types.hh"
#include "SeD.hh"
#include "agent.hh"
#include "DIET_server.h"


/****************************************************
 * All slimfast API types are included in DIET types
 ****************************************************/
#if HAVE_FAST
#include "slimfast_api.h"
#else  // HAVE_FAST
#include "slimfast_api_local.h"
#endif // HAVE_FAST

/*************************/
/********** MAs **********/
/*************************/

struct diet_MA_desc {
  char* name;
  Agent_var ior;
};

typedef struct diet_MA_desc diet_MA_desc_t;
      
class dietMADescListElt : public _dlink {
public:
  diet_MA_desc_t MA;

  dietMADescListElt() {}
  ~dietMADescListElt() {}
};

class dietMADescList : public _dlist {
public:
  omni_mutex write;

  dietMADescList():_dlist() {}
  ~dietMADescList() {}
};

class dietMADescListIterator : public _dlistIterator {
public:
  dietMADescListIterator(dietMADescList *l):_dlistIterator(l) {}
  ~dietMADescListIterator() {}
};

/*************************/
/***** SONS DATABASE *****/
/*************************/

/*------------------------------------------------------------*/
/* Agent's son descriptor. Describes a server/agent and its   */
/* properties. The Corba IOR is meant to be replaced by a     */
/* generic communicator in the final version.                 */
/*------------------------------------------------------------*/

typedef struct {
  int id;
  SeD_var ior;
  char *hostname;
} diet_SeD_desc_t;

typedef struct {
  int id;
  Agent_var ior;
  char *hostname;
} diet_Agent_desc_t;

/*------------------------------------------------------------*/
/* Agent's sons lists.                                        */
/*                                                            */
/* FIXME: beeing copyrighted, dlists should be rewritten.     */
/*------------------------------------------------------------*/

class dietAgentDescListElt : public _dlink {
public:
  diet_Agent_desc_t desc;
  dietAgentDescListElt() {}
  ~dietAgentDescListElt() {}
};

class dietAgentDescList : public _dlist {
public:
  dietAgentDescList():_dlist() {}
  ~dietAgentDescList() {}
};

class dietAgentDescListIterator : public _dlistIterator {
public:
  dietAgentDescListIterator(dietAgentDescList *l):_dlistIterator(l) {}
  ~dietAgentDescListIterator() {}
};

class dietSeDDescListElt : public _dlink {
public:
  diet_SeD_desc_t desc;
  dietSeDDescListElt() {}
  ~dietSeDDescListElt() {}
};

class dietSeDDescList : public _dlist {
public:
  dietSeDDescList():_dlist() {}
  ~dietSeDDescList() {}
};

class dietSeDDescListIterator : public _dlistIterator {
public:
  dietSeDDescListIterator(dietSeDDescList *l):_dlistIterator(l) {}
  ~dietSeDDescListIterator() {}
};


/*******************************/
/***** PROPERTIES DATABASE *****/
/*******************************/

#if 0

/*------------------------------------------------------------*/
/* Agent's sonIds lists.                                      */
/*                                                            */
/* FIXME: beeing copyrighted, dlists should be rewritten.     */
/*------------------------------------------------------------*/

class dietSonIdListElt : public _dlink {
public:
  int sonId;
  int nbCapableServers;

  dietSonIdListElt() {}
  ~dietSonIdListElt() {}
};


class dietSonIdList : public _dlist {
public:
  dietSonIdList():_dlist() {}
  ~dietSonIdList() {}
};

class dietSonIdListIterator : public _dlistIterator {
public:
  dietSonIdListIterator(dietSonIdList *l):_dlistIterator(l) {}
  ~dietSonIdListIterator() {}
};

/*------------------------------------------------------------*/
/* Agent's property descriptor . Describes a problem and      */
/* lists the Ids of all sons that are able to solve it.       */
/*------------------------------------------------------------*/

struct diet_prop_desc_ {
  diet_profile_desc_t *profile;
  dietSonIdList *capableSons;
};
typedef struct diet_prop_desc_ diet_prop_desc_t;

/*------------------------------------------------------------*/
/* Agent's properties lists.                                  */
/*                                                            */
/* FIXME: beeing copyrighted, dlists should be rewritten.     */
/*------------------------------------------------------------*/

class dietPropDescListElt : public _dlink {
public:
  diet_prop_desc_t prop;

  dietPropDescListElt() {}
  ~dietPropDescListElt() {}
};

class dietPropDescList : public _dlist {
public:
  dietPropDescList():_dlist() {}
  ~dietPropDescList() {}
};

class dietPropDescListIterator : public _dlistIterator {
public:
  dietPropDescListIterator(dietPropDescList *l):_dlistIterator(l) {}
  ~dietPropDescListIterator() {}
};

#endif // 0


/************/
/* DECISION */
/************/

#if 0
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
  double *tComm;
};

typedef struct diet_server_comp_ diet_server_comp_t;

struct diet_response_ {
  int reqId;

  int myId;
  int nbIn;
  int nbServers;

  diet_data_loc_t *data;
  diet_server_comp_t *comp;
};

typedef struct diet_response_ diet_response_t;

#endif // 0

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

  corba_response_t *responses; /* this is where responses are gathered */

} diet_request_log_t;


/*------------------------------------------------------------*/
/* Request log structure list :                               */
/* This is the type of the list where an agent stores all its */
/* requests. Write access id protected by an encapsulated     */
/* mutex.                                                     */
/*------------------------------------------------------------*/

class dietRequestLogListElt : public _dlink {
public:
  diet_request_log_t log;

  dietRequestLogListElt() {}
  ~dietRequestLogListElt() {}
};

class dietRequestLogList : public _dlist {
public:
  omni_mutex logMutex;
  dietRequestLogList():_dlist() {}
  ~dietRequestLogList() {}
};

class dietRequestLogListIterator : public _dlistIterator {
public:
  dietRequestLogListIterator(dietRequestLogList *l):_dlistIterator(l) {}
  ~dietRequestLogListIterator() {}
};

#if 0
/*------------------------------------------------------------*/
/* Decision descriptor :                                      */
/* Allows temporary storage of decisions                      */
/*------------------------------------------------------------*/

struct diet_decision_desc_ {
  SeD_var chosenServer; /* Chosen server */

  char *chosenServerName;
  int chosenServerPort;  

  int nbIn;

  diet_data_loc_t *dataLocs; /* Localisation of each parameter */
                             /* (localization==nil means that the parameter was not located, */
                             /* i.e, it's still on the client) */

  char *implPath; /* name of the implementation to call */
};

typedef struct  diet_decision_desc_ diet_decision_desc_t;

struct diet_decision_sequence_ {
  int nbElts;

  diet_decision_desc_t* decisions;
};

typedef diet_decision_sequence_ diet_decision_sequence_t; 

#endif // 0


/*------------------------------------------------------------*/
/* Decision descriptor list :                                 */
/* Stores all decisions in progress in a Master Agent         */
/*------------------------------------------------------------*/

class dietDecisionDescListElt : public _dlink {
public:
  int reqId;
  SeqCorbaDecision_t decision;

  /* This condition variable allow to wake the 'client thread' up */
  /* when the decision is taken :                                 */

  omni_mutex mutex; 

  omni_condition *decisionTaken;

  dietDecisionDescListElt() {}
  ~dietDecisionDescListElt() {}
};

class dietDecisionDescList : public _dlist {
public:
  omni_mutex logMutex;
  dietDecisionDescList():_dlist() {}
  ~dietDecisionDescList() {}
};

class dietDecisionDescListIterator : public _dlistIterator {
public:
  dietDecisionDescListIterator(dietDecisionDescList *l):_dlistIterator(l) {}
  ~dietDecisionDescListIterator() {}
};

/************************/
/* LOCALIZATION SERVICE */
/************************/

/*------------------------------------------------------------*/
/* The following list is used on each agent to store the data */
/* localization. It associates a data identifier (int) which  */
/* must be unique in the system with a son Id. If a data      */
/* identifier is in the list, then this data is in the        */
/* subtree which root is the agent. The sonId field is the ID */
/* of the son under which the data is.                        */
/*------------------------------------------------------------*/

class dietDataLocListElt : public _dlink {
public:
  int dataId;
  int sonId;

  dietDataLocListElt() {}
  ~dietDataLocListElt() {}
};

class dietDataLocList : public _dlist {
public:
  omni_mutex logMutex;
  dietDataLocList():_dlist() {}
  ~dietDataLocList() {}
};

class dietDataLocListIterator : public _dlistIterator {
public:
  dietDataLocListIterator(dietDataLocList *l):_dlistIterator(l) {}
  ~dietDataLocListIterator() {}
};

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


class dietServerDataDescListElt : public _dlink {
public:
  diet_server_data_desc_t *data;
  
  dietServerDataDescListElt() {}
  ~dietServerDataDescListElt() {}
};

class dietServerDataDescList : public _dlist {
public:
  omni_mutex logMutex;
  dietServerDataDescList():_dlist() {}
  ~dietServerDataDescList() {}
};

class dietServerDataDescListIterator : public _dlistIterator {
public:
  dietServerDataDescListIterator(dietServerDataDescList *l):_dlistIterator(l) {}
  ~dietServerDataDescListIterator() {}
};


/****************************************************************************/
/* Useful functions for data descriptors manipulation                       */
/* <implemented in DIET_data.cc>                                            */
/****************************************************************************/

/* Compute size in bytes of data from its descriptor. */
size_t data_sizeof(const diet_data_desc_t *desc);
size_t data_sizeof(const corba_data_desc_t *desc);


/* There should be no use of allocating and freeing functions */

/*----[ Descriptors altering ]----------------------------------------------*/
/* Each -1 (or NULL for pointers) argument does not alter the corresponding
   field. */

int scalar_desc_set(diet_data_desc_t *desc, diet_persistence_mode_t mode,
		    diet_base_type_t base_type, void *value);
int vector_desc_set(diet_data_desc_t *desc, diet_persistence_mode_t mode,
		    diet_base_type_t base_type, size_t size);
int matrix_desc_set(diet_data_desc_t *desc, diet_persistence_mode_t mode,
		    diet_base_type_t base_type, size_t nb_r, size_t nb_c,
		    int istrans);
int string_desc_set(diet_data_desc_t *desc, diet_persistence_mode_t mode,
		    size_t length);
/* Computes the file size */
int file_desc_set(diet_data_desc_t *desc, diet_persistence_mode_t mode,
		  char *path);



/****************************************************************************/
/* Useful functions for profile manipulation                                */
/* <implemented in DIET_data.cc>                                            */
/****************************************************************************/

/* Profile descriptor comparison: as strcmp, return 0 if equal. */
int profile_desc_cmp(const corba_profile_desc_t *p1,
		     const corba_profile_desc_t *p2);

/* Return true if p1 is exactly identical to p2. */
int profile_desc_match(const corba_profile_desc_t *p1,
		       const corba_profile_desc_t *p2);

/* Return true if sv_profile describes a service that matches the problem
   that pb_desc describes. */
int profile_match(const corba_profile_desc_t *sv_profile,
		  const corba_pb_desc_t      *pb_desc);

/* Return true if sv_profile describes a service that matches the problem
   that pb and path describe. */
int profile_match(const corba_profile_desc_t *sv_profile,
		  const char *path, const corba_profile_t *pb);

#endif // _DIETTYPES_HH_





