/**
 * @file  UserSchedulers.hh
 *
 * @brief   DIET UserScheduler class header
 *
 * @author   Gael LE MAHEC (lemahec@clermont.in2p3.fr)
 *
 * @section Licence
 *   |LICENCE|
 */
#ifndef _USERSCHEDULER_HH_
#define _USERSCHEDULER_HH_
#include "GlobalSchedulers.hh"
#include <list>


/** Utility macro to simplify the user source code. */
#define SCHEDULER_CLASS(T)                                      \
  extern "C" GlobalScheduler * constructor() {                   \
    return new(T) ();                                           \
  }                                                             \
  extern "C" void destructor(UserScheduler * scheduler) {        \
    delete scheduler;                                           \
  }

/* Utility function converting the responses given by the children to a STL
   list of servers. */
std::list<corba_server_estimation_t>
CORBA_to_STL(const corba_response_t *responses, int nb_responses);

/* Utility function converting a STL list of servers to a CORBA sequence. */
void
STL_to_CORBA(std::list<corba_server_estimation_t> &servers,
             corba_response_t * &aggrResp);

/* To simplify the declarations. */
typedef std::list<corba_server_estimation_t> ServerList;

/** A simple exception when a module failed to be loaded. */
class InstanciationError {
public:
  explicit
  InstanciationError(const char *error)
    : errorCode(error) {
  }

  InstanciationError(const InstanciationError &error)
    : errorCode(error.errorCode) {
  }

  const char *
  message() {
    return errorCode;
  }

private:
  const char *errorCode;
};

/** The UserScheduler class declaration. */
class UserScheduler : public GlobalScheduler {
  typedef GlobalScheduler *constructor ();
  typedef void destructor (UserScheduler *);

public:
  static const char *stName;

  UserScheduler();

  virtual ~UserScheduler();

  /** These methods are used to load the user module and to obtain an
      instance of the scheduler. */
  static UserScheduler *
  getInstance(const char *moduleName);

  static GlobalScheduler *
  instanciate(const char *moduleName);

  void
  destroy(GlobalScheduler *scheduler);

  static
  GlobalScheduler *
  deserialize(const char *serializedScheduler, const char *moduleName);

  static char *
  serialize(GlobalScheduler *GS);

  /** The method that has to be overloaded to define a new scheduler. */
  virtual int
  aggregate(corba_response_t *aggrResp,
            size_t max_srv,
            const size_t nb_responses,
            const corba_response_t *responses);

private:
  /** The UserScheduler class is a singleton class. Its constructor is
      private. */
  explicit
  UserScheduler(const char *moduleName);

  static UserScheduler *instance;

  void *module;

  /** These two methods are obtained from the loaded module. */
  constructor *constructs;

  destructor *destroys;
};

/* */
#define HOSTNAME(server) ((server).loc.hostName.in())
#define SED_REF(server) ((server).loc.ior)

/* Estimation values macros. */
#define TOTALTIME(server) (diet_est_get_internal(&(server).estim,       \
                                                 EST_TOTALTIME, 0))
#define COMMTIME(server) (diet_est_get_internal(&(server).estim,        \
                                                EST_COMMTIME, 0))
#define TCOMP(server) (diet_est_get_internal(&(server).estim,   \
                                             EST_TCOMP, 0))
#define TIMESINCELASTSOLVE(server)                      \
  (diet_est_get_internal(&(server).estim,               \
                         EST_TIMESINCELASTSOLVE, 0))
#define COMMPROXIMITY(server) (diet_est_get_internal(&(server).estim,   \
                                                     EST_COMMPROXIMITY, 0))
#define TRANSFEREFFORT(server) (diet_est_get_internal(&(server).estim,  \
                                                      EST_TRANSFEREFFORT, 0))
#define FREECPU(server) (diet_est_get_internal(&(server).estim, EST_FREECPU, 0))
#define FREEMEM(server) (diet_est_get_internal(&(server).estim, EST_FREEMEM, 0))
#define NBCPU(server) (diet_est_get_internal(&(server).estim, EST_NBCPU, 0))
#define CPUSPEED(server, n) (diet_est_array_get_internal(&(server).estim, \
                                                         EST_CPUSPEED, (n), 0))
#define TOTALMEM(server) (diet_est_get_internal(&(server).estim,        \
                                                EST_TOTALMEM, 0))
#define AVGFREEMEM(server) (diet_est_get_internal(&(server).estim,      \
                                                  EST_AVGFREEMEM, 0))
#define AVGFREECPU(server) (diet_est_get_internal(&(server).estim,      \
                                                  EST_AVGFREECPU, 0))
#define BOGOMIPS(server, n) (diet_est_array_get_internal(&(server).estim, \
                                                         EST_BOGOMIPS, (n), 0))
#define CACHECPU(server, n) (diet_est_array_get_internal(&(server).estim, \
                                                         EST_TOTALTIME, (n), 0))
#define TOTALSIZEDISK(server) (diet_est_get_internal(&(server).estim,   \
                                                     EST_TOTALSIZEDISK, 0))
#define FREESIZEDISK(server) (diet_est_get_internal(&(server).estim,    \
                                                    EST_FREESIZEDISK, 0))
#define DISKACCESSREAD(server) (diet_est_get_internal(&(server).estim,  \
                                                      EST_DISKACCESREAD, 0))
#define DISKACCESSWRITE(server) (diet_est_get_internal(&(server).estim, \
                                                       EST_DISKACCESWRITE, 0))
#define NUMWAITINGJOBS(server) (diet_est_get_internal((&(server).estim, \
                                                       EST_NUMWAITINGJOBS, 0))
#define USERDEFINED(server, nb) (diet_est_get_internal(&(server).estim, \
                                                       EST_USERDEFINED + (nb), \
                                                       0))

/* To define sort functions to use with a STL list. */

/* Sorts in descending order (list[0] >= list[1] >= ...)
   Used to sort a single-value metric. */
#define SORTFUN(name, metric)                   \
  bool                                          \
  name(const corba_server_estimation_t &r1,     \
       const corba_server_estimation_t &r2) {   \
    return metric(r2) < metric(r1);             \
  }

/* Sorts in descending order (list[0] >= list[1] >= ...)
   Used to sort a multi-value metric. nb is the index of the value on which
   the sort have to be made. */
#define SORTFUN_NB(name, metric, nb)            \
  bool                                          \
  name(const corba_server_estimation_t &r1,     \
       const corba_server_estimation_t &r2) {   \
    return metric(r2, nb) < metric(r1, nb);     \
  }

/* Sorts in ascending order (list[0] < list[1] < ...)
   Used to sort a single-value metric. */
#define REV_SORTFUN(name, metric)               \
  bool                                          \
  name(const corba_server_estimation_t &r1,     \
       const corba_server_estimation_t &r2) {   \
    return metric(r1) < metric(r2);             \
  }

/* Sorts in ascending order (list[0] < list[1] < ...)
   Used to sort a multi-value metric. nb is the index of the value on which
   the sort have to be made. */
#define REV_SORTFUN_NB(name, metric, nb)        \
  bool                                          \
  name(const corba_server_estimation_t &r1,     \
       const corba_server_estimation_t &r2) {   \
    return metric(r1, nb) < metric(r2, nb);     \
  }

/* Sorts the list using the comparison function "fun". */
#define SORT(list, fun) ((list).sort(fun))

#endif /* ifndef _USERSCHEDULER_HH_ */
