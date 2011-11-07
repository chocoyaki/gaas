/**
 * @file Transfers.hh
 * @brief  Object to obtain transfers progression.
 *
 * @author Gael Le Mahec (lemahec@clermont.in2p3.fr)
 *
 * @section Licence
 *   |LICENCE|
 */


#ifndef TRANSFERS_HH
#define TRANSFERS_HH

#include <map>
#include <string>
#include "DagdaImpl.hh"

class Transfers {
public:
  static Transfers *
  getInstance();

  void
  newTransfer(const std::string &id, const unsigned long long total);

  unsigned long long
  totalSize(const std::string &id) const;

  unsigned long long
  getTransfered(const std::string &id) const;

  double
  getProgress(const std::string &id);

  void
  remTransfer(const std::string &id);

private:
  struct transfer_t {
    unsigned long long total;
    unsigned long long transfered;
  };
  static Transfers *instance;
  unsigned long long msgSize;
  std::map<std::string, struct transfer_t> progress;

  void
  incProgress(const std::string &id);

  friend char *
  DagdaImpl::sendFile(const corba_data_t &, const char *destName);

  friend char *
  DagdaImpl::sendData(const char *, const char *destName);

  explicit
  Transfers(unsigned long long msgSize);
};

#endif /* ifndef TRANSFERS_HH */
