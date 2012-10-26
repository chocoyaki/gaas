/**
 * @file NetworkStats.hh
 *
 * @brief  Network statistics utility classes headers.
 *
 * @author Gael Le Mahec (lemahec@clermont.in2p3.fr)
 *
 * @section Licence
 *   |LICENSE|
 */

#ifndef __NETWORKSTATS_HH__
#define __NETWORKSTATS_HH__

#include <map>
#include <string>

class NetworkStats {
public:
  NetworkStats()
    : values() {
  }

  NetworkStats(const NetworkStats &stat)
    : values(stat.values) {
  }

  virtual ~NetworkStats() {
  }

  virtual double
  getStat(std::string src, std::string dest);

  virtual bool
  cmpStats(double s1, double s2) = 0;

  virtual void
  addStat(std::string src, std::string dest, double value) = 0;

protected:
  std::map<std::string, std::map<std::string, double> > values;
};

class AvgNetworkStats : public NetworkStats {
public:
  AvgNetworkStats()
  : NetworkStats(), nbStats() {
  }

  AvgNetworkStats(const AvgNetworkStats &stat)
  : NetworkStats(stat), nbStats(stat.nbStats) {
  }

  virtual ~AvgNetworkStats() {
  }

  virtual double
  getStat(std::string src, std::string dest);

  virtual bool
  cmpStats(double s1, double s2);

  virtual void
  addStat(std::string src, std::string dest, double value);

protected:
  std::map<std::string, std::map<std::string, unsigned long> > nbStats;
};
#endif /* ifndef __NETWORKSTATS_HH__ */
