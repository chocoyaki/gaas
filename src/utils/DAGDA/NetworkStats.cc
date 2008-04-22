/***********************************************************/
/* Network statistics utility classes.                     */
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
#include "NetworkStats.hh"

double NetworkStats::getStat(std::string src, std::string dest) {
  if (values.find(src)!=values.end())
    return (values[src])[dest];
  else return -1;
}

double AvgNetworkStats::getStat(std::string src, std::string dest) {
  double ret = NetworkStats::getStat(src, dest);
  if (ret==-1) return -1;
  return ret/(nbStats[src])[dest];
}

bool AvgNetworkStats::cmpStats(double s1, double s2) {
  if (s1>s2) return true;
  return false;
}

void AvgNetworkStats::addStats(std::string src, std::string dest, double value) {
  double currentStat=getStat(src, dest);
  if (currentStat==-1)
    (values[src])[dest]=value;
  else
    (values[src])[dest]+=value;
  (nbStats[src])[dest]++;
}
