/****************************************************************************/
/* Type for SeD Service description                                         */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.3  2011/01/21 18:20:21  bdepardo
 * Set a few methods to const
 *
 * Revision 1.2  2011/01/21 18:17:11  bdepardo
 * Prefer prefix ++/-- operators for non-primitive types.
 *
 * Revision 1.1  2010/04/06 15:02:37  bdepardo
 * Added SeDWrapper example. This example is compiled when workflows are activated.
 *
 */

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include "boost/filesystem.hpp"
#include <map>

#include "DIET_data.h"
#include "DIET_Dagda.h"
#include "SeDService.hh"
#include "SeDDescrParser.hh"

#define TRACE(mess) {                           \
    cout << "[GASW] " << mess << endl;          \
  }
#define WARN(mess) {                            \
    cerr << "[GASW] " << mess << endl;          \
  }

namespace fs = boost::filesystem;

/*****************************************************************************/
/*                           CLASS SeDArgument                               */
/*****************************************************************************/

static const pair<string,short> argTypes[] = {
  pair<string,short>( "string", SeDArgument::STRING),
  pair<string,short>( "double", SeDArgument::DOUBLE),
  pair<string,short>( "integer", SeDArgument::INT),
  pair<string,short>( "boolean", SeDArgument::BOOLEAN),
  pair<string,short>( "file", SeDArgument::URI),
  pair<string,short>( "directory", SeDArgument::DIR),
};

static map<string,short> StrTypesToArgTypes(argTypes, argTypes
                                            + sizeof(argTypes)/sizeof(argTypes[0]));

SeDArgument::SeDArgument(SeDService* parent)
  : myParent(parent), myIo(NIO), myType(URI) {
}

SeDArgument::SeDArgument(const SeDArgument& src, SeDService* parent)
  : myParent(parent), myIo(src.myIo), myType(src.myType),
    myLabel(src.myLabel), myOption(src.myOption), myValue(src.myValue),
    myTemplate(src.myTemplate) {
}

const string&
SeDArgument::getValue() {
  if (!myTemplate.empty()) {
    getService()->getParser()->evalTemplate(this, myValue);
  }
  return myValue;
}

void
SeDArgument::setType(const string& strType) {
  if (!strType.empty())
    setType((Type_t) StrTypesToArgTypes[strType]);
}

void
SeDArgument::setTemplate(const string& templ) {
  myTemplate = templ;
  TRACE(" Argument '" << myLabel << "' template = " << myTemplate );
}

/*****************************************************************************/
/*                           CLASS SeDService                                */
/*****************************************************************************/

SeDService::SeDService(const string& name)
  : myName(name), myAvgCompTime(0), myTotalReqNb(0), myParser(NULL) {
  TRACE( "Created new service (name=" << myName << ")" );
}

SeDService::SeDService(SeDDescrParser *parser, const string& name)
  : myName(name), myAvgCompTime(0), myTotalReqNb(0), myParser(parser) {
  TRACE( "Created new service (name=" << myName << ")" );
}

SeDService::SeDService(const SeDService& src)
  : myName(src.myName), myExecName(src.myExecName),
    myAvgCompTime(src.myAvgCompTime),
    myDeps(src.myDeps), myParser(src.myParser)
{
  list<SeDArgument*>::const_iterator srcArgIter = src.myArgs.begin();
  while (srcArgIter != src.myArgs.end())
    myArgs.push_back(new SeDArgument(**(srcArgIter++), this));
  TRACE( "Created new service (name=" << myName << ")" );
}

SeDService::~SeDService() {
  // Free the args list
  while (! myArgs.empty() ) {
    SeDArgument * a = *(myArgs.begin());
    myArgs.erase( myArgs.begin() );
    delete a;
  }
}

SeDDescrParser *SeDService::getParser()
{
  if (myParser == NULL) {
    WARN( __FUNCTION__ << " : Parser not defined" );
    exit(1);
  }
  return myParser;
}

SeDArgument*
SeDService::addInput(const string& name,
                     const string& option,
                     const string& type)
{
  SeDArgument * arg = new SeDArgument(this);
  arg->setIo(SeDArgument::IN);
  arg->setLabel(name);
  arg->setOption(option);
  arg->setType(type);
  myArgs.push_back(arg);
  TRACE( "Created INPUT (name=" << name << "/ type=" << type << ")" );
  return arg;
}

SeDArgument*
SeDService::addOutput(const string& name,
                      const string& option,
                      const string& type)
{
  SeDArgument * arg = new SeDArgument(this);
  arg->setIo(SeDArgument::OUT);
  arg->setLabel(name);
  arg->setOption(option);
  arg->setType(type);
  myArgs.push_back(arg);
  TRACE( "Created OUTPUT (name=" << name << "/ type=" << type << ")" );
  return arg;
}

void
SeDService::addDependency(const string& name,
                          const string& localPath)
{
  myDeps.insert(make_pair<string,string>(name,localPath));
  TRACE( "Created DEPENDENCY (path=" << localPath << ")" );
}

unsigned int
SeDService::getArgumentNb() const {
  return myArgs.size();
}

unsigned int
SeDService::getInputNb() const {
  unsigned int count = 0;
  for (list<SeDArgument*>::const_iterator argIter = myArgs.begin();
       argIter != myArgs.end();
       ++argIter)
  {
    if ((*argIter)->getIo() == SeDArgument::IN)
      count++;
  }
  return count;
}
unsigned int
SeDService::getOutputNb() const {
  unsigned int count = 0;
  for (list<SeDArgument*>::const_iterator argIter = myArgs.begin();
       argIter != myArgs.end();
       ++argIter)
  {
    if ((*argIter)->getIo() == SeDArgument::OUT)
      count++;
  }
  return count;
}

SeDArgument *
SeDService::getArg(unsigned int idx) const {
  list<SeDArgument*>::const_iterator argIter = myArgs.begin();
  for (int ix = 0; ix < idx; ++ix) ++argIter;
  return *argIter;
}

diet_profile_desc_t *
SeDService::createAndDeclareProfile() {
  // Initialize profile
  diet_profile_desc_t *profile
    = diet_profile_desc_alloc(getName().c_str(),
                              getInputNb()-1,
                              getInputNb()-1,
                              getArgumentNb()-1);

  // Set arguments in the profile
  int argIndex = 0;
  for (list<SeDArgument*>::const_iterator argIter = myArgs.begin();
       argIter != myArgs.end();
       ++argIter)
  {
    SeDArgument *currArg = (SeDArgument*) *argIter;
    switch(currArg->getType()) {
    case SeDArgument::STRING :
      diet_generic_desc_set(diet_param_desc(profile,argIndex++), DIET_STRING, DIET_CHAR);
      break;
    case SeDArgument::URI :
      diet_generic_desc_set(diet_param_desc(profile,argIndex++), DIET_FILE, DIET_CHAR);
      break;
    case SeDArgument::DIR :
      diet_generic_desc_set(diet_param_desc(profile,argIndex++), DIET_CONTAINER, DIET_CHAR);
      break;
    case SeDArgument::INT :
      diet_generic_desc_set(diet_param_desc(profile,argIndex++), DIET_SCALAR, DIET_INT);
      break;
    case SeDArgument::DOUBLE :
      diet_generic_desc_set(diet_param_desc(profile,argIndex++), DIET_SCALAR, DIET_DOUBLE);
      break;
    case SeDArgument::BOOLEAN :
      diet_generic_desc_set(diet_param_desc(profile,argIndex++), DIET_SCALAR, DIET_CHAR);
      break;
    default:
      WARN( __FUNCTION__ << " : Invalid argument type" );
      exit(1);
    }
  }

  return profile;
}

/* MUST BE REENTRANT */
int
SeDService::createWorkingDirectory(const string& currentDirectory) {
  string  dirName = "w_" + getName() + "-" + getReqId();
  fs::path dir = fs::path(currentDirectory) / dirName;
  try
  {
    fs::create_directory( dir );
  }
  catch ( const fs::filesystem_error & x )
  {
    WARN( x.what() );
    WARN( "Creating directory " << dir.string() << " failed." );
    return 1;
  }
  catch ( ... )
  {
    WARN( "Creating directory " << dir.string() << " failed." );
    return 1;
  }
  myWorkingDir = dir.string();
  TRACE( "Creating directory " << dir.string() << " done." );
  return 0;
}

const string&
SeDService::getWorkingDirectory() const {
  return myWorkingDir;
}

int
SeDService::removeWorkingDirectory() {
  try {
    fs::remove_all(myWorkingDir);
  }
  catch ( const fs::filesystem_error & x )
  {
    WARN( x.what() );
    WARN( "Removing directory " << myWorkingDir << " failed." );
    return 1;
  }
  catch ( ... )
  {
    WARN( "Creating directory " << myWorkingDir << " failed." );
    return 1;
  }
  TRACE( "Remove directory " << myWorkingDir << " done." );
  return 0;
}

/* MUST BE REENTRANT */
int
SeDService::cpyDependencies() {
  for (map<string,string>::const_iterator depIter = myDeps.begin();
       depIter != myDeps.end();
       ++depIter)
  {
    fs::path  depPath(depIter->second);
    if (!fs::is_regular_file(depPath)) {
      WARN( "Invalid dependency: " << depPath.string() );
      return 1;
    }
    fs::path  destPath = fs::path(myWorkingDir) / depPath.leaf();
    try
    {
      fs::copy_file( depPath, destPath );
    }
    catch (const fs::filesystem_error & x )
    {
      WARN( x.what() );
      WARN( "Copy of dependency '" << depIter->first << "' from "
            << depPath.string() << " failed." );
      return 1;
    }
    catch ( ... )
    {
      WARN( "Copy of dependency '" << depIter->first << "' from "
            << depPath.string() << " failed." );
      return 1;
    }
    TRACE( "Copy of dependency '" << depIter->first << "' from "
           << depPath.string() << " done." );
  }
  return 0;
}

/* MUST BE REENTRANT */
int
SeDService::cpyFileToWorkingDir( SeDArgument *arg,
                                 const string& srcFile,
                                 const string& dstDir,
                                 string& dstFile )
{
  fs::path  srcPath(srcFile);
  if (!fs::is_regular_file(srcPath)) {
    WARN( "Invalid input file: " << srcPath.string() );
    return 1;
  }
  fs::path  destPath = fs::path(dstDir) / srcPath.leaf();
  try
  {
    fs::copy_file( srcPath, destPath );
  }
  catch (const fs::filesystem_error & x )
  {
    WARN( x.what() );
    WARN( "Copy of input '" << arg->getLabel() << "' from "
          << srcPath.string() << " failed." );
    return 1;
  }
  catch ( ... )
  {
    WARN( "Copy of input '" << arg->getLabel() << "' from "
          << srcPath.string() << " failed." );
    return 1;
  }
  TRACE( "Copy of input '" << arg->getLabel() << "' from "
         << srcPath.string() << " done." );
  dstFile = destPath.leaf();
  return 0;
}

/* MUST BE REENTRANT */
/** DIRECTORY as INPUT
 * Create a directory within the working directory
 * Store all container elements (files) in the directory
 * TODO create a data file containing a XML-encoded structure
 */
int
SeDService::cpyContainerToDir( SeDArgument *arg,
                               const string& containerID,
                               string& createdDir)
{
  // create directory within working directory
  fs::path  dirPath = fs::path(myWorkingDir) / arg->getLabel();
  createdDir = arg->getLabel();
  try
  {
    fs::create_directory( dirPath );
  }
  catch ( const fs::filesystem_error & x )
  {
    WARN( x.what() );
    WARN( "Creating directory " << dirPath.string() << " failed." );
    return 1;
  }
  catch ( ... )
  {
    WARN( "Creating directory " << dirPath.string() << " failed." );
    return 1;
  }

  // retrieve container content
  diet_container_t  content;
  int status = dagda_get_container(containerID.c_str());
  if (status != 0) {
    WARN( "Cannot download container: " << containerID );
    return 1;
  }
  status = dagda_get_container_elements(containerID.c_str(), &content);
  if (status != 0) {
    WARN( "Cannot get container elements list: " << containerID );
    return 1;
  }

  // download all elements from the dataMgr
  // & copy them into the created directory (skip empty container elts)
  for (int ix = 0; ix < content.size; ++ix) {
    char *currEltPathStr;
    if (content.elt_ids[ix]) {
      // download
      status = dagda_get_file(content.elt_ids[ix], &currEltPathStr);
      if (status != 0) {
        WARN( "Warning: Cannot get container element: " << content.elt_ids[ix] );
      } else {
        // let's copy it into the created directory
        string fileName;
        cpyFileToWorkingDir(arg, currEltPathStr, dirPath.string(), fileName);
      }
    }
  }
  return 0;
}

/* MUST BE REENTRANT */
/* IS RECURSIVE */
/** DIRECTORY as OUTPUT
 * Store all directory elements (files or dir) in the container
 * If element is a directory, creates a sub-container and fills it rec.
 * TODO create a data file containing a XML-encoded structure
 */
void
SeDService::cpyDirToContainer( SeDArgument *arg,
                               const string& containerID,
                               const string& dirPath )
{
  // get the list of files in the container
  TRACE( "Opening directory: " << dirPath );
  fs::path dirPathB(dirPath);
  fs::directory_iterator end_itr;
  unsigned int eltIdx = 0;
  for ( fs::directory_iterator itr(dirPathB);
        itr != end_itr;
        ++itr )
  {
    if ( fs::is_directory(itr->status()) )
    {
      TRACE( "Found directory: " << itr->path().leaf() );
      char *contID;
      dagda_create_container(&contID);
      dagda_add_container_element(containerID.c_str(), contID, eltIdx++);
      TRACE( " -> dir stored with id : " << contID );
      cpyDirToContainer(arg, string(contID), itr->path().string());
    }
    else if ( fs::is_regular_file(itr->status()) )
    {
      TRACE( "Found file: " << itr->path().string() );
      char *fileID;
      char *curFilePath = const_cast<char*>(itr->path().string().c_str());
      dagda_put_file(curFilePath, DIET_PERSISTENT, &fileID);
      dagda_add_container_element(containerID.c_str(), fileID, eltIdx++);
      TRACE( " -> file stored with id : " << fileID );
    }
  }

}

/* MUST BE REENTRANT */
void
SeDService::cpyProfileToArgs(diet_profile_t* pb)
{
  int argIndex = 0;
  for (list<SeDArgument*>::const_iterator argIter = myArgs.begin();
       argIter != myArgs.end();
       ++argIter)
  {
    SeDArgument *currArg = (SeDArgument*) *argIter;

    // IN ARGUMENT (get from DIET and copy value to argument)
    if (currArg->getIo() == SeDArgument::IN) {
      char *inputStr = NULL;
      double *inputDouble = NULL;
      int *inputInt = NULL;
      char *inputChar = NULL;
      ostringstream cvt;
      string path;
      size_t inputFileSize;
      switch(currArg->getType()) {
      case SeDArgument::STRING :
        diet_string_get(diet_parameter(pb, argIndex), &inputStr, NULL);
        cvt << inputStr;
        break;
      case SeDArgument::URI :
        diet_file_get(diet_parameter(pb, argIndex), NULL, &inputFileSize, &inputStr);
        if (cpyFileToWorkingDir(currArg, inputStr, myWorkingDir, path))
          continue;
        cvt << path;
        break;
      case SeDArgument::BOOLEAN :
        diet_scalar_get(diet_parameter(pb, argIndex), &inputChar, NULL);
        cvt << *inputChar;
        break;
      case SeDArgument::INT :
        diet_scalar_get(diet_parameter(pb, argIndex), &inputInt, NULL);
        cvt << *inputInt;
        break;
      case SeDArgument::DOUBLE :
        diet_scalar_get(diet_parameter(pb, argIndex), &inputDouble, NULL);
        cvt << *inputDouble;
        break;
      case SeDArgument::DIR :
        if (cpyContainerToDir(currArg, (pb->parameters[argIndex]).desc.id, path))
          continue;
        cvt << path;
        break;
      default:
        WARN( __FUNCTION__ << " : Invalid argument type" );
        exit(1);
      }
      currArg->setValue(cvt.str());
    }

    ++argIndex;
  }
}

/* MUST BE REENTRANT */
bool
SeDService::cpyArgsToProfile(diet_profile_t* pb)
{
  int argIndex = 0;
  bool success = true;
  for (list<SeDArgument*>::const_iterator argIter = myArgs.begin();
       argIter != myArgs.end();
       ++argIter)
  {
    SeDArgument *currArg = (SeDArgument*) *argIter;
    string argName = currArg->getLabel();

    if (currArg->getIo() == SeDArgument::OUT) {
      if (currArg->getType() == SeDArgument::URI) {
        fs::path  filePath(currArg->getValue());
        // check if relative path, and prefix with working directory path if yes
        if (!filePath.has_root_path()) {
          filePath = fs::path(myWorkingDir) / filePath;
        }
        TRACE( "Output '" << argName << "' (src file) = " << filePath.string() );

        if (!fs::exists(filePath)) {
          WARN( "ERROR: argument " << argName << " is not found" );
          success = false;
          continue;
        }
        if (!fs::is_regular(filePath)) {
          WARN( "ERROR: argument " << argName << " is not a file" );
          success = false;
          continue;
        }
        string filePathStr = filePath.string();
        char *outputStr = (char*) malloc(filePathStr.size()+1);
        strcpy(outputStr, filePathStr.c_str());
        diet_file_set(diet_parameter(pb, argIndex), DIET_PERSISTENT, outputStr);

      } else if (currArg->getType() == SeDArgument::DIR) {
        fs::path dirPath(currArg->getValue());
        // check if relative path, and prefix with working directory path if yes
        if (!dirPath.has_root_path()) {
          dirPath = fs::path(myWorkingDir) / dirPath;
        }
        TRACE( "Output '" << argName << "' (src dir) = " << dirPath.string() );
        if (!fs::exists(dirPath)) {
          WARN( "ERROR: argument " << argIndex << " is not found" );
          success = false;
          continue;
        }
        if (!fs::is_directory(dirPath)) {
          WARN( "ERROR: argument " << argName << " is not a directory" );
          success = false;
          continue;
        }
        dagda_init_container(diet_parameter(pb, argIndex));
        string  containerID((*diet_parameter(pb, argIndex)).desc.id);
        if (containerID.empty()) continue;
        cpyDirToContainer(currArg, containerID, dirPath.string());
        TRACE( "Stored output container ID: " << containerID );
      }
    }
    argIndex++;
  }
  return success;
}

void
SeDService::genCommandLine(string& cmdLine) {
  cmdLine = getExecName();
  for (list<SeDArgument*>::const_iterator argIter = myArgs.begin();
       argIter != myArgs.end();
       ++argIter)
  {
    cmdLine += " " + (*argIter)->getValue();
  }
}

void
SeDService::genUniqueReqId() {
  ostringstream id;
  struct timeval t;
  gettimeofday(&t, NULL);
  id << getpid() << "-" << random() << "-" << t.tv_sec;
  myReqId = id.str();
}

/* NOT REENTRANT */
void
SeDService::setComputationTimeMeasure(double time_ms) {
  if (myTotalReqNb = 0) {
    myAvgCompTime = time_ms;
  } else {
    myAvgCompTime = (time_ms + myTotalReqNb * myAvgCompTime) / (myTotalReqNb+1);
  }
  myTotalReqNb += 1;
}
