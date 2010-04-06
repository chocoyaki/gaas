/****************************************************************************/
/* Types for SeD Service description                                        */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2010/04/06 15:02:37  bdepardo
 * Added SeDWrapper example. This example is compiled when workflows are activated.
 *
 */

#ifndef _SEDSERVICE_HH_
#define _SEDSERVICE_HH_

#include <string>
#include <list>

#include "DIET_server.h"

using namespace std;
class SeDService;
class SeDDescrParser;

/*****************************************************************************/
/*                           CLASS SeDArgument                               */
/*****************************************************************************/

class SeDArgument {

  public:

    SeDArgument(SeDService* parent);
    SeDArgument(const SeDArgument& src, SeDService* parent);

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

//     bool isSpace();
//     bool isImplicit();

    SeDService *getService() { return myParent; }

    Io_t getIo() { return myIo; }
    Type_t getType() { return myType; }
    const string& getOption() { return myOption; }
    const string& getValue();
    const string& getTemplate() { return myTemplate; }
    const string& getLabel() { return myLabel; }

    void setIo(Io_t io) { myIo = io; }
    void setType(Type_t type) { myType = type; }
    void setType(const string& strType);
    void setLabel(const string& label) { myLabel = label; }
    void setOption(const string& option) { myOption = option; }
//     void setSpace(bool space);
//     void setImplicit(bool implicit);
    void setValue(const string& value) { myValue = value; }
    void setTemplate(const string& templ);

  private:

    SeDArgument(const SeDArgument& src) {}

    SeDService *myParent;
    Io_t    myIo;
    Type_t  myType;
    string  myLabel;
    string  myOption;
    string  myValue;
    string  myTemplate;
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
    SeDService(const string& name);
    SeDService(SeDDescrParser *parser, const string& name);
    SeDService(const SeDService& src);
    ~SeDService();

    SeDDescrParser * getParser();

    /* STRUCTURE */

    /**
     * Set the executable name for the service
     * This will be used 'as is' in the command line used in the system call
     * @param execName  the script name (absolute or relative path)
     */
    void setExecutableName(const string& execName) { myExecName = execName; }

    /**
     * Set the initial value of computation time
     * This will be used as the value for comp. time estimation until one
     * service request is completed. After that request the service will compute
     * the mean value of all computation time measures.
     * @param time  computation time of the service in ms
     */
    void setAvgComputationTime(double time) { myAvgCompTime = time; }

    // Properties getters
    const string& getName() { return myName; }
    const string& getExecName() { return myExecName; }
    double getAvgComputationTime() const { return myAvgCompTime; }

    // Elements builders (order of creation = order of the params)
    SeDArgument* addInput(const string& name,
                          const string& option,
                          const string& type);

    SeDArgument* addOutput(const string& name,
                           const string& option,
                           const string& type);
    void  addDependency(const string& name,
                        const string& localPath);

    // Elements access
    unsigned int getArgumentNb() const;  // total nb of args (in + out)
    unsigned int getInputNb() const;
    unsigned int getOutputNb() const;
    const list<SeDArgument*>&  getArgs() const { return myArgs; }
    SeDArgument *getArg(unsigned int idx) const; // idx starts at 0

    /* DECLARATION */

    diet_profile_desc_t * createAndDeclareProfile();

    /* EXECUTION */

    int createWorkingDirectory(const string& currentDirectory);
    const string& getWorkingDirectory();
    int removeWorkingDirectory();

    int cpyDependencies();

    void cpyProfileToArgs(diet_profile_t* pb);
    bool cpyArgsToProfile(diet_profile_t* pb);
    void genCommandLine(string& cmdLine);
    void genUniqueReqId();
    const string& getReqId() const { return myReqId; }

    /* PERFORMANCE */
    void  setComputationTimeMeasure(double time_ms);

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
    int cpyFileToWorkingDir( SeDArgument *arg,
                             const string& srcFile,
                             const string& dstDir,
                             string& dstFile );
    /**
     * Make a copy of files contained in a DAGDA container
     * Assumes the container is containing only files (depth=1)
     * (REENTRANT)
     * @param arg the current argument
     * @param containerID the container ID
     * @param createdDir returns the directory path containing the files
     */
    int cpyContainerToDir( SeDArgument *arg,
			   const string& containerID,
			   string& createdDir);

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
    void cpyDirToContainer( SeDArgument *arg,
                            const string& containerID,
                            const string& dirPath);

    /**
     * Service name (identifier)
     */
    string myName;

    /**
     * Executable name (command used to launch the service)
     */
    string myExecName;

    /**
     * Average computation time for the service
     * This is used to answer DIET requests for performance estimation
     */
    double  myAvgCompTime;

    /**
     * Number of service requests received
     * This is used to answer DIET requests for performance estimation
     */
    int myTotalReqNb;

    /**
     * List of all arguments (same order as command line)
     */
    list<SeDArgument*>  myArgs;

    /**
     * Container for all dependencies (no order)
     */
    map<string,string>  myDeps;

    /**
     * Parser used to process templates
     */
    SeDDescrParser * myParser;

    /**
     * Unique request ID used to generate directory or file names for the
     * current service request
     * Note: not copied in copy constructor
     */
    string myReqId;

    /**
     * Working directory path for the current service request
     * Note: not copied in copy constructor
     */
    string myWorkingDir;

};



#endif
