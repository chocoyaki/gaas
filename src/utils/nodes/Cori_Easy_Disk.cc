/****************************************************************************/
/* CoRI: Collectors for disk ressource                                      */
/*                                                                          */
/* Author(s):                                                               */
/*    - Frauenkron Peter (Peter.Frauenkron@ens-lyon.fr)                     */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2010/03/03 14:26:35  bdepardo
 * BEWARE!!!
 * Huge modifications to take into account CYGWIN.
 * Lots of files' directory have been changed.
 *
 * Revision 1.10  2008/11/08 19:12:38  bdepardo
 * A few warnings removal
 *
 * Revision 1.9  2006/11/16 09:55:55  eboix
 *   DIET_config.h is no longer used. --- Injay2461
 *
 * Revision 1.8  2006/10/31 23:01:54  ecaron
 * CoRI: Disk ressource collector
 *
 ****************************************************************************/
#if CORI_HAVE_SYS_TIME
#include <sys/time.h>
#endif //CORI_HAVE_SYS_TIME
#include <string.h>
#include <signal.h>
#ifdef HAVE_UNISTD_H 
#include <unistd.h>//alarm
#endif //HAVE_UNISTD_H
#include "Cori_Easy_Disk.hh"
#include <math.h> //rand
#include <stdlib.h>//popen - pclose
#include "debug.hh" 

using namespace std;

int sigalarm;

int 
Easy_Disk::get_Write_Speed(const char* path, 
			   double * result)
{
  if ((!get_Write_Speed_by_gettimeofday(path,result))
      ||(!get_Write_Speed_by_sig_alarm(path,result)))
    return 0;
  else {
    *result=0;
    return 1;
  }    
}

int 
Easy_Disk::get_Read_Speed(const char* path, 
			  double* result)
{

  if ((!get_Read_Speed_by_gettimeofday(path,result))
      ||(!get_Read_Speed_by_sig_alarm(path,result)))
    return 0;
  else {
    *result=0;
    return 1;
  }     
}

int 
Easy_Disk::get_Available_DiskSpace(const char* path, 
				   double * result){
  if (!gatherSizeDisks(0,result , path))
    return 0;
  else {
    *result=0;
    return 1;
  }
}

int 
Easy_Disk::get_Total_DiskSpace(const char* path, 
			       double * result){
if (!gatherSizeDisks(1,result , path))
    return 0;
  else {
    *result=0;
    return 1;
  }
}

/****************************************************************************/
/* Private methods                                                          */
/****************************************************************************/
void 
stop_count(int a){
  extern int sigalarm;
  sigalarm = 1;
}
void 
removePath_file(char** path){
  remove (*path);
  delete[]*path;
}

bool is_readable( const std::string & file ) 
{ 
    std::ifstream fichier( file.c_str() ); 
    return fichier != 0; 
} 

void
createPath(char **path_file,const char* path)
{
  char *namefile = new char[8];
  int nombre= (int)((double)rand() / ((double)RAND_MAX + 1) * 9999);

  sprintf(namefile, "%i", nombre);

  *path_file=new char[strlen(path)+strlen(namefile)];
  strcpy(*path_file,path);
  strcat(*path_file,namefile);

  while ( is_readable( *path_file ) ) 
  {
  nombre = (int)((double)rand() / ((double)RAND_MAX + 1) * 9999);
  sprintf(namefile, "%i",  nombre);
  strcpy(*path_file,path);
  strcat(*path_file,namefile);
  }
}

double  
Easy_Disk::search_for_percent(FILE * file,
			      int typeOfInfo){
  typedef char mystring_t[256];
  mystring_t last="";
  mystring_t beforelast="";
  mystring_t beforebeforelast="";  
  mystring_t bbblast="";
  double returnnumb=-1;

  while (!feof(file)){
     //looking for the last character of the word
    if (strstr(last,"%\0")!=NULL){       
      if (typeOfInfo==1)returnnumb=atof(bbblast);
      else if (typeOfInfo==0)returnnumb=atof(beforelast);    
      break;
    }
    strcpy(bbblast,beforebeforelast);
    strcpy(beforebeforelast,beforelast);
    strcpy(beforelast,last); 
    fscanf (file, "%s", last);
  }
  return returnnumb;
}

int 
Easy_Disk::create_file(char** path_file){
  int seconds=2*FILESIZE_MB;
  ofstream outfile;
  int sizeofchar=sizeof(char);

  int sizeTab=BUFFSIZE/sizeofchar;
  long rounds=FILESIZE/sizeTab;

  if (openfile(path_file,&outfile))
     return 1;

  sigalarm = 0; /* reset alarm flag */

  char *buffer = new char [sizeTab];
   for (long j=0; j<sizeTab;j++)
     buffer[j]=rand()%26+'a';
#if defined HAVE_ALARM && defined HAVE_SIGNAL
  alarm(seconds);
  signal(SIGALRM,stop_count);
#else
#ifdef HAVE_GETTIMEOFDAY
  timeval tim;
  gettimeofday(&tim, NULL);
  double t1=tim.tv_sec+(tim.tv_usec/1000000.0);
  double t2=0;
  double elapsed_time=0;
#endif // HAVE_GETTIMEOFDAY
#endif // HAVE_ALARM
  int i=0;
 
  while((!sigalarm)&&(i<rounds)) {
    outfile.write(buffer, BUFFSIZE);
    outfile.flush();
    i++;
#if !defined HAVE_ALARM || !defined HAVE_SIGNAL
#ifdef HAVE_GETTIMEOFDAY
    gettimeofday (&tim, NULL);
    t2=tim.tv_sec+(tim.tv_usec/1000000.0);	
    elapsed_time= t2-t1;
    if (elapsed_time>seconds)
      sigalarm=1;
#endif // HAVE_GETTIMEOFDAY
#endif // HAVE_ALARM HAVE_SIGNAL
  }
 outfile.close();
 delete []buffer;
 if (sigalarm) {
   TRACE_TEXT(TRACE_MAX_VALUE,
	       "creating file for Disk perf  took to long");   
   return 1;
 }
 else return (0);
}

int 
Easy_Disk::openfile(char ** path_file,ofstream* outfile){
 
  (*outfile).open (*path_file, ofstream::out | ofstream::trunc);
  if ((*outfile).is_open())   
    return 0;
  else {
    TRACE_TEXT(TRACE_MAX_VALUE,
	       "impossible to open in the directory:"<<*path_file<<" for disk perf");   
    return 1;
  }
}

/**
 * returns 0 if no problem, 
 * returns 1 if problem
 * arrayDisk = array of integer, 
 * pre: path must be accessible- it must be possible to create a file in this path
 * post: arrayDisk[i] is the size of partition i in GBytes if static==1
 *                       the size of available place in partition if static==0
 */

int 
Easy_Disk::gatherSizeDisks(int typeOfInfo,double *result,
			   const char* path){ 
  // information is collected from POSIX standard 'df $path -k'
  typedef char mystring_t[256];
  int retval=0;
  char *str; 
  str=new char[(strlen(path)*2)+20];
  strcpy (str,"df ");
  strcat(str,path);
  strcat(str, " -k");
  FILE * myfile =popen(str,"r");

  mystring_t word;
  double number=0;
  
  if (myfile!=NULL){
      /* read the first line*/
      if (!feof(myfile)){
	fgets(word, 256, myfile);  
	if (!feof(myfile)){
	    //look for the '%' and save the isstatic th element before this    
	    number= search_for_percent(myfile,typeOfInfo); 
	    if (number!=-1){ //found the percent
	      *result=number/1024;
	      number=-1;		      
	      //   if (file!=NULL)
	    }
	}
      }
      pclose(myfile);
  }
  if (number==0){    
    TRACE_TEXT(TRACE_MAX_VALUE,
	       "Error in file structure for partition info!");
    retval=1;
  }
delete[]str;
return retval;
}

int 
Easy_Disk::get_Write_Speed_by_gettimeofday(const char* path, 
					   double * result){
#ifndef HAVE_GETTIMEOFDAY
  return 1;
#endif

  char* path_file;
  createPath(&path_file,path);
  ofstream outfile;
  outfile.exceptions ( ofstream::eofbit | ofstream::failbit | ofstream::badbit );
  if (openfile(&path_file,&outfile)){ 
    return 1;
  }
  int seconds =FILESIZE_MB*2;
  double writespeed=0;
  int sizeofchar=sizeof(char);
  int sizeTab=BUFFSIZE/sizeofchar;
  long  rounds=FILESIZE/sizeTab;

  char *buffer = new char [sizeTab];
   for (long j=0; j<sizeTab;j++)
     buffer[j]='o';
  int i=0;

  timeval tim;
  gettimeofday(&tim, NULL);
  double t1=tim.tv_sec+(tim.tv_usec/1000000.0);

 /*start clock*/ 
  gettimeofday(&tim, NULL);
  double t2=0;
  double elapsed_time=0;
  
  while((elapsed_time<seconds)
	&&(i<rounds)) {
       try{  
	 if (!outfile.is_open()){
	    TRACE_TEXT(TRACE_MAX_VALUE,
		       "error: can not create a test file for writing");
	    removePath_file(&path_file);
	    *result=0;
	    return 1;
	  }
	 outfile.write(buffer,sizeTab);
	 outfile.flush();
	 i++;
       }
       catch (ofstream::failure e) {
	 removePath_file(&path_file);
	 *result=0;
	 return 1;
    }
    gettimeofday (&tim, NULL);
    t2=tim.tv_sec+(tim.tv_usec/1000000.0);	
    elapsed_time= t2-t1;
   }
// To avoid std::ios_base::failure from libstdc++ after a remove function close seems not required.   
// outfile.close();  
 double Mpersec;
 if(elapsed_time==0)
   return 1;
 Mpersec=sizeTab*sizeofchar*i/(elapsed_time*MEGABYTE);
 writespeed=Mpersec/COUNTPERBUFFER;
 removePath_file(&path_file); 
 *result=writespeed;
 return 0;
}

int 
Easy_Disk::get_Write_Speed_by_sig_alarm(const char* path, 
					double * result){
#if !(defined HAVE_SIGNAL) && !(defined HAVE_ALARM)
   //no possibility to take the time->exit
  *result=0;
  return 1;
#endif

  *result=0;
  char* path_file;
  createPath(&path_file,path);
  ofstream outfile;
  outfile.exceptions ( ofstream::eofbit | ofstream::failbit | ofstream::badbit );
  if (openfile(&path_file,&outfile)){ 
    return 1;
  }
  int seconds =FILESIZE_MB*2;
  
  double writespeed=0;
  int sizeofchar=sizeof(char);
  int sizeTab=BUFFSIZE/sizeofchar;
  long  rounds=FILESIZE/sizeTab;

  signal(SIGALRM,stop_count);
  sigalarm = 0; /* reset alarm flag */

  char *buffer = new char [sizeTab];
   for (long j=0; j<sizeTab;j++)
     buffer[j]='o';
  int i=0;
  alarm(seconds);

#ifdef HAVE_GETTIMEOFDAY
  timeval tim;
  gettimeofday(&tim, NULL);
  double t1=tim.tv_sec+(tim.tv_usec/1000000.0); 
#endif //HAVE_GETTIMEOFDAY

 /*start clock*/  

 while((!sigalarm)
	&&(i<rounds)) {
       try{  
	 if (!outfile.is_open()){
	    TRACE_TEXT(TRACE_MAX_VALUE,
		       "Can not create a test file for partition perf test");
	    removePath_file(&path_file);
	    *result=0;
	    return 1;
	  }
	 outfile.write(buffer,sizeTab);
	 outfile.flush();
	 i++;
       }
       catch (ofstream::failure e) {
	 removePath_file(&path_file);
	 *result=0;
	 return 1;
    }
   }
#ifdef HAVE_GETTIMEOFDAY
 double t2=tim.tv_sec+(tim.tv_usec/1000000.0);	
 double elapsed_time= t2-t1;
#endif //HAVE_GETTIMEOFDAY
 outfile.close();
  
/* stop clock */
 int alarm=sigalarm;

 double Mpersec;
 if (!alarm)
     Mpersec=FILESIZE/(seconds*MEGABYTE); 
 else{
#if HAVE_GETTIMEOFDAY
    gettimeofday(&tim, NULL);
    t2=tim.tv_sec+(tim.tv_usec/1000000.0);	
    elapsed_time= t2-t1;
    Mpersec=sizeTab*sizeofchar*i/(elapsed_time*MEGABYTE);
#else
    Mpersec=sizeTab*sizeofchar*i/(seconds*MEGABYTE);
#endif // HAVE_GETTIMEOFDAY
}  
  writespeed=Mpersec/COUNTPERBUFFER;
  removePath_file(&path_file); 
  *result=writespeed;
  return 0;
}

int 
Easy_Disk::get_Read_Speed_by_gettimeofday(const char* path, 
					   double * result){
#ifndef HAVE_GETTIMEOFDAY
  return 1;
#endif
  char* path_file;
  createPath(&path_file,path);
  *result=0;
  int seconds=3;
  int sizeofchar=sizeof(char);
  int sizeTab=BUFFSIZE/sizeofchar;
  
  if (create_file(&path_file)){
    *result=0;
   return 1;
  }

  ifstream infile;
  infile.exceptions ( ifstream::eofbit | ifstream::failbit | ifstream::badbit );
  double readspeed=0;

  char *buffer = new char [sizeTab];
  double j=0;
  double i=0;

  timeval tim;
  gettimeofday(&tim, NULL);
  double t1=tim.tv_sec+(tim.tv_usec/1000000.0);
  /*start clock*/
  infile.open (path_file, ifstream::in);
  if ((!infile.is_open())||(infile.eof())){
    removePath_file(&path_file);
    delete [] buffer;
    *result=0;
    return 1;
  }

  gettimeofday(&tim, NULL);
  double elapsed_time= 0;
  double t2=0;
  while((elapsed_time<seconds)
	&&(infile.is_open())) {
    if (infile.eof()){  
       infile.close();
       infile.open (path_file,ifstream::in);
       j++;
      i=0;	       
    }
    try{      
      infile.read(buffer,sizeTab);      
      i++;
    }
    catch (ifstream::failure e) {//not so dangerous -> shi* happens
    }
    gettimeofday(&tim, NULL);
    t2=tim.tv_sec+(tim.tv_usec/1000000.0);	
    elapsed_time= t2-t1;
  }
  infile.close();
  /*STOP clock*/
  double Mbytepersecond;
  if (elapsed_time==0)
    return 1;
  Mbytepersecond=j*FILESIZE_MB/elapsed_time;
  readspeed=Mbytepersecond/COUNTPERBUFFER;
  
  delete []buffer;
  removePath_file(&path_file);
  *result=readspeed;
  return 0;
}

int 
Easy_Disk::get_Read_Speed_by_sig_alarm(const char* path, 
					   double * result)
{
#if !(defined HAVE_SIGNAL) || !(defined HAVE_ALARM)
  return 1;
#endif

  char* path_file;
  createPath(&path_file,path);
  *result=0;

  int seconds=3;
  int sizeofchar=sizeof(char);
  int sizeTab=BUFFSIZE/sizeofchar;
  
  if (create_file(&path_file)){
    *result=0;
   return 1;
  }
  ifstream infile;
  infile.exceptions ( ifstream::eofbit | ifstream::failbit | ifstream::badbit );
  double readspeed=0;
  signal(SIGALRM,stop_count);
  sigalarm = 0; /* reset alarm flag */
  char *buffer = new char [sizeTab];
  double j=0;
  double i=0;
  alarm(seconds);
  /*start clock*/
  infile.open (path_file, ifstream::in);
  if ((!infile.is_open())||(infile.eof())){
    removePath_file(&path_file);
    delete [] buffer;
    *result=0;
    return 1;
  }
  while((!sigalarm)
	&&(infile.is_open())) {
    if (infile.eof()){  
       infile.close();
       infile.open (path_file,ifstream::in);
       j++;
      i=0;	       
    }
    try{      
      infile.read(buffer,sizeTab);      
      i++;
    }
    catch (ifstream::failure e) {//not so dangerous -> shi* happens
    }
  }
  infile.close();
  /*STOP clock*/
  double Mbytepersecond;
  Mbytepersecond=j*FILESIZE_MB/seconds; 
  readspeed=Mbytepersecond/COUNTPERBUFFER;
  
  delete []buffer;
  removePath_file(&path_file);
  *result=readspeed;
  return 0;
}
