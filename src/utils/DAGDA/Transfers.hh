/****************************************************************************/
/* Object to obtain transfers progression.                                  */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Gaël Le Mahec (gael.le.mahec@ens-lyon.fr)                           */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.2  2010/04/20 12:30:33  glemahec
 * Ajout des fichiers Transfers.?? dans Distribution_files.lst et des en-tetes dans lesdits fichiers
 *
 ****************************************************************************/


#ifndef TRANSFERS_HH
#define TRANSFERS_HH

#include <string>
#include <map>
#include "DagdaImpl.hh"

class Transfers {
private:
  struct transfer_t {
    unsigned long long total;
    unsigned long long transfered;
  };
  static Transfers* instance;
  unsigned long long msgSize;
  std::map<std::string, struct transfer_t> progress;
  void incProgress(const std::string& id);
  
  friend char* DagdaImpl::sendFile(const corba_data_t&, Dagda_ptr dest);
  friend char* DagdaImpl::sendData(const char*, Dagda_ptr dest);
  Transfers(unsigned long long msgSize);
public:
  static Transfers* getInstance();
  void newTransfer(const std::string& id, const unsigned long long total);
  unsigned long long totalSize(const std::string& id) const;
  unsigned long long getTransfered(const std::string& id) const;
  double getProgress(const std::string& id);
  void remTransfer(const std::string& id);
};

#endif
