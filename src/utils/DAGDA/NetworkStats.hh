/***********************************************************/
/* Network statistics utility classes headers.             */
/*                                                         */
/*  Author(s):                                             */
/*    - Gael Le Mahec (lemahec@clermont.in2p3.fr)          */
/*                                                         */
/* $LICENSE$                                               */
/***********************************************************/
/* $Id
/* $Log
/*														   */
/***********************************************************/
#ifndef __NETWORKSTATS_HH__
#define __NETWORKSTATS_HH__

#include <map>
#include <string>

class NetworkStats {
protected:
  std::map<std::string, std::map<std::string, double> > values;
public:
  NetworkStats(const NetworkStats& stat) : values(stat.values) { }
  virtual double getStat(std::string src, std::string dest);
  virtual bool cmpStats(double s1, double s2) = 0;
  virtual void addStat(std::string src, std::string dest, double value) = 0;
};

class AvgNetworkStats : public NetworkStats {
protected:
  std::map<std::string, std::map<std::string, unsigned long> > nbStats;
public:
  AvgNetworkStats(const AvgNetworkStats& stat) : NetworkStats(stat), nbStats(stat.nbStats) {}
  virtual double getStat(std::string src, std::string dest);
  virtual bool cmpStats(double s1, double s2);
  virtual void addStats(std::string src, std::string dest, double value);
};
#endif
