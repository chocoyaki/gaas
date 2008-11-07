/***********************************************************/
/* Cache replacements algorithms used by DAGDA when        */
/* a data needs to be removed.                             */
/*                                                         */
/*  Author(s):                                             */
/*    - Gael Le Mahec (lemahec@clermont.in2p3.fr)          */
/*                                                         */
/* $LICENSE$                                               */
/***********************************************************/
/* $Id$
 * $Log$
 * Revision 1.3  2008/11/07 14:32:14  bdepardo
 * Headers correction
 *
 *
 ***********************************************************/

#ifndef __CACHEALGORITHMS_HH__
#define __CACHEALGORITHMS_HH__

#include "AdvancedDagdaComponent.hh"

int LRU(AdvancedDagdaComponent* manager, size_t size, dagda_object_type_t type);
int LFU(AdvancedDagdaComponent* manager, size_t size, dagda_object_type_t type);
int FIFO(AdvancedDagdaComponent* manager, size_t size, dagda_object_type_t type);

#endif
