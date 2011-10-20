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
 * Revision 1.3  2010/07/12 17:23:07  glemahec
 * DIET 2.5 beta 1 - The missing commit
 *
 * Revision 1.2  2010/04/20 12:30:33  glemahec
 * Ajout des fichiers Transfers.?? dans Distribution_files.lst et des en-tetes dans lesdits fichiers
 *
 ****************************************************************************/


#ifndef TRANSFERS_HH
#define TRANSFERS_HH

#include <map>
#include <string>
#include "DagdaImpl.hh"

class Transfers {
public:
  static Transfers*
  getInstance();

  void
  newTransfer(const std::string& id, const unsigned long long total);

  unsigned long long
  totalSize(const std::string& id) const;

  unsigned long long
  getTransfered(const std::string& id) const;

  double
  getProgress(const std::string& id);

  void
  remTransfer(const std::string& id);

private:
  struct transfer_t {
    unsigned long long total;
    unsigned long long transfered;
  };
  static Transfers* instance;
  unsigned long long msgSize;
  std::map<std::string, struct transfer_t> progress;

  void
  incProgress(const std::string& id);

  friend char*
  DagdaImpl::sendFile(const corba_data_t&, const char* destName);

  friend char*
  DagdaImpl::sendData(const char*, const char* destName);

  explicit Transfers(unsigned long long msgSize);
};

#endif
