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
 * Revision 1.1  2010/05/05 13:13:51  amuresan
 * First commit for the Eucalyptus BatchSystem.
 * Added SOAP client for the Amazon EC2 SOAP interface and
 * a new implementation of the BatchSystem base-class.
 *
 ****************************************************************************/


#include "debug.hh"
#include "Eucalyptus_BatchSystem.hh"
#include "Parsers.hh"

#define INFTY 999999
#define CLOUD_DEBUG

const char * Eucalyptus_BatchSystem::vmTypes[] = { 
    "m1.small",
    "c1.medium",
    "m1.large",
    "m1.xlarge",
    "c1.xlarge"
};

Eucalyptus_BatchSystem::Eucalyptus_BatchSystem(int ID, const char * batchname)
{
    char * tmp = NULL;
    void * vpTmp = NULL;
    int index;
    char * pathToCert;
    char * pathToPrivateKey;
    
    // Member initialization here
    state = WAITING;
    vmNames = NULL;
    vmIPs = NULL;
    vmPrivIPs = NULL;
    reservationId = NULL;
    actualCount = 0;
    rsa_private_key = NULL;
    cert = NULL;
    maxTries = 50;
    sleepTimeout = 5;

    // Parameters from the config file here
    eucaURL = GetStringValueOrNull(tmp, Parsers::Results::CLOUDURL);
    emiName = GetStringValueOrNull(tmp, Parsers::Results::EMINAME);
    eriName = GetStringValueOrNull(tmp, Parsers::Results::ERINAME);
    ekiName = GetStringValueOrNull(tmp, Parsers::Results::EKINAME);
    tmp = (char*) Parsers::Results::getParamValue(Parsers::Results::VMTYPE);
    for(index = 0;index<sizeof(vmTypes) / sizeof(char*);index++)
        if(!strcmp(tmp, vmTypes[index]))
        {
            break;
        }
    if(index >= sizeof(vmTypes) / sizeof(char*))
        index = 0;
    vmType = (VMTYPE)index; // VM type 
    keyName = GetStringValueOrNull(tmp, Parsers::Results::KEYNAME);
    vpTmp = Parsers::Results::getParamValue(Parsers::Results::VMMINCOUNT);
    if(vpTmp != NULL)
        vmMinCount = *(int*)vpTmp;
    else
        vmMinCount = 1;
    vpTmp = Parsers::Results::getParamValue(Parsers::Results::VMMAXCOUNT);
    if(vpTmp != NULL)
        vmMaxCount = *(int*)vpTmp;
    else
        vmMaxCount = 1;
    pathToCert = GetStringValueOrNull(tmp, Parsers::Results::PATHTOCERT);
    pathToPrivateKey = GetStringValueOrNull(tmp, Parsers::Results::PATHTOPK);
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
    if(makeEucalyptusReservation(vmMinCount, vmMaxCount))
	{
        state = ERROR;
#ifdef CLOUD_DEBUG
        fprintf(stderr, "ERROR: Cannot perform reservation\n");
#endif
	}
	else
	{
        int count = 0;
        char unassigned = 1;
        int index = 0;
        // wait while the VMs have no assigned IP addresses
        state = SUBMITTED;
        while(unassigned)
        {
            unassigned = 0;
            if(count > maxTries)
            {
#ifdef CLOUD_DEBUG
                fprintf(stderr, "ERROR: Cannot retrieve IP addresses after %d tries to cloud at address '%s'\n",
                        count, eucaURL);
#endif
                return 0;
            }
            for(index = 0;index < actualCount;index++)
                if(strcmp(vmIPs[index], "0.0.0.0") == 0)
                {
                    unassigned = 1;
                    break;
                }
            if(unassigned)
            {
#ifdef CLOUD_DEBUG
                fprintf(stdout, "INFO: Some VMs do not have an IP address assigned, waiting for %d seconds...\n",
                        sleepTimeout);
#endif
                sleep(sleepTimeout);
                if(describeInstances())
                {
                    terminateEucalyptusInstance();
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
        for(index=0;index<actualCount;index++)
            fprintf(stdout, "\t%s -> %s\n", vmNames[index], vmIPs[index]);
#endif
        // prepare the script for running, replace the meta-variables inside the script
        char *script = (char*)malloc(9000 * sizeof(char));
        char *addresses = (char*)malloc((16 * actualCount + 1) * sizeof(char)); // 16 is the max length of an IP address plus a whitespace
        addresses[0] = '\0';
        for(index = 0;index < actualCount;index++)
            sprintf(addresses, "%s%s ", addresses, vmIPs[index]);
        sprintf(script, "%s", command);
        replaceAllOccurencesInString(&script, "$DIET_CLOUD_VMS", addresses);

        state = RUNNING;
        if(system(script))
		{
            state = ERROR;
#ifdef CLOUD_DEBUG
            fprintf(stderr, "ERROR: Could not run system command\n");
#endif
		}
        terminateEucalyptusInstance();
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
    if( (state == TERMINATED) || (state == CANCELED) || (state == ERROR) )
        return 1 ; 
    else if( state == NB_STATUS )
        return -1;
    return 0 ; 
}

/********** Batch static information accessing Functions **********/

int
Eucalyptus_BatchSystem::getNbTotResources()
{
  INTERNAL_WARNING(__FUNCTION__ << " not yet implemented\n\n") ;
  return INFTY;
}

/* TODO: this function should be C++ written
   or, as OAR relies on Perl, use a Perl script which has to be
   deployed
*/
int
Eucalyptus_BatchSystem::getNbResources() /* in the queue interQueueName */
{
  INTERNAL_WARNING(__FUNCTION__ << " not yet implemented\n\n") ;
  return INFTY;
}

char *
Eucalyptus_BatchSystem::getResourcesName()
{
  INTERNAL_WARNING(__FUNCTION__ << " not yet implemented\n\n") ;
  return "Cloud VM nodes";
}

int
Eucalyptus_BatchSystem::getMaxWalltime()
{
  INTERNAL_WARNING(__FUNCTION__ << " not yet implemented\n\n") ;
  return INFTY ;
}

int
Eucalyptus_BatchSystem::getMaxProcs()
{
  INTERNAL_WARNING(__FUNCTION__ << " not yet implemented\n\n") ;
  return INFTY ;
}

/********** Batch dynamic information accessing Functions *********/

int
Eucalyptus_BatchSystem::getNbTotFreeResources()
{
  INTERNAL_WARNING(__FUNCTION__ << " not yet implemented\n\n") ;
  return INFTY;
}

int
Eucalyptus_BatchSystem::getNbFreeResources()
{
  INTERNAL_WARNING(__FUNCTION__ << " not yet implemented\n\n") ;
  return INFTY;
}

/*************************** Performance Prediction *************************/

/*************************** Eucalyptus VM Management ***********************/
int Eucalyptus_BatchSystem::makeEucalyptusReservation(int minVMCount, int maxVMCount)
{
    // TODO: fix this crude way of saying "I'm empty":
    vmNames = NULL;
    vmIPs = NULL;
    vmPrivIPs = NULL;

    actualCount = 0;
    struct soap *s = newDefaultSoap();

    if(s)
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
        rirespp->groupSet->__sizeitem = 0;
        rirespp->groupSet->item = NULL;
        struct ec2__RunInstancesResponseType response;
        int index;
        if(soap_call___ec2__RunInstances(s, eucaURL, "RunInstances", rireqp, rirespp) == SOAP_OK)
        {
            actualCount = rirespp->instancesSet->__sizeitem;
#ifdef CLOUD_DEBUG
            fprintf(stdout, "INFO: Called RunInstances, reserved %d resources\n", actualCount);
#endif
            reservationId = strdup(rirespp->reservationId);
            vmNames = (char**)malloc(actualCount * sizeof(char*));
            vmIPs = (char**)malloc(actualCount * sizeof(char*));
            vmPrivIPs = (char**)malloc(actualCount * sizeof(char*));
            for(index=0;index<actualCount;index++)
            {
                item = &rirespp->instancesSet->item[index];
                vmNames[index] = strdup(item->instanceId);
                vmIPs[index] = strdup(item->dnsName);
                vmPrivIPs[index] = strdup(item->privateDnsName);
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
        // TODO: do the WHOLE cleanup, a lot more structs were allocated
        free(rireqp);
        free(rirespp);
    }
    else
    {
#ifdef CLOUD_DEBUG
        fprintf(stderr, "ERROR: Could not initialize default SOAP message.\n", eucaURL);
#endif
        return 1;
    }
    return 0;
}

int Eucalyptus_BatchSystem::terminateEucalyptusInstance()
{
    struct soap *s = newDefaultSoap();

    if(s)
    {
        int index;
        struct ec2__TerminateInstancesType * titp;
        struct ec2__TerminateInstancesResponseType * tirtp;

        titp = (struct ec2__TerminateInstancesType *)malloc(sizeof(struct ec2__TerminateInstancesType));
        titp->instancesSet = (struct ec2__TerminateInstancesInfoType*)malloc(sizeof(struct ec2__TerminateInstancesInfoType));
        titp->instancesSet->__sizeitem = actualCount;
        titp->instancesSet->item = (struct ec2__TerminateInstancesItemType*)malloc(actualCount * sizeof(struct ec2__TerminateInstancesItemType));
        for(index = 0;index<actualCount;index++)
            titp->instancesSet->item[index].instanceId = vmNames[index];

        tirtp = (struct ec2__TerminateInstancesResponseType*)malloc(sizeof(struct ec2__TerminateInstancesResponseType));
        tirtp->instancesSet = (struct ec2__TerminateInstancesResponseInfoType*)malloc(sizeof(struct ec2__TerminateInstancesResponseInfoType));
        if(soap_call___ec2__TerminateInstances(s, eucaURL, "TerminateInstances", titp, tirtp) == SOAP_OK)
        {
#ifdef CLOUD_DEBUG
            fprintf(stdout, "INFO: Called 'TerminateInstances'\n");
            for(index=0;index<tirtp->instancesSet->__sizeitem;index++)
                fprintf(stdout, "\nnew state is: %s\n", tirtp->instancesSet->item[index].shutdownState->name);
#endif
        }
        else // an error occurred
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
        fprintf(stderr, "ERROR: Could not initialize default SOAP message.\n", eucaURL);
#endif
        return 1;
    }
    return 0;
}

int Eucalyptus_BatchSystem::describeInstances()
{
    struct soap * s;
    int index;
    s = newDefaultSoap();
    if(s == NULL)
    {
#ifdef CLOUD_DEBUG
        fprintf(stderr, "ERROR: Could not create default SOAP message\n");
#endif
        return 1;
    }
    struct ec2__DescribeInstancesType*ditp;
    struct ec2__DescribeInstancesResponseType *respp;
    struct ec2__InstanceStateType *state;
    struct ec2__DescribeInstancesItemType *item;
    struct ec2__RunningInstancesItemType *runningItem;
    struct ec2__ReservationInfoType *reservation;

    ditp = (struct ec2__DescribeInstancesType*)malloc(sizeof(struct ec2__DescribeInstancesType));
    ditp->instancesSet = (struct ec2__DescribeInstancesInfoType*)malloc(sizeof(struct ec2__DescribeInstancesInfoType));
    ditp->instancesSet->item = (struct ec2__DescribeInstancesItemType*)malloc(actualCount * sizeof(struct ec2__DescribeInstancesItemType));
    ditp->instancesSet->__sizeitem = actualCount;
    for(index = 0;index<actualCount;index++)
        ditp->instancesSet->item[index].instanceId = vmNames[index];
    respp = (struct ec2__DescribeInstancesResponseType *)malloc(sizeof(struct ec2__DescribeInstancesResponseType));
    respp->reservationSet = NULL;

    if(s != NULL && soap_call___ec2__DescribeInstances(s, eucaURL, "DescribeInstances", ditp, respp) == SOAP_OK)
    {
#ifdef CLOUD_DEBUG
        fprintf(stderr, "INFO: Found %d reservations(s)\n", respp->reservationSet->__sizeitem);
#endif
        int r;
        for(r=0;r<respp->reservationSet->__sizeitem;r++)
            if(strcmp(respp->reservationSet->item[r].reservationId, reservationId) == 0)
            {
                reservation = &respp->reservationSet->item[r];
                for(index=0;index<reservation->instancesSet->__sizeitem;index++)
                {
                    runningItem = &reservation->instancesSet->item[index];
                    state = runningItem->instanceState;
                    vmNames[index] = strdup(runningItem->instanceId);
                    vmIPs[index] = strdup(runningItem->dnsName);
                    vmPrivIPs[index] = strdup(runningItem->privateDnsName);
#ifdef CLOUD_DEBUG
                    fprintf(stdout, "INFO: VM with ID '%s' has state '%s' and IP '%s'\n",
                        runningItem->instanceId, state->name, runningItem->dnsName);
#endif
                }
            }
    }
    else // an error occurred
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
char * Eucalyptus_BatchSystem::GetStringValueOrNull(char*tmp, Parsers::Results::param_type_t param)
{
    tmp = (char*)Parsers::Results::getParamValue(param);
    if(tmp == NULL)
        return NULL;
    return strdup(tmp);
}

int Eucalyptus_BatchSystem::init(char*pathToPrivateKey, char*pathToCert)
{
    if(PEM_read_PK(pathToPrivateKey, &rsa_private_key))
    {
#ifdef CLOUD_DEBUG
        fprintf(stderr, "ERROR: Cannot read Private Key from file '%s'.\n", pathToPrivateKey);
#endif
        return 1;
    }
    else if(PEM_read_X509_cert(pathToCert, &cert))
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
    if(s == NULL)
    {
#ifdef CLOUD_LOG
        fprintf(stderr, "ERROR: Cannot create new SOAP message\n");
#endif
        return NULL;
    }
#ifdef CLOUD_LOG
    soap_set_recv_logfile(s, "RECV.log");
    soap_set_sent_logfile(s, "SENT.log");
    soap_set_test_logfile(s, "TEST.log");
#endif
    soap_register_plugin(s, soap_wsse);
    if(soap_wsse_add_security_header(s, rsa_private_key, cert))
    { // Error branch
        free(s);
#ifdef CLOUD_LOG
        fprintf(stderr, "ERROR: Cannot add security headers to the SOAP message\n");
#endif
    }
    else
    { // ok branch
        return s;
    }
    return NULL;
}
