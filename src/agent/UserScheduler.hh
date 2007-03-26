/****************************************************************************/
/* DIET UserScheduler header.                                               */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Gael LE MAHEC (lemahec@clermont.in2p3.fr)                           */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
#ifndef _USERSCHEDULER_HH_
#define _USERSCHEDULER_HH_
#include "GlobalSchedulers.hh"

/** Utility macro to simplify the user source code. */
#define SCHEDULER_CLASS(T) \
  extern "C" GlobalScheduler* constructor() { \
    return new (T)(); \
  } \
  extern "C" void destructor(UserScheduler* scheduler) { \
    delete scheduler; \
  }

/** A simple exception when a module failed to be loaded. */
class InstanciationError
{
public:
  InstanciationError(const char* error):errorCode(error) {}
  InstanciationError(const InstanciationError &error) :
    errorCode(error.errorCode) {};
  const char* message() { return errorCode; }
private:
  const char* errorCode;
};

/** The UserScheduler class declaration. */
class UserScheduler : public GlobalScheduler
{
  typedef GlobalScheduler* constructor();
  typedef void destructor(UserScheduler*);

public:
  static const char* stName;
  UserScheduler();
  virtual
  ~UserScheduler();
  static UserScheduler* getInstance(const char* moduleName);

  static GlobalScheduler * instanciate(const char* moduleName);
  void destroy(GlobalScheduler* scheduler);

  static
  GlobalScheduler* deserialize(const char* serializedScheduler,
					      const char* moduleName);
  static
  char* serialize(GlobalScheduler* GS);
  virtual int
  aggregate(corba_response_t* aggrResp,
            size_t max_srv,
            const size_t nb_responses,
            const corba_response_t* responses);

private:
  UserScheduler(const char* moduleName);
  static UserScheduler* instance;
  void* module;
  constructor* constructs;
  destructor* destroys;
};

#endif
