/*
 * @file Spy.hh
 *
 * @brief  
 *
 * @author  Guillaume Verger (guillaume.verger@inria.fr)
 *
 * @section Licence
 *   |LICENCE|
 */

#ifndef _SPY_HH_
#define _SPY_HH_

#include <vector>
#include <set>
#include "DietLogComponent.hh"
#include "Address.hh"

class Spy {
public:
  Spy(int argc, char **argv);
  virtual ~Spy();

  void spyOn(std::string &name);
  void listenToPort(int port);
  void stopListeningPort(int port);
  std::string createFilter();

private:

  int initORB(int argc, char **argv);
  void updateSpiedComponents();


  std::set<std::string> spiedComponents;
  std::vector<int> ports;
  DietLogComponent *dietLogComponent;

  std::map<std::string, std::vector<spy::Address> > watch;
  std::map<ushort, std::string> portOf;

};

#endif /* _SPY_HH_ */
