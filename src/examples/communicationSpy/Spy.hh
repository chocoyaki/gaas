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
  typedef std::map<std::string, std::vector<spy::Address> > mapAgentAddresses;


private:
  explicit Spy(int argc, char **argv);
  virtual ~Spy();

public:

  static Spy * getSpy();
  static void init(int argc, char **argv);
  static void kill();

  void spyOn(std::string name);
  bool isListeningToPort(ushort port);
  std::string createFilter();
  int run();
  std::string isBindedToPort(ushort port);


  friend void analysePacket(u_char *args, const struct pcap_pkthdr* pkthdr, const u_char* packet);

private:

  int initORB(int argc, char **argv);
  void updateSpiedComponents();



  std::set<std::string> spiedComponents;
  DietLogComponent *dietLogComponent;

  mapAgentAddresses watch;
  std::map<ushort, std::string> portOf;

  static Spy * instance;

};



#endif /* _SPY_HH_ */
