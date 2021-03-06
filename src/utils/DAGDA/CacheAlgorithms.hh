/**
 * @file CacheAlgorithms.hh
 *
 * @brief  Cache replacements algorithms used by DAGDA when we remove data
 *
 * @author  Gael Le Mahec (lemahec@clermont.in2p3.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#ifndef __CACHEALGORITHMS_HH__
#define __CACHEALGORITHMS_HH__

#include "AdvancedDagdaComponent.hh"
#ifdef WIN32
   #define DIET_API_LIB __declspec(dllexport)
#else
   #define DIET_API_LIB
#endif

extern "C" DIET_API_LIB int
LRU(AdvancedDagdaComponent *manager, size_t size, dagda_object_type_t type);

DIET_API_LIB int
LFU(AdvancedDagdaComponent *manager, size_t size, dagda_object_type_t type);

DIET_API_LIB int
FIFO(AdvancedDagdaComponent *manager, size_t size, dagda_object_type_t type);

#endif /* ifndef __CACHEALGORITHMS_HH__ */
