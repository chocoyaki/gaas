/****************************************************************************/
/* $Id$ */
/* DIET service table header (this is used by agents and SeDs)              */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES           - LIP ENS-Lyon (France)                   */
/*                                                                          */
/*  This is part of DIET software.                                          */
/*  Copyright (C) 2002 ReMaP/INRIA                                          */
/*                                                                          */
/****************************************************************************/
/*
 * $Log$
 * Revision 1.3  2002/10/03 17:58:21  pcombes
 * Add trace levels (for Bert): traceLevel = n can be added in cfg files.
 * An agent son can now be killed (^C) without crashing this agent.
 * DIET with FAST: compilation is OK, but run time is still to be fixed.
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
 * Revision 1.1  2002/08/09 14:30:34  pcombes
 * This is commit set the frame for version 1.0 - does not work properly yet
 *
 ****************************************************************************/

#ifndef _SERVICETABLE_HH_
#define _SERVICETABLE_HH_

#include "DIET_server.h"

#include <stdio.h>
#include "types.hh"



/* This class should implement an STL "map" or "multimap" container.
   But I am not that performant with STL, so C++ experts, help ! */

/* This is initial numbers of sons (for an agent) and offered services.
   They might be configured at compilation time or set at runtime. */
#define MAX_NB_SONS 10
#define MAX_NB_SERVICES 20

class ServiceTable
{
  
public:
  typedef int ServiceReference_t;
  typedef struct {
    int  nb_sons;
    int *sons;
  } matching_sons_t;

  // Equivalent to ServiceTable(MAX_NB_SERVICES, MAX_NB_SONS)
  ServiceTable();
  // Equivalent to ServiceTable(max_nb_services, 0)  
  ServiceTable(int max_nb_services);
  // Allocate memory with given inital numbers, but:
  //  - the solvers part of the table is nil if max_nb_sons > 0
  //  - the matching sons part is nil either
  ServiceTable(int max_nb_services, int max_nb_sons);
  virtual ~ServiceTable();
  
  int  maxSize();
  
  ServiceReference_t lookupService(const corba_profile_desc_t *profile);
  ServiceReference_t lookupService(const corba_profile_t *profile);

  // All data structures are duplicated in add methods
  int addService(const corba_profile_desc_t *profile, diet_solve_t solver);
  int addService(const corba_profile_desc_t *profile, int son);

  int rmService(const corba_profile_desc_t *profile);
  int rmService(const ServiceReference_t ref);
  int     rmSon(const int son);

  // Return a pointer to a copy of all profiles.
  // Caller is responsible for freeing the result.
  SeqCorbaProfileDesc_t *getProfiles();
  diet_solve_t   getSolver(const corba_profile_desc_t *profile);
  diet_solve_t   getSolver(const ServiceReference_t ref);
  matching_sons_t *getSons(const corba_profile_desc_t *profile);
  matching_sons_t *getSons(const ServiceReference_t ref);
  
  void dump(FILE *f);

  int max_nb_sons;
  int traceLevel;

private:
  
  // number of couples {service,profile} in the table
  int nb_s, max_nb_s, max_nb_s_step;
  // max number of capable sons for one service
  // array of name and generic data description (a profile description)
  SeqCorbaProfileDesc_t profiles;
  // array of solving functions 
  diet_solve_t *solvers;
  // array of int arrays: each element is an array of sons ID, which offer the
  // corresponding service
  matching_sons_t *matching_sons;

  // private methods
  inline int ServiceTableInit(int max_nb_services, int max_nb_sons);

};

#endif // _SERVICETABLE_HH_



