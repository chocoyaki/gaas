#include "RemoteAccess.hh"

#include <iostream>
#include <stdlib.h>
#include <string>

int test_ssh_connection(std::string ssh_user, std::string ip) {
  std::string cmd = "ssh -q "  + ssh_user + "@" + ip +
    " -o StrictHostKeyChecking=no PasswordAuthentication=no 'exit'";
  int ret = system(cmd.c_str());
  return ret;
}

int rsync_to_vm(std::string local_path, std::string remote_path, std::string user, std::string ip) {

  std::string cmd = "rsync -avz -e 'ssh -o StrictHostKeyChecking=no' " + local_path + " " + user + "@" + ip + ":" + remote_path;
  std::cout << cmd << std::endl;
  int ret = system(cmd.c_str());

  return ret;
}

int rsync_from_vm(std::string remote_path, std::string local_path, std::string user, std::string ip) {
  std::string cmd = "rsync -avz -e 'ssh -o StrictHostKeyChecking=no' " + user + "@" + ip +
    ":" + remote_path + " " + local_path;
  std::cout << cmd << std::endl;
  int ret = system(cmd.c_str());

  return ret;
}

int execute_command_in_vm(const std::string& remote_cmd, std::string vm_user, std::string ip, std::string args) {
  std::string cmd = "ssh "  + vm_user+ "@" + ip + " -o StrictHostKeyChecking=no '" + remote_cmd + " " + args + "'";
  std::cout << cmd << std::endl;
  int ret = system(cmd.c_str());

  return ret;
}

int create_directory_in_vm(const std::string& remote_path, std::string user, std::string ip, std::string args) {
  std::cout << ">>>>>>>>create directory in machine : " << ip << std::endl;
  std::string cmd = "ssh "  + user + "@" + ip + " -o StrictHostKeyChecking=no 'mkdir " + args + remote_path + "'";
  int ret = system(cmd.c_str());
  return ret;
}
