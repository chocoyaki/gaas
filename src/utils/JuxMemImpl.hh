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
 * Revision 1.3  2005/06/03 16:25:57  mjan
 * Adding tricks for using GoDIET with DIET/JuxMem
 * Using name of DIET SeDs and clients to generate the name of JXTA peers
 * Client side of DIET no longer generates a warning message when name = client is in .cfg
 * This is of course not required and optionnal!
 *
 *
 ****************************************************************************/

#ifndef _JUXMEM_IMPL_HH_
#define _JUXMEM_IMPL_HH_

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
  
  long JuxMemRead(char* data_id, void* buffer, int offset, int length);

  long JuxMemWrite(char* data_id, void* buffer, int offset, int length);

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
