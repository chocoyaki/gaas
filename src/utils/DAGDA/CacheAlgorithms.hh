/**
 * @file CacheAlgorithms.hh
 *
 * @brief  Cache replacements algorithms used by DAGDA when we remove data
 *
 * @author  Gael Le Mahec (lemahec@clermont.in2p3.fr)
 *
 * @section Licence
 *   |LICENCE|
 */


#ifndef __CACHEALGORITHMS_HH__
#define __CACHEALGORITHMS_HH__

#include "AdvancedDagdaComponent.hh"

extern "C" __declspec (dllexport) int
LRU(AdvancedDagdaComponent *manager, size_t size, dagda_object_type_t type);

int
LFU(AdvancedDagdaComponent *manager, size_t size, dagda_object_type_t type);

int
FIFO(AdvancedDagdaComponent *manager, size_t size, dagda_object_type_t type);

#endif /* ifndef __CACHEALGORITHMS_HH__ */
