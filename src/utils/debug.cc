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

  fprintf(os, "Response structure for request %ld :\n\n", resp->reqId);
  fprintf(os, "I'm son nb %ld\n", resp->myId);
  fprintf(os, "There are %ld parameters\n", resp->nbIn);
  fprintf(os, "%ld servers are able to solve the problem\n", resp->nbServers);
  fprintf(os, "Data :\n");
  
  for (i = 0; i < resp->nbIn; i++) {
    if (CORBA::is_nil(resp->data[i].localization)) {
      fprintf(os, "Data %i is not located yet\n", i);
    } else {
      fprintf(os, "Time2Me for data %i is %f\n",i,resp->data[i].timeToMe);
    }
  }
  
  fprintf(os,"\nComp :\n");
  
  for (i = 0; i < resp->nbServers; i++) {
    strcpy(implName, resp->comp[i].implName);
    fprintf(os, "This server can solve the problem in %f seconds\n",
	    resp->comp[i].tComp);
    fprintf(os, "Implementation name: %s\n", implName);
    fprintf(os, "TComms for each parameter :\n");
    for (j = 0; j < resp->nbIn; j++) {
      fprintf(os, "%f ", resp->comp[i].tComm[j]);
    }      
    fprintf(os,"\n");
  }
  fprintf(os,"\n\n");
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


