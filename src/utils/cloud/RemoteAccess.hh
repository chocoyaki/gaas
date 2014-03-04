#ifndef _REMOTE_ACCESS_HH_
#define _REMOTE_ACCESS_HH_

#include <string>

int test_ssh_connection(std::string ssh_user, std::string ip);

int rsync_to_vm(std::string local_path, std::string remote_path, std::string user, std::string ip);

int rsync_from_vm(std::string remote_path, std::string local_path, std::string user, std::string ip);

int execute_command_in_vm(const std::string& remote_cmd, std::string user, std::string ip, std::string args);

int create_directory_in_vm(const std::string& remote_path, std::string user, std::string ip, std::string args = " -p ");

#endif
