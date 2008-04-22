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

#ifndef __ADVANCED_DAGDA_COMPONENT_HH__
#define __ADVANCED_DAGDA_COMPONENT_HH__

#include "DagdaImpl.hh"
#include "NetworkStats.hh"

#include <map>

#include <ctime>

#define DGD_OBJ_TYPE(obj) ((obj).desc.specific._d()==DIET_FILE ? DIET_FILE_OBJ:DIET_MEM_OBJ)


class AdvancedDagdaComponent;

typedef enum { DIET_MEM_OBJ, DIET_FILE_OBJ } dagda_object_type_t;
typedef int (*managementFunc)(AdvancedDagdaComponent* manager, size_t needed,
  dagda_object_type_t type);
class AdvancedDagdaComponent : public SimpleDagdaImpl {
private:
  managementFunc mngFunction;
  std::map<std::string, time_t> registerTime;
  std::map<std::string, time_t> lastUsageTime;
  std::map<std::string, unsigned long> nbUsage;
  NetworkStats* stats;
public:
  AdvancedDagdaComponent(dagda_manager_type_t t) : SimpleDagdaImpl(t), mngFunction(NULL) {}
  AdvancedDagdaComponent(dagda_manager_type_t t, managementFunc function) :
    SimpleDagdaImpl(t), mngFunction(function) {};
  AdvancedDagdaComponent(dagda_manager_type_t t, managementFunc function,
    NetworkStats* stats) : SimpleDagdaImpl(t), mngFunction(function), stats(stats) {};
	
  virtual char* sendFile(const corba_data_t &data, Dagda_ptr dest);
  virtual char* sendData(const char* ID, Dagda_ptr dest);

  virtual void lclAddData(Dagda_ptr src, const corba_data_t& data);
  virtual void registerFile(const corba_data_t& data);
  std::map<std::string, corba_data_t>* getData() { return SimpleDagdaImpl::getData(); }
  virtual corba_data_t* getData(const char* dataID);

  virtual Dagda_ptr getBestSource(Dagda_ptr dest, const char* dataID);
  
  virtual time_t getRegisterTime(const char* dataID);
  virtual time_t getLastUsageTime(const char* dataID);
  virtual unsigned long getNbUsage(const char* dataID);
};
#endif
