/****************************************************************************/
/* $Id$          */
/* DIET CORBA marshalling header                                            */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Frederic LOMBARD          - LIFC Besançon (France)                  */
/*    - Philippe COMBES           - LIP ENS Lyon (France)                   */
/*                                                                          */
/*  This is part of DIET software.                                          */
/*  Copyright (C) 2002 ReMaP/INRIA                                          */
/*                                                                          */
/****************************************************************************/
/*
 * $Log$
 * Revision 1.2  2002/05/17 20:35:16  pcombes
 * Version alpha without FAST
 *
 */


#ifndef _MARSHALLING_HH_
#define _MARSHALLING_HH_

#include "DIET.h"
#include "dietTypes.hh"
#include "types.hh"

/*-------------------------------------------------------------------------*/
/* These functions allow to copy the content of diet structures between    */
/* Corba and standard versions. Whenever it's possible, no data should be  */
/* shared between the two copies. This should prevent the Corba stub/skel  */
/* from deallocating usefull datas.                                        */
/*                                                                         */
/* Standars format: [srcLayer]2[destLayer][type](dest,src);                */
/*-------------------------------------------------------------------------*/

/* Conversion for data descriptors */
void
sf2corbaDataDesc(corba_data_desc_t *dest, const sf_data_desc_t *src);
void
corba2sfDataDesc(sf_data_desc_t *dest, const corba_data_desc_t *src);

/* Conversion for data themselves */
void
diet2corbaData(corba_data_t *dest, const diet_data_t *src);
void
corba2dietData(diet_data_t *dest, const corba_data_t *src);
// Both following functions allocate dest sequence
void
diet2corbaDataSeq(SeqCorba_data_t *dest, const diet_data_seq_t *src);
void
corba2dietDataSeq(diet_data_seq_t *dest, const SeqCorba_data_t *src);

/* Conversion for a pb_desc */
void
sf2corbaPbDesc(corba_pb_desc_t *dest, const sf_pb_desc_t *src);
void
corba2sfPbDesc(sf_pb_desc_t *dest, const corba_pb_desc_t *src);

/* Conversion for an inst_desc */
void
sf2corbaInstDesc(corba_inst_desc_t *dest, const sf_inst_desc_t *src);
void
corba2sfInstDesc(sf_inst_desc_t *dest, const corba_inst_desc_t *src);


/* Conversion for agent communications */ 

void diet2corbaRequest(corba_request_t *dest,const diet_request_t *src);

void corba2dietRequest(diet_request_t *dest,const corba_request_t *src);

void diet2corbaResponse(corba_response_t *dest,const diet_response_t *src);

void corba2dietResponse(diet_response_t *dest,const corba_response_t *src);

void diet2corbaDecisionDesc(corba_decision_desc_t *dest,const diet_decision_desc_t *src);

void corba2dietDecisionDesc(diet_decision_desc_t *dest,const corba_decision_desc_t *src);

void diet2corbaDecisionSequence(corba_decision_sequence_t *dest,const diet_decision_sequence_t *src);

void corba2dietDecisionSequence(diet_decision_sequence_t *dest,const corba_decision_sequence_t *src);

#endif
