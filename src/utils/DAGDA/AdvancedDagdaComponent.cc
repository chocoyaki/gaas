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
#include <ctime>
#include "debug.hh"

using namespace std;

char* AdvancedDagdaComponent::sendFile(const corba_data_t &data, Dagda_ptr dest) {
  clock_t begin;
  clock_t end;
  char* ret;
  
  begin = clock();
  ret = DagdaImpl::sendFile(data, dest);
  end = clock();
  
  double elapsed=end-begin;
  TRACE_TEXT(TRACE_ALL_STEPS, "Data " << data.desc.id.idNumber <<
    " transfered in " << elapsed << " time unit(s)." << endl);

  if (stats!=NULL && elapsed!=0) {
    stats->addStat(string(getID()), string(dest->getID()),
      ((double) data_sizeof(&data.desc))/elapsed);
	stats->addStat(string(dest->getID()), string(getID()),
      ((double) data_sizeof(&data.desc))/elapsed);
  }
  return ret;
}

char* AdvancedDagdaComponent::sendData(const char* ID, Dagda_ptr dest) {
  clock_t begin;
  clock_t end;
  char* ret;
  corba_data_t* data = getData(ID);
  size_t dataSize = data_sizeof(&data->desc);
  
  begin = clock();
  ret = DagdaImpl::sendData(ID, dest);
  end = clock();
  
  double elapsed=end-begin;
  TRACE_TEXT(TRACE_ALL_STEPS, "Data " << data->desc.id.idNumber <<
    " transfered in " << elapsed << " time unit(s)." << endl);

  if (stats!=NULL && elapsed!=0) {
    stats->addStat(string(getID()), string(dest->getID()),
      ((double) dataSize)/elapsed);
	stats->addStat(string(dest->getID()), string(getID()),
      ((double) dataSize)/elapsed);
  }
  return ret;
}

void AdvancedDagdaComponent::lclAddData(Dagda_ptr src, const corba_data_t& data) {
  try {
    SimpleDagdaImpl::lclAddData(src, data);
	setRegisterTime(data.desc.id.idNumber);
	if (shareFiles && DGD_OBJ_TYPE(data)==DIET_FILE_OBJ)
	  shareData(data);
  } catch (Dagda::NotEnoughSpace ex) {
	TRACE_TEXT(TRACE_ALL_STEPS, "Needs more space. Try to call the selected cache "
	  << "algorithm." << endl);
    if (mngFunction!=NULL) {
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
	  setRegisterTime(data.desc.id.idNumber);
	  if (shareFiles && DGD_OBJ_TYPE(data)==DIET_FILE_OBJ)
	    shareData(data);
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
  string id(dataID);

  setUsageTime(id);
  incNbUsage(dataID);

  return SimpleDagdaImpl::getData(dataID);
}

Dagda_ptr AdvancedDagdaComponent::getBestSource(Dagda_ptr dest, const char* dataID) {
  SeqDagda_t* managers = pfmGetDataManagers(dataID);

  if (managers->length()==0)
    throw Dagda::DataNotFound(dataID);

  if (stats!=NULL) {
    TRACE_TEXT(TRACE_ALL_STEPS, "Data " << dataID << " has " <<
             managers->length() << " replica(s) on the platform." << endl);
    double maxStat=0;
	Dagda_ptr found = (*managers)[0];

    for (unsigned int i=0; i<managers->length(); ++i) {
	  Dagda_ptr curMngr = (*managers)[i];
      double curStat = stats->getStat(string(getID()), string(curMngr->getID()));
	  if (stats->cmpStats(curStat, maxStat)) {
	    maxStat = curStat;
		found = curMngr;
	  }
    }
	return found;
  }

  return SimpleDagdaImpl::getBestSource(dest, dataID);
}

clock_t AdvancedDagdaComponent::getRegisterTime(const char* dataID) {
  registerMutex.lock();
  clock_t ret = clock();
  if (registerTime.find(dataID)!=registerTime.end())
    ret = registerTime[dataID];
  registerMutex.unlock();
  return ret;
}

void AdvancedDagdaComponent::setRegisterTime(const char* dataID, clock_t time) {
  registerMutex.lock();
  registerTime[dataID] = time;
  registerMutex.unlock();
}

void AdvancedDagdaComponent::setRegisterTime(const char* dataID) {
  setRegisterTime(dataID, clock());
}

clock_t AdvancedDagdaComponent::getLastUsageTime(const char* dataID) {
  lastUsageMutex.lock();
  clock_t ret = clock();
  if (lastUsageTime.find(dataID)!=lastUsageTime.end()) {
    ret = lastUsageTime[dataID];
  }
  lastUsageMutex.unlock();
  return ret;
}

void AdvancedDagdaComponent::setUsageTime(string id, clock_t time) {
  lastUsageMutex.lock();
  lastUsageTime[id] = time;
  lastUsageMutex.unlock();
}

void AdvancedDagdaComponent::setUsageTime(string id) {
  setUsageTime(id, clock());
}

unsigned long AdvancedDagdaComponent::getNbUsage(const char* dataID) {
  nbUsageMutex.lock();
  unsigned long ret = 0;
  if (nbUsage.find(dataID)!=nbUsage.end())
    ret = nbUsage[dataID];
  nbUsageMutex.unlock();
  return ret;
}

void AdvancedDagdaComponent::incNbUsage(const char* dataID) {
  nbUsageMutex.lock();
  nbUsage[dataID]++;
  nbUsageMutex.unlock();
}

void AdvancedDagdaComponent::shareData(const corba_data_t& data) {
  std::map<string,Dagda_ptr>::iterator itch;
  
  childrenMutex.lock();
  for (itch=getChildren()->begin();itch!=getChildren()->end();)
    try {
      (*itch).second->registerFile(data);
	  itch++;
    } catch (CORBA::COMM_FAILURE& e1) {
	  getChildren()->erase(itch++);
    } catch (CORBA::TRANSIENT& e2) {
      getChildren()->erase(itch++);
    } catch (Dagda::InvalidPathName& e3) {
	  WARNING("Fail to share the file \"" << e3.path << "\" (invalid path name) on " <<
			  (*itch).second->getID() << " data manager.");
	} catch (Dagda::UnreachableFile& e4) {
	  WARNING("File \"" << e4.path << "\" is unreachable on " << (*itch).second->getID() <<
	          " data manager.");
	}
  childrenMutex.unlock();
}
