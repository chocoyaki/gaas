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
 * Revision 1.5  2011/02/24 16:52:40  bdepardo
 * Use new parser
 *
 * Revision 1.4  2010/11/16 01:42:28  amuresan
 *  - added proper concurrency support for cloud part
 *  - fixed small data initialization bug with cloud server example
 *
 * Revision 1.3  2010/11/15 07:17:13  amuresan
 * added dirty mutex hack to stop multiple requests from not working correctly (TODO: fix elegantly)
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


#ifndef _EUCALYPTUS_BATCH_SYSTEM_HH_
#define _EUCALYPTUS_BATCH_SYSTEM_HH_

#define CLOUD_LOG
#define CLOUD_DEBUG

#include "BatchSystem.hh"
#include "configuration.hh"
#include "EucaLib/ec2wrapper.h"

/* The type of the Virtual Machine that is to be instantiated */
typedef enum {
  M1_SMALL,
  C1_MEDIUM,
  M1_LARGE,
  M1_XLARGE,
  C1_XLARGE,
  T1_MICRO
} VMTYPE;

class Eucalyptus_BatchSystem : public BatchSystem {
public:
  Eucalyptus_BatchSystem(int batchID, const char * batchName);

  ~Eucalyptus_BatchSystem();

  /** If job not terminated, ask the batch system for the status of job
      whose ID is @param batchJobID .
      Updates the internal structure.
      Returns NB_STATUS on error, the status otherwise.
  */
  batchJobState
  askBatchJobStatus(int batchJobID);

  /** If job whose id is @param batchJobID is:
      - not finished, returns 0
      - terminated, returns 1
      - not found, -1
  */
  int
  isBatchJobCompleted(int batchJobID);

  /** Override for diet_submit_parallel */
  int
  diet_submit_parallel(diet_profile_t * profile,
                       const char * addon_prologue,
                       const char * command);

  /********** Batch static information accessing Functions **********/
  /* These should soon change for they assume a default queue and we
     want to be able to manage all queues of a system! */

  int
  getNbTotResources();

  int
  getNbResources();

  char *
  getResourcesName();

  int
  getMaxWalltime();

  int
  getMaxProcs();

  /********** Batch dynamic information accessing Functions *********/
  /* These should soon change for they assume a default queue and we
     want to be able to manage all queues of a system! */

  int
  getNbTotFreeResources();

  int
  getNbFreeResources();

  /****************** Performance Prediction Functions ***************/

private:
  /* Struct to hold per request thread data */

  typedef struct {
    /* This holds the instance states given by the Cloud system
       to the running instances */
    char **vmStates;

    /* This holds the instance names given by the Cloud system
       to the running instances */
    char **vmNames;

    /* This holds the public IP addresses of the VM instances given depending on
     * the Cloud system's networking configuration i.e. for Eucalyptus this
     * can be: managed, system (an external dhcp) or static.
     */
    char **vmIPs;

    /* Same as above, but this holds the private IP addresses.
       Note that they might be the same. */
    char **vmPrivIPs;

    /* When isueing a reservation to an Amazon-like cloud
     * an identifier is received for that reservation.
     * This string holds that value
     */
    char *reservationId;

    /* Actual number of VMs to instantiated and
       is read from the SeD .cfg file. */
    int actualCount;
  } request_data_t;

  int VM_Buff_count;

  int
  init(char*pathToPrivateKey, char*pathToCert);

  void
  doWait(int count, char* addresses);

  void
  allocVmNames(request_data_t * state);

  /****************** Eucalyptus VM Management ***********************/
  /* Makes a reservation on an Eucalyptus cloud and returns the number of instantiated VMs.
   * Output parameters:
   *  vmInstNames - names of the instantiated virtual machines
   *  vmIPs - ip addresses of the instantiated vms
   */
  int
  makeEucalyptusReservation(request_data_t * state,
                            int minVMCount,
                            int maxVMCount);

  /* Calls 'DescribeInstances' for the resources in the current reservation and updates the
   * VM instance names and IP addresses.
   * This is used internally for polling until the VMs have IP addresses associated to them.
   *
   * Returns:
   *    0 - OK
   *    != 0 - an error
   */
  int
  describeInstances(request_data_t * state);

  /* Terminates an Eucalyptus VM and returns a status code
     representing the success or failure of the operation. */
  int
  terminateEucalyptusInstance(request_data_t * state);

  /* Utility function to create a default SOAP message
   * with the three required security headers.
   * Returns:
   *    an instance of the "soap" structure if ok
   *    NULL if error
   */
  struct soap *
  newDefaultSoap();

  /* Utility function that reads a string parameter from the configuration file and either duplicates it
   * via strdup or returns NULL if the parameter is NULL.
   */
  char *
  GetStringValueOrNull(diet::param_type_t param);

  /************* Multi-threading helpers **************/

  /* Initiate data for a new request's state */
  request_data_t *
  request_begin(int thread_id);

  /* Uninitialize data when a request is done */
  void
  request_end(int thread_id);

  /* Job status */
  batchJobState state;

  /* VM type that is defined as a string value inside the config file. */
  static const char * vmTypes[];

  /* Security group
   */
  char *securityGroup;

  /* Minimum number of VMs to instantiate and is read from the SeD .cfg file. */
  int vmMinCount;

  /* Maximum number of VMs to instantiate and is read from the SeD .cfg file. */
  int vmMaxCount;

  /* Path to the ssh key used for authentication on the VMs */
  char * pathToSSHKey;

  /* Flag that tells wether to instantiate new VMs or use existing ones */
  int instantiateVMs;

  /* This holds the private key of the Cloud user used to sign the SOAP requests
   * and its path is read from the SeD .cfg file.
   */
  EVP_PKEY *rsa_private_key;

  /* This holds the X509 certificate of the Cloud system and
     its path is read from the SeD .cfg file. */
  X509 *cert;

  /* This holds the URL of the Cloud system and
     is read from the SeD .cfg file. */
  char * eucaURL;

  /* This holds the name of the Eucalyptus Machine Image
     to be instantiated and is read from the SeD .cfg file. */
  char * emiName;

  /* This holds the name of the Eucalyptus Kernel Image
     to be instantiated and is read from the SeD .cfg file. */
  char * ekiName;

  /* This holds the name of the Eucalyptus Ramdisk Image
     to be instantiated and is read from the SeD .cfg file. */
  char * eriName;

  /* This holds the name of the Eucalyptus Ramdisk Image
     to be instantiated and is read from the SeD .cfg file. */
  char * userName;

  /* This holds the name of the Eucalyptus virtual machine
     type to be instantiated and is read from the SeD .cfg file. */
  VMTYPE vmType;

  /* This holds the name of the keypair to be used by to be used for instanting
   * images and is read from the SeD .cfg file.
   */
  char * keyName;

  /* This is an internal configuration and represents
   * the maximum number of polling tries to perform when waiting
   * for the VMs to be usable and having an IP address attached.
   * Current value is 50.
   */
  int maxTries;

  /* This is an internal configuration and
   * represents the interval in seconds to wait between each polling request.
   * Current value is 5.
   */
  int sleepTimeout;

  /********** Thread management ************/

  /* Mutex for synchronizing requests received in parallel */
  omni_mutex request_mutex;

  /* Thread id to local index mapping */
  int *thread_local_id;

  /* Maximum number of threads */
  unsigned int max_threads;

  /********** thread local data **********/

  /* Per-request thread data.
     Used for simultaneous multiple-request handling. */
  request_data_t ** request_state;
};

#endif  // _EUCALYPTUS_BATCH_SYSTEM_HH_
