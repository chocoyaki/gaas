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
 * Revision 1.2  2005/05/18 21:14:58  mjan
 * minor bug fix inside JuxMem wrapper for DIET
 *
 * Revision 1.1  2005/05/18 14:18:09  mjan
 * Initial adding of JuxMem support inside DIET. dmat_manips examples tested without JuxMem and with JuxMem
 *
 *
 ****************************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "JuxMemImpl.hh"

/** DIET's include **/
#include "debug.hh"

/** APR's include **/
#include <apr_hash.h>

/** JXTA's and co include **/
#include <jpr/jpr_thread.h>
#include <jxta.h>

/** Include JuxMem header **/
#include "consistency_protocol.h"
#include "sog_client_stub.h"

/** JuxMem Constructor */
JuxMemImpl::JuxMemImpl()
{
}

JuxMemImpl::~JuxMemImpl()
{
  /** Stop JXTA **/
  jxta_module_stop((Jxta_module *) this->juxmem_group);
  JXTA_OBJECT_RELEASE(this->juxmem_group);

  apr_pool_destroy(this->pool);

  /** Stop log facility **/
  jxta_log_file_close(this->log_f);
  jxta_log_terminate();
} 

/** JuxMem Launcher */
int 
JuxMemImpl::run()
{
  Jxta_status status;

  jxta_initialize();
  jxta_log_initialize();

  assert(APR_STATUS_IS_SUCCESS(apr_pool_create(&(this->pool), NULL)));

  this->log_s = jxta_log_selector_new_and_set("*.*", &status);
  if (NULL == this->log_s || status != JXTA_SUCCESS) {
    ERROR("JuxMemImpl::Failed to init default log selector",1);
  }

  jxta_log_file_open(&(this->log_f),"jxta.log");
  jxta_log_using(jxta_log_file_append, this->log_f);
  jxta_log_file_attach_selector(this->log_f, this->log_s, NULL);

  /** Start JXTA */
  status = jxta_PG_new_netpg (&(this->juxmem_group));
  if (status != JXTA_SUCCESS) {
    ERROR("JuxMemImpl::Failed to start JuxMem group!",1);
  }
  TRACE_TEXT(TRACE_MAIN_STEPS, "Started the JuxMem client inside the SeD!\n");

  /** Get useful services */
  jxta_PG_get_juxmem_service (this->juxmem_group, &(this->juxmem_service));
  if (this->juxmem_service == NULL) {
    ERROR("JuxMemImpl::Failed to get JuxMem service!",1);
  }
  TRACE_TEXT(TRACE_MAIN_STEPS, "Getting the JuxMem service!\n");

  return 0;
}

long 
JuxMemImpl::JuxMemAlloc(char** data_id, long size, apr_hash_t* attributes)
{
  Jxta_status status = JXTA_SUCCESS;
  JString *jxta_data_id_jstring = NULL;
  Jxta_id *jxta_data_id = NULL;
  char *number_of_providers = NULL;

  TRACE_TEXT(TRACE_MAIN_STEPS, "Calling JuxMemAlloc");
  
  if (attributes == NULL) {
    attributes = apr_hash_make(this->pool);
  }
  
  number_of_providers = (char *) malloc(sizeof(char) * 10);
  sprintf(number_of_providers, "%i", 1);

  /** Add common required attributes for JuxMem **/
  apr_hash_set(attributes, (const void *) NUMBER_OF_COPIES, (apr_ssize_t) strlen(NUMBER_OF_COPIES),
	       (const void *) number_of_providers);
  apr_hash_set(attributes, (const void *) CP_TYPE, (apr_ssize_t) strlen(CP_TYPE), (const void *) FLAT_EC_PROTOCOL);
  apr_hash_set(attributes, (const void *) SOG_TYPE, (apr_ssize_t) strlen(SOG_TYPE), (const void *) NO_REPL_SOG);

  status = juxmem_alloc(this->juxmem_service, &jxta_data_id, 10, attributes);
  if (status != JXTA_SUCCESS) {
    ERROR("JuxMemImpl::Failed to call JuxMemAlloc!",1);
  } 

  jxta_id_to_jstring(jxta_data_id, &jxta_data_id_jstring);
  *data_id = strdup(jstring_get_string(jxta_data_id_jstring));

  free(number_of_providers);
  JXTA_OBJECT_RELEASE(jxta_data_id_jstring);
  JXTA_OBJECT_RELEASE(jxta_data_id);

  return 0;
}

long 
JuxMemImpl::JuxMemMap(char* data_id, long size, apr_hash_t* attributes) 
{
  Jxta_status status = JXTA_SUCCESS;
  Jxta_id *jxta_data_id = NULL;
  JString *jxta_data_id_jstring = NULL;
  char *data_size_char = NULL;
  char *number_of_providers = NULL;

  TRACE_TEXT(TRACE_MAIN_STEPS, "Calling JuxMemMap");

  if (attributes == NULL) {
    attributes = apr_hash_make(this->pool);
  }

  /** Add common required attributes for JuxMem **/
  number_of_providers = (char *) malloc(sizeof(char) * 10);
  sprintf(number_of_providers, "%i", 1);
  apr_hash_set(attributes, (const void *) NUMBER_OF_COPIES, (apr_ssize_t) strlen(NUMBER_OF_COPIES),
	       (const void *) number_of_providers);

  apr_hash_set(attributes, (const void *) CP_TYPE, (apr_ssize_t) strlen(CP_TYPE), (const void *) FLAT_EC_PROTOCOL);
  apr_hash_set(attributes, (const void *) SOG_TYPE, (apr_ssize_t) strlen(SOG_TYPE), (const void *) NO_REPL_SOG);

  data_size_char = (char *) malloc(sizeof(char) * 10);
  sprintf(data_size_char, "%li", size);
  apr_hash_set(attributes, (const void *) MEMORY_SIZE, (apr_ssize_t) strlen(MEMORY_SIZE), (const void *) data_size_char);
  
  jxta_data_id_jstring = jstring_new_2(data_id);
  assert(jxta_id_from_jstring(&jxta_data_id, jxta_data_id_jstring) == JXTA_SUCCESS);

  JXTA_OBJECT_SHARE(jxta_data_id);
  status = juxmem_map(this->juxmem_service, jxta_data_id, attributes);
  if (status != JXTA_SUCCESS) {
    ERROR("JuxMemImpl::Failed to call JuxMemMap!",1);
  } 

  free(number_of_providers);
  free(data_size_char);

  JXTA_OBJECT_RELEASE(jxta_data_id);
  JXTA_OBJECT_RELEASE(jxta_data_id_jstring);

  return 0;
}

long 
JuxMemImpl::JuxMemRead(char* data_id, char* buffer, int offset, int length)
{
  Jxta_status status = JXTA_SUCCESS;
  Jxta_id *jxta_data_id = NULL;
  JString *jxta_data_id_jstring = NULL;

  TRACE_TEXT(TRACE_MAIN_STEPS, "Calling JuxMemRead");
  
  jxta_data_id_jstring = jstring_new_2(data_id);
  assert(jxta_id_from_jstring(&jxta_data_id, jxta_data_id_jstring) == JXTA_SUCCESS);

  JXTA_OBJECT_SHARE(jxta_data_id);
  status = juxmem_read(this->juxmem_service, jxta_data_id, buffer, offset, length);
  if (status != JXTA_SUCCESS) {
    ERROR("JuxMemImpl::Failed to call JuxMemRead!",1);
  }

  JXTA_OBJECT_RELEASE(jxta_data_id);
  JXTA_OBJECT_RELEASE(jxta_data_id_jstring);

  return 0;
}

long 
JuxMemImpl::JuxMemWrite(char* data_id, char* buffer, int offset, int length) 
{
  Jxta_status status = JXTA_SUCCESS;
  JString *jxta_data_id_jstring = NULL;
  Jxta_id *jxta_data_id = NULL;

  TRACE_TEXT(TRACE_MAIN_STEPS, "Calling JuxMemWrite");
  
  jxta_data_id_jstring = jstring_new_2(data_id);
  assert(jxta_id_from_jstring(&jxta_data_id, jxta_data_id_jstring) == JXTA_SUCCESS);

  JXTA_OBJECT_SHARE(jxta_data_id);
  status = juxmem_write(this->juxmem_service, jxta_data_id, buffer, offset, length);
  if (status != JXTA_SUCCESS) {
    ERROR("JuxMemImpl::Failed to call JuxMemWrite!",1);
  }

  JXTA_OBJECT_RELEASE(jxta_data_id);
  JXTA_OBJECT_RELEASE(jxta_data_id_jstring);

  return 0;
}

long 
JuxMemImpl::JuxMemFlush(char* data_id)
{
  Jxta_status status = JXTA_SUCCESS;
  JString *jxta_data_id_jstring = NULL;
  Jxta_id *jxta_data_id = NULL;

  TRACE_TEXT(TRACE_MAIN_STEPS, "Calling JuxMemFlush");
  
  jxta_data_id_jstring = jstring_new_2(data_id);
  assert(jxta_id_from_jstring(&jxta_data_id, jxta_data_id_jstring) == JXTA_SUCCESS);

  JXTA_OBJECT_SHARE(jxta_data_id);
  status = juxmem_flush(this->juxmem_service, jxta_data_id);
  if (status != JXTA_SUCCESS) {
    ERROR("JuxMemImpl::Failed to call JuxMemFlush!",1);
  }

  JXTA_OBJECT_RELEASE(jxta_data_id);
  JXTA_OBJECT_RELEASE(jxta_data_id_jstring);

  return 0;
}
  
long 
JuxMemImpl::JuxMemAcquire(char* data_id)
{
  Jxta_status status = JXTA_SUCCESS;
  JString *jxta_data_id_jstring = NULL;
  Jxta_id *jxta_data_id = NULL;
  TRACE_TEXT(TRACE_MAIN_STEPS, "Calling JuxMemAcquire");
  
  jxta_data_id_jstring = jstring_new_2(data_id);
  assert(jxta_id_from_jstring(&jxta_data_id, jxta_data_id_jstring) == JXTA_SUCCESS);

  JXTA_OBJECT_SHARE(jxta_data_id);
  status = juxmem_acquire(this->juxmem_service, jxta_data_id);
  if (status != JXTA_SUCCESS) {
    ERROR("JuxMemImpl::Failed to call JuxMemAcquire!",1);
  }

  JXTA_OBJECT_RELEASE(jxta_data_id);
  JXTA_OBJECT_RELEASE(jxta_data_id_jstring);

  return 0;
}

long 
JuxMemImpl::JuxMemAcquireRead(char* data_id)
{
  Jxta_status status = JXTA_SUCCESS;
  JString *jxta_data_id_jstring = NULL;
  Jxta_id *jxta_data_id = NULL;

  TRACE_TEXT(TRACE_MAIN_STEPS, "Calling JuxMemAcquireRead");
  
  jxta_data_id_jstring = jstring_new_2(data_id);
  assert(jxta_id_from_jstring(&jxta_data_id, jxta_data_id_jstring) == JXTA_SUCCESS);

  JXTA_OBJECT_SHARE(jxta_data_id);
  status = juxmem_acquire_read(this->juxmem_service, jxta_data_id);
  if (status != JXTA_SUCCESS) {
    ERROR("JuxMemImpl::Failed to call JuxMemAcquireRead!",1);
  }

  JXTA_OBJECT_RELEASE(jxta_data_id);
  JXTA_OBJECT_RELEASE(jxta_data_id_jstring);

  return 0;
}

long 
JuxMemImpl::JuxMemRelease(char* data_id)
{
  Jxta_status status = JXTA_SUCCESS;
  JString *jxta_data_id_jstring = NULL;
  Jxta_id *jxta_data_id = NULL;

  TRACE_TEXT(TRACE_MAIN_STEPS, "Calling JuxMemRelease");
  
  jxta_data_id_jstring = jstring_new_2(data_id);
  assert(jxta_id_from_jstring(&jxta_data_id, jxta_data_id_jstring) == JXTA_SUCCESS);

  JXTA_OBJECT_SHARE(jxta_data_id);
  status = juxmem_release(this->juxmem_service, jxta_data_id);
  if (status != JXTA_SUCCESS) {
    ERROR("JuxMemImpl::Failed to call JuxMemRelease!",1);
  }

  JXTA_OBJECT_RELEASE(jxta_data_id);
  JXTA_OBJECT_RELEASE(jxta_data_id_jstring);

  return 0;
}

