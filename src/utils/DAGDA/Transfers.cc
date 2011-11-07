/**
 * @file Transfers.cc
 *
 * @brief  Object to obtain transfers progression.
 *
 * @author Gael Le Mahec (lemahec@clermont.in2p3.fr)
 *
 * @section Licence
 *   |LICENCE|
 */


#include <string>
#include <map>
#include <stdexcept>
#include <iostream>
#include "Transfers.hh"
#include "DagdaFactory.hh"

Transfers *Transfers::instance = NULL;

Transfers *
Transfers::getInstance() {
  if (instance == NULL) {
    instance = new Transfers(DagdaFactory::getMaxMsgSize());
  }
  return instance;
}

void
Transfers::newTransfer(const std::string &id,
                       const unsigned long long total) {
  progress[id].total = total;
  progress[id].transfered = 0;
}


void
Transfers::incProgress(const std::string &id) {
  std::map<std::string, struct transfer_t>::iterator it;

  if ((it = progress.find(id)) != progress.end()) {
    it->second.transfered += msgSize;
    if (it->second.transfered > it->second.total) {
      it->second.transfered = it->second.total;
    }
  } else {
    throw std::runtime_error("Unknown transfer: " + id);
  }
} // incProgress

Transfers::Transfers(unsigned long long msgSize) {
  this->msgSize = msgSize;
}

unsigned long long
Transfers::totalSize(const std::string &id) const {
  std::map<std::string, struct transfer_t>::const_iterator it;

  if ((it = progress.find(id)) != progress.end()) {
    return it->second.total;
  }
  throw std::runtime_error("Unknown transfer: " + id);
}

unsigned long long
Transfers::getTransfered(const std::string &id) const {
  std::map<std::string, struct transfer_t>::const_iterator it;

  if ((it = progress.find(id)) != progress.end()) {
    return it->second.transfered;
  }
  throw std::runtime_error("Unknown transfer: " + id);
}

double
Transfers::getProgress(const std::string &id) {
  return ((double) getTransfered(id)) / ((double) totalSize(id));
}

void
Transfers::remTransfer(const std::string &id) {
  std::map<std::string, struct transfer_t>::iterator it;

  if ((it = progress.find(id)) != progress.end()) {
    progress.erase(it);
  }
  throw std::runtime_error("Unknown transfer: " + id);
}
