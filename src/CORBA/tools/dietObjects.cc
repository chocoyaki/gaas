/****************************************************************************/
/* dietObjects tool                                                         */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Gael Le Mahec   (gael.le.mahec@u-picardie.fr)                       */
/*                                                                          */
/* Options:                                                                 */
/*  list: without argument show the DIET corba contexts                     */
/*        show the DIET objects of given context(s)                         */
/*  forwarder: without argument show the active DIET forwarders             */
/*             show the active forwarders of given name(s)                  */
/*  resolve: display the IOR(s) of the DIET objects                         */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 ****************************************************************************/
#include <iostream>
#include <cstdlib>
#include <string>
#include <list>
#include <algorithm>
#include <iterator>
#include <vector>
#include <stdexcept>

#include "ORBMgr.hh"
#include "DIETForwarder.hh"

using namespace std;

/* Sorted DIET contexts list. */
static const char* DIET_CTXTS[] = {DAGDACTXT, LOGCOMPCTXT, WFLOGCTXT,
                                   DATAMGRCTXT, AGENTCTXT, CLIENTCTXT,
                                   MADAGCTXT, SEDCTXT, WFMGRCTXT,
                                   LOCMGRCTXT, NULL};

/* Specific class to manage arguments of dietObjects. */
/* TODO: Use a more generic class. */
class Args {
public:
  Args(unsigned int argc, char* argv[]) : argc(argc), argv(argv, argv+argc) {}
  string getArg(const unsigned int idx) const {
    if (idx>=argc) return "";
    return argv.at(idx);
  }
  std::list<string> values() const {
    if (argc<=2) return std::list<string>();
    vector<string>::const_iterator beg = argv.begin();
    advance(beg, 2);
    return std::list<string>(beg, argv.end());
  }
  
  bool list() const { return getArg(1)=="list" && argc==2; }
  bool objList() const { return getArg(1)=="list" && argc>2; }
  bool forwarder() const { return getArg(1)=="forwarder" && argc==2; }
  bool objForwarder() const { return getArg(1)=="forwarder" && argc>2; }
  bool resolve() const { return getArg(1)=="resolve" && argc>2; }
private:
  unsigned int argc;
  vector<string> argv;
};

string getName(const string& namectxt) {
  size_t pos = namectxt.find('/');
  if (pos==string::npos) return namectxt;
  return namectxt.substr(pos+1);
}

string getCtxt(const string& namectxt) {
  size_t pos = namectxt.find('/');
  if (pos==string::npos) return "";
  return namectxt.substr(0, pos);
}


int main(int argc, char* argv[]) {
  ORBMgr::init(argc, argv);
  ORBMgr* mgr = ORBMgr::getMgr();
  list<string>::iterator it;

  Args args(argc, argv);
  
  if (args.list()) { // Display DIET CORBA contexts
    list<string> allCtxts = mgr->contextList();
    allCtxts.sort();
  
    list<string> dietCtxts;
  
    set_intersection(DIET_CTXTS, DIET_CTXTS+10, allCtxts.begin(),
                     allCtxts.end(), insert_iterator<list<string> >(dietCtxts, dietCtxts.begin()));

    for (it=dietCtxts.begin(); it!=dietCtxts.end(); ++it)
      cout << *it << endl;
  }
  
  if (args.objList()) { // Display DIET objects on a given contexts
    list<string> contexts = args.values();
    contexts.sort();
    list<string> dietCtxts;
    
    set_intersection(DIET_CTXTS, DIET_CTXTS+10, contexts.begin(),
                     contexts.end(), insert_iterator<list<string> >(dietCtxts, dietCtxts.begin()));
    
    for (it=dietCtxts.begin(); it!=dietCtxts.end(); ++it) {
      list<string> objects;
      list<string>::const_iterator jt;
      cout << "Object type: " << *it << " (";
      objects=mgr->list(*it);
      cout << objects.size() << " object" << (objects.size()>1 ? "s)":")") << endl;
      for (jt=objects.begin(); jt!=objects.end(); ++jt) {
        bool isLocal = mgr->isLocal(*it, *jt);
        cout << "  " << *jt;
        if (isLocal)
          cout << " (local object)" << endl;
        else {
          cout << " (proxy object reachable through " << mgr->forwarderName(*it, *jt);
          cout << " forwarder)" << endl;
        }
      }
    }
  }
  
  if (args.forwarder() || args.objForwarder()) { // Display the Forwarders
    list<string> forwarders;
    if (args.objForwarder())
      forwarders = args.values();
    else
      forwarders = mgr->list(FWRDCTXT);
    
    for (it=forwarders.begin(); it!=forwarders.end(); ++it) {
      Forwarder_var fwd;
      list<string> objects;
      
      for (const char** jt=DIET_CTXTS; *jt!=NULL; ++jt) {
        list<string> obj = mgr->list(*jt);
        for (list<string>::const_iterator kt=obj.begin(); kt!=obj.end(); ++kt) {
          CORBA::Object_ptr object = mgr->simpleResolve(*jt, *kt);
          string ior = mgr->getIOR(object);
          
          if (mgr->getHost(ior)=="@"+*it)
            objects.push_back(string(*jt)+"/"+*kt);
        }
      }
      cout << "Forwarder " << *it << endl;
      try {
        fwd = mgr->resolve<Forwarder, Forwarder_var>(FWRDCTXT, *it);
      } catch (runtime_error& err) {
        cerr << "Unknown forwarder: " << *it << endl;
        continue;
      }
      
      cout << "  " << fwd->getName() << " (" << fwd->getHost() << ")";
      cout << " <=> " << fwd->getPeerName() << " (" << fwd->getPeerHost() << ")" << endl;
      cout << "  Managing: " << endl;
      for (list<string>::const_iterator jt = objects.begin(); jt!=objects.end(); ++jt)
        cout << "    - " << *jt << endl;
    }
  }
  
  if (args.resolve()) {
    list<string> objects = args.values();
    
    for (it=objects.begin(); it!=objects.end(); ++it) {
      string ctxt = getCtxt(*it);
      string name = getName(*it);
      CORBA::Object_ptr localObject;
      CORBA::Object_ptr proxyObject;
      
      if (ctxt=="" || name=="") {
        cerr << "Error parsing " << *it << ". Use <context>/<name> syntax." << endl;
        continue;
      }

      try {
        localObject = mgr->simpleResolve(ctxt, name);
        proxyObject = mgr->resolveObject(ctxt, name);
      } catch (runtime_error& err) {
        cerr << "Unknown object " << *it << endl;
        continue;
      }
      if (mgr->isLocal(ctxt, name)) {
        string ior = mgr->getIOR(localObject);
        
        cout << *it << " is a local object" << endl;
        cout << ior << endl;
      } else {
        string localIor = mgr->getIOR(localObject);
        string proxyIor = mgr->getIOR(proxyObject);
        cout << *it << " is reachable through " << mgr->forwarderName(ctxt, name) << endl;
        cout << "Local object IOR: " << localIor << endl;
        cout << "Proxy object IOR: " << proxyIor << endl;
      }
    }
  }
  mgr->shutdown(true);
}
