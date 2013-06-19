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
#include <boost/foreach.hpp>

#include <omniORB4/CORBA.h>
#include <omniORB4/sslContext.h>

#include "debug.hh"
#include "configuration.hh"

namespace po = boost::program_options;

SecurityManager::SecurityManager() {

  this->enabled = false;
  this->cAFile = "UNDEFINED";
  this->keyFile = "UNDEFINED";
  this->passwordKey = "";
}

bool
SecurityManager::initSSLContext() {
  /*
   *  Setting SSL Contex
   */
  CONFIG_BOOL(diet::SSLENABLED, this->enabled);

  if (this->enabled) {

    CONFIG_STRING(diet::SSLROOTCERTIFICATE, this->cAFile);
    CONFIG_STRING(diet::SSLPRIVATEKEY, this->keyFile);
    CONFIG_STRING(diet::SSLPRIVATEKEYPASSWORD, this->passwordKey);

    sslContext::certificate_authority_file = cAFile.c_str();
    sslContext::key_file = keyFile.c_str();
    sslContext::key_file_password = passwordKey.c_str();

    sslContext::verify_mode = SSL_VERIFY_FAIL_IF_NO_PEER_CERT | SSL_VERIFY_PEER;
  }
  else {
    sslContext::verify_mode = SSL_VERIFY_NONE;
  }
  return true;
}


SecurityManager::~SecurityManager() {
  if (this->enabled) {
    BOOST_FOREACH(char * opt, secuOptions) {
      delete[] opt;
    }
  }
}

char *
strToCharPtr(const std::string str) {
  char * result = new char[str.size()+1];
  strcpy(result, str.c_str());
  return result;
}

void
insertOptions(const std::string &option, const std::vector<std::string> &values, std::vector< char * > &toFill) {
  BOOST_FOREACH(std::string val, values) {
    toFill.push_back(strToCharPtr(option));
    toFill.push_back(strToCharPtr(val));
  }
}

void
insertOptions(const std::string &option, const std::set<std::string> &values, std::vector< char * > &toFill) {
  BOOST_FOREACH(std::string val, values) {
    toFill.push_back(strToCharPtr(option));
    toFill.push_back(strToCharPtr(val));
  }
}


bool
SecurityManager::secureORBOptions(int argc, char * argv[]) {


  if (!this-> enabled) {

    for (int i = 0; i < argc; ++i) {
      this->secuOptions.push_back(argv[i]);
    }
    return true;
  }


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
    BOOST_FOREACH(std::string uOpt, unmodifiedOptions) {
      secuOptions.push_back(strToCharPtr(uOpt));
      TRACE_TEXT(TRACE_MAIN_STEPS, "unmodified option : " << uOpt << std::endl);
    }

    po::store(opts, vm);
    po::notify(vm);

    // Securing end points
    std::set<std::string> endPointToSet;
    if (oEndPoint.empty()) {
      endPointToSet.insert("giop:tcp::");
      endPointToSet.insert("giop:ssl::");
      TRACE_TEXT(TRACE_MAIN_STEPS, "Changing end point configuration : allowing ssl and tcp endpoints." << std::endl);
    } else {
      BOOST_FOREACH(std::string endPoint, oEndPoint) {
        std::vector<std::string>::size_type idx = endPoint.find(":tcp:");
        if (idx != std::string::npos) {
          std::string secuEP = std::string(endPoint).replace(idx, 5, ":ssl:");
          endPointToSet.insert(secuEP);
        }
        endPointToSet.insert(endPoint);
      }
    }

    // Filling end point options
    insertOptions("-ORBendPoint", endPointToSet, secuOptions);


    // Securing server rules: only accept incoming ssl requests
    std::vector<std::string> serverRuleToSet;
    if (!oServerTransportRule.empty()) {
      BOOST_FOREACH(std::string serverTR, oServerTransportRule) {
        std::vector<std::string>::size_type idx = serverTR.find("tcp");
        if (idx == std::string::npos) {
          serverRuleToSet.push_back(serverTR);
        } else {
          WARNING("Ignoring server transport rule [" << serverTR
              << "] : tcp is not allowed !" << std::endl);
        }
      }
    }
    if (serverRuleToSet.empty()) {
      serverRuleToSet.push_back("localhost unix,tcp,ssl");
      serverRuleToSet.push_back("* ssl");
    }
    // Filling server rule options
    insertOptions("-ORBserverTransportRule", serverRuleToSet, secuOptions);


    // Securing client rules: prefer sending ssl requests
    std::vector<std::string> clientRuleToSet;
    if (!oClientTransportRule.empty()) {
      BOOST_FOREACH(std::string clientTR, oClientTransportRule) {
        std::vector<std::string>::size_type idxTCP = clientTR.find("tcp");
        std::string rule = clientTR;
        if (idxTCP != std::string::npos) {
          std::vector<std::string>::size_type idxSSL = clientTR.find("ssl");
          if (idxSSL == std::string::npos || idxSSL > idxTCP) {
            rule.replace(idxTCP, 3, "ssl");
            if (idxSSL > idxTCP) {
              rule.replace(idxSSL, 3, "tcp");
            }
            TRACE_TEXT(
                TRACE_ALL_STEPS,
                "Rewriting client transport rule [" << clientTR << "] into [" <<
                rule << "] to fit security needs." << std::endl);

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
