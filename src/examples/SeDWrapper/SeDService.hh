/**
 * @file SeDService.hh
 *
 * @brief   Type for SeD Service description
 *
 * @author  Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */

#ifndef _SEDSERVICE_HH_
#define _SEDSERVICE_HH_

#include <list>
#include <map>
#include <string>

#include "DIET_server.h"

class SeDService;
class SeDDescrParser;

/*****************************************************************************/
/*                           CLASS SeDArgument                               */
/*****************************************************************************/

class SeDArgument {
public:
  explicit
  SeDArgument(SeDService *parent);

  SeDArgument(const SeDArgument &src, SeDService * parent);

  typedef enum {
    IN,
    OUT,
    NIO
  } Io_t;

  typedef enum {
    STRING,
    INT,
    DOUBLE,
    BOOLEAN,
    URI,
    DIR
  } Type_t;

  SeDService *
  getService() {
    return myParent;
  }

  Io_t
  getIo() const {
    return myIo;
  }

  Type_t
  getType() const {
    return myType;
  }

  const std::string &
  getOption() const {
    return myOption;
  }

  const std::string &
  getValue();

  const std::string &
  getTemplate() const {
    return myTemplate;
  }

  const std::string &
  getLabel() const {
    return myLabel;
  }

  void
  setIo(Io_t io) {
    myIo = io;
  }

  void
  setType(Type_t type) {
    myType = type;
  }

  void
  setType(const std::string &strType);

  void
  setLabel(const std::string &label) {
    myLabel = label;
  }

  void
  setOption(const std::string &option) {
    myOption = option;
  }

  void
  setValue(const std::string &value) {
    myValue = value;
  }

  void
  setTemplate(const std::string &templ);

private:
  SeDArgument(const SeDArgument &src) {
  }

  SeDService *myParent;
  Io_t myIo;
  Type_t myType;
  std::string myLabel;
  std::string myOption;
  std::string myValue;
  std::string myTemplate;
};

/*****************************************************************************/
/*                           CLASS SeDService                                */
/*****************************************************************************/

class SeDService {
public:
  /**
   * Constructor
   * @param name  the identifier of the service
   */
  explicit
  SeDService(const std::string &name);

  SeDService(SeDDescrParser * parser, const std::string & name);

  SeDService(const SeDService &src);

  ~SeDService();

  SeDDescrParser *
  getParser();

  /* STRUCTURE */

  /**
   * Set the executable name for the service
   * This will be used 'as is' in the command line used in the system call
   * @param execName  the script name (absolute or relative path)
   */
  void
  setExecutableName(const std::string &execName) {
    myExecName = execName;
  }

  /**
   * Set the initial value of computation time
   * This will be used as the value for comp. time estimation until one
   * service request is completed. After that request the service will compute
   * the mean value of all computation time measures.
   * @param time  computation time of the service in ms
   */
  void
  setAvgComputationTime(double time) {
    myAvgCompTime = time;
  }

  // Properties getters
  const std::string &
  getName() const {
    return myName;
  }

  const std::string &
  getExecName() const {
    return myExecName;
  }

  double
  getAvgComputationTime() const {
    return myAvgCompTime;
  }

  // Elements builders (order of creation = order of the params)
  SeDArgument *
  addInput(const std::string &name,
           const std::string &option,
           const std::string &type);

  SeDArgument *
  addOutput(const std::string &name,
            const std::string &option,
            const std::string &type);

  void
  addDependency(const std::string &name,
                const std::string &localPath);

  // Elements access
  unsigned int
  getArgumentNb() const;  // total nb of args (in + out)

  unsigned int
  getInputNb() const;

  unsigned int
  getOutputNb() const;

  const std::list<SeDArgument *> &
  getArgs() const {
    return myArgs;
  }

  SeDArgument
  *
  getArg(unsigned int idx) const;   // idx starts at 0

  /* DECLARATION */

  diet_profile_desc_t *
  createAndDeclareProfile();

  /* EXECUTION */

  int
  createWorkingDirectory(const std::string &currentDirectory);

  const std::string &
  getWorkingDirectory() const;

  int
  removeWorkingDirectory();

  int
  cpyDependencies();

  void
  cpyProfileToArgs(diet_profile_t *pb);

  bool
  cpyArgsToProfile(diet_profile_t *pb);

  void
  genCommandLine(std::string &cmdLine);

  void
  genUniqueReqId();

  const std::string &
  getReqId() const {
    return myReqId;
  }

  /* PERFORMANCE */
  void
  setComputationTimeMeasure(double time_ms);

protected:
  /**
   * Make a copy of local file to a working directory
   * Returns only the filename (without path)
   * @param arg the current argument
   * @param srcFile the current path of the file
   * @param dstDir  the destination directory
   * @param dstFile returns the filename
   * @return 0 on success, 1 on failure
   */
  int
  cpyFileToWorkingDir(SeDArgument *arg,
                      const std::string &srcFile,
                      const std::string &dstDir,
                      std::string &dstFile);
  /**
   * Make a copy of files contained in a DAGDA container
   * Assumes the container is containing only files (depth = 1)
   * (REENTRANT)
   * @param arg the current argument
   * @param containerID the container ID
   * @param createdDir returns the directory path containing the files
   */
  int
  cpyContainerToDir(SeDArgument *arg,
                    const std::string &containerID,
                    std::string &createdDir);

  /**
   * Make a copy of files into a DAGDA container
   * Assumes the container is already created and dir is correct
   * Can handle non-homogeneous container (containing files and dirs)
   * Goes recursively into directories (creates new containers)
   * (REENTRANT)
   * @param arg the current argument
   * @param containerID the container ID
   * @param dirPath the directory path
   */
  void
  cpyDirToContainer(SeDArgument *arg,
                    const std::string &containerID,
                    const std::string &dirPath);

  /**
   * Service name (identifier)
   */
  std::string myName;

  /**
   * Executable name (command used to launch the service)
   */
  std::string myExecName;

  /**
   * Average computation time for the service
   * This is used to answer DIET requests for performance estimation
   */
  double myAvgCompTime;

  /**
   * Number of service requests received
   * This is used to answer DIET requests for performance estimation
   */
  int myTotalReqNb;

  /**
   * List of all arguments (same order as command line)
   */
  std::list<SeDArgument *>  myArgs;

  /**
   * Container for all dependencies (no order)
   */
  std::map<std::string, std::string>  myDeps;

  /**
   * Parser used to process templates
   */
  SeDDescrParser *myParser;

  /**
   * Unique request ID used to generate directory or file names for the
   * current service request
   * Note: not copied in copy constructor
   */
  std::string myReqId;

  /**
   * Working directory path for the current service request
   * Note: not copied in copy constructor
   */
  std::string myWorkingDir;
};

#endif /* ifndef _SEDSERVICE_HH_ */
