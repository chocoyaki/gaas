/****************************************************************************/
/* Batch System implementation for Amazon EC2 compatible clouds             */
/*                                                                          */
/* Author(s):                                                               */
/*    - Adrian Muresan (adrian.muresan@ens-lyon.fr)                         */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.7  2011/02/24 16:52:40  bdepardo
 * Use new parser
 *
 * Revision 1.6  2011/01/20 23:52:34  bdepardo
 * Removed unused variable.
 * Reduced scope of a variable.
 * Removed potential bug: Undefined behaviour: addresses is used wrong in call
 * to sprintf or snprintf. Quote: If copying takes place between objects that
 * overlap as a result of a call to sprintf() or snprintf(), the results are undefined.
 *
 * Revision 1.5  2010/11/16 01:42:28  amuresan
 *  - added proper concurrency support for cloud part
 *  - fixed small data initialization bug with cloud server example
 *
 * Revision 1.4  2010/11/15 07:17:13  amuresan
 * added dirty mutex hack to stop multiple requests from not working correctly (TODO: fix elegantly)
 *
 * Revision 1.3  2010/10/27 10:53:05  amuresan
 * modified cloud examples to take config files as command line args
 *
 * Revision 1.2  2010/10/27 06:41:30  amuresan
 * modified Eucalyptus_BatchSystem to be able to use existing VMs also
 *
 * Revision 1.1  2010/05/05 13:13:51  amuresan
 * First commit for the Eucalyptus BatchSystem.
 * Added SOAP client for the Amazon EC2 SOAP interface and
 * a new implementation of the BatchSystem base-class.
 *
 ****************************************************************************/


#include "debug.hh"
#include "Eucalyptus_BatchSystem.hh"

#define INFTY 999999

const char * Eucalyptus_BatchSystem::vmTypes[] = {
  "m1.small",
  "c1.medium",
  "m1.large",
  "m1.xlarge",
  "c1.xlarge",
  "t1.micro"
};

Eucalyptus_BatchSystem::Eucalyptus_BatchSystem(int ID, const char * batchname)
{
  char * tmp = NULL;
  unsigned int index;
  char * pathToCert;
  char * pathToPrivateKey;

  /* Member initialization here */
  /* TODO: move these as SeD config parameters where applicable */
  VM_Buff_count = 100;
  state = WAITING;

  /*
    vmStates = NULL;
    vmNames = NULL;
    vmIPs = NULL;
    vmPrivIPs = NULL;
    reservationId = NULL;
    actualCount = 0;
  */

  rsa_private_key = NULL;
  cert = NULL;
  maxTries = 50;
  sleepTimeout = 5;
  max_threads = 100;

  /* Parameters from the config file here */
  securityGroup = GetStringValueOrNull(diet::SECURITYGROUP);
#ifdef CLOUD_DEBUG
  if (securityGroup != NULL)
    fprintf(stdout, "Security group: %s\n", securityGroup);
#endif
  eucaURL = GetStringValueOrNull(diet::CLOUDURL);
  emiName = GetStringValueOrNull(diet::EMINAME);
  eriName = GetStringValueOrNull(diet::ERINAME);
  ekiName = GetStringValueOrNull(diet::EKINAME);
  pathToSSHKey = GetStringValueOrNull(diet::PATHTOSSHKEY);

  instantiateVMs = 1;
  CONFIG_INT(diet::INSTANTIATEVMS, instantiateVMs);

  std::string tmpString;
  if (CONFIG_STRING(diet::VMTYPE, tmpString)) {
    for (index = 0; index < sizeof(vmTypes) / sizeof(char*); index++) {
      if (tmpString == vmTypes[index]) {
        break;
      }
    }
  } else {
    index = 0;
  }

  if (index >= sizeof(vmTypes) / sizeof(char*)) {
    index = 0;
  }
  vmType = (VMTYPE)index;

  keyName = GetStringValueOrNull(diet::KEYNAME);
  vmMinCount = 1;
  CONFIG_INT(diet::VMMINCOUNT, vmMinCount);

  vmMaxCount = 1;
  CONFIG_INT(diet::VMMAXCOUNT, vmMinCount);

  pathToCert = GetStringValueOrNull(diet::PATHTOCERT);
  pathToPrivateKey = GetStringValueOrNull(diet::PATHTOPK);
  userName = GetStringValueOrNull(diet::USERNAME);
#ifdef CLOUD_DEBUG
  fprintf(stdout, "VMType = %s and index = %d\n", tmp, index);
  if (userName == NULL)
    fprintf(stdout, "INFO: userName is NULL\n");
  else
    fprintf(stdout, "INFO: userName = '%s'\n", userName);
#endif
  /* initialize per thread data */
  thread_local_id = (int*)malloc(max_threads * sizeof(int));
  request_state = (request_data_t**)malloc(max_threads * sizeof(request_data_t*));

  for (index = 0;index < max_threads;index++)
  {
    thread_local_id[index] = -1;
    request_state[index] = NULL;
  }

  init(pathToPrivateKey, pathToCert);
}

Eucalyptus_BatchSystem::~Eucalyptus_BatchSystem()
{
}

/*********************** Job Managing ******************************/

int Eucalyptus_BatchSystem::diet_submit_parallel(diet_profile_t * profile,
                                                 const char * addon_prologue,
                                                 const char * command)
{
  request_data_t * req_state = request_begin(omni_thread::self()->id());
  if (req_state == NULL)
  {
#ifdef CLOUD_DEBUG
    fprintf(stderr, "ERROR: Cannot create local data for new request. Increase number of maximum threads local variable.\n");
#endif
    state = ERROR;
    return 0;
  }
#ifdef CLOUD_DEBUG
  fprintf(stdout, "INFO: omnithread id: %d\n", omni_thread::self()->id());
#endif
  if (instantiateVMs)
  {
    if (makeEucalyptusReservation(req_state, vmMinCount, vmMaxCount))
    {
      state = ERROR;
#ifdef CLOUD_DEBUG
      fprintf(stderr, "ERROR: Cannot perform reservation\n");
#endif

      return 0;
    }
  }
  else if (describeInstances(req_state))
  {
    printf("error describing\n");
    terminateEucalyptusInstance(req_state);
    state = ERROR;
#ifdef CLOUD_DEBUG
    fprintf(stderr, "ERROR: Cannot call 'DescribeInstances' to cloud at address '%s'\n", eucaURL);
#endif
    return 0;
  }

  printf("Described instances\n");

  int count = 0;
  char unassigned = 1;
  int index = 0;
  /* wait while the VMs have no assigned IP addresses */
  state = SUBMITTED;
  while(unassigned)
  {
    unassigned = 0;
    if (count > maxTries)
    {
#ifdef CLOUD_DEBUG
      fprintf(stderr, "ERROR: Cannot retrieve IP addresses after %d tries to cloud at address '%s'\n",
              count, eucaURL);
#endif
      return 0;
    }
    for (index = 0;index < req_state->actualCount;index++)
      if (strcmp(req_state->vmIPs[index], "0.0.0.0") == 0 || strlen(req_state->vmIPs[index]) <= 1 || strcmp(req_state->vmStates[index], "running") != 0)
      {
        unassigned = 1;
        break;
      }
    if (req_state->actualCount == 0)
      unassigned = 1;
    if (unassigned)
    {
#ifdef CLOUD_DEBUG
      fprintf(stdout, "INFO: Some VMs do not have an IP address assigned, waiting for %d seconds...\n",
              sleepTimeout);
#endif
      sleep(sleepTimeout);
      if (describeInstances(req_state))
      {
        terminateEucalyptusInstance(req_state);
        state = ERROR;
#ifdef CLOUD_DEBUG
        fprintf(stderr, "ERROR: Cannot call 'DescribeInstances' to cloud at address '%s'\n", eucaURL);
#endif
        return 0;
      }
    }
    count++;
  }
#ifdef CLOUD_DEBUG
  fprintf(stdout, "INFO: Created instances with the following IP address mapping:\n");
  for (index = 0;index<req_state->actualCount;index++)
    fprintf(stdout, "\t%s -> %s\n", req_state->vmNames[index], req_state->vmIPs[index]);
#endif
  /* prepare the script for running, replace the meta-variables inside the script */
  char *script = (char*)malloc(9000 * sizeof(char));
  std::string addr = "";
  for (index = 0;index < req_state->actualCount;index++)
    addr += req_state->vmIPs[index];
  sprintf(script, "%s", command);
  char *addresses = strdup(addr.c_str());
  replaceAllOccurencesInString(&script, "$USERNAME", userName);
  replaceAllOccurencesInString(&script, "$DIET_CLOUD_VMS", addresses);
  replaceAllOccurencesInString(&script, "$PATH_TO_SSH_KEY", pathToSSHKey);

#ifdef CLOUD_DEBUG
  fprintf(stdout, "INFO: running:\n%s", script);
#endif

  if (instantiateVMs)
    doWait(20, addresses);

  state = RUNNING;
  if (system(script))
  {
    state = ERROR;
#ifdef CLOUD_DEBUG
    fprintf(stderr, "ERROR: Could not run system command\n");
#endif
  }
  if (instantiateVMs)
  {
    terminateEucalyptusInstance(req_state);
#ifdef CLOUD_DEBUG
    fprintf(stdout, "INFO: Terminated instances\n");
#endif
  }
  state = TERMINATED;

  return 0;
}

BatchSystem::batchJobState
Eucalyptus_BatchSystem::askBatchJobStatus(int batchJobID)
{
  return state;
}

int
Eucalyptus_BatchSystem::isBatchJobCompleted(int batchJobID)
{
  if ((state == TERMINATED) || (state == CANCELED) || (state == ERROR))
    return 1;
  else if (state == NB_STATUS)
    return -1;
  return 0;
}

/********** Batch static information accessing Functions **********/

int
Eucalyptus_BatchSystem::getNbTotResources()
{
  INTERNAL_WARNING(__FUNCTION__ << " not yet implemented\n\n");
  return INFTY;
}

/* TODO: this function should be C++ written
   or, as OAR relies on Perl, use a Perl script which has to be
   deployed
*/
int
Eucalyptus_BatchSystem::getNbResources() /* in the queue interQueueName */
{
  INTERNAL_WARNING(__FUNCTION__ << " not yet implemented\n\n");
  return INFTY;
}

char *
Eucalyptus_BatchSystem::getResourcesName()
{
  INTERNAL_WARNING(__FUNCTION__ << " not yet implemented\n\n");
  return "Cloud VM nodes";
}

int
Eucalyptus_BatchSystem::getMaxWalltime()
{
  INTERNAL_WARNING(__FUNCTION__ << " not yet implemented\n\n");
  return INFTY;
}

int
Eucalyptus_BatchSystem::getMaxProcs()
{
  INTERNAL_WARNING(__FUNCTION__ << " not yet implemented\n\n");
  return INFTY;
}

/********** Batch dynamic information accessing Functions *********/

int
Eucalyptus_BatchSystem::getNbTotFreeResources()
{
  INTERNAL_WARNING(__FUNCTION__ << " not yet implemented\n\n");
  return INFTY;
}

int
Eucalyptus_BatchSystem::getNbFreeResources()
{
  INTERNAL_WARNING(__FUNCTION__ << " not yet implemented\n\n");
  return INFTY;
}

/*************************** Performance Prediction *************************/

/*************************** Eucalyptus VM Management ***********************/
void Eucalyptus_BatchSystem::doWait(int count, char*addresses)
{
  int result,
    index = 0;

  char * s =
    "#!/bin/bash\n\n"
    "for h in $DIET_CLOUD_VMS\n"
    "do\n"
    "ssh $USERNAME@$h -i $PATH_TO_SSH_KEY -o StrictHostKeyChecking = no 'hostname; uname -a'\n"
    "done";
  char*script = (char*)malloc(1000);
  sprintf(script, "%s", s);

  replaceAllOccurencesInString(&script, "$USERNAME", userName);
  replaceAllOccurencesInString(&script, "$DIET_CLOUD_VMS", addresses);
  replaceAllOccurencesInString(&script, "$PATH_TO_SSH_KEY", pathToSSHKey);

#ifdef CLOUD_DEBUG
  fprintf(stdout, "INFO: waiting with script:\n%s\n", script);
#endif
  do
  {
    result = system(script);
    index++;
#ifdef CLOUD_DEBUG
    fprintf(stdout, "Attempt %d with result %d...\n", index, result);
#endif
    if (result)
      sleep(sleepTimeout);
  }
  while(index < count && result);

}

void Eucalyptus_BatchSystem::allocVmNames(request_data_t * req_state)
{
  req_state->vmStates = (char**)malloc(VM_Buff_count * sizeof(char*));
  req_state->vmNames = (char**)malloc(VM_Buff_count * sizeof(char*));
  req_state->vmIPs = (char**)malloc(VM_Buff_count * sizeof(char*));
  req_state->vmPrivIPs = (char**)malloc(VM_Buff_count * sizeof(char*));
}

int Eucalyptus_BatchSystem::makeEucalyptusReservation(request_data_t * req_state, int minVMCount, int maxVMCount)
{
  /* TODO: fix this crude way of saying "I'm empty": */
  req_state->vmNames = NULL;
  req_state->vmIPs = NULL;
  req_state->vmPrivIPs = NULL;
  req_state->actualCount = 0;

  struct soap *s = newDefaultSoap();

  if (s)
  {
    struct ec2__RunInstancesType * rireqp;
    struct ec2__RunInstancesResponseType * rirespp;
    struct ec2__RunningInstancesItemType * item;

    rireqp = (struct ec2__RunInstancesType*)malloc(sizeof(struct ec2__RunInstancesType));
    rirespp = (struct ec2__RunInstancesResponseType*)malloc(sizeof(struct ec2__RunInstancesResponseType));

    rireqp->additionalInfo = NULL;
    rireqp->addressingType = NULL;
    rireqp->imageId = emiName;
    rireqp->instanceType = const_cast<char*>(vmTypes[vmType]);
    rireqp->kernelId = ekiName;
    rireqp->minCount = minVMCount;
    rireqp->maxCount = maxVMCount;
    rireqp->keyName = keyName;
    rireqp->ramdiskId = eriName;
    rireqp->groupSet = (struct ec2__GroupSetType*)malloc(sizeof(struct ec2__GroupSetType));
    rireqp->groupSet->__sizeitem = 0;
    rireqp->groupSet->item = NULL;
    rireqp->userData = NULL;
    rireqp->placement = NULL;
    rireqp->blockDeviceMapping = (struct ec2__BlockDeviceMappingType*)malloc(sizeof(struct ec2__BlockDeviceMappingType));
    rireqp->blockDeviceMapping->__sizeitem = 0;
    rireqp->blockDeviceMapping->item = NULL;

    rirespp->instancesSet = (struct ec2__RunningInstancesSetType*)malloc(sizeof(struct ec2__RunningInstancesSetType));
    rirespp->instancesSet->__sizeitem = 0;
    rirespp->instancesSet->item = NULL;
    rirespp->groupSet = (struct ec2__GroupSetType*)malloc(sizeof(struct ec2__GroupSetType));
    if (securityGroup == NULL)
    {
      rirespp->groupSet->__sizeitem = 0;
      rirespp->groupSet->item = NULL;
    }
    else
    {
      struct ec2__GroupItemType* gitem = (struct ec2__GroupItemType*)malloc(sizeof(struct ec2__GroupItemType));
      gitem->groupId = strdup(securityGroup);
      rirespp->groupSet->__sizeitem = 1;
      rirespp->groupSet->item = gitem;
    }

    if (soap_call___ec2__RunInstances(s, eucaURL, "RunInstances", rireqp, rirespp) == SOAP_OK)
    {
      int index;
      req_state->actualCount = rirespp->instancesSet->__sizeitem;
#ifdef CLOUD_DEBUG
      fprintf(stdout, "INFO: Called RunInstances, reserved %d resources\n", req_state->actualCount);
#endif
      req_state->reservationId = strdup(rirespp->reservationId);
      allocVmNames(req_state);
      for (index = 0; index < req_state->actualCount; index++)
      {
        item = &rirespp->instancesSet->item[index];
        req_state->vmStates[index] = strdup(item->instanceState->name);
        req_state->vmNames[index] = strdup(item->instanceId);
        req_state->vmIPs[index] = strdup(item->dnsName);
        req_state->vmPrivIPs[index] = strdup(item->privateDnsName);
      }
    }
    else
    {
#ifdef CLOUD_DEBUG
      fprintf(stderr, "ERROR: Cannot perform call to 'RunInstances' to URL '%s'\n", eucaURL);
#endif
      soap_print_fault(s, stderr);
      return 1;
    }
    /* TODO: do the WHOLE cleanup, a lot more structs were allocated */
    free(rireqp);
    free(rirespp);
  }
  else
  {
#ifdef CLOUD_DEBUG
    fprintf(stderr, "ERROR: Could not initialize default SOAP message.\n");
#endif
    return 1;
  }
  return 0;
}

int Eucalyptus_BatchSystem::terminateEucalyptusInstance(request_data_t * req_state)
{
  struct soap *s = newDefaultSoap();

  if (s)
  {
    int index;
    struct ec2__TerminateInstancesType * titp;
    struct ec2__TerminateInstancesResponseType * tirtp;

    titp = (struct ec2__TerminateInstancesType *)malloc(sizeof(struct ec2__TerminateInstancesType));
    titp->instancesSet = (struct ec2__TerminateInstancesInfoType*)malloc(sizeof(struct ec2__TerminateInstancesInfoType));
    titp->instancesSet->__sizeitem = req_state->actualCount;
    titp->instancesSet->item = (struct ec2__TerminateInstancesItemType*)malloc(req_state->actualCount * sizeof(struct ec2__TerminateInstancesItemType));
    for (index = 0; index < req_state->actualCount; index++)
      titp->instancesSet->item[index].instanceId = req_state->vmNames[index];

    tirtp = (struct ec2__TerminateInstancesResponseType*)malloc(sizeof(struct ec2__TerminateInstancesResponseType));
    tirtp->instancesSet = (struct ec2__TerminateInstancesResponseInfoType*)malloc(sizeof(struct ec2__TerminateInstancesResponseInfoType));
    if (soap_call___ec2__TerminateInstances(s, eucaURL, "TerminateInstances", titp, tirtp) == SOAP_OK)
    {
#ifdef CLOUD_DEBUG
      fprintf(stdout, "INFO: Called 'TerminateInstances'\n");
      for (index = 0;index<tirtp->instancesSet->__sizeitem;index++)
        fprintf(stdout, "\nnew state is: %s\n", tirtp->instancesSet->item[index].shutdownState->name);
#endif
    }
    else /* an error occurred */
    {
#ifdef CLOUD_DEBUG
      fprintf(stderr, "ERROR: Cannot perform call to 'TerminateInstances' to URL '%s'\n", eucaURL);
#endif
      soap_print_fault(s, stderr);
      return 1;
    }
  }
  else
  {
#ifdef CLOUD_DEBUG
    fprintf(stderr, "ERROR: Could not initialize default SOAP message.\n");
#endif
    return 1;
  }
  return 0;
}

int Eucalyptus_BatchSystem::describeInstances(request_data_t * req_state)
{
  struct soap * s;
  int index;
  s = newDefaultSoap();
  if (s == NULL)
  {
#ifdef CLOUD_DEBUG
    fprintf(stderr, "ERROR: Could not create default SOAP message\n");
#endif
    return 1;
  }
  struct ec2__DescribeInstancesType*ditp;
  struct ec2__DescribeInstancesResponseType *respp;
  struct ec2__InstanceStateType *state;
  struct ec2__RunningInstancesItemType *runningItem;
  struct ec2__ReservationInfoType *reservation;

  ditp = (struct ec2__DescribeInstancesType*)malloc(sizeof(struct ec2__DescribeInstancesType));
  ditp->instancesSet = (struct ec2__DescribeInstancesInfoType*)malloc(sizeof(struct ec2__DescribeInstancesInfoType));
  ditp->instancesSet->item = (struct ec2__DescribeInstancesItemType*)malloc(req_state->actualCount * sizeof(struct ec2__DescribeInstancesItemType));
  ditp->instancesSet->__sizeitem = req_state->actualCount;
  for (index = 0; index < req_state->actualCount; index++)
    ditp->instancesSet->item[index].instanceId = req_state->vmNames[index];
  respp = (struct ec2__DescribeInstancesResponseType *)malloc(sizeof(struct ec2__DescribeInstancesResponseType));
  respp->reservationSet = NULL;

  if (s != NULL && soap_call___ec2__DescribeInstances(s, eucaURL, "DescribeInstances", ditp, respp) == SOAP_OK)
  {
#ifdef CLOUD_DEBUG
    fprintf(stderr, "INFO: Found %d reservations(s)\n", respp->reservationSet->__sizeitem);
#endif
    int r;
    req_state->actualCount = 0;
    for (r = 0;r<respp->reservationSet->__sizeitem;r++)
    {

      if (req_state->reservationId == NULL || instantiateVMs == 0)
        req_state->reservationId = strdup(respp->reservationSet->item[r].reservationId);
      printf("INFO: ReservationId: %s\n", req_state->reservationId);
      if (strcmp(respp->reservationSet->item[r].reservationId, req_state->reservationId) == 0)
      {
        reservation = &respp->reservationSet->item[r];
        /* Update the number of instances in case the service uses existing VMs */
        for (index = 0;index<reservation->instancesSet->__sizeitem;index++)
        {
          printf("in reservation\n");
          if (req_state->vmNames == NULL)
            allocVmNames(req_state);
          runningItem = &reservation->instancesSet->item[index];
          state = runningItem->instanceState;
          if (strcmp(state->name, "running") == 0 || strcmp(state->name, "pending") == 0)
          {
            req_state->vmStates[req_state->actualCount] = strdup(state->name);
            req_state->vmNames[req_state->actualCount] = strdup(runningItem->instanceId);
            req_state->vmIPs[req_state->actualCount] = strdup(runningItem->dnsName);
            req_state->vmPrivIPs[req_state->actualCount] = strdup(runningItem->privateDnsName);
            req_state->actualCount++;
          }
#ifdef CLOUD_DEBUG
          fprintf(stdout, "INFO: VM with ID '%s' has state '%s' and IP '%s'\n",
                  runningItem->instanceId, state->name, runningItem->dnsName);
#endif
        }
      }
    }
  }
  else /* an error occurred */
  {
#ifdef CLOUD_DEBUG
    fprintf(stderr, "ERROR: Cannot perform call to 'DescribeInstances' to URL '%s'\n", eucaURL);
#endif
    soap_print_fault(s, stderr);
    return 1;
  }
  free(respp);
  free(ditp);
  return 0;
}

/*************************** Utility functions ***********************/
char *
Eucalyptus_BatchSystem::GetStringValueOrNull(diet::param_type_t param) {
  std::string tmpValue;
  if (!CONFIG_STRING(param, tmpValue)) {
    return NULL;
  }
  return strdup(tmpValue.c_str());
}

int Eucalyptus_BatchSystem::init(char*pathToPrivateKey, char*pathToCert)
{
  if (PEM_read_PK(pathToPrivateKey, &rsa_private_key))
  {
#ifdef CLOUD_DEBUG
    fprintf(stderr, "ERROR: Cannot read Private Key from file '%s'.\n", pathToPrivateKey);
#endif
    return 1;
  }
  else if (PEM_read_X509_cert(pathToCert, &cert))
  {
#ifdef CLOUD_DEBUG
    fprintf(stderr, "ERROR: Cannot read certificate from file '%s'.\n", pathToCert);
#endif
    return 1;
  }
  return 0;
}

struct soap * Eucalyptus_BatchSystem::newDefaultSoap()
{
  struct soap *s = soap_default_new();
  if (s == NULL)
  {
#ifdef CLOUD_LOG
    fprintf(stderr, "ERROR: Cannot create new SOAP message\n");
#endif
    return NULL;
  }
#ifdef SOAP_DEBUG
  soap_set_recv_logfile(s, "RECV.log");
  soap_set_sent_logfile(s, "SENT.log");
  soap_set_test_logfile(s, "TEST.log");
#endif
  soap_register_plugin(s, soap_wsse);
  if (soap_wsse_add_security_header(s, rsa_private_key, cert))
  { /* Error branch */
    free(s);
#ifdef CLOUD_LOG
    fprintf(stderr, "ERROR: Cannot add security headers to the SOAP message\n");
#endif
  }
  else
  { /* ok branch */
    return s;
  }
  return NULL;
}

/********************* Multi-threading ***********************/
Eucalyptus_BatchSystem::request_data_t * Eucalyptus_BatchSystem::request_begin(int thread_id)
{
  unsigned int index;
  index = 0;

  request_mutex.lock();

  while(index < max_threads && thread_local_id[index] != -1)
    index++;
  if (index >= max_threads)
    return NULL;
  thread_local_id[index] = thread_id;

  request_mutex.unlock();

  if (request_state[index] == NULL)
    request_state[index] = (request_data_t*)malloc(sizeof(request_data_t));

  request_state[index]->vmStates = NULL;
  request_state[index]->vmNames = NULL;
  request_state[index]->vmIPs = NULL;
  request_state[index]->vmPrivIPs = NULL;
  request_state[index]->reservationId = NULL;
  request_state[index]->actualCount = 0;

  return request_state[index];
}

void Eucalyptus_BatchSystem::request_end(int thread_id)
{
  unsigned int index;
  index = 0;

  request_mutex.lock();

  while(index < max_threads && thread_local_id[index] != -1)
    index++;
  if (index >= max_threads)
    return;
  thread_local_id[index] = -1;

  request_mutex.unlock();
}
