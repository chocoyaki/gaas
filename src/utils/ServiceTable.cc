/****************************************************************************/
/* $Id$ */
/* DIET service table source code (this is used by agents and SeDs)         */
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


#include "ServiceTable.hh"
#include <iostream.h>
#include <stdlib.h>
#include <string.h>

#include "debug.hh"
#include "dietTypes.hh"



ServiceTable::ServiceTable() 
{
  ServiceTableInit(MAX_NB_SERVICES, MAX_NB_SONS);
}

ServiceTable::ServiceTable(int max_nb_services) 
{
  ServiceTableInit(max_nb_services, 0);
}

ServiceTable::ServiceTable(int max_nb_services, int max_nb_sons)
{
  ServiceTableInit(max_nb_services, max_nb_sons);
}

ServiceTable::~ServiceTable()
{
  cerr << "~ServiceTable\n";
  if (solvers) {
    for (int i = 0; i < max_nb_s; i++) {
      profiles[i].param_desc.length(0);
    }
    free(solvers);
  } else if (matching_sons) {
    for (int i = 0; i < max_nb_s; i++) {
      profiles[i].param_desc.length(0);
      free(matching_sons[i].sons);
    }
    free(matching_sons);
  }
  profiles.length(0);
}

int ServiceTable::maxSize()
{
  return max_nb_s;
}

ServiceTable::ServiceReference_t
ServiceTable::lookupService(const corba_profile_desc_t *sv_profile)
{
  int i = 0;
  for (; (i < nb_s) && (profile_desc_cmp(&(profiles[i]), sv_profile)); i++);
  return (ServiceReference_t) ((i == nb_s) ? -1 : i);
}
  
ServiceTable::ServiceReference_t
ServiceTable::lookupService(const corba_profile_t *pb_profile)
{
  int i = 0;
  for (; (i < nb_s) && (!profile_match(&(profiles[i]), pb_profile)); i++);
  return (ServiceReference_t) ((i == nb_s) ? -1 : i);
}


int ServiceTable::addService(const corba_profile_desc_t *profile,
			     diet_solve_t solver)
{
  ServiceReference_t service_idx;
  
  if (matching_sons) {
    cerr << "ServiceTable::addService - Attempting to add a service with\n"
	 << "                    solver in a table initialized with sons.\n";
    return 1;
  }

  if ((service_idx = lookupService(profile)) == -1) {
    // Then add a brand new service
    if (nb_s == max_nb_s) {
      max_nb_s += max_nb_s_step;
      profiles.length(max_nb_s);
      solvers  = (diet_solve_t *)
	realloc(solvers,  max_nb_s * sizeof(diet_solve_t));
      for (int i = nb_s; i < max_nb_s; i++) {
	profiles[i].param_desc.length(0);
	solvers[i] = NULL;
      }
    }
    profiles[nb_s] = *profile;
    //profiles[nb_s].path = CORBA::string_dup(profile->path);
    solvers[nb_s] = solver;
    nb_s++;

  } else if (solver == solvers[service_idx]) {
    return -1;
    // service is already in table
  } else {
    cerr << "ServiceTable::addService - Attempting to add 2 services with\n"
	 << "           same path and profile, but with different solvers.\n";
    return 1;
  }
  return 0;
}


int ServiceTable::addService(const corba_profile_desc_t *profile, int son)
{
  ServiceReference_t service_idx;
  
  if (solvers) {
    cerr << "ServiceTable::addService - Attempting to add a service with\n"
	 << "                    son in a table initialized with solvers.\n";
    return 1;
  }

  if ((service_idx = lookupService(profile)) == -1) {
    // Then add a brand new service
    if ((nb_s != 0) && (nb_s % max_nb_s) == 0) {
      max_nb_s += max_nb_s_step;
      profiles.length(max_nb_s);
      matching_sons = (matching_sons_t *)
	realloc(matching_sons, max_nb_s * sizeof(matching_sons_t));
      for (int i = nb_s; i < max_nb_s; i++) {
	profiles[i].param_desc.length(0);
	matching_sons[i].nb_sons = 0;
	matching_sons[i].sons    = new int[max_nb_sons];
      }
    }
    profiles[nb_s] = *profile;
    matching_sons[nb_s].sons[matching_sons[nb_s].nb_sons++] = son;
    nb_s++;
  
  } else {
    int nb_sons = matching_sons[service_idx].nb_sons;
    int *sons   = matching_sons[service_idx].sons;
    
    for (int i = 0; i < nb_sons; i++)
      if (sons[i] == son)
	return -1; // service already associated to son
    // Here, we must add the son in matching_sons[service_idx].sons
    if ((nb_sons % max_nb_sons) == 0) {
      // Then realloc sons array
      sons = (int *) realloc(sons, (nb_sons + max_nb_sons) * sizeof(int));
      matching_sons[service_idx].sons = sons;
    }
    sons[nb_sons] = son;
    matching_sons[service_idx].nb_sons++;
  }
  return 0;
}



int ServiceTable::rmService(const corba_profile_desc_t *profile)
{
  ServiceReference_t ref;
  
  if ((ref = lookupService(profile)) == -1) {
    cerr << "ServiceTable::rmService - Attempting to rm a service "
	 << "that is not in table.\n";
    return 1;
  }
  return rmService(ref);
}


int ServiceTable::rmService(const ServiceReference_t ref)
{
  int i;

  if (((int) ref < 0) || ((int) ref >= nb_s)) {
    cerr << "ServiceTable::rmService - Wrong service reference.\n";
    return 1;
  }
  
  if (solvers) {
    profiles[ref].param_desc.length(0);
    for (i = (int) ref; i < (nb_s - 1); i++) {
      profiles[i] = profiles[i+1];
      solvers[i]  = solvers[i+1];
    }
    profiles[i].param_desc.length(0);
    solvers[i] = NULL;

  } else {
    profiles[ref].param_desc.length(0);
    free(matching_sons[ref].sons);
    for (i = (int) ref; i < (nb_s - 1); i++) {
      profiles[i] = profiles[i+1];
      matching_sons[i] = matching_sons[i+1];
    }
    profiles[i].param_desc.length(0);
    matching_sons[i].nb_sons = 0;
    matching_sons[i].sons = new int[max_nb_sons];

  }
  
  nb_s--;
  return 0;
}


int ServiceTable::rmSon(const int son)
{
  ServiceReference_t ref;
  
  if (solvers) {
    cerr << "ServiceTable::rmSon - Attempting to remove a son from\n"
	 << "                     a table initialized with solvers.\n";
    return 1;
  }
  for (ref = 0; ref < nb_s; ref++) {
    int i;
    for (i = 0; ((i < matching_sons[ref].nb_sons)
		 && (matching_sons[ref].sons[i] != son)); i++);
    if (i < matching_sons[ref].nb_sons) {
      for (int j = i; j < matching_sons[ref].nb_sons; j++){
	matching_sons[ref].sons[j] = matching_sons[ref].sons[j+1];
      }
      // Remove service if there is no son is left
      if ((--(matching_sons[ref].nb_sons)) == 0)
	rmService(ref);
    }
  }
  return 0;
}

SeqCorbaProfileDesc_t *ServiceTable::getProfiles()
{
  SeqCorbaProfileDesc_t *res = new SeqCorbaProfileDesc_t(nb_s);
  res->length(nb_s);
  for (int i = 0; i < nb_s; i++) {
    (*res)[i] = profiles[i];
  }
  return res;
}


diet_solve_t ServiceTable::getSolver(const corba_profile_desc_t *profile)
{
  ServiceReference_t ref;
  
  if ((ref = lookupService(profile)) == -1) {
    cerr << "ServiceTable::getSons - Attempting to get solver\n"
	 << "               of a service that is not in table.\n";
    return NULL;
  }
  return getSolver(ref);
}


diet_solve_t ServiceTable::getSolver(const ServiceReference_t ref)
{
  if (!solvers) {
    cerr << "ServiceTable::getSolver - Attempting to get a solver\n"
	 << "                    in a table initialized with sons.\n";
    return NULL;
  }
  if (((int) ref < 0) || ((int) ref >= nb_s)) {
    cerr << "ServiceTable::getSolver - Wrong service reference.\n";
    return NULL;
  }
  return solvers[ref];
}


ServiceTable::matching_sons_t *
ServiceTable::getSons(const corba_profile_desc_t *profile)
{
  ServiceReference_t ref;
  
  if ((ref = lookupService(profile)) == -1) {
    cerr << "ServiceTable::getSons - Attempting to get sons\n"
	 << "             of a service that is not in table.\n";
    return NULL;
  }
  return getSons(ref);
}


ServiceTable::matching_sons_t *ServiceTable::getSons(const ServiceReference_t ref)
{
  if (solvers) {
    cerr << "ServiceTable::getSolver - Attempting to get sons\n"
	 << "             in a table initialized with solvers.\n";
    return NULL;
  }
  if (((int) ref < 0) || ((int) ref >= nb_s)) {
    cerr << "ServiceTable::getSons - Wrong service reference.\n";
    return NULL;
  }
  return &(matching_sons[ref]);
}


void ServiceTable::dump(FILE *f)
{
  char path[257];
  
  // FIXME: stat the FILE to check if write OK (take care that this method can
  // be called on stdout !

  fprintf(f, "\n--------------------------------------------------\n");
  fprintf(f,   "Service Table (%d services)\n", nb_s);
  fprintf(f,   "--------------------------------------------------\n\n");

  for (int i = 0; i < nb_s; i++) {
    strcpy(path, profiles[i].path);
    fprintf(f, "- Service %s", path);

    if (matching_sons) {
      fprintf(f, " offered by ");
      if ((matching_sons[i].nb_sons <= 0)
	  || ((matching_sons[i].nb_sons == 1)
	      && (matching_sons[i].sons[0] == -1)))
	fprintf(f, "no son\n");
      else {
	if (matching_sons[i].nb_sons == 1)
	  fprintf(f, "son %d", matching_sons[i].sons[0]);
	else {
	  fprintf(f, "sons %d", matching_sons[i].sons[0]);
	  for (int j = 1; j < (matching_sons[i].nb_sons - 1); j++)
	    fprintf(f, ", %d", matching_sons[i].sons[j]);
	  fprintf(f, " and %d.",
		  matching_sons[i].sons[matching_sons[i].nb_sons - 1]);
	}
      }
    }

    for (int j = 0; j <= profiles[i].last_out; j++) {
      fprintf(f, "\n     %s ",
	      (j <= profiles[i].last_in) ? "IN   "
	      : (j <= profiles[i].last_inout) ? "INOUT"
	      : "OUT  ");
      switch(profiles[i].param_desc[j].type) {
      case DIET_SCALAR: fprintf(f, "scalar"); break;
      case DIET_VECTOR: fprintf(f, "vector"); break;
      case DIET_MATRIX: fprintf(f, "matrix"); break;
      case DIET_STRING: fprintf(f, "string"); break;
      case DIET_FILE:   fprintf(f, "file");   break;
      }
      if ((profiles[i].param_desc[j].type != DIET_STRING)
	  && (profiles[i].param_desc[j].type != DIET_FILE)) {
	fprintf(f, " of ");
	switch (profiles[i].param_desc[j].base_type) {
	case DIET_CHAR:     fprintf(f, "char");           break;
	case DIET_BYTE:     fprintf(f, "byte");           break;
	case DIET_INT:      fprintf(f, "int");            break;
	case DIET_LONGINT:  fprintf(f, "long int");       break;
	case DIET_FLOAT:    fprintf(f, "float");          break;
	case DIET_DOUBLE:   fprintf(f, "double");         break;
	case DIET_SCOMPLEX: fprintf(f, "float complex");  break;
	case DIET_DCOMPLEX: fprintf(f, "double complex"); break;
	}
      }
    }
    fprintf(f, "\n");
  }
}


int ServiceTable::ServiceTableInit(int max_nb_services, int max_nb_sons)
{
  nb_s              = 0;
  max_nb_s          = (max_nb_services <= 0) ? MAX_NB_SERVICES : max_nb_services;
  max_nb_s_step     = max_nb_s;
  this->max_nb_sons = (max_nb_sons < 0)      ? 0 : max_nb_sons;
  profiles.length(max_nb_s);

  if (max_nb_sons > 0) {
    solvers = NULL;
    matching_sons = new matching_sons_t[max_nb_s];
    for (int i = 0; i < max_nb_s; i++) {
      profiles[i].param_desc.length(0);
      matching_sons[i].nb_sons = 0;
      matching_sons[i].sons    = new int[max_nb_sons];
    }
  } else {
    solvers  = new diet_solve_t[max_nb_s];
    for (int i = 0; i < max_nb_s; i++) {
      profiles[i].param_desc.length(0);
      solvers[i] = NULL;
    }
    matching_sons = NULL;
  }
  return 0;
}
  
