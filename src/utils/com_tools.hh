/****************************************************************************/
/* $Id$ */
/* DIET communication tools specification                                   */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Ludovic BERTSCH           - LIP ENS-Lyon (France)                   */
/*                                                                          */
/*  This is part of DIET software.                                          */
/*  Copyright (C) 2002 ReMaP/INRIA                                          */
/*                                                                          */
/****************************************************************************/
/*
 * $Log$
 * Revision 1.2  2002/12/03 19:08:24  pcombes
 * Update configure, update to FAST 0.3.15, clean CVS logs in files.
 * Put main Makefile in root directory.
 *
 ****************************************************************************/


#ifndef _COM_TOOLS_H_
#define _COM_TOOLS_H_

#include "DIET_client.h"

#if HAVE_CICHLID


/* Retourne la taille d'un objet d'un des types de base
 * 
 * Rq: La taille est alignee (on utilise sizeof)
 */
int base_type_size(diet_base_type_t base_type);

/* Retourne une estimation de la taille du parametre, qui va
 * transiter sur le reseau lors de l'envoi des donnees 
 * (c'est pourquoi on ne prend pas en compte la taille de
 *  toutes les structures)
 */
long parameter_size(diet_arg_t *p);

/* Retourne le nombre d'octets (appoximatif) contenu dans le profile
 * En l'occurrence, c'est la somme des tailles de tous les parametres
 */
long profile_size(diet_profile_t *p);

/* Initialise la transmission des informations concernant la
 * circulation des paquets sur le reseau 
 */
void init_communications();

/* Indique a un programme externe (typiquement la demo de visualisation
 * du reseau) qu'une communication a lieu entre deux noeuds du reseau
 */
void add_communication(char* node0, char* node1, long size);


#endif // HAVE_CICHLID
#endif // _COM_TOOLS_H_
