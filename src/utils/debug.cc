/****************************************************************************/
/* $Id$ */
/* DIET debug utils source code                                             */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*    - Frederic LOMBARD (Frederic.Lombard@lifc.univ-fcomte.fr)             */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/*
 * $Log$
 * Revision 1.10  2003/02/04 10:08:23  pcombes
 * Apply Coding Standards
 *
 * Revision 1.9  2003/01/22 17:06:43  pcombes
 * API 0.6.4 : istrans -> order (row- or column-major)
 *
 * Revision 1.8  2002/12/03 19:08:24  pcombes
 * Update configure, update to FAST 0.3.15, clean CVS logs in files.
 * Put main Makefile in root directory.
 *
 * Revision 1.7  2002/11/22 14:49:28  lbertsch
 * Suppressed silly warnings on alpha
 *
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
 ****************************************************************************/


#include <stdio.h>
#include <stdlib.h>

#include "debug.hh"
#include "dietTypes.hh"


void
displayResponse(FILE* os, const corba_response_t* resp)
{
  int i,j;
  char implName[256];

  fprintf(os, "\n----------------------------------------\n");
  fprintf(os, " Response structure for request %ld :\n\n", (long)resp->reqId);
  fprintf(os, " I'm son nb %ld\n", (long)resp->myId);
  fprintf(os, " There are %ld parameters\n", (long)resp->nbIn);
  fprintf(os, " %ld servers are able to solve the problem\n", (long)resp->nbServers);
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


void
displayMAList(FILE* os,dietMADescList* MAs)
{
  dietMADescListIterator* iter= new dietMADescListIterator(MAs);
  
  while (iter->next())
    {
      fprintf(os,"Name : %s",((dietMADescListElt*)(iter->curr()))->MA.name);
      if (CORBA::is_nil(((dietMADescListElt*)(iter->curr()))->MA.ior))
	fprintf(os," (down)\n");
      else
        fprintf(os,"(up)\n");
    }
}

void
displayArgDesc(FILE* f, int type, int base_type) 
{
  switch(type) {
  case DIET_SCALAR: fprintf(f, "scalar"); break;
  case DIET_VECTOR: fprintf(f, "vector"); break;
  case DIET_MATRIX: fprintf(f, "matrix"); break;
  case DIET_STRING: fprintf(f, "string"); break;
  case DIET_FILE:   fprintf(f, "file");   break;
  }
  if ((type != DIET_STRING) && (type != DIET_FILE)) {
    fprintf(f, " of ");
    switch (base_type) {
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

void
displayArg(FILE* f, const corba_data_desc_t* arg)
{
  switch(arg->specific._d()) {
  case DIET_SCALAR: fprintf(f, "scalar");            break;
  case DIET_VECTOR: fprintf(f, "vector (%ld)",
			    (long)arg->specific.vect().size);  break;
  case DIET_MATRIX: fprintf(f, "matrix (%ldx%ld)",
			    (long)arg->specific.mat().nb_r,
			    (long)arg->specific.mat().nb_c);   break;
  case DIET_STRING: fprintf(f, "string (%ld)",
			    (long)arg->specific.str().length); break;
  case DIET_FILE:   fprintf(f, "file (%ld)",
			    (long)arg->specific.file().size);  break;
  }
  if ((arg->specific._d() != DIET_STRING)
      && (arg->specific._d() != DIET_FILE)) {
    fprintf(f, " of ");
    switch (arg->base_type) {
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

void
displayArg(FILE* f, const diet_data_desc_t* arg)
{
  switch((int) arg->generic.type) {
  case DIET_SCALAR: fprintf(f, "scalar");                break;
  case DIET_VECTOR: fprintf(f, "vector (%ld)",
			    (long)arg->specific.vect.size);    break;
  case DIET_MATRIX: fprintf(f, "matrix (%ldx%ld)",
			    (long)arg->specific.mat.nb_r,
			    (long)arg->specific.mat.nb_c);   break;
  case DIET_STRING: fprintf(f, "string (%ld)",
			    (long)arg->specific.str.length); break;
  case DIET_FILE:   fprintf(f, "file (%ld)",
			    (long)arg->specific.file.size);  break;
  }
  if ((arg->generic.type != DIET_STRING)
      && (arg->generic.type != DIET_FILE)) {
    fprintf(f, " of ");
    switch ((int) arg->generic.base_type) {
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


void
displayProfileDesc(const diet_profile_desc_t* desc, const char* path)
{
  FILE* f = stdout;
  fprintf(f, " - Service %s", path);
  for (int i = 0; i <= desc->last_out; i++) {
    fprintf(f, "\n     %s ",
	    (i <= desc->last_in) ? "IN   "
	    : (i <= desc->last_inout) ? "INOUT"
	    : "OUT  ");
    displayArgDesc(f, desc->param_desc[i].type, desc->param_desc[i].base_type);
  }
  fprintf(f, "\n");
}


void
displayProfileDesc(const corba_profile_desc_t* desc)
{
  FILE* f = stdout;
  char* path = CORBA::string_dup(desc->path);
  fprintf(f, " - Service %s", path);
  CORBA::string_free(path);
  for (int j = 0; j <= desc->last_out; j++) {
    fprintf(f, "\n     %s ",
	    (j <= desc->last_in) ? "IN   "
	    : (j <= desc->last_inout) ? "INOUT"
	    : "OUT  ");
    displayArgDesc(f, desc->param_desc[j].type, desc->param_desc[j].base_type);
  }
  fprintf(f, "\n");
  free(path);
}
  
void
displayProfile(const diet_profile_t* profile, const char* path) 
{
  FILE* f = stdout;
  fprintf(f, " - Service %s", path);
  for (int i = 0; i <= profile->last_out; i++) {
    fprintf(f, "\n     %s ",
	    (i <= profile->last_in) ? "IN   "
	    : (i <= profile->last_inout) ? "INOUT"
	    : "OUT  ");
    displayArg(f, &(profile->parameters[i].desc));
  }
  fprintf(f, "\n");  
}

void
displayProfile(const corba_profile_t* profile, const char* path)
{
  FILE* f = stdout;
  fprintf(f, " - Service %s", path);
  for (int i = 0; i <= profile->last_out; i++) {
    fprintf(f, "\n     %s ",
	    (i <= profile->last_in) ? "IN   "
	    : (i <= profile->last_inout) ? "INOUT"
	    : "OUT  ");
    displayArg(f, &(profile->parameters[i].desc));
  }
  fprintf(f, "\n");  
}

void
displayPbDesc(const corba_pb_desc_t* profile)
{
  FILE* f = stdout;
  char* path = CORBA::string_dup(profile->path);
  fprintf(f, " - Service %s", path);
  CORBA::string_free(path);
  for (int j = 0; j <= profile->last_out; j++) {
    fprintf(f, "\n     %s ",
	    (j <= profile->last_in) ? "IN   "
	    : (j <= profile->last_inout) ? "INOUT"
	    : "OUT  ");
    displayArg(f, &(profile->param_desc[j]));
  }  
  fprintf(f, "\n");
}

void
displayConvertor(FILE* f, const diet_convertor_t* cvt)
{
  fprintf(f, " - Convertor to %s", cvt->path);
  for (int i = 0; i <= cvt->last_out; i++) {
    fprintf(f, "\n     %s ",
	    (i <= cvt->last_in) ? "IN   "
	    : (i <= cvt->last_inout) ? "INOUT"
	    : "OUT  ");
    switch((int)cvt->arg_convs[i].f) {
    case DIET_CVT_IDENTITY:   fprintf(f, "IDENT  of "); break;
    case DIET_CVT_FILE_SIZE:
    case DIET_CVT_VECT_SIZE:  fprintf(f, "SIZE   of "); break;
    case DIET_CVT_MAT_NB_ROW: fprintf(f, "NB_ROW of "); break;
    case DIET_CVT_MAT_NB_COL: fprintf(f, "NB_COL of "); break;
    case DIET_CVT_MAT_ORDER:  fprintf(f, "ORDER  of "); break;
    case DIET_CVT_STR_LEN:    fprintf(f, "LENGTH of "); break;
    }
    if (cvt->arg_convs[i].arg)
      displayArg(f, &(cvt->arg_convs[i].arg->desc));
    else
      fprintf(f, "argument %d", cvt->arg_convs[i].arg_idx);
  }
  fprintf(f, "\n");  
}
