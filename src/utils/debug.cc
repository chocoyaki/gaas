/****************************************************************************/
/* $Id$ */
/* DIET debug utils source code                                             */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Frederic LOMBARD          - LIFC Besançon (France)                  */
/*                                                                          */
/*  This is part of DIET software.                                          */
/*  Copyright (C) 2002 ReMaP/LIFC/INRIA                                     */
/*                                                                          */
/****************************************************************************/
/*
 * $Log$
 * Revision 1.5  2002/08/30 16:50:16  pcombes
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
 * Revision 1.4  2002/08/09 14:30:34  pcombes
 * This is commit set the frame for version 1.0 - does not work properly yet
 *
 * Revision 1.3  2002/05/24 19:36:54  pcombes
 * Add BLAS/dgemm example (implied bug fixes)
 *
 * Revision 1.2  2002/05/17 20:35:18  pcombes
 * Version alpha without FAST 
 *
 ****************************************************************************/


#include <stdlib.h>
#include <stdio.h>

#include "debug.hh"
#include "dietTypes.hh"


void displayResponse(FILE *os, const corba_response_t *resp)
{
  int i,j;
  char implName[256];

  fprintf(os, "\n----------------------------------------\n");
  fprintf(os, " Response structure for request %ld :\n\n", resp->reqId);
  fprintf(os, " I'm son nb %ld\n", resp->myId);
  fprintf(os, " There are %ld parameters\n", resp->nbIn);
  fprintf(os, " %ld servers are able to solve the problem\n", resp->nbServers);
  fprintf(os, " Data:\n");
  
  for (i = 0; i < resp->nbIn; i++) {
    if (CORBA::is_nil(resp->data[i].localization)) {
      fprintf(os, "  Data %i is not located yet\n", i);
    } else {
      fprintf(os, "  Time2Me for data %i is %f\n",i,resp->data[i].timeToMe);
    }
  }
  
  fprintf(os," Estimated computation time:\n");
  
  for (i = 0; i < resp->nbServers; i++) {
    strcpy(implName, resp->comp[i].implName);
    fprintf(os, "  %ldth server can solve the problem in %f seconds\n",
	    (long) i, resp->comp[i].tComp);
    fprintf(os, "  Implementation name: %s\n", implName);
    fprintf(os, "  TComms for each parameter: ");
    for (j = 0; j < resp->nbIn; j++) {
      fprintf(os, " %f", resp->comp[i].tComm[j]);
    }      
    fprintf(os,"\n");
  }
  fprintf(os, "----------------------------------------\n");
}


void displayMAList(FILE* os,dietMADescList *MAs)
{
  dietMADescListIterator *iter= new dietMADescListIterator(MAs);
  
  while (iter->next())
    {
      fprintf(os,"Name : %s",((dietMADescListElt *)(iter->curr()))->MA.name);
      if (CORBA::is_nil(((dietMADescListElt *)(iter->curr()))->MA.ior))
	fprintf(os," (down)\n");
      else
        fprintf(os,"(up)\n");
    }
}

void displayProfileDesc(const diet_profile_desc_t *profile, const char *path)
{
  FILE *f = stdout;
  fprintf(f, " - Service %s", path);
  for (int j = 0; j <= profile->last_out; j++) {
    fprintf(f, "\n     %s ",
	    (j <= profile->last_in) ? "IN   "
	    : (j <= profile->last_inout) ? "INOUT"
	    : "OUT  ");
    switch((int) profile->param_desc[j].type) {
    case DIET_SCALAR: fprintf(f, "scalar"); break;
    case DIET_VECTOR: fprintf(f, "vector"); break;
    case DIET_MATRIX: fprintf(f, "matrix"); break;
    case DIET_STRING: fprintf(f, "string"); break;
    case DIET_FILE:   fprintf(f, "file");   break;
    }
    if ((profile->param_desc[j].type != DIET_STRING)
	&& (profile->param_desc[j].type != DIET_FILE)) {
      fprintf(f, " of ");
      switch ((int) profile->param_desc[j].base_type) {
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

void displayCorbaProfileDesc(const corba_profile_desc_t *profile)
{
  FILE *f = stdout;
  char *path = CORBA::string_dup(profile->path);
  fprintf(f, " - Service %s", path);
  CORBA::string_free(path);
  for (int j = 0; j <= profile->last_out; j++) {
    fprintf(f, "\n     %s ",
	    (j <= profile->last_in) ? "IN   "
	    : (j <= profile->last_inout) ? "INOUT"
	    : "OUT  ");
    switch(profile->param_desc[j].type) {
    case DIET_SCALAR: fprintf(f, "scalar"); break;
    case DIET_VECTOR: fprintf(f, "vector"); break;
    case DIET_MATRIX: fprintf(f, "matrix"); break;
    case DIET_STRING: fprintf(f, "string"); break;
    case DIET_FILE:   fprintf(f, "file");   break;
    }
    if ((profile->param_desc[j].type != DIET_STRING)
	&& (profile->param_desc[j].type != DIET_FILE)) {
      fprintf(f, " of ");
      switch (profile->param_desc[j].base_type) {
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
  
void displayProfile(const corba_profile_t *profile)
{
  FILE *f = stdout;
  char *path = CORBA::string_dup(profile->path);
  fprintf(f, " - Service %s", path);
  CORBA::string_free(path);
  for (int j = 0; j <= profile->last_out; j++) {
    fprintf(f, "\n     %s ",
	    (j <= profile->last_in) ? "IN   "
	    : (j <= profile->last_inout) ? "INOUT"
	    : "OUT  ");
    switch(profile->param_desc[j].specific._d()) {
    case DIET_SCALAR: fprintf(f, "scalar"); break;
    case DIET_VECTOR: fprintf(f, "vector (%ld)",
			      profile->param_desc[j].specific.vect().size); break;
    case DIET_MATRIX: fprintf(f, "matrix (%ldx%ld)",
			      profile->param_desc[j].specific.mat().nb_r,
			      profile->param_desc[j].specific.mat().nb_c); break;
    case DIET_STRING: fprintf(f, "string (%ld)",
			       profile->param_desc[j].specific.str().length); break;
    case DIET_FILE:   fprintf(f, "file (%ld)",
			      profile->param_desc[j].specific.file().size);   break;
    }
    if ((profile->param_desc[j].specific._d() != DIET_STRING)
	&& (profile->param_desc[j].specific._d() != DIET_FILE)) {
      fprintf(f, " of ");
      switch (profile->param_desc[j].base_type) {
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


