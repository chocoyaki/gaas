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
 * Revision 1.5  2005/06/14 16:17:12  mjan
 * Added support of DIET_FILE inside JuxMem-DIET for TLSE code
 *
 *
 ****************************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>

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
  apr_pool_destroy(this->pool);

  jxta_module_stop((Jxta_module*) this->juxmem_group);
  JXTA_OBJECT_RELEASE(this->juxmem_group);

  /** Stop JXTA **/
  jxta_log_file_attach_selector(log_f, NULL, &log_s);
  jxta_log_selector_delete(log_s);
  jxta_log_file_close(log_f);
  jxta_terminate();
} 

/** JuxMem Launcher */
int 
JuxMemImpl::run(const char* userDefName)
{
  Jxta_status status;
  JString *peer_name = NULL;
  Jxta_id *peer_id = NULL;
  JString *peer_group_id_jstring = NULL;
  Jxta_id *peer_group_id = NULL;
  Jxta_PA *config_adv;
  FILE *advfile;
  JString *advs;

  jxta_initialize();

  assert(APR_STATUS_IS_SUCCESS(apr_pool_create(&(this->pool), NULL)));

  this->log_s = jxta_log_selector_new_and_set("*.*", &status);
  if (NULL == this->log_s || status != JXTA_SUCCESS) {
    ERROR("JuxMemImpl::Failed to init default log selector\n",1);
  }

  jxta_log_file_open(&(this->log_f),"jxta.log");
  jxta_log_using(jxta_log_file_append, this->log_f);
  jxta_log_file_attach_selector(this->log_f, this->log_s, NULL);

  /** Trick for GoDIET: peerID and peer name are generated at run-time */
  config_adv = jxta_PA_new();
  assert(config_adv != NULL);
  advfile = fopen("PlatformConfig", "r");
  if (advfile == NULL) {
    JXTA_OBJECT_RELEASE(config_adv);
    /** There is no PlatformConfig file ! We can't rely on JXTA-C to create our own ... */
    printf("Since you are using JuxMem for handling data, you must have a PlatformConfig file before launching your SeD or client!\n");
    printf("Please look at the user manuel of DIET in order to correct this.\n");
    exit(0);
  }
  jxta_PA_parse_file(config_adv, advfile);
  fclose(advfile);
  if (config_adv != NULL) {
    /** Setting the name of the peer */
    peer_name = jstring_new_2(userDefName);
    jxta_PA_set_Name(config_adv, peer_name);
    JXTA_OBJECT_RELEASE(peer_name);
    
    /** Setting the id of the peer */
    peer_group_id_jstring = jstring_new_2("urn:jxta:uuid-3E5B4F7DD63D4212BDC6A63B4A6096C802");
    jxta_id_from_jstring(&peer_group_id,peer_group_id_jstring);
    jxta_id_peerid_new_1(&peer_id, peer_group_id);
    jxta_PA_set_PID(config_adv, peer_id);
    JXTA_OBJECT_RELEASE(peer_group_id_jstring);
    JXTA_OBJECT_RELEASE(peer_group_id);
    JXTA_OBJECT_RELEASE(peer_id);

    /** Writing config file */
    advfile = fopen("PlatformConfig", "w");
    assert(advfile != NULL);
    jxta_PA_get_xml(config_adv, &advs);
    fwrite(jstring_get_string(advs), jstring_length(advs), 1, advfile);

    JXTA_OBJECT_RELEASE(advs);
    fclose(advfile);
  } else {
    /** There is no PlatformConfig file ! We can't rely on JXTA-C to create our own ... */
    printf("Since you are using JuxMem for handling data, you must have a PlatformConfig file before launching your SeD or client!\n");
    printf("Please look at the user manuel of DIET in order to correct this.\n");
    exit(0);
  }
  JXTA_OBJECT_RELEASE(config_adv);

  /** Start JXTA */
  status = jxta_PG_new_netpg (&(this->juxmem_group));
  if (status != JXTA_SUCCESS) {
    ERROR("JuxMemImpl::Failed to start JuxMem group!\n",1);
  }
  TRACE_TEXT(TRACE_MAIN_STEPS, "Started the JuxMem client!\n");

  /** Get useful services */
  jxta_PG_get_juxmem_service (this->juxmem_group, &(this->juxmem_service));
  if (this->juxmem_service == NULL) {
    ERROR("JuxMemImpl::Failed to get JuxMem service!\n",1);
  }
  TRACE_TEXT(TRACE_MAIN_STEPS, "Getting the JuxMem service!\n");

  return 0;
}

long 
JuxMemImpl::JuxMemAlloc(char** data_id, int size, apr_hash_t* attributes)
{
  Jxta_status status = JXTA_SUCCESS;
  JString *jxta_data_id_jstring = NULL;
  Jxta_id *jxta_data_id = NULL;
  char *number_of_providers = NULL;

  TRACE_TEXT(TRACE_MAIN_STEPS, "Calling JuxMemAlloc for a data of size = " << size << "\n");
  
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

  status = juxmem_alloc(this->juxmem_service, &jxta_data_id, size, attributes);
  if (status != JXTA_SUCCESS) {
    ERROR("JuxMemImpl::Failed to call JuxMemAlloc!\n",1);
  } 

  jxta_id_to_jstring(jxta_data_id, &jxta_data_id_jstring);
  *data_id = strdup(jstring_get_string(jxta_data_id_jstring));

  apr_hash_set(attributes, (const void *) NUMBER_OF_COPIES, (apr_ssize_t) strlen(NUMBER_OF_COPIES), NULL);
  free(number_of_providers);

  JXTA_OBJECT_RELEASE(jxta_data_id_jstring);
  JXTA_OBJECT_RELEASE(jxta_data_id);

  return 0;
}

long 
JuxMemImpl::JuxMemMap(char* data_id, int size, apr_hash_t* attributes) 
{
  Jxta_status status = JXTA_SUCCESS;
  Jxta_id *jxta_data_id = NULL;
  JString *jxta_data_id_jstring = NULL;
  char *data_size_char = NULL;
  char *number_of_providers = NULL;

  TRACE_TEXT(TRACE_MAIN_STEPS, "Calling JuxMemMap for a data of size = " << size << "\n");

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
  sprintf(data_size_char, "%i", size);
  apr_hash_set(attributes, (const void *) MEMORY_SIZE, (apr_ssize_t) strlen(MEMORY_SIZE), (const void *) data_size_char);
  
  jxta_data_id_jstring = jstring_new_2(data_id);
  assert(jxta_id_from_jstring(&jxta_data_id, jxta_data_id_jstring) == JXTA_SUCCESS);

  status = juxmem_map(this->juxmem_service, jxta_data_id, attributes);
  if (status != JXTA_SUCCESS) {
    ERROR("JuxMemImpl::Failed to call JuxMemMap!\n",1);
  } 

  apr_hash_set(attributes, (const void *) NUMBER_OF_COPIES, (apr_ssize_t) strlen(NUMBER_OF_COPIES), NULL);
  free(number_of_providers);
  apr_hash_set(attributes, (const void *) MEMORY_SIZE, (apr_ssize_t) strlen(MEMORY_SIZE), NULL);
  free(data_size_char);

  JXTA_OBJECT_RELEASE(jxta_data_id);
  JXTA_OBJECT_RELEASE(jxta_data_id_jstring);

  return 0;
}

long 
JuxMemImpl::JuxMemRead(diet_arg_s* arg)
{
  Jxta_status status = JXTA_SUCCESS;
  int offset = 0; /** In DIET all the data is currently read  */
  int length = (int) data_sizeof(&(arg->desc));
  Jxta_id *jxta_data_id = NULL;
  JString *jxta_data_id_jstring = NULL;

  TRACE_TEXT(TRACE_MAIN_STEPS, "Calling JuxMemRead\n");
  
  jxta_data_id_jstring = jstring_new_2(arg->desc.id);
  assert(jxta_id_from_jstring(&jxta_data_id, jxta_data_id_jstring) == JXTA_SUCCESS);

  if (arg->desc.generic.type == DIET_FILE) {
    int file = 0;
    void *pt = NULL;

    file = open((const char*) arg->desc.specific.file.path, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    assert(file >= 0);
    ftruncate(file, length);
    pt = mmap(NULL, length, PROT_WRITE | PROT_READ, MAP_SHARED, file, offset);
    assert(pt != MAP_FAILED);
    TRACE_TEXT(TRACE_MAIN_STEPS, "The DIET_FILE " << (char *) arg->desc.specific.file.path << " has been opened and mapped inside JuxMemWrite\n");
    status = juxmem_read(this->juxmem_service, jxta_data_id, pt, offset, length);
    assert(msync(pt, length, MS_SYNC) == 0);
    assert(munmap(pt, length) == 0);
    assert(close(file) == 0);
    TRACE_TEXT(TRACE_MAIN_STEPS, "The DIET_FILE " << (char *) arg->desc.specific.file.path << " has been unmapped and closed inside JuxMemWrite\n");
  } else {
    status = juxmem_read(this->juxmem_service, jxta_data_id, arg->value, offset, length);
  }
  if (status != JXTA_SUCCESS) {
    ERROR("JuxMemImpl::Failed to call JuxMemRead!\n",1);
  }

  JXTA_OBJECT_RELEASE(jxta_data_id);
  JXTA_OBJECT_RELEASE(jxta_data_id_jstring);

  return 0;
}

long 
JuxMemImpl::JuxMemWrite(diet_arg_s* arg)
{
  Jxta_status status = JXTA_SUCCESS;
  int offset = 0; /** In DIET all the data is currently written  */
  int length = (int) data_sizeof(&(arg->desc));
  JString *jxta_data_id_jstring = NULL;
  Jxta_id *jxta_data_id = NULL;

  TRACE_TEXT(TRACE_MAIN_STEPS, "Calling JuxMemWrite\n");
  
  jxta_data_id_jstring = jstring_new_2(arg->desc.id);
  assert(jxta_id_from_jstring(&jxta_data_id, jxta_data_id_jstring) == JXTA_SUCCESS);

  if (arg->desc.generic.type == DIET_FILE) {
    int file = 0;
    void *pt = NULL;

    file = open((const char*) arg->desc.specific.file.path, O_RDONLY);
    assert(file >= 0);
    pt = mmap(NULL, length, PROT_READ, MAP_SHARED, file, offset);
    assert(pt != MAP_FAILED);
    TRACE_TEXT(TRACE_MAIN_STEPS, "The DIET_FILE " << (char *) arg->desc.specific.file.path << " has been opened and mapped inside JuxMemWrite\n");
    status = juxmem_write(this->juxmem_service, jxta_data_id, pt, offset, length);
    assert(munmap(pt, length) == 0);
    assert(close(file) == 0);
    TRACE_TEXT(TRACE_MAIN_STEPS, "The DIET_FILE " << (char *) arg->desc.specific.file.path << " has been unmapped and closed inside JuxMemWrite\n");
  } else {
    status = juxmem_write(this->juxmem_service, jxta_data_id, arg->value, offset, length);
  }
  if (status != JXTA_SUCCESS) {
    ERROR("JuxMemImpl::Failed to call JuxMemWrite\n!",1);
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

  TRACE_TEXT(TRACE_MAIN_STEPS, "Calling JuxMemFlush\n");
  
  jxta_data_id_jstring = jstring_new_2(data_id);
  assert(jxta_id_from_jstring(&jxta_data_id, jxta_data_id_jstring) == JXTA_SUCCESS);

  status = juxmem_flush(this->juxmem_service, jxta_data_id);
  if (status != JXTA_SUCCESS) {
    ERROR("JuxMemImpl::Failed to call JuxMemFlush!\n",1);
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
  TRACE_TEXT(TRACE_MAIN_STEPS, "Calling JuxMemAcquire\n");
  
  jxta_data_id_jstring = jstring_new_2(data_id);
  assert(jxta_id_from_jstring(&jxta_data_id, jxta_data_id_jstring) == JXTA_SUCCESS);

  status = juxmem_acquire(this->juxmem_service, jxta_data_id);
  if (status != JXTA_SUCCESS) {
    ERROR("JuxMemImpl::Failed to call JuxMemAcquire!\n",1);
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

  TRACE_TEXT(TRACE_MAIN_STEPS, "Calling JuxMemAcquireRead\n");
  
  jxta_data_id_jstring = jstring_new_2(data_id);
  assert(jxta_id_from_jstring(&jxta_data_id, jxta_data_id_jstring) == JXTA_SUCCESS);

  status = juxmem_acquire_read(this->juxmem_service, jxta_data_id);
  if (status != JXTA_SUCCESS) {
    ERROR("JuxMemImpl::Failed to call JuxMemAcquireRead!\n",1);
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

  TRACE_TEXT(TRACE_MAIN_STEPS, "Calling JuxMemRelease\n");
  
  jxta_data_id_jstring = jstring_new_2(data_id);
  assert(jxta_id_from_jstring(&jxta_data_id, jxta_data_id_jstring) == JXTA_SUCCESS);

  status = juxmem_release(this->juxmem_service, jxta_data_id);
  if (status != JXTA_SUCCESS) {
    ERROR("JuxMemImpl::Failed to call JuxMemRelease!\n",1);
  }

  JXTA_OBJECT_RELEASE(jxta_data_id);
  JXTA_OBJECT_RELEASE(jxta_data_id_jstring);

  return 0;
}

