/***********************************************************/
/* Advanced Dagda component implementation.                */
/*                                                         */
/*  Author(s):                                             */
/*    - Gael Le Mahec (lemahec@clermont.in2p3.fr)          */
/*                                                         */
/* $LICENSE$                                               */
/***********************************************************/
/* $Id
/* $Log
/*														   */
/***********************************************************/

#include "AdvancedDagdaComponent.hh"
#include "DIET_data_internal.hh"

#include <iostream>
#include <fstream>

using namespace std;

void AdvancedDagdaComponent::lclAddData(Dagda_ptr src, const corba_data_t& data) {
  try {
    SimpleDagdaImpl::lclAddData(src, data);
	registerTime[string(data.desc.id.idNumber)]=clock();
  } catch (Dagda::NotEnoughSpace ex) {
    cout << "****************************************" << endl;
	cout << "* Call to the data management function *" << endl;
	cout << "****************************************" << endl;
    if (mngFunction!=NULL) {
	  cout << "On essaye de trouver " << data_sizeof(&data.desc) << " octets..." << endl;
	  // !!!! CALCULER LA TAILLE NECESSAIRE !!!!
	  size_t needed = data_sizeof(&data.desc);
	  size_t max;
	  size_t used;
	  if (DGD_OBJ_TYPE(data)==DIET_MEM_OBJ) {
	    max = getMemMaxSpace();
		used = getUsedMemSpace();
	  } else {
	    max = getDiskMaxSpace();
		used = getUsedDiskSpace();
	  }
      if (used+needed>max) needed += used-max;
	  
	  if (mngFunction(this, needed, DGD_OBJ_TYPE(data)))
	    throw Dagda::NotEnoughSpace(ex.available);
	  SimpleDagdaImpl::lclAddData(src, data);
	  registerTime[string(data.desc.id.idNumber)]=clock();
	} else
	  throw Dagda::NotEnoughSpace(ex.available);
  }
}

void AdvancedDagdaComponent::registerFile(const corba_data_t& data) {
  if (strcmp(data.desc.specific.file().path, "")==0)
    throw Dagda::InvalidPathName(data.desc.id.idNumber,
	  data.desc.specific.file().path);
  ifstream file(data.desc.specific.file().path);
  if (!file.is_open())
    throw Dagda::UnreachableFile(data.desc.specific.file().path);
  file.close();
  addData(data);
  unlockData(data.desc.id.idNumber);
}

corba_data_t* AdvancedDagdaComponent::getData(const char* dataID) {
  lastUsageTime[string(dataID)]=clock();
  nbUsage[string(dataID)]++;
  return SimpleDagdaImpl::getData(dataID);
}

Dagda_ptr AdvancedDagdaComponent::getBestSource(Dagda_ptr dest, const char* dataID) {
  return SimpleDagdaImpl::getBestSource(dest, dataID);
}

time_t AdvancedDagdaComponent::getRegisterTime(const char* dataID) {
  if (registerTime.find(dataID)==registerTime.end())
    return clock();
  return registerTime[dataID];
}

time_t AdvancedDagdaComponent::getLastUsageTime(const char* dataID) {
  if (lastUsageTime.find(dataID)==lastUsageTime.end())
    return clock();
  return lastUsageTime[dataID];
}

unsigned long AdvancedDagdaComponent::getNbUsage(const char* dataID) {
  if (nbUsage.find(dataID)==nbUsage.end())
    return 0;
  return nbUsage[dataID];
}
