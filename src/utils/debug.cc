/****************************************************************************/
/* $Id$                  */
/* DIET debug utils source code                                             */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Frederic LOMBARD          - LIFC Besançon (France)                  */
/*                                                                          */
/*  This is part of DIET software.                                          */
/*  Copyright (C) 2002 ReMaP/LIFC/INRIA                                     */
/*                                                                          */
/****************************************************************************/

/* $Log$
/* Revision 1.1.1.1  2002/05/16 10:49:09  pcombes
/* Set source tree for DIET alpha version
/* */


#include <stdio.h>

#include "debug.hh"

#include "dietTypes.hh"
#include "SeD.hh"

void displayResponse(FILE *os,const diet_response_t *resp)
{
  int i,j;

  fprintf(os,"Response structure for request %i :\n\n",resp->reqId);

  fprintf(os,"I'm son nb %i\n",resp->myId);

  fprintf(os,"There are %i parameters\n",resp->nbIn);

  fprintf(os,"%i servers are able to solve the problem\n",resp->nbServers);

  fprintf(os,"Data :\n");
  
  for (i=0;i<resp->nbIn;i++)
    {
      if (CORBA::is_nil(resp->data[i].localization))
	{
	  fprintf(os,"Data %i is not located yet\n",i);
	}
      else
	{
	  fprintf(os,"Time2Me for data %i is %f\n",i,resp->data[i].timeToMe);
	}
    }
  
  fprintf(os,"\n");

  fprintf(os,"Comp :\n");
  
  for (i=0;i<resp->nbServers;i++)
    {    
      fprintf(os,"This server can solve the problem in %f seconds\n",resp->comp[i].tComp);

      fprintf(os,"Implementation name: %s\n",resp->comp[i].implName);

      fprintf(os,"TComms for each parameter :\n");
      for (j=0;j<resp->nbIn;j++)
	{
	  fprintf(os,"%f ",resp->comp[i].tComm[j]);
	  
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

