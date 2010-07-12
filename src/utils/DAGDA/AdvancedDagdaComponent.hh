/***********************************************************/
/* Advanced Dagda component implementation.                */
/*                                                         */
/*  Author(s):                                             */
/*    - Gael Le Mahec (lemahec@clermont.in2p3.fr)          */
/*                                                         */
/* $LICENSE$                                               */
/***********************************************************/
/* $Id$
 * $Log$
 * Revision 1.6  2010/07/12 16:14:12  glemahec
 * DIET 2.5 beta 1 - Use the new ORB manager and allow the use of SSH-forwarders for all DIET CORBA objects
 *
 * Revision 1.5  2008/11/07 14:32:14  bdepardo
 * Headers correction
 *
 *
 ***********************************************************/

#ifndef __ADVANCED_DAGDA_COMPONENT_HH__
#define __ADVANCED_DAGDA_COMPONENT_HH__

#include "DagdaImpl.hh"
#include "NetworkStats.hh"

#include <map>
#include <string>

#include <ctime>

#define DGD_OBJ_TYPE(obj) ((obj).desc.specific._d()==DIET_FILE ? DIET_FILE_OBJ:DIET_MEM_OBJ)


class AdvancedDagdaComponent;

typedef enum { DIET_MEM_OBJ, DIET_FILE_OBJ } dagda_object_type_t;
typedef int (*managementFunc)(AdvancedDagdaComponent* manager, size_t needed,
  dagda_object_type_t type);
class AdvancedDagdaComponent : public SimpleDagdaImpl {
private:
  managementFunc mngFunction;
  std::map<std::string, clock_t> registerTime;
  std::map<std::string, clock_t> lastUsageTime;
  std::map<std::string, unsigned long> nbUsage;
  omni_mutex registerMutex;
  omni_mutex lastUsageMutex;
  omni_mutex nbUsageMutex;
  NetworkStats* stats;
  bool shareFiles;
public:
  AdvancedDagdaComponent(dagda_manager_type_t t) :
    SimpleDagdaImpl(t), mngFunction(NULL), /*lastUsageTime(),*/
	/*registerTime(), nbUsage(),*/ stats(NULL), shareFiles(false) {}
	
  AdvancedDagdaComponent(dagda_manager_type_t t, managementFunc function) :
    SimpleDagdaImpl(t), mngFunction(function), /*lastUsageTime(),*/
	/*registerTime(), nbUsage(),*/ stats(NULL), shareFiles(false) {}
	
  AdvancedDagdaComponent(dagda_manager_type_t t, managementFunc function,
    NetworkStats* stats) :
	SimpleDagdaImpl(t), mngFunction(function), /*lastUsageTime(),*/
	/*registerTime(), nbUsage(),*/ stats(stats), shareFiles(false) {}
	
  AdvancedDagdaComponent(dagda_manager_type_t t, NetworkStats* stats) :
    SimpleDagdaImpl(t), mngFunction(NULL), /*lastUsageTime(),*/
	/*registerTime(), nbUsage(),*/ stats(stats), shareFiles(false) {}

  // With file sharing.
  AdvancedDagdaComponent(dagda_manager_type_t t, bool shareFiles) :
    SimpleDagdaImpl(t), mngFunction(NULL), /*lastUsageTime(),*/
	/*registerTime(), nbUsage(),*/ stats(NULL), shareFiles(shareFiles) {}
	
  AdvancedDagdaComponent(dagda_manager_type_t t, managementFunc function, bool shareFiles) :
    SimpleDagdaImpl(t), mngFunction(function), /*lastUsageTime(),*/
	/*registerTime(), nbUsage(),*/ stats(NULL), shareFiles(shareFiles) {}
	
  AdvancedDagdaComponent(dagda_manager_type_t t, managementFunc function,
    NetworkStats* stats, bool shareFiles) :
	SimpleDagdaImpl(t), mngFunction(function), /*lastUsageTime(),*/
	/*registerTime(), nbUsage(),*/ stats(stats), shareFiles(shareFiles) {}
	
  AdvancedDagdaComponent(dagda_manager_type_t t, NetworkStats* stats, bool shareFiles) :
    SimpleDagdaImpl(t), mngFunction(NULL), /*lastUsageTime(),*/
	/*registerTime(), nbUsage(),*/ stats(stats), shareFiles(shareFiles) {}


  virtual char* sendFile(const corba_data_t &data, const char* dest);
  virtual char* sendData(const char* ID, const char* dest);

  virtual void lclAddData(const char* src, const corba_data_t& data);
  virtual void registerFile(const corba_data_t& data);
  std::map<std::string, corba_data_t>* getData() { return SimpleDagdaImpl::getData(); }
  virtual corba_data_t* getData(const char* dataID);

  virtual char* getBestSource(const char* dest, const char* dataID);
  
  virtual clock_t getRegisterTime(const char* dataID);
  virtual clock_t getLastUsageTime(const char* dataID);
  virtual unsigned long getNbUsage(const char* dataID);
  
  virtual void setRegisterTime(const char* dataID, clock_t time);
  virtual void setRegisterTime(const char* dataID);
  virtual void setUsageTime(std::string id, clock_t time);
  virtual void setUsageTime(std::string id);
  virtual void incNbUsage(const char* dataID);
  virtual void shareData(const corba_data_t& data);
};
#endif
