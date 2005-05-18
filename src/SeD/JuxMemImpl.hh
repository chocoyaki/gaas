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
 * Revision 1.1  2005/05/18 14:18:09  mjan
 * Initial adding of JuxMem support inside DIET. dmat_manips examples tested without JuxMem and with JuxMem
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
  
  int run();

  long JuxMemAlloc(char** data_id, long size, apr_hash_t* attributes);

  long JuxMemMap(char* data_id, long size, apr_hash_t* attrib);
  
  long JuxMemRead(char* data_id, char* buffer, int offset, int length);

  long JuxMemWrite(char* data_id, char* buffer, int offset, int length);

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
