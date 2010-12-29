/****************************************************************************/
/* ORB manager v. 2.0 - CORBA management with DIET forwarders               */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Gaël Le Mahec (gael.le.mahec@ens-lyon.fr)                           */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.33  2010/12/29 14:56:13  hguemar
 * set static ORBMgr instance to 0
 *
 * Revision 1.32  2010/11/02 21:34:26  bdepardo
 * Typo in message
 *
 * Revision 1.31  2010/11/02 05:53:18  bdepardo
 * Correct a bug preventing the SeDs from connecting to an LA through the
 * forwarders: when resolving an agent we first try to resolve MA or LA
 * depending on the context, but if this fails, then we fall back to resolving
 * a general agent.
 *
 * Revision 1.30  2010/10/18 07:18:26  bisnard
 * allow re-initialization of the ORB
 *
 * Revision 1.29  2010/07/27 16:16:48  glemahec
 * Forwarders robustness
 *
 * Revision 1.28  2010/07/27 10:24:32  glemahec
 * Improve robustness & general performance
 *
 * Revision 1.27  2010/07/14 23:45:30  bdepardo
 * Header corrections
 *
 *
 ****************************************************************************/

#include <string>
#include <list>
#include <map>
#include <sstream>
#include <stdexcept>
#include <iostream>

#include <omniORB4/CORBA.h>
#include <omniORB4/omniURI.h>

#include "ORBMgr.hh"
#include "Forwarder.hh"

using namespace std;

ORBMgr * ORBMgr::theMgr = 0;

/* Manager initialization. */
void ORBMgr::init(CORBA::ORB_ptr ORB) {
  CORBA::Object_var object;
  PortableServer::POA_var initPOA;
  PortableServer::POAManager_var manager;
  CORBA::PolicyList policies;
  CORBA::Any policy;
  CosNaming::NamingContext_var rootContext, context;
  CosNaming::Name cosName;
  
  if (CORBA::is_nil(ORB))
    throw runtime_error("ORB not initialized");
  
  object = ORB->resolve_initial_references("RootPOA");
  initPOA = PortableServer::POA::_narrow(object);
  manager = initPOA->the_POAManager();
  policies.length(1);
  policy <<= BiDirPolicy::BOTH;
  policies[0] = ORB->create_policy(BiDirPolicy::BIDIRECTIONAL_POLICY_TYPE,
                                   policy);
  POA = initPOA->create_POA("bidir", manager, policies);
  
  manager->activate();
}

ORBMgr::ORBMgr(int argc, char* argv[]) {
  const char* opts[][2]= {{0,0}};
  
  ORB = CORBA::ORB_init(argc, argv, "omniORB4", opts);
  init(ORB);
}

ORBMgr::ORBMgr(CORBA::ORB_ptr ORB) {
  this->ORB = ORB;
  init(ORB);
}

ORBMgr::ORBMgr(CORBA::ORB_ptr ORB, PortableServer::POA_var POA) {
  this->ORB=ORB;
  this->POA=POA;
}

ORBMgr::~ORBMgr() {
  ORB->shutdown(true);
  ORB->destroy();
  theMgr = 0;
}

void ORBMgr::bind(const string& ctxt, const string& name,
                  CORBA::Object_ptr object,
		  const bool rebind)
{
  CORBA::Object_var obj;
  CosNaming::NamingContext_var rootContext, context;
  CosNaming::Name cosName;
	
  obj = ORB->resolve_initial_references("NameService");
  if (CORBA::is_nil(obj))
    throw runtime_error("Error resolving initial references");
  
  rootContext = CosNaming::NamingContext::_narrow(obj);
  
  if (CORBA::is_nil(rootContext))
    throw runtime_error("Error initializing root context");
  
  cosName.length(1);
  cosName[0].id = ctxt.c_str();
  cosName[0].kind = "";
  try {
    context = rootContext->bind_new_context(cosName);
  } catch (CosNaming::NamingContext::AlreadyBound& err) {
    obj = rootContext->resolve(cosName);
    context = CosNaming::NamingContext::_narrow(obj);
    if (CORBA::is_nil(context))
      throw runtime_error(string("Error creating context ")+ctxt);
  }
  cosName[0].id = name.c_str();
  cosName[0].kind = "";
  try {
    context->bind(cosName, object);
  } catch (CosNaming::NamingContext::AlreadyBound& err) {
    if (rebind)
      context->rebind(cosName, object);
    else
      throw runtime_error("Already bound!");
  }
}

void ORBMgr::bind(const string& ctxt, const string& name,
		  const string& IOR, const bool rebind) {
  CORBA::Object_ptr object = ORB->string_to_object(IOR.c_str());
	
  bind(ctxt, name, object, rebind);
}

void ORBMgr::rebind(const string& ctxt, const string& name,
		    CORBA::Object_ptr object) {
  bind(ctxt, name, object, true);
}

void ORBMgr::rebind(const string& ctxt, const string& name,
                    const string& IOR) {
  CORBA::Object_ptr object = ORB->string_to_object(IOR.c_str());
  
  rebind(ctxt, name, object);
}

void ORBMgr::unbind(const string& ctxt, const string& name) {
  CORBA::Object_var obj;
  CosNaming::NamingContext_var rootContext, context;
  CosNaming::Name cosName;
  std::list<pair<string,string> >::iterator it;
  
  obj = ORB->resolve_initial_references("NameService");
  rootContext = CosNaming::NamingContext::_narrow(obj);
  
  cosName.length(1);
  cosName[0].id = ctxt.c_str();
  cosName[0].kind = "";
  
  obj = rootContext->resolve(cosName);
  context = CosNaming::NamingContext::_narrow(obj);
  if (CORBA::is_nil(context))
    throw runtime_error(string("Error retrieving context ")+ctxt);
  cosName[0].id = name.c_str();
  cosName[0].kind = "";
  try {
    removeObjectFromCache(ctxt, name);
    context->unbind(cosName);
  } catch (CosNaming::NamingContext::NotFound& err) {
    throw runtime_error("Object "+name+" not found in " + ctxt +" context");
  }
}

void ORBMgr::fwdsBind(const string& ctxt, const string& name,
		      const string& ior, const string& fwName)
{
  std::list<string> forwarders = ORBMgr::list(FWRDCTXT);
  std::list<string>::const_iterator it;
	
  for (it=forwarders.begin(); it!=forwarders.end(); ++it) {
    if (fwName==*it) continue;
    Forwarder_var fwd = resolve<Forwarder, Forwarder_var>(FWRDCTXT, *it);
    string objName = ctxt+"/"+name;
    try {
      fwd->bind(objName.c_str(), ior.c_str());
    } catch (const CORBA::TRANSIENT& err) {
      continue;
    } catch (BadNameException& err) {
    }
  }
}

void ORBMgr::fwdsUnbind(const string& ctxt, const string& name,
			const string& fwName)
{
  std::list<string> forwarders = ORBMgr::list(FWRDCTXT);
  std::list<string>::const_iterator it;
	
  for (it=forwarders.begin(); it!=forwarders.end(); ++it) {
    if (fwName==*it) continue;
    Forwarder_var fwd = resolve<Forwarder, Forwarder_var>(FWRDCTXT, *it);
    string objName = ctxt+"/"+name;
    try {
      fwd->unbind(objName.c_str());
    } catch (const CORBA::TRANSIENT& err) {
      cerr << "Unable to contact DIET forwarder " << *it << endl;
      continue;
    }
  }
}

CORBA::Object_ptr ORBMgr::resolveObject(const string& IOR) const {
  return ORB->string_to_object(IOR.c_str());
}

CORBA::Object_ptr ORBMgr::resolveObject(const string& context, const string& name,
					const string& fwdName) const {
  string ctxt = context;
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
  if (cache.find(ctxt+"/"+name)!=cache.end()) {
    CORBA::Object_ptr ptr = cache[ctxt+"/"+name];
    cacheMutex.unlock();
		
    try {
      cout << "Check if the object is still present" << endl;
      if (ptr->_non_existent()) {
	cout << "Remove non existing object from cache (" << ctxt
	     << "/" << name << ")" << endl;
	removeObjectFromCache(name);
      } else {
	cout << "Use object from cache (" << ctxt
	     << "/" << name << ")" << endl;
	return ptr;
      }
    } catch (const CORBA::OBJECT_NOT_EXIST& err) {
      cout << "Remove non existing object from cache (" << ctxt
	   << "/" << name << ")" << endl;
      removeObjectFromCache(name);
    } catch (const CORBA::TRANSIENT& err) {
      cout << "Remove unreachable object from cache (" << ctxt
	   << "/" << name << ")" << endl;
      removeObjectFromCache(name);
    } catch (const CORBA::COMM_FAILURE& err) {
      cout << "Remove unreachable object from cache (" << ctxt
	   << "/" << name << ")" << endl;
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
      string objIOR = getIOR(object);
      IOP::IOR ior;
      makeIOR(objIOR, ior);
			
      std::list<string> forwarders = ORBMgr::list(FWRDCTXT);
      std::list<string>::const_iterator it;
      for (it=forwarders.begin(); it!=forwarders.end(); ++it) {
	// This is the same forwarder...
	if (*it==fwdName) continue;
	Forwarder_var fwd = resolve<Forwarder, Forwarder_var>(FWRDCTXT, *it);
	string objName = ctxt+"/"+name;
	string ior = getIOR(object);
	string objHost = getHost(ior);
	cout << "Ask forwarder " << *it << " for host " << objHost << endl;
	try {
	  if (fwd->manage(objHost.c_str())) {
	    cout << "Object (" << ctxt << "/" << name << ")"
		 << " is reachable through forwarder " << *it << endl;
	    if (ctxt==AGENTCTXT) {
	      if (!localAgent)
		object = fwd->getMasterAgent(name.c_str());
	      else
		object = fwd->getLocalAgent(name.c_str());
	     
	      /* Necessary to check if we were able to resolve
	       * the MA or LA, otherwise try a last time with a more
	       * general call to getAgent()
	       */
	      if (CORBA::is_nil(object))
		object = fwd->getAgent(name.c_str());
	      break;
	    }
	    if (ctxt==CLIENTCTXT) {
	      object = fwd->getCallback(name.c_str());
	      break;
	    }
	    if (ctxt==SEDCTXT) {
	      object = fwd->getSeD(name.c_str());
	      break;
	    }
#ifdef HAVE_DAGDA
	    if (ctxt==DAGDACTXT) {
	      object = fwd->getDagda(name.c_str());
	      break;
	    }
#else
	    if (ctxt==DATAMGRCTXT) {
	      object = fwd->getDataMgr(name.c_str());
	      break;
	    }
	    if (ctxt==LOCMGRCTXT) {
	      object = fwd->getLocMgr(name.c_str());
	      break;
	    }
#endif
#ifdef HAVE_WORKFLOW
	    if (ctxt==WFMGRCTXT) {
	      object = fwd->getCltMan(name.c_str());
	      break;
	    }
	    if (ctxt==MADAGCTXT) {
	      object = fwd->getMaDag(name.c_str());
	      break;
	    }
#endif
	  } else {
	    cout << "Direct access to object " << ctxt << "/" << name << endl;
	  }
	} catch (const CORBA::TRANSIENT& err) {
	  cerr << "Unable to contact DIET forwarder \"" << *it << "\"" << endl;
	  continue;
	}
      }
    }
  } catch (CosNaming::NamingContext::NotFound& err) {
    cerr << "Error resolving " << ctxt << "/" << name << endl;
    throw runtime_error("Error resolving "+ctxt+"/"+name);
  }
  cacheMutex.lock();
  cache[ctxt+"/"+name] = CORBA::Object::_duplicate(object);
  cacheMutex.unlock();
	
  return CORBA::Object::_duplicate(object);
}

std::list<string> ORBMgr::list(CosNaming::NamingContext_var& ctxt) const {
  std::list<string> result;
  CosNaming::BindingList_var ctxtList;
  CosNaming::BindingIterator_var it;
  CosNaming::Binding_var bv;

  ctxt->list(256, ctxtList, it);
  for (unsigned int i=0; i<ctxtList->length(); ++i)
    if (ctxtList[i].binding_type==CosNaming::nobject)
      for (unsigned int j=0; j<ctxtList[i].binding_name.length(); ++j) {
	result.push_back(string(ctxtList[i].binding_name[j].id));
      }
  if (CORBA::is_nil(it)) return result;
  while (it->next_one(bv)) {
    if (bv->binding_type==CosNaming::nobject)
      for (unsigned int j=0; j<bv->binding_name.length(); ++j) {
	result.push_back(string(bv->binding_name[j].id));
      }
  }
  return result;
}

std::list<string> ORBMgr::list(const std::string& ctxtName) const {
  std::list<string> result;
	
  CORBA::Object_ptr object = ORB->resolve_initial_references("NameService");
  CosNaming::NamingContext_var rootContext =
    CosNaming::NamingContext::_narrow(object);
  CosNaming::BindingList_var bindingList;
  CosNaming::BindingIterator_var it;
	
  rootContext->list(256, bindingList, it);

  for (unsigned int i=0; i<bindingList->length(); ++i) {
    if (bindingList[i].binding_type==CosNaming::ncontext)
      if (string(bindingList[i].binding_name[0].id)==ctxtName) {
	std::list<string> tmpRes;
	CORBA::Object_ptr ctxtObj = rootContext->resolve(bindingList[i].binding_name);
	CosNaming::NamingContext_var ctxt =
	  CosNaming::NamingContext::_narrow(ctxtObj);
			
	tmpRes = list(ctxt);
	result.insert(result.end(), tmpRes.begin(), tmpRes.end());
      }
  }
  if (CORBA::is_nil(it)) return result;
  CosNaming::Binding_var bv;
  while (it->next_one(bv))
    if (bv->binding_type==CosNaming::ncontext)
      if (string(bv->binding_name[0].id)==ctxtName) {
	std::list<string> tmpRes;
	CORBA::Object_ptr ctxtObj = rootContext->resolve(bv->binding_name);
	CosNaming::NamingContext_var ctxt =
	  CosNaming::NamingContext::_narrow(ctxtObj);
				
	tmpRes = list(ctxt);
	result.insert(result.end(), tmpRes.begin(), tmpRes.end());
      }
  return result;
}


string ORBMgr::getIOR(CORBA::Object_ptr object) const {
  return ORB->object_to_string(object);
}

string ORBMgr::getIOR(const string& ctxt, const string& name) const {
  return ORB->object_to_string(resolveObject(ctxt, name));
}

void ORBMgr::activate(PortableServer::ServantBase* object) const {
  POA->activate_object(object);
  object->_remove_ref();
}

void ORBMgr::deactivate(PortableServer::ServantBase* object) const {
  PortableServer::ObjectId* id = POA->servant_to_id(object);
  POA->deactivate_object(*id);
}


void ORBMgr::wait() const {
  ORB->run();
}

ORBMgr* ORBMgr::getMgr() {
  if (theMgr==NULL)
    throw runtime_error("ORB manager not initialized!");
  else return theMgr;
}

void ORBMgr::init(int argc, char* argv[]) {
  if (theMgr)
    delete theMgr;
  theMgr = new ORBMgr(argc, argv);
}

/* Translate the string passed as first argument in bytes and
 * record them into the buffer.
 */
void hexStringToBuffer(const char* ptr, const size_t size,
		       cdrMemoryStream& buffer) {
  stringstream ss;
  int value;
  CORBA::Octet c;
	
  for (unsigned int i=0; i<size; i+=2) {
    ss << ptr[i] << ptr[i+1];
    ss >> hex >> value;
    c = value;
    buffer.marshalOctet(c);
    ss.flush();
    ss.clear();
  }
}

/* Make an IOP::IOR object using a stringified IOR. */
void ORBMgr::makeIOR(const string& strIOR, IOP::IOR& ior)
{
  /* An IOR must start with "IOR:" or "ior:" */
  if (strIOR.find("IOR:")!=0 && strIOR.find("ior:")!=0)
    throw runtime_error("Bad IOR: "+strIOR);

  const char* tab = strIOR.c_str();
  size_t size = (strIOR.length()-4);
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
void ORBMgr::makeString(const IOP::IOR& ior, string& strIOR) {
  strIOR = "IOR:";
  cdrMemoryStream buffer(0, true);
  stringstream ss;
  unsigned char* ptr;
		
  buffer.marshalBoolean(omni::myByteOrder);
  buffer.marshalRawString(ior.type_id);
  ior.profiles >>= buffer;
	
  buffer.rewindInputPtr();
  ptr = static_cast<unsigned char*>(buffer.bufPtr());
	
  for (unsigned long i=0; i < buffer.bufSize(); ++ptr, ++i)
    {
      string str;
      unsigned char c = *ptr;
      if (c<16) ss << '0';
      ss << (unsigned short) c;
      ss >> hex  >> str;
      ss.flush();
      ss.clear();
      strIOR+=str;
    }
}

/* Get the hostname of the first profile in this IOR. */
string ORBMgr::getHost(IOP::IOR& ior) {
  IIOP::ProfileBody body;
	
  if (ior.profiles.length()==0)
    return "nohost";
	
  IIOP::unmarshalProfile(ior.profiles[0], body);
  return string(body.address.host);
}

/* Get the hostname of the first profile in IOR passed
 * as a string.
 */
string ORBMgr::getHost(const string& strIOR) {
  IOP::IOR ior;
  makeIOR(strIOR, ior);
  return getHost(ior);
}

/* Get the port of the first profile in this IOR. */
unsigned int ORBMgr::getPort(IOP::IOR& ior) {
  IIOP::ProfileBody body;
	
  if (ior.profiles.length()==0)
    return 0;
	
  IIOP::unmarshalProfile(ior.profiles[0], body);
  return body.address.port;
}

/* Get the port of the first profile in IOR passed
 * as a string.
 */
unsigned int ORBMgr::getPort(const string& strIOR) {
  IOP::IOR ior;
  makeIOR(strIOR, ior);
  return getPort(ior);
}

/* Get the type id of the IOR. */
string ORBMgr::getTypeID(IOP::IOR& ior) {
  return string(ior.type_id);
}

/* Get the type id of the IOR passed as a string. */
std::string ORBMgr::getTypeID(const string& strIOR) {
  IOP::IOR ior;
  makeIOR(strIOR, ior);
  return getTypeID(ior);
}

std::string ORBMgr::convertIOR(IOP::IOR& ior, const std::string& host,
			       const unsigned int port)
{
  IIOP::ProfileBody body;
  IOP::TaggedProfile profile;
  CORBA::ULong max_data;
  CORBA::ULong nb_data;
  CORBA::Octet* buffer;
  std::string result;

  if (ior.profiles.length()==0)
    throw runtime_error("Invalid IOR");
	
  for (unsigned int i=0; i<ior.profiles.length(); ++i)
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
  makeString(ior, result);
  return result;
}

std::string ORBMgr::convertIOR(const std::string& strIOR, const std::string& host,
			       const unsigned int port)
{
  IOP::IOR ior;
  makeIOR(strIOR, ior);
  return convertIOR(ior, host, port);
}

/* Object cache management functions. */
void ORBMgr::resetCache() const {
  cacheMutex.lock();
  cache.clear();
  cacheMutex.unlock();
}

void ORBMgr::removeObjectFromCache(const string& name) const {
  map<string, CORBA::Object_ptr>::iterator it;
  cacheMutex.lock();
  if ((it=cache.find(name))!=cache.end())	cache.erase(it);
  cacheMutex.unlock();
}

void ORBMgr::removeObjectFromCache(const string& ctxt,
				   const string& name) const {
  removeObjectFromCache(ctxt+"/"+name);
}

void ORBMgr::cleanCache() const {
  map<string, CORBA::Object_ptr>::iterator it;
  std::list<string> toRemove;
  std::list<string>::const_iterator jt;

  cacheMutex.lock();
  for (it=cache.begin(); it!=cache.end(); ++it) {
    try {
      if (it->second->_non_existent())
	toRemove.push_back(it->first);
    } catch (const CORBA::OBJECT_NOT_EXIST& err) {
      toRemove.push_back(it->first);
    }
  }
  cacheMutex.unlock();
  for (jt=toRemove.begin(); jt!=toRemove.end(); ++jt)
    removeObjectFromCache(*jt);
}
