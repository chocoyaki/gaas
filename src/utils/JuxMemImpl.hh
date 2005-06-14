/****************************************************************************/
/* JuxMem wrapper                                                           */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Mathieu Jan (Mathieu.Jan@irisa.fr)                                  */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.4  2005/06/14 16:17:12  mjan
 * Added support of DIET_FILE inside JuxMem-DIET for TLSE code
 *
 *
 ****************************************************************************/

#ifndef _JUXMEM_IMPL_HH_
#define _JUXMEM_IMPL_HH_

#include "DIET_data.h"

#include <jxta_log.h>
#include <jxta_peergroup.h>

#include <juxmem_service.h>

/****************************************************************************/
/* JuxMem class                                                                */
/****************************************************************************/

class JuxMemImpl
{

public:                                              

  JuxMemImpl();

  ~JuxMemImpl();
  
  int run(const char * userDefName);

  long JuxMemAlloc(char** data_id, int size, apr_hash_t* attributes);

  long JuxMemMap(char* data_id, int size, apr_hash_t* attrib);
  
  long JuxMemRead(diet_arg_s* arg);

  long JuxMemWrite(diet_arg_s* arg);

  long JuxMemFlush(char* data_id);
  
  long JuxMemAcquire(char* data_id);

  long JuxMemAcquireRead(char* data_id);

  long JuxMemRelease(char* data_id);

private:

  /** The service itself **/
  JuxMem_service*    juxmem_service;
  Jxta_PG*           juxmem_group;

  /** The log facility **/
  Jxta_log_file*     log_f;
  Jxta_log_selector* log_s;

  /** APR **/
  apr_pool_t *pool;
};


#endif // _JUXMEM_IMPL_HH_
