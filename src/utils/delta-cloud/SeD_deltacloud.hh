/**
 * @file Sed_deltacloud.hh
 *
 * @brief  DIET SeD_deltacloud class header
 *
 * @author  Lamiel TOCH (lamiel.toch@ens-lyon.fr)
 *          Yulin ZHANG (huaxi.zhang@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */

#ifndef _SED_DELTACLOUD_HH_
#define _SED_DELTACLOUD_HH_

#include "DIET_data.h"
#include "DIET_server.h"
#include "ServiceWrapperArgument.hh"
#include "ServiceWrapper.hh"
#include "SeDCloud.hh"

#include <string>
#include <vector>
#include <map>

#include "Instance.hh"
#include "Tools.hh"

#include <stdio.h>
#include <set>

#include "DagdaImpl.hh"
#include "DagdaFactory.hh"

#ifdef USE_LOG_SERVICE
#include "DietLogComponent.hh"
#endif

#define HOMOGENEOUS_VM_INSTANCIATION_SERVICE "homogeneous_vm_instanciation"

typedef int (* dietcloud_callback_t)(diet_profile_t*);

class ServiceWrapper;

/*typedef union {
	int diet_profile_arg;
	char* command_line_arg;
} UServiceWrapperArgument;
*/






//template <class Actions>
//std::map<std::string, CloudServiceBinary> SeDCloudActions::cloud_service_binaries;

//std::map<std::string, IaaS::VMInstances*> SeDCloud::reserved_vms;

/**************BEGIN : classical DIET services**************/

void service_time_solve_add();
int time_solve(diet_profile_t *pb);

void service_add_seq_in_data_xml_add();
int add_seq_in_data_xml_solve(diet_profile_t *pb);

/**************END : classical DIET services**************/


#endif
