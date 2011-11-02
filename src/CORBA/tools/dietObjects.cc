/**
* @file	  dietObjects.cc
* 
* @brief  dietObjects tool
* 
* @author - Gael Le Mahec   (gael.le.mahec@ens-lyon.fr) 
* 
* @section Licence
*   |LICENSE|                                                                
*/
/****************************************************************************/
/* Options:                                                                 */
/*  list: without argument show the DIET corba contexts                     */
/*        show the DIET objects of given context(s)                         */
/*  forwarder: without argument show the active DIET forwarders             */
/*             show the active forwarders of given name(s)                  */
/*  resolve: display the IOR(s) of the DIET objects                         */
/*                                                                          */
/****************************************************************************/
/* $Id$
 * $Log$
 ****************************************************************************/
#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <list>
#include <string>
#include <stdexcept>
#include <vector>

#include "DIETForwarder.hh"
#include "ORBMgr.hh"

/* Sorted DIET contexts list. */
static const char* DIET_CTXTS[] = {DAGDACTXT,
                                   LOGCOMPCTXT,
                                   WFLOGCTXT,
                                   AGENTCTXT,
                                   CLIENTCTXT,
                                   MADAGCTXT,
                                   SEDCTXT,
                                   WFMGRCTXT,
                                   NULL};

/* Specific class to manage arguments of dietObjects. */
/* TODO: Use a more generic class. */
class Args {
public:
  Args(unsigned int argc, char* argv[]) : argc(argc), argv(argv, argv+argc) {
  }

  std::string
  getArg(const unsigned int idx) const {
    if (idx >= argc) {
      return "";
    }
    return argv.at(idx);
  }

  std::list<std::string>
  values() const {
    if (argc <= 2) {
      return std::list<std::string>();
    }

    std::vector<std::string>::const_iterator beg = argv.begin();
    std::advance(beg, 2);
    return std::list<std::string>(beg, argv.end());
  }

  bool
  list() const {
    return (getArg(1) == "list") && (argc == 2);
  }

  bool
  objList() const {
    return (getArg(1) == "list") && (argc > 2);
  }

  bool
  forwarder() const {
    return (getArg(1) == "forwarder") && (argc == 2);
  }

  bool
  objForwarder() const {
    return (getArg(1) == "forwarder") && (argc > 2);
  }

  bool
  resolve() const {
    return (getArg(1) == "resolve") && (argc > 2);
  }

private:
  unsigned int argc;
  std::vector<std::string> argv;
};

std::string
getName(const std::string& namectxt) {
  size_t pos = namectxt.find('/');
  if (pos == std::string::npos) {
    return namectxt;
  }

  return namectxt.substr(pos+1);
}

std::string
getCtxt(const std::string& namectxt) {
  size_t pos = namectxt.find('/');
  if (pos == std::string::npos) {
    return "";
  }

  return namectxt.substr(0, pos);
}


int
main(int argc, char* argv[]) {
  ORBMgr::init(argc, argv);
  ORBMgr* mgr = ORBMgr::getMgr();
  std::list<std::string>::iterator it;

  Args args(argc, argv);

  if (args.list()) {
    // Display DIET CORBA contexts
    std::list<std::string> allCtxts = mgr->contextList();
    allCtxts.sort();

    std::list<std::string> dietCtxts;

    set_intersection(DIET_CTXTS, DIET_CTXTS+10,
                     allCtxts.begin(), allCtxts.end(),
                     std::insert_iterator<std::list<
                       std::string> >(dietCtxts, dietCtxts.begin()));

    for (it = dietCtxts.begin(); it != dietCtxts.end(); ++it) {
      std::cout << *it << "\n";
    }
  }

  if (args.objList()) {
    // Display DIET objects on a given contexts
    std::list<std::string> contexts = args.values();
    contexts.sort();
    std::list<std::string> dietCtxts;

    set_intersection(DIET_CTXTS, DIET_CTXTS+10,
                     contexts.begin(), contexts.end(),
                     std::insert_iterator<std::list<
                     std::string> >(dietCtxts, dietCtxts.begin()));

    for (it = dietCtxts.begin(); it != dietCtxts.end(); ++it) {
      std::list<std::string> objects;
      std::list<std::string>::const_iterator jt;
      std::cout << "Object type: " << *it << " (";
      objects = mgr->list(*it);
      std::cout << objects.size() << " object"
                << (objects.size()>1 ? "s)":")") << "\n";
      for (jt = objects.begin(); jt != objects.end(); ++jt) {
        bool isLocal = mgr->isLocal(*it, *jt);
        std::cout << "  " << *jt;
        if (isLocal) {
          std::cout << " (local object)\n";
        } else {
          std::cout << " (proxy object reachable through "
                    << mgr->forwarderName(*it, *jt);
          std::cout << " forwarder)\n";
        }
      }
    }
  }

  if (args.forwarder() || args.objForwarder()) {
    // Display the Forwarders
    std::list<std::string> forwarders;
    if (args.objForwarder()) {
      forwarders = args.values();
    } else {
      forwarders = mgr->list(FWRDCTXT);
    }

    for (it = forwarders.begin(); it != forwarders.end(); ++it) {
      Forwarder_var fwd;
      std::list<std::string> objects;

      for (const char** jt = DIET_CTXTS; *jt != NULL; ++jt) {
        std::list<std::string> obj = mgr->list(*jt);
        for (std::list<std::string>::const_iterator kt = obj.begin();
             kt != obj.end();
             ++kt) {
          CORBA::Object_ptr object = mgr->simpleResolve(*jt, *kt);
          std::string ior = mgr->getIOR(object);

          if (mgr->getHost(ior) == ("@" + *it)) {
            objects.push_back(std::string(*jt) + "/" + *kt);
          }
        }
      }
      std::cout << "Forwarder " << *it << "\n";
      try {
        fwd = mgr->resolve<Forwarder, Forwarder_var>(FWRDCTXT, *it);
      } catch (std::runtime_error& err) {
        std::cerr << "Unknown forwarder: " << *it << "\n";
        continue;
      }

      try {
        std::cout << "  " << fwd->getName() << " (" << fwd->getHost() << ")";
        std::cout << " <=> " << fwd->getPeerName()
                  << " (" << fwd->getPeerHost() << ")\n";
        std::cout << "  Managing: \n";
        for (std::list<std::string>::const_iterator jt = objects.begin();
           jt != objects.end();
           ++jt) {
          std::cout << "    - " << *jt << "\n";
        }
      } catch (...) {
        std::cerr << "Error: forwarder \""
                  << *it
                  << "\" is unreachable\n";
      }
    }
  }

  if (args.resolve()) {
    std::list<std::string> objects = args.values();

    for (it = objects.begin(); it != objects.end(); ++it) {
      std::string ctxt = getCtxt(*it);
      std::string name = getName(*it);
      CORBA::Object_ptr localObject;
      CORBA::Object_ptr proxyObject;

      if (ctxt.empty() || name.empty()) {
        std::cerr << "Error parsing " << *it
                  << ". Use <context>/<name> syntax.\n";
        continue;
      }

      try {
        localObject = mgr->simpleResolve(ctxt, name);
        proxyObject = mgr->resolveObject(ctxt, name);
      } catch (std::runtime_error& err) {
        std::cerr << "Unknown object " << *it << "\n";
        continue;
      }
      if (mgr->isLocal(ctxt, name)) {
        std::string ior = mgr->getIOR(localObject);

        std::cout << *it << " is a local object\n";
        std::cout << ior << "\n";
      } else {
        std::string localIor = mgr->getIOR(localObject);
        std::string proxyIor = mgr->getIOR(proxyObject);
        std::cout << *it << " is reachable through "
                  << mgr->forwarderName(ctxt, name) << "\n";
        std::cout << "Local object IOR: " << localIor << "\n";
        std::cout << "Proxy object IOR: " << proxyIor << "\n";
      }
    }
  }
  mgr->shutdown(true);
}
