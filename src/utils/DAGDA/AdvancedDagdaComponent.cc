/**
* @file AdvancedDagdaComponent.cc
* 
* @brief  Advanced Dagda component implementation   
* 
* @author  - Gael Le Mahec (lemahec@clermont.in2p3.fr)
* 
* @section Licence
*   |LICENSE|                                                                
*/
/* $Id$
 * $Log$
 * Revision 1.11  2011/02/10 23:19:02  hguemar
 * fixes some issues detected by latest cppcheck (1.47)
 *
 * Revision 1.10  2011/02/08 23:56:31  bdepardo
 * Removed useless \ in code
 *
 * Revision 1.9  2011/02/08 23:49:30  bdepardo
 * Removed useless \ in code
 *
 * Revision 1.8  2011/01/21 18:12:02  bdepardo
 * Prefer prefix ++/-- operators for non-primitive types.
 *
 * Revision 1.7  2010/07/12 16:14:12  glemahec
 * DIET 2.5 beta 1 - Use the new ORB manager and allow the use of SSH-forwarders for all DIET CORBA objects
 *
 * Revision 1.6  2010/03/08 13:58:51  bisnard
 * compute data transfer time using real elapsed time instead of process time
 *
 * Revision 1.5  2008/11/07 14:32:14  bdepardo
 * Headers correction
 *
 *
 ***********************************************************/

#include "AdvancedDagdaComponent.hh"
#include "DIET_data_internal.hh"

#include <iostream>
#include <fstream>
#include <ctime>
#include <sys/time.h> // modif bisnard_logs_1
#include "debug.hh"
//#include "ORBMgr2.hh"

using namespace std;

// modif bisnard_logs_1
char* AdvancedDagdaComponent::sendFile(const corba_data_t &data, const char* destName) {
  struct timeval tv1;
  struct timeval tv2;
  char* ret;

  gettimeofday(&tv1, NULL);
  ret = DagdaImpl::sendFile(data, destName);
  gettimeofday(&tv2, NULL);

  double elapsed = (tv2.tv_sec-tv1.tv_sec) * 1000000
    + (tv2.tv_usec-tv1.tv_usec);
  TRACE_TEXT(TRACE_ALL_STEPS, "Data " << data.desc.id.idNumber
             << " transfered in " << elapsed << " microsecond(s)." << endl);

  //   if (getLogComponent())
  //     getLogComponent()->logDataTransferTime(data.desc.id.idNumber,
  //                                            dest->getID(),
  //                                            (unsigned long) elapsed);

  if (stats != NULL && elapsed != 0) {
    stats->addStat(string(getID()), destName,
                   ((double) data_sizeof(&data.desc))/elapsed);
    stats->addStat(string(destName), string(getID()),
                   ((double) data_sizeof(&data.desc))/elapsed);
  }
  return ret;
}

char* AdvancedDagdaComponent::sendData(const char* ID, const char* destName) {
  struct timeval tv1;
  struct timeval tv2;
  char* ret;
  corba_data_t* data = getData(ID);
  size_t dataSize = data_sizeof(&data->desc);

  gettimeofday(&tv1, NULL);
  ret = DagdaImpl::sendData(ID, destName);
  gettimeofday(&tv2, NULL);

  double elapsed = (tv2.tv_sec-tv1.tv_sec) * 1000000
    + (tv2.tv_usec-tv1.tv_usec);
  TRACE_TEXT(TRACE_ALL_STEPS, "Data " << data->desc.id.idNumber
             << " transfered in " << elapsed << " microsecond(s)." << endl);

  //   if (getLogComponent())
  //     getLogComponent()->logDataTransferTime(data->desc.id.idNumber,
  //                                            dest->getID(),
  //                                            (unsigned long) elapsed);

  if (stats != NULL && elapsed != 0) {
    stats->addStat(string(getID()), destName,
                   ((double) dataSize)/elapsed);
    stats->addStat(destName, string(getID()),
                   ((double) dataSize)/elapsed);
  }
  return ret;
}
// end modif bisnard_logs_1

void AdvancedDagdaComponent::lclAddData(const char* srcName, const corba_data_t& data) {
  try {
    SimpleDagdaImpl::lclAddData(srcName, data);
    setRegisterTime(data.desc.id.idNumber);
    if (shareFiles && DGD_OBJ_TYPE(data)==DIET_FILE_OBJ)
      shareData(data);
  } catch (Dagda::NotEnoughSpace& ex) {
    TRACE_TEXT(TRACE_ALL_STEPS, "Needs more space. Try to call the selected cache "
               << "algorithm." << endl);
    if (mngFunction != NULL) {
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
      SimpleDagdaImpl::lclAddData(srcName, data);
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

char* AdvancedDagdaComponent::getBestSource(const char* dest, const char* dataID) {
  SeqString* managers = pfmGetDataManagers(dataID);

  if (managers->length()==0)
    throw Dagda::DataNotFound(dataID);

  if (stats != NULL) {
    TRACE_TEXT(TRACE_ALL_STEPS, "Data " << dataID << " has " <<
               managers->length() << " replica(s) on the platform." << endl);
    double maxStat = 0;
    string found = string((*managers)[0]);

    for (unsigned int i = 0; i<managers->length(); ++i) {
      string curMngr = string((*managers)[i]);
      double curStat = stats->getStat(string(getID()), curMngr);
      if (stats->cmpStats(curStat, maxStat)) {
        maxStat = curStat;
        found = curMngr;
      }
    }
    return CORBA::string_dup(found.c_str());
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
  std::map<string, Dagda_ptr>::iterator itch;

  childrenMutex.lock();
  for (itch = getChildren()->begin();itch != getChildren()->end();)
    try {
      (*itch).second->registerFile(data);
      ++itch;
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
