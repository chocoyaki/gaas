/**
 * @file  GlobalSchedulers.cc
 *
 * @brief  DIET agent globlal schedulers implementation : add yours !!!
 *
 * @author  Philippe COMBES (Philippe.Combes@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */


#include <cstdio>
#include <cstring>
#include <iostream>

#include "GlobalSchedulers.hh"
#include "debug.hh"
#include "Vector.h"

#if WIN32 
#define snprintf sprintf_s 
#endif 

/* New : For scheduler load support. */
#ifdef USERSCHED
#include "UserScheduler.hh"
#include "configuration.hh"
#endif

/** The trace level. */
extern unsigned int TRACE_LEVEL;

// Use SCHED_CLASS for the name of the class
// (this->name cannot be used in static member functions)
#define SCHED_TRACE_FUNCTION(formatted_text)            \
  TRACE_TEXT(TRACE_ALL_STEPS, SCHED_CLASS << "::");     \
  TRACE_FUNCTION(TRACE_ALL_STEPS, formatted_text)


/****************************************************************************/
/* GlobalScheduler: parent class                                            */
/****************************************************************************/
#undef SCHED_CLASS
#define SCHED_CLASS "GlobalScheduler"

GlobalScheduler::GlobalScheduler() {
  this->name = "GlobalScheduler";
}

/** Destroy the schedulers list */
GlobalScheduler::~GlobalScheduler() {
  this->schedulers.emptyIt();
}


/**
 * Return the GlobalScheduler deserialized from the string
 * \c serializedScheduler.
 */
GlobalScheduler *
GlobalScheduler::deserialize(const char *serializedScheduler) {
  SCHED_TRACE_FUNCTION(serializedScheduler);
  int nameLength;

  // TODO: is the isolated scope required ?
  {
    const char *colon;
    if ((colon = strchr(serializedScheduler, ':'))) {
      nameLength = colon - serializedScheduler;
    } else {
      nameLength = strlen(serializedScheduler);
    }
  }

  if (!strncmp(serializedScheduler, StdGS::stName, nameLength)) {
    return StdGS::deserialize(serializedScheduler);
  } else if (!strncmp(serializedScheduler, PriorityGS::stName, nameLength)) {
    return PriorityGS::deserialize(serializedScheduler);
  } else
  /* New : For scheduler load support. */
#ifdef USERSCHED
  if (!strncmp(serializedScheduler, UserScheduler::stName, nameLength)) {
    std::string moduleName;
    if (!CONFIG_STRING(diet::MODULENAME, moduleName)) {
      WARNING("moduleName parameter is not set in the configuration file; "
              << "reverting to default (StdGS)\n");
      return (GlobalScheduler::chooseGlobalScheduler());
    }
    try {
      return UserScheduler::deserialize(serializedScheduler,
                                        moduleName.c_str());
    } catch (InstanciationError &error) {
      WARNING("unable to load module " << moduleName << "; "
                                       << "reverting to default (StdGS)\n" <<
              "Message : " << error.message() << "\n");
      return (GlobalScheduler::chooseGlobalScheduler());
    }
  } else
#endif // ifdef USERSCHED
       /***********************************/
  {
    WARNING("unable to deserialize global scheduler; "
            << "reverting to default (StdGS)");
    WARNING("scheduler was \""
            << serializedScheduler << "\"\n");
    return (GlobalScheduler::chooseGlobalScheduler());
  }
} // deserialize

/** Return the serialized global scheduler (a string). */
char *
GlobalScheduler::serialize(GlobalScheduler *GS) {
  SCHED_TRACE_FUNCTION(GS->name);

  if (!strncmp(GS->name, StdGS::stName, GS->nameLength)) {
    return StdGS::serialize((StdGS *) GS);
  } else if (!strncmp(GS->name, PriorityGS::stName, GS->nameLength)) {
    return PriorityGS::serialize((PriorityGS *) GS);

    /* New : For scheduler load support. */
#ifdef USERSCHED
  } else if (!strncmp(GS->name, UserScheduler::stName, GS->nameLength)) {
    return UserScheduler::serialize(GS);

#endif
    /*************************************/
  } else {
    ERROR_DEBUG("unable to serialize global scheduler named " << GS->name, NULL);
  }
} // serialize

/** Returns a global scheduler adapted to the request. */
GlobalScheduler *
GlobalScheduler::chooseGlobalScheduler() {
  StdGS *res = new StdGS();
  res->init();
  return res;
}

/* Let the corba_request be a parameter in order to have the possibility
   of a globalScheduler being function for example from where the request
   comes from, etc.
 */

GlobalScheduler *
GlobalScheduler::chooseGlobalScheduler(const corba_request_t *req,
                                       const corba_profile_desc_t *profile) {
  corba_aggregator_desc_t agg = profile->aggregator;

  /* New : For scheduler load support. */
#ifdef USERSCHED
  GlobalScheduler *loaded;
#endif
  /*************************************/

  switch (agg.agg_specific._d()) {
  case DIET_AGG_DEFAULT:
    return (GlobalScheduler::chooseGlobalScheduler());

    break;
  case DIET_AGG_PRIORITY: {
    PriorityGS *ps = new PriorityGS(agg.agg_specific.agg_priority());
    ps->init();
    return (ps);
  }
  break;
    /* New : For scheduler load support. */
#ifdef USERSCHED
  case DIET_AGG_USER: {
    std::string moduleName;

    if (!CONFIG_STRING(diet::MODULENAME, moduleName)) {
      WARNING("moduleName parameter is not set in the configuration file; "
              << "reverting to default (StdGS)\n");
      return (GlobalScheduler::chooseGlobalScheduler());
    }
    try {
      loaded = UserScheduler::instanciate(moduleName.c_str());
    } catch (InstanciationError &error) {
      WARNING("unable to load module " << moduleName << "; "
                                       << "reverting to default (StdGS)\n"
                                       << "Error : " << error.message() << "\n");
      return (GlobalScheduler::chooseGlobalScheduler());
    }
    SCHED_TRACE_FUNCTION("Module " << moduleName << " loaded.");
    return loaded;
  }
  break;
#endif // ifdef USERSCHED
       /*************************************/
  } // switch
  ERROR_DEBUG(__FUNCTION__ <<
        ": unhandled aggregator (" <<
        agg.agg_specific._d() <<
        ")\n", 0);
  return 0;
} // chooseGlobalScheduler


/**
 * Aggregate and sort all servers of the \c responses. This method provides a
 * default behaviour for aggregation of sorted responses: a simple merge.
 * @param aggrResp     the aggregated response (caller-allocated).
 * @param max_srv      maximum number of servers to aggregate (all if 0)
 * @param nb_responses number of responses to aggregate.
 * @param responses    array of the responses to aggregate.
 */
int
GlobalScheduler::aggregate(corba_response_t *aggrResp, size_t max_srv,
                           const size_t nb_responses,
                           const corba_response_t *responses) {
  size_t total_size = 0;
  // lock the schedulers
  SchedList::Iterator *iter = this->schedulers.getIterator();

  int lastAggregated = -1;
  int *lastAggr = new int[nb_responses];

  /*
  ** structure for caching estVector structures.  initially this
  ** is a vector with as many empty vectors as there are
  ** responses.  during the aggregation phase, this cache will
  ** be populated.  at the end of aggregation, this cache needs
  ** to be purged (see below).
  */
  Vector_t evCache = new_Vector();

  SCHED_TRACE_FUNCTION("nb_responses=" << nb_responses
                                       << ", max_srv=" << max_srv);

  for (size_t i = 0; i < nb_responses; i++) {
    lastAggr[i] = -1;
    total_size += responses[i].servers.length();
    Vector_add(evCache, new_Vector());
  }
  if (max_srv == 0) {
    max_srv = total_size;  // keep all servers
  }
  aggrResp->servers.length(MIN(total_size, max_srv));

  while (iter->hasCurrent()) {
    Scheduler *sched = iter->getCurrent();

    sched->aggregate(*aggrResp,
                     &lastAggregated,
                     nb_responses,
                     responses,
                     lastAggr,
                     evCache);
    iter->next();
  }

  {  /* purge the estVector cache */
    while (!Vector_isEmpty(evCache)) {
      Vector_t respV = (Vector_t) Vector_removeAtPosition(evCache, 0);
      while (!Vector_isEmpty(respV)) {
        Vector_removeAtPosition(respV, 0);
      }
      free_Vector(respV);
    }
    free_Vector(evCache);
  }

  delete iter;  // unlock the schedulers list
  delete [] lastAggr;

  return 0;
} // aggregate


/****************************************************************************/
/* StdGS (Standard Global Scheduler)                                        */
/****************************************************************************/
#undef SCHED_CLASS
#define SCHED_CLASS "StdGS"

const char *StdGS::stName = "StdGS";

StdGS::StdGS() {
  this->name = this->stName;
  this->nameLength = strlen(this->name);
}

StdGS::~StdGS() {
}

/** Initialize this global scheduler (build its list of schedulers). */
void
StdGS::init() {
  // schedulers has already been contructed by implicit call to parent class
  // constructor.
  this->schedulers.addElement(new RRScheduler());
  this->schedulers.addElement(new RandScheduler());
}

StdGS *
StdGS::deserialize(const char *serializedScheduler) {
  char *token(0);
  char *ser_sched = strdup(serializedScheduler);
  char *ptr = ser_sched;
  StdGS *res = new StdGS();

  TRACE_TEXT(TRACE_ALL_STEPS,
             "StdGS::deserialize(" << serializedScheduler << ")\n");
  token = strsep(&ptr, ":");
  assert(!strcmp(token, StdGS::stName));
  while (ptr) {
    // ptr == NULL when the last token is identified (no more ':')
    // If the string was not duplicated in this function, we should
    // reset the delimiter with
    // ptr[-1] = ':';
    token = strsep(&ptr, ":");
    res->schedulers.addElement(Scheduler::deserialize(token));
  }
  free(ser_sched);
  return res;
} // deserialize

char *
StdGS::serialize(StdGS *GS) {
  size_t maxLength = 128;
  char *res = new char[maxLength];
  SchedList::Iterator *iter = GS->schedulers.getIterator();
  size_t length = GS->nameLength;

  SCHED_TRACE_FUNCTION(GS->name);
  snprintf(res, 128, "%s", GS->name);
  while (iter->hasCurrent()) {
    Scheduler *sched = iter->getCurrent();
    char *tmp = Scheduler::serialize(sched);
    size_t tmp_length = strlen(tmp) + 1;  // Add 1 for the ':'
    if ((length + tmp_length) >= maxLength) {
      maxLength += 128;
      char *new_res = new char[maxLength];
      snprintf(new_res, 128, "%s:%s", res, tmp);
      delete [] res;
      res = new_res;
    } else {
      snprintf((res + length), (128 - length), ":%s", tmp);
    }
    length += tmp_length;
    delete [] tmp;
    iter->next();
  }
  delete iter;
  return res;
} // serialize


/****************************************************************************/
/* PriorityGS (Priority List Global Scheduler)                              */
/****************************************************************************/
#undef SCHED_CLASS
#define SCHED_CLASS "PriorityGS"

const char *PriorityGS::stName = "PriorityGS";

PriorityGS::PriorityGS() {
  this->name = this->stName;
  this->nameLength = strlen(this->name);
}

PriorityGS::PriorityGS(corba_agg_priority_t priority) {
  this->name = this->stName;
  this->nameLength = strlen(this->name);
  this->numPValues = priority.priorityList.length();
  this->pValues = new int[this->numPValues];
  for (int pvIter = 0; pvIter < this->numPValues; pvIter++) {
    this->pValues[pvIter] = priority.priorityList[pvIter];
  }
}

PriorityGS::~PriorityGS() {
}

/** Initialize this global scheduler (build its list of schedulers). */
void
PriorityGS::init() {
  this->schedulers.addElement(new PriorityScheduler(this->numPValues,
                                                    this->pValues));
}

PriorityGS *
PriorityGS::deserialize(const char *serializedScheduler) {
  char *token;
  // duplicate the string because of CONST specifier in argument
  char *ser_sched = strdup(serializedScheduler);
  char *ptr = ser_sched;
  PriorityGS *res = new PriorityGS();

  TRACE_TEXT(TRACE_ALL_STEPS,
             "PriorityGS::deserialize("
             << serializedScheduler << ")\n");

  // Eliminate the first token, which is to be stName
  token = strsep(&ptr, ":");
  assert(!strcmp(token, PriorityGS::stName));

  // Then for each token add associated scheduler
  while (ptr) {
    // ptr == NULL when the last token is identified (no more ':')
    // If the string was not duplicated in this function, we should
    // reset the delimiter with
    // ptr[-1] = ':';
    token = strsep(&ptr, ":");
    res->schedulers.addElement(Scheduler::deserialize(token));
  }
  free(ser_sched);
  return res;
} // deserialize

char *
PriorityGS::serialize(PriorityGS *GS) {
  size_t maxLength = 128;
  char *res = new char[maxLength];
  SchedList::Iterator *iter = GS->schedulers.getIterator();
  size_t length = GS->nameLength;

  SCHED_TRACE_FUNCTION(GS->name);
  snprintf(res, 128, "%s", GS->name);
  while (iter->hasCurrent()) {
    Scheduler *sched = iter->getCurrent();
    char *tmp = Scheduler::serialize(sched);
    size_t tmp_length = strlen(tmp) + 1;  // Add 1 for the ':'
    if ((length + tmp_length) >= maxLength) {
      maxLength += 128;
      char *new_res = new char[maxLength];
      snprintf(new_res, maxLength, "%s:%s", res, tmp);
      delete [] res;
      res = new_res;
    } else {
      snprintf((res + length), (128 - length), ":%s", tmp);
    }
    length += tmp_length;
    delete [] tmp;
    iter->next();
  }
  delete iter;
  return res;
} // serialize
