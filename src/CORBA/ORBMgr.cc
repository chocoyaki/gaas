/****************************************************************************/
/* ORB manager v. 2.0 - CORBA management with DIET forwarders               */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Gaël Le Mahec (gael.le.mahec@ens-lyon.fr)                           */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/

#include <string>
#include <list>
#include <map>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <csignal>

#include <omniORB4/CORBA.h>
#include <omniORB4/omniURI.h>

#include "ORBMgr.hh"
#include "Forwarder.hh"
#include "debug.hh"

ORBMgr *ORBMgr::theMgr = NULL;

#ifndef __cygwin__
omni_mutex ORBMgr::waitLock;
#else
sem_t ORBMgr::waitLock;
#endif

/* Maximum number of retries for communication failures
 * TODO: should be a parameter in the configuration file
 */
static const unsigned int maxNbRetries = 3;

/* Handler for call resubmission when TRANSIENT exceptions occur */
CORBA::Boolean
transientHandler (void* cookie, CORBA::ULong retries,
                  const CORBA::TRANSIENT& ex) {
  /* We only retry if the number of retries is lower than maxNbRetries */
  if (retries < maxNbRetries) {
    TRACE_TEXT(TRACE_ALL_STEPS,
               "Handler for transient exception called (nb retries: "
               << retries << "/"
               << maxNbRetries << "). Will now retry." << endl);
    return 1;           // retry immediately.
  }
  TRACE_TEXT(TRACE_ALL_STEPS,
             "Handler for transient exception called (nb retries: "
             << retries << "/" << maxNbRetries << ")." << endl);
  return 0;
}

/* Handler for call resubmission when COMM_FAILURE exceptions occur */
CORBA::Boolean
commFailureHandler (void* cookie, CORBA::ULong retries,
                    const CORBA::COMM_FAILURE& ex) {
  /* We only retry if the number of retries is lower than maxNbRetries */
  if (retries < maxNbRetries) {
    TRACE_TEXT(
      TRACE_ALL_STEPS,
      "Handler for communication failures exception called (nb retries: "
      << retries << "/" << maxNbRetries << "). Will now retry." << endl);
    return 1;           // retry immediately.
  }
  TRACE_TEXT(TRACE_ALL_STEPS,
             "Handler for communication failures exception called (nb retries: "
             << retries << "/" << maxNbRetries << ")." << endl);
  return 0;
}


/* Manager initialization. */
void ORBMgr::init(CORBA::ORB_ptr ORB) {
  CORBA::Object_var object;
  PortableServer::POA_var initPOA;
  PortableServer::POAManager_var manager;
  CORBA::PolicyList policies;
  CORBA::Any policy;
  CosNaming::NamingContext_var rootContext, context;
  CosNaming::Name cosName;

  if (CORBA::is_nil(ORB)) {
    throw runtime_error("ORB not initialized");
  }
  object = ORB->resolve_initial_references("RootPOA");
  initPOA = PortableServer::POA::_narrow(object);
  manager = initPOA->the_POAManager();
  policies.length(1);
  policy <<= BiDirPolicy::BOTH;
  policies[0] = ORB->create_policy(BiDirPolicy::BIDIRECTIONAL_POLICY_TYPE,
                                   policy);

  // If poa already exist, use the same
  try {
    POA = initPOA->create_POA("bidir", manager, policies);
  } catch (PortableServer::POA::AdapterAlreadyExists &e) {
    POA = initPOA->find_POA("bidir", false);
  }

  manager->activate();

  /* Install handlers for automatically handle call resubmissions */
  omniORB::installTransientExceptionHandler(0, transientHandler);
  omniORB::installCommFailureExceptionHandler(0, commFailureHandler);
}

ORBMgr::ORBMgr(int argc, char* argv[]) {
  const char* opts[][2]= {{0,0}};

  ORB = CORBA::ORB_init(argc, argv, "omniORB4", opts);
  init(ORB);
  down = false;
}

ORBMgr::ORBMgr(CORBA::ORB_ptr ORB) {
  this->ORB = ORB;
  init(ORB);
  down = false;
}

ORBMgr::ORBMgr(CORBA::ORB_ptr ORB, PortableServer::POA_var POA) {
  this->ORB=ORB;
  this->POA=POA;
  down = false;
}

ORBMgr::~ORBMgr() {
  shutdown(true);
  ORB->destroy();
  theMgr = NULL;
}

void
ORBMgr::bind(const string& ctxt, const string& name, CORBA::Object_ptr object,
             const bool rebind) const {
  CORBA::Object_var obj;
  CosNaming::NamingContext_var rootContext, context;
  CosNaming::Name cosName;

  obj = ORB->resolve_initial_references("NameService");
  if (CORBA::is_nil(obj)) {
    throw runtime_error("Error resolving initial references");
  }

  rootContext = CosNaming::NamingContext::_narrow(obj);

  if (CORBA::is_nil(rootContext)) {
    throw runtime_error("Error initializing root context");
  }

  cosName.length(1);
  cosName[0].id = ctxt.c_str();
  cosName[0].kind = "";
  try {
    context = rootContext->bind_new_context(cosName);
  } catch (CosNaming::NamingContext::AlreadyBound& err) {
    obj = rootContext->resolve(cosName);
    context = CosNaming::NamingContext::_narrow(obj);
    if (CORBA::is_nil(context)) {
      throw runtime_error(string("Error creating context ")+ctxt);
    }
  }
  cosName[0].id = name.c_str();
  cosName[0].kind = "";
  try {
    context->bind(cosName, object);
  } catch (CosNaming::NamingContext::AlreadyBound& err) {
    if (rebind) {
      context->rebind(cosName, object);
    } else {
      throw runtime_error("Already bound!");
    }
  }
}

void
ORBMgr::bind(const string& ctxt, const string& name,
             const string& IOR, const bool rebind) const {
  CORBA::Object_ptr object = ORB->string_to_object(IOR.c_str());

  bind(ctxt, name, object, rebind);
}

void
ORBMgr::rebind(const string& ctxt, const string& name,
               CORBA::Object_ptr object) const {
  bind(ctxt, name, object, true);
}

void
ORBMgr::rebind(const string& ctxt, const string& name,
               const string& IOR) const {
  CORBA::Object_ptr object = ORB->string_to_object(IOR.c_str());

  rebind(ctxt, name, object);
}

void
ORBMgr::unbind(const string& ctxt, const string& name) const {
  CORBA::Object_var obj;
  CosNaming::NamingContext_var rootContext, context;
  CosNaming::Name cosName;
  std::list<std::pair<std::string, std::string> >::iterator it;

  obj = ORB->resolve_initial_references("NameService");
  rootContext = CosNaming::NamingContext::_narrow(obj);

  cosName.length(1);
  cosName[0].id = ctxt.c_str();
  cosName[0].kind = "";

  obj = rootContext->resolve(cosName);
  context = CosNaming::NamingContext::_narrow(obj);
  if (CORBA::is_nil(context)) {
    throw runtime_error(string("Error retrieving context ") + ctxt);
  }

  cosName[0].id = name.c_str();
  cosName[0].kind = "";
  try {
    removeObjectFromCache(ctxt, name);
    context->unbind(cosName);
  } catch (CosNaming::NamingContext::NotFound& err) {
    throw runtime_error("Object "+name+" not found in " + ctxt +" context");
  } catch (...) {
    WARNING("Exception caught while unbinding " << ctxt << "/" << name);
  }
}

void
ORBMgr::fwdsBind(const string& ctxt, const string& name,
                 const string& ior, const string& fwName) const {
  std::list<std::string> forwarders = ORBMgr::list(FWRDCTXT);
  std::list<std::string>::const_iterator it;

  for (it=forwarders.begin(); it != forwarders.end(); ++it) {
    if (fwName == *it) {
      continue;
    }
    Forwarder_var fwd = resolve<Forwarder, Forwarder_var>(FWRDCTXT, *it);
    std::string objName = ctxt + "/" + name;
    try {
      fwd->bind(objName.c_str(), ior.c_str());
    } catch (const CORBA::TRANSIENT& err) {
      continue;
    } catch (BadNameException& err) {
    }
  }
}

void
ORBMgr::fwdsUnbind(const string& ctxt, const string& name,
                   const string& fwName) const {
  std::list<std::string> forwarders = ORBMgr::list(FWRDCTXT);
  std::list<std::string>::const_iterator it;

  for (it = forwarders.begin(); it != forwarders.end(); ++it) {
    if (fwName == *it) {
      continue;
    }
    Forwarder_var fwd = resolve<Forwarder, Forwarder_var>(FWRDCTXT, *it);
    std::string objName = ctxt + "/" + name;
    try {
      fwd->unbind(objName.c_str());
    } catch (const CORBA::TRANSIENT& err) {
      TRACE_TEXT(TRACE_ALL_STEPS,
                 "Unable to contact DIET forwarder " << *it << endl);
      continue;
    }
  }
}

CORBA::Object_ptr
ORBMgr::resolveObject(const string& IOR) const {
  return ORB->string_to_object(IOR.c_str());
}

CORBA::Object_ptr
ORBMgr::resolveObject(const string& context, const string& name,
                      const string& fwdName) const {
  std::string ctxt = context;
  bool localAgent = false;

  /* The object to resolve is it a local agent ?*/
  if (ctxt == LOCALAGENT) {
    ctxt = AGENTCTXT;
    localAgent = true;
  }
  if (ctxt == MASTERAGENT) {
    ctxt = AGENTCTXT;
  }
  cacheMutex.lock();
  /* Use object cache. */
  if (cache.find(ctxt+"/"+name) != cache.end()) {
    CORBA::Object_ptr ptr = cache[ctxt+"/"+name];
    cacheMutex.unlock();

    try {
      TRACE_TEXT(TRACE_ALL_STEPS,
                 "Check if the object is still present" << endl);
      if (ptr->_non_existent()) {
        TRACE_TEXT(TRACE_ALL_STEPS,
                   "Remove non existing object from cache (" << ctxt
                   << "/" << name << ")" << endl);
        removeObjectFromCache(name);
      } else {
        TRACE_TEXT(TRACE_ALL_STEPS, "Use object from cache (" << ctxt
                   << "/" << name << ")" << endl);
        return CORBA::Object::_duplicate(ptr);
      }
    } catch (const CORBA::OBJECT_NOT_EXIST& err) {
      TRACE_TEXT(TRACE_ALL_STEPS,
                 "Remove non existing object from cache (" << ctxt
                 << "/" << name << ")" << endl);
      removeObjectFromCache(name);
    } catch (const CORBA::TRANSIENT& err) {
      TRACE_TEXT(TRACE_ALL_STEPS,
                 "Remove unreachable object from cache (" << ctxt
                 << "/" << name << ")" << endl);
      removeObjectFromCache(name);
    } catch (const CORBA::COMM_FAILURE& err) {
      TRACE_TEXT(TRACE_ALL_STEPS,
                 "Remove unreachable object from cache (" << ctxt
                 << "/" << name << ")" << endl);
      removeObjectFromCache(name);
    } catch (...) {
      TRACE_TEXT(TRACE_ALL_STEPS,
                 "Remove unreachable object from cache (" << ctxt
                 << "/" << name << ")" << endl);
      removeObjectFromCache(name);
    }
  }
  cacheMutex.unlock();
  CORBA::Object_ptr object = ORB->resolve_initial_references("NameService");
  CosNaming::NamingContext_var rootContext =
    CosNaming::NamingContext::_narrow(object);
  CosNaming::Name cosName;

  cosName.length(2);
  cosName[0].id   = ctxt.c_str();
  cosName[0].kind = "";
  cosName[1].id   = name.c_str();
  cosName[1].kind = "";

  try {
    object = rootContext->resolve(cosName);

    /* If the object is not a forwarder object, then
     * search if we need to use a forwarder to reach it.
     */
    if (ctxt!=FWRDCTXT && fwdName!="no-Forwarder") {
      std::string objIOR = getIOR(object);
      IOP::IOR ior;
      makeIOR(objIOR, ior);

      // Get the object host to check if it is a forwarder reference
      std::string objHost = getHost(ior);
      try {
        if (objHost.size()>0 && objHost.at(0)=='@') {
          objHost.erase(0,1); // Remove '@' before the forwarder name
          Forwarder_var fwd =
            resolve<Forwarder, Forwarder_var>(FWRDCTXT, objHost);

          TRACE_TEXT(TRACE_ALL_STEPS, "Object (" << ctxt << "/" << name << ")"
                     << " is reachable through forwarder " << objHost << endl);
          if (ctxt == AGENTCTXT) {
            if (!localAgent) {
              object = fwd->getMasterAgent(name.c_str());
            } else {
              object = fwd->getLocalAgent(name.c_str());
            }
            /* Necessary to check if we were able to resolve
             * the MA or LA, otherwise try a last time with a more
             * general call to getAgent()
             */
            if (CORBA::is_nil(object)) {
              object = fwd->getAgent(name.c_str());
            }
          }
          if (ctxt==CLIENTCTXT) {
            object = fwd->getCallback(name.c_str());
          }
          if (ctxt==SEDCTXT) {
            object = fwd->getSeD(name.c_str());
          }

          if (ctxt==DAGDACTXT) {
            object = fwd->getDagda(name.c_str());
          }
#ifdef HAVE_WORKFLOW
          if (ctxt==WFMGRCTXT) {
            object = fwd->getCltMan(name.c_str());
          }
          if (ctxt==MADAGCTXT) {
            object = fwd->getMaDag(name.c_str());
          }
#endif
        } else {
          TRACE_TEXT(TRACE_ALL_STEPS,
                     "Direct access to object " << ctxt << "/" << name << endl);
        }
      } catch (const CORBA::TRANSIENT& err) {
        TRACE_TEXT(TRACE_ALL_STEPS,
                   "Unable to contact DIET forwarder \""
                   << objHost << "\"" << endl);
      }
    }
  } catch (CosNaming::NamingContext::NotFound& err) {
    TRACE_TEXT(TRACE_ALL_STEPS,
               "Error resolving " << ctxt << "/" << name << endl);
    throw runtime_error("Error resolving " + ctxt + "/" + name);
  }
  cacheMutex.lock();
  cache[ctxt + "/" + name] = CORBA::Object::_duplicate(object);
  cacheMutex.unlock();

  return CORBA::Object::_duplicate(object);
}

/* Resolve objects without invoking forwarders. */
CORBA::Object_ptr
ORBMgr::simpleResolve(const std::string& context,
                      const std::string& name) const {
  std::string ctxt = context;
  bool localAgent = false;

  /* The object to resolve is it a local agent ?*/
  if (ctxt == LOCALAGENT) {
    ctxt = AGENTCTXT;
    localAgent = true;
  }
  if (ctxt == MASTERAGENT) {
    ctxt = AGENTCTXT;
  }

  CORBA::Object_ptr object = ORB->resolve_initial_references("NameService");
  CosNaming::NamingContext_var rootContext =
    CosNaming::NamingContext::_narrow(object);
  CosNaming::Name cosName;

  cosName.length(2);
  cosName[0].id   = ctxt.c_str();
  cosName[0].kind = "";
  cosName[1].id   = name.c_str();
  cosName[1].kind = "";

  try {
    object = rootContext->resolve(cosName);
  } catch (CosNaming::NamingContext::NotFound& err) {
    TRACE_TEXT(TRACE_ALL_STEPS,
               "Error resolving " << ctxt << "/" << name << endl);
    throw runtime_error("Error resolving " + ctxt + "/" + name);
  }
  return CORBA::Object::_duplicate(object);
}


std::list<std::string>
ORBMgr::list(CosNaming::NamingContext_var& ctxt) const {
  std::list<std::string> result;
  CosNaming::BindingList_var ctxtList;
  CosNaming::BindingIterator_var it;
  CosNaming::Binding_var bv;

  ctxt->list(256, ctxtList, it);
  for (unsigned int i = 0; i < ctxtList->length(); ++i)
    if (ctxtList[i].binding_type == CosNaming::nobject) {
      for (unsigned int j = 0; j < ctxtList[i].binding_name.length(); ++j) {
        result.push_back(string(ctxtList[i].binding_name[j].id));
      }
    }
  if (CORBA::is_nil(it)) {
    return result;
  }
  while (it->next_one(bv)) {
    if (bv->binding_type == CosNaming::nobject) {
      for (unsigned int j = 0; j < bv->binding_name.length(); ++j) {
        result.push_back(string(bv->binding_name[j].id));
      }
    }
  }
  return result;
}

std::list<std::string>
ORBMgr::list(const std::string& ctxtName) const {
  std::list<std::string> result;

  CORBA::Object_ptr object = ORB->resolve_initial_references("NameService");
  CosNaming::NamingContext_var rootContext =
    CosNaming::NamingContext::_narrow(object);
  CosNaming::BindingList_var bindingList;
  CosNaming::BindingIterator_var it;

  rootContext->list(256, bindingList, it);

  for (unsigned int i = 0; i < bindingList->length(); ++i) {
    if (bindingList[i].binding_type == CosNaming::ncontext) {
      if (string(bindingList[i].binding_name[0].id) == ctxtName) {
        std::list<std::string> tmpRes;
        CORBA::Object_ptr ctxtObj =
          rootContext->resolve(bindingList[i].binding_name);
        CosNaming::NamingContext_var ctxt =
          CosNaming::NamingContext::_narrow(ctxtObj);

        tmpRes = list(ctxt);
        result.insert(result.end(), tmpRes.begin(), tmpRes.end());
      }
    }
  }
  if (CORBA::is_nil(it)) {
    return result;
  }

  CosNaming::Binding_var bv;
  while (it->next_one(bv)) {
    if (bv->binding_type == CosNaming::ncontext) {
      if (string(bv->binding_name[0].id) == ctxtName) {
        std::list<std::string> tmpRes;
        CORBA::Object_ptr ctxtObj = rootContext->resolve(bv->binding_name);
        CosNaming::NamingContext_var ctxt =
          CosNaming::NamingContext::_narrow(ctxtObj);

        tmpRes = list(ctxt);
        result.insert(result.end(), tmpRes.begin(), tmpRes.end());
      }
    }
  }
  return result;
}

std::list<std::string>
ORBMgr::contextList() const {
  std::list<std::string> result;

  CORBA::Object_ptr object = ORB->resolve_initial_references("NameService");
  CosNaming::NamingContext_var rootContext =
    CosNaming::NamingContext::_narrow(object);
  CosNaming::BindingList_var bindingList;
  CosNaming::BindingIterator_var it;

  rootContext->list(256, bindingList, it);

  for (unsigned int i = 0; i < bindingList->length(); ++i) {
    if (bindingList[i].binding_type == CosNaming::ncontext) {
      string ctxt = string(bindingList[i].binding_name[0].id);
      result.push_back(ctxt);
    }
  }
  if (CORBA::is_nil(it)) {
    return result;
  }

  CosNaming::Binding_var bv;
  while (it->next_one(bv)) {
    if (bv->binding_type == CosNaming::ncontext) {
      result.push_back(string(bv->binding_name[0].id));
    }
  }
  return result;
}

bool
ORBMgr::isLocal(const std::string& ctxt, const std::string& name) const {
  CORBA::Object_ptr obj = simpleResolve(ctxt, name);
  return getHost(getIOR(obj)).at(0) != '@';
}

std::string
ORBMgr::forwarderName(const std::string& ctxt, const std::string& name) const {
  CORBA::Object_ptr obj = simpleResolve(ctxt, name);
  std::string host;
  if ((host = getHost(getIOR(obj))).at(0) != '@') {
    return "";
  }

  host.erase(0, 1);
  return host;
}


std::list<std::string>
ORBMgr::forwarderObjects(const std::string& fwdName,
                         const std::string& ctxt) const {
  std::list<std::string> result = list(ctxt);
  std::list<std::string>::iterator it, next;
  std::string fwdTag = '@'+fwdName;

  for (it = result.begin(); it != result.end(); it = next) {
    std::string ior, iorHost;
    next = it;
    ++next;
    CORBA::Object_ptr obj = simpleResolve(ctxt, *it);
    ior = getIOR(obj);
    iorHost = getHost(ior);
    if (iorHost != fwdTag) {
      result.erase(it);
    }
  }
  return result;
}

std::list<std::string>
ORBMgr::localObjects(const std::string& ctxt) const {
  std::list<std::string> result = list(ctxt);
  std::list<std::string>::iterator it, next;

  for (it = result.begin(); it != result.end(); it = next) {
    std::string ior, iorHost;
    next=it;
    ++next;
    CORBA::Object_ptr obj = simpleResolve(ctxt, *it);
    ior = getIOR(obj);
    iorHost = getHost(ior);
    if (iorHost.length() < 1) {
      continue;
    }
    if (iorHost.at(0) == '@') {
      result.erase(it);
    }
  }
  return result;
}


std::string
ORBMgr::getIOR(CORBA::Object_ptr object) const {
  return ORB->object_to_string(object);
}

std::string
ORBMgr::getIOR(const std::string& ctxt, const std::string& name) const {
  return ORB->object_to_string(resolveObject(ctxt, name));
}

void
ORBMgr::activate(PortableServer::ServantBase* object) const {
  POA->activate_object(object);
  object->_remove_ref();
}

void ORBMgr::deactivate(PortableServer::ServantBase* object) const {
  PortableServer::ObjectId* id = POA->servant_to_id(object);
  POA->deactivate_object(*id);
}

void
ORBMgr::shutdown(bool waitForCompletion) {
  if (!down) {
    ORB->shutdown(waitForCompletion);
    down = true;
  }
}

void
ORBMgr::sigIntHandler(int sig) {
  /* Prevent from raising a new SIGINT handler */
  signal(SIGINT, SIG_IGN);
  signal(SIGTERM, SIG_IGN);
#ifndef __cygwin__
  ORBMgr::getMgr()->waitLock.unlock();
#else
  sem_post(&(ORBMgr::getMgr()->waitLock));
#endif
  signal(SIGINT, SIG_DFL);
  signal(SIGTERM, SIG_DFL);
}

void
ORBMgr::wait() const {
  /* FIXME: this is pretty ugly,
   * but currently I do not see how to do so properly
   */
  signal(SIGINT, ORBMgr::sigIntHandler);
  signal(SIGTERM, ORBMgr::sigIntHandler);
  try {
    TRACE_TEXT(TRACE_MAIN_STEPS, "Press CTRL+C to exit" << std::endl);
#ifdef __cygwin__
    sem_init(&waitLock,0,1);
    sem_wait(&waitLock);
    sem_wait(&waitLock);
    sem_post(&waitLock);
#else
    waitLock.lock();
    waitLock.lock();
    waitLock.unlock();
#endif
  } catch (...) {
  }
}

ORBMgr*
ORBMgr::getMgr() {
  if (!theMgr) {
    throw std::runtime_error("ORB manager not initialized!");
  } else {
    return theMgr;
  }
}

void
ORBMgr::init(int argc, char* argv[]) {
  if (!theMgr) {
    theMgr = new ORBMgr(argc, argv);
  }
}

/* Translate the string passed as first argument in bytes and
 * record them into the buffer.
 */
void
ORBMgr::hexStringToBuffer(const char* ptr, const size_t size,
                          cdrMemoryStream& buffer) {
  std::stringstream ss;
  int value;
  CORBA::Octet c;

  for (unsigned int i = 0; i < size; i += 2) {
    ss << ptr[i] << ptr[i+1];
    ss >> hex >> value;
    c = value;
    buffer.marshalOctet(c);
    ss.flush();
    ss.clear();
  }
}

/* Make an IOP::IOR object using a stringified IOR. */
void
ORBMgr::makeIOR(const std::string& strIOR, IOP::IOR& ior) {
  /* An IOR must start with "IOR:" or "ior:" */
  if ((strIOR.find("IOR:") != 0) && (strIOR.find("ior:") != 0)) {
    throw std::runtime_error("Bad IOR: " + strIOR);
  }

  const char* tab = strIOR.c_str();
  size_t size = (strIOR.length() - 4);
  cdrMemoryStream buffer(size, false);
  CORBA::Boolean byteOrder;

  /* Convert the hex bytes string into buffer. */
  hexStringToBuffer(tab+4, size, buffer);
  buffer.rewindInputPtr();
  /* Get the endianness and init the buffer flag. */
  byteOrder = buffer.unmarshalBoolean();
  buffer.setByteSwapFlag(byteOrder);

  /* Get the object type id. */
  ior.type_id = IOP::IOR::unmarshaltype_id(buffer);
  /* Get the IOR profiles. */
  ior.profiles <<= buffer;
}

/* Convert IOP::IOR to a stringified IOR. */
void
ORBMgr::makeString(const IOP::IOR& ior, std::string& strIOR) {
  strIOR = "IOR:";
  cdrMemoryStream buffer(0, true);
  std::stringstream ss;
  unsigned char* ptr;

  buffer.marshalBoolean(omni::myByteOrder);
  buffer.marshalRawString(ior.type_id);
  ior.profiles >>= buffer;

  buffer.rewindInputPtr();
  ptr = static_cast<unsigned char*>(buffer.bufPtr());

  for (unsigned long i = 0; i < buffer.bufSize(); ++ptr, ++i) {
    std::string str;
    unsigned char c = *ptr;
    if (c < 16) {
      ss << '0';
    }

    ss << (unsigned short) c;
    ss >> hex >> str;
    ss.flush();
    ss.clear();
    strIOR += str;
  }
}

/* Get the hostname of the first profile in this IOR. */
std::string
ORBMgr::getHost(IOP::IOR& ior) {
  IIOP::ProfileBody body;

  if (ior.profiles.length() == 0) {
    return "nohost";
  }

  IIOP::unmarshalProfile(ior.profiles[0], body);
  return string(body.address.host);
}

/* Get the hostname of the first profile in IOR passed
 * as a string.
 */
std::string
ORBMgr::getHost(const std::string& strIOR) {
  IOP::IOR ior;
  makeIOR(strIOR, ior);
  return getHost(ior);
}

/* Get the port of the first profile in this IOR. */
unsigned int
ORBMgr::getPort(IOP::IOR& ior) {
  IIOP::ProfileBody body;

  if (ior.profiles.length() == 0) {
    return 0;
  }

  IIOP::unmarshalProfile(ior.profiles[0], body);
  return body.address.port;
}

/* Get the port of the first profile in IOR passed
 * as a string.
 */
unsigned int
ORBMgr::getPort(const std::string& strIOR) {
  IOP::IOR ior;
  makeIOR(strIOR, ior);
  return getPort(ior);
}

/* Get the type id of the IOR. */
std::string
ORBMgr::getTypeID(IOP::IOR& ior) {
  return std::string(ior.type_id);
}

/* Get the type id of the IOR passed as a string. */
std::string
ORBMgr::getTypeID(const std::string& strIOR) {
  IOP::IOR ior;
  makeIOR(strIOR, ior);
  return getTypeID(ior);
}

std::string
ORBMgr::convertIOR(IOP::IOR& ior, const std::string& host,
                   const unsigned int port) {
  IIOP::ProfileBody body;
  IOP::TaggedProfile profile;
  CORBA::ULong max_data;
  CORBA::ULong nb_data;
  CORBA::Octet* buffer;
  std::string result;

  if (ior.profiles.length() == 0) {
    throw std::runtime_error("Invalid IOR");
  }

  for (unsigned int i = 0; i < ior.profiles.length(); ++i) {
    if (ior.profiles[i].tag==IOP::TAG_INTERNET_IOP) {
      IIOP::unmarshalProfile(ior.profiles[i], body);

      body.address.host = host.c_str();
      body.address.port = port;

      IIOP::encodeProfile(body, profile);

      max_data = profile.profile_data.maximum();
      nb_data = profile.profile_data.length();
      buffer = profile.profile_data.get_buffer(true);
      ior.profiles[i].profile_data.replace(max_data, nb_data, buffer, true);
    }
  }

  makeString(ior, result);
  return result;
}

std::string
ORBMgr::convertIOR(const std::string& strIOR, const std::string& host,
                   const unsigned int port) {
  IOP::IOR ior;
  makeIOR(strIOR, ior);
  return convertIOR(ior, host, port);
}

/* Object cache management functions. */
void
ORBMgr::resetCache() const {
  cacheMutex.lock();
  cache.clear();
  cacheMutex.unlock();
}

void
ORBMgr::removeObjectFromCache(const std::string& name) const {
  std::map<std::string, CORBA::Object_ptr>::iterator it;
  cacheMutex.lock();
  if ((it = cache.find(name)) != cache.end()) {
    cache.erase(it);
  }

  cacheMutex.unlock();
}

void
ORBMgr::removeObjectFromCache(const std::string& ctxt,
                              const std::string& name) const {
  removeObjectFromCache(ctxt+"/"+name);
}

void
ORBMgr::cleanCache() const {
  std::map<std::string, CORBA::Object_ptr>::iterator it;
  std::list<std::string> toRemove;
  std::list<std::string>::const_iterator jt;

  cacheMutex.lock();
  for (it = cache.begin(); it != cache.end(); ++it) {
    try {
      if (it->second->_non_existent()) {
        toRemove.push_back(it->first);
      }
    } catch (const CORBA::OBJECT_NOT_EXIST& err) {
      toRemove.push_back(it->first);
    } catch (...) {
      toRemove.push_back(it->first);
    }
  }
  cacheMutex.unlock();

  for (jt = toRemove.begin(); jt != toRemove.end(); ++jt) {
    removeObjectFromCache(*jt);
  }
}
