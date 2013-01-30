/*
 * @file SecurityManager.cc
 *
 * @brief  Utility class for handling security for DIET
 *
 * @author   Guillaume Verger(guillaume.verger@inria.fr)
 */

// CMake variables
#include "security_config.h"

#ifdef DIET_USE_SECURITY
#include "SecurityManager.hh"

#include <string>
#include <set>
#include <vector>
#include <boost/program_options.hpp>

#include <omniORB4/CORBA.h>
#include <omniORB4/sslContext.h>

#include "debug.hh"
#include "configuration.hh"

namespace po = boost::program_options;

SecurityManager::SecurityManager() {

  this->cAFile = "UNDEFINED";
  this->keyFile = "UNDEFINED";
  this->passwordKey = "";
}

bool
SecurityManager::initSSLContext() {
  /*
   *  Setting SSL Contex
   */
  CONFIG_STRING(diet::SSLROOTCERTIFICATE, this->cAFile);
  CONFIG_STRING(diet::SSLPRIVATEKEY, this->keyFile);
  CONFIG_STRING(diet::SSLPRIVATEKEYPASSWORD, this->passwordKey);

  sslContext::certificate_authority_file = cAFile.c_str();
  sslContext::key_file = keyFile.c_str();
  sslContext::key_file_password = passwordKey.c_str();

  sslContext::verify_mode = SSL_VERIFY_FAIL_IF_NO_PEER_CERT;

  return true;
}


SecurityManager::~SecurityManager() {
  for (int i = 0; i < secuOptions.size(); ++i) {
      free(secuOptions[i]);
    }
}



void
insertOptions(const std::string &option, const std::vector<std::string> &values, std::vector< char * > &toFill) {

  char * name = new char[option.size()];
  strcpy(name, option.c_str());
  toFill.push_back(name);
  for (int i = 0; i < values.size(); ++i) {
       char * o = new char[values[i].size()];
       strcpy(o, values[i].c_str());
       toFill.push_back(o);
     }

}

void
insertOptions(const std::string &option, const std::set<std::string> &values, std::vector< char * > &toFill) {

  char * name = new char[option.size()];
  strcpy(name, option.c_str());
  toFill.push_back(name);
  for (std::set<std::string>::iterator it = values.begin(); it != values.end(); ++it) {
       char * o = new char[it->size()];
       strcpy(o, it->c_str());
       toFill.push_back(o);
     }

}


bool
SecurityManager::secureORBOptions(int argc, char * argv[]) {

  // Updated options
  //std::vector<char *> secuOptions;

  std::vector<std::string> oEndPoint;
  //  std::vector<std::string> oEndPointPublish;
  std::vector<std::string> oServerTransportRule;
  std::vector<std::string> oClientTransportRule;

  po::options_description desc("Handled Options");
  desc.add_options()
      ("ORBendPoint", po::value<std::vector<std::string> >(&oEndPoint), "end point description")
      // ("ORBendPointPublish", po::value<std::vector <std::string> >(&oEndPointPublish), "end point publishing description")
      ("ORBserverTransportRule", po::value<std::vector<std::string> >(&oServerTransportRule), "server transport rule")
      ("ORBclientTransportRule", po::value<std::vector<std::string> >(&oClientTransportRule), "client transport rule")
      ;

  po::variables_map vm;

  try {
    po::parsed_options opts = po::command_line_parser(argc, argv).style(
        po::command_line_style::allow_long_disguise
            | po::command_line_style::long_allow_next).allow_unregistered().options(desc).run();

    std::vector<std::string> unmodifiedOptions = po::collect_unrecognized(opts.options, po::include_positional);

    for (int i = 0; i < unmodifiedOptions.size(); ++i) {
      char * o = new char[unmodifiedOptions[i].size()];
      strcpy(o, unmodifiedOptions[i].c_str());
      secuOptions.push_back(o);
    }

    po::store(opts, vm);
    po::notify(vm);

    std::cout << "Variables : ("<< vm.size() <<")\n";
    for (po::variables_map::iterator it = vm.begin(); it != vm.end(); ++it) {
       std::cout << it->first  << std::endl;
     }



    // Securing end points
    std::set<std::string> endPointToSet;
    if (oEndPoint.empty()) {
      endPointToSet.insert("giop:tcp::");
      endPointToSet.insert("giop:ssl::");
      TRACE_TEXT(TRACE_MAIN_STEPS, "Changing end point configuration : allowing ssl and tcp endpoints." << std::endl);
    } else {
      for (std::vector<std::string>::iterator iEndPoint = oEndPoint.begin();
          iEndPoint != oEndPoint.end(); ++iEndPoint) {
        int idx = iEndPoint->find(":tcp:");
        if (idx != std::string::npos) {
          std::string secuEP = std::string(*iEndPoint).replace(idx, 5, ":ssl:");
          endPointToSet.insert(secuEP);
        }
        endPointToSet.insert(*iEndPoint);
      }
      TRACE_TEXT(TRACE_MAIN_STEPS, "End points are : " << std::endl);
      for (std::set<std::string>::iterator iEndPointToSet =
          endPointToSet.begin(); iEndPointToSet != endPointToSet.end();
          ++iEndPointToSet) {
        TRACE_TEXT(TRACE_MAIN_STEPS, *iEndPointToSet << std::endl);
      }
    }

    // Filling end point options
    insertOptions("-ORBendPoint", endPointToSet, secuOptions);


    // Securing server rules: only accept incoming ssl requests
    std::vector<std::string> serverRuleToSet;
    if (!oServerTransportRule.empty()) {
      for (std::vector<std::string>::iterator iServerTR = oServerTransportRule
          .begin(); iServerTR != oServerTransportRule.end(); ++iServerTR) {
        int idx = iServerTR->find("tcp");
        if (idx == std::string::npos) {
          serverRuleToSet.push_back(*iServerTR);
        } else {
          WARNING("Ignoring server transport rule [" << *iServerTR
              << "] : tcp is not allowed !" << std::endl);
        }
      }
    }
    if (serverRuleToSet.empty()) {
      serverRuleToSet.push_back("* ssl");
    }
    // Filling server rule options
    insertOptions("-ORBserverTransportRule", serverRuleToSet, secuOptions);


    // Securing client rules: prefer sending ssl requests
    std::vector<std::string> clientRuleToSet;
    if (!oClientTransportRule.empty()) {
      for (std::vector<std::string>::iterator iClientTR = oClientTransportRule
          .begin(); iClientTR != oClientTransportRule.end(); ++iClientTR) {
        int idxTCP = iClientTR->find("tcp");
        std::string rule = *iClientTR;
        if (idxTCP != std::string::npos) {
          int idxSSL = iClientTR->find("ssl");
          if (idxSSL == std::string::npos || idxSSL > idxTCP) {
            rule.replace(idxTCP, 3, "ssl");

            if (idxSSL > idxTCP) {
              rule.replace(idxSSL, 3, "tcp");
            }
            TRACE_TEXT(
                TRACE_ALL_STEPS,
                "Rewriting client transport rule [" << *iClientTR << "] to fit security needs into [" <<
                rule << "]" << std::endl);

          }
        }
        clientRuleToSet.push_back(rule);
      }
    }
    if (clientRuleToSet.empty()) {
      clientRuleToSet.push_back("* ssl,tcp");
    }

    // Filling server rule options
    insertOptions("-ORBclientTransportRule", clientRuleToSet, secuOptions);


    for (int i = 0; i < secuOptions.size(); ++i) {
      std::cout << "opt : " << secuOptions[i] << std::endl;
    }

  } catch (const std::exception &e) {
    WARNING("Exception raised while processing security options for the Orb : " << e.what() << std::endl);
    return false;
  }
  return true;
}

bool SecurityManager::enableSecurity(int argc, char * argv[]) {
  this->initSSLContext();
  this->secureORBOptions(argc, argv);

  return true;
}

#endif
