#include "DIET_config.h"
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
using namespace std;

int sigalarm;

void 
stop_count(int a){

	extern int sigalarm;
	sigalarm = 1;
}
void 
removePath_file(char* path){
  remove (path);
  delete []path;
}
void
createPath(char **path_file,const char* path)
{
  
#if HAVE_GETPID
  char * pid;
  sprintf(pid, "%i",  getpid());
#else //HAVE_GETPID
  *path_file=new char[strlen(path)+9];
  strcpy(*path_file,path);
  strcat(*path_file,"testfile");
  int i=1;
  while (fopen(*path_file,"r")!=NULL){
    *path_file=new char[strlen(path)+9+i];;
    i++;
    strcat(*path_file,"a");
  } 
#endif //HAVE_GETPID
}

int 
Easy_Disk::get_Write_Speed(const char* path, 
			   double * result)
{
  if (get_Write_Speed_by_gettimeofday(path,result)
      ||get_Write_Speed_by_sig_alarm(path,result))
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
  if (get_Read_Speed_by_gettimeofday(path,result)
      ||get_Read_Speed_by_sig_alarm(path,result))
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

double  
Easy_Disk::search_for_percent(ifstream &file,
			      int typeOfInfo){
typedef char mystring_t[256];
  mystring_t last="";
  mystring_t beforelast="";
  mystring_t beforebeforelast="";  
  mystring_t bbblast="";
  double returnnumb=-1;
  if(!file.eof())
    file >>last; 
  while (!file.eof()){
     //looking for the last character of the word
    if (strstr(last,"%\0")!=NULL){       
      if (typeOfInfo==1)returnnumb=atof(bbblast);
      else if (typeOfInfo==0)returnnumb=atof(beforelast);    
      break;
    }
    strcpy(bbblast,beforebeforelast);
    strcpy(beforebeforelast,beforelast);
    strcpy(beforelast,last); 
    file >>last;
  }
 //  delete(last);
//   delete(beforelast);
//   delete(beforebeforelast);
//   delete(bbblast);
  return returnnumb;
}

int 
Easy_Disk::create_file(const char* path_file){
  //cout<<"creating file..."<<endl;
  int seconds=2*FILESIZE_MB;
  ofstream outfile;
 int sizeofchar=sizeof(char);

  int sizeTab=BUFFSIZE/sizeofchar;
   long rounds=FILESIZE/sizeTab;

   if (!is_Accessible(path_file))
     return 1;
  outfile.open (path_file, ofstream::out | ofstream::trunc);

  if (!outfile.is_open()){
    cerr<<"can not create test file :" << path_file;
    return 1;
  }
  signal(SIGALRM,stop_count);
  sigalarm = 0; /* reset alarm flag */

  char *buffer = new char [sizeTab];
   for (long j=0; j<sizeTab;j++)
     buffer[j]='o';

  alarm(seconds);
 
  int i=0;
 
  while((!sigalarm)&&(i<rounds)) {
    outfile.write(buffer, BUFFSIZE);
    outfile.flush();
    i++;
  }
 outfile.close();

  if (sigalarm) {
    cerr<<"error creating file for reading: took to long"<<endl;
    delete []buffer;
    return 1;
  }

 delete[]buffer;
  // cout<<"file created!"<<endl;
  return (0);
}

int 
Easy_Disk::is_Accessible(const char * path_file){
   FILE * file;
  file=fopen (path_file,"w+");
  int result=0;
  if (file!=NULL){
    fclose (file);
    remove (path_file);   
    result= 1;
  }
  else {
    cerr<<"impossible to write in the directory:"<<path_file<<endl; 
    result= 0;
  }
   return result;
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
  strcat(str, " -k >> ");
  char *path_file; 
  path_file=new char[strlen(path)+10];
  strcpy(path_file, path);
  strcat(path_file, "file.temp");
  strcat(str,path_file);
  if (!is_Accessible(path_file))
    retval=1;

  mystring_t word;
  double number=-1;

  // 	/*creation of the file */
  
  if (system(str)==-1)
    retval=1;
  else{
	/* looking in the file.temp data file*/
    ifstream file(path_file);//,ios_base::trunc);       
    if (file.is_open())
    {     
      /* read the first line*/
      if ( ! file.eof() )
	{
	file.getline(word,256);  
	if ( !file.eof() )
	  {
	    //look for the '%' and save the isstatic th element before this 		       
	    number= search_for_percent(file,typeOfInfo); 
	    if (number!=-1){ //found the percent
	      *result=number/1024;
	      number=-1;		      
	      //   if (file!=NULL)
	      file.close();
	    }
	    else {
	      //cerr<<"Error in file structure!"<<endl;
	      retval=1;
	    }     
	  }
	else 
	   retval=1; // file error
	} 
      else
	{//cerr<<"error in file structure!"<<endl;
	  retval=1;
	}
    }
    else{
      //cerr<<"Error opening file";
      retval=1;
    }
  }
 delete[]str;
 remove(path_file);
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

  if (!is_Accessible(path_file)){ 
    delete []path_file;
    *result=0;
    return 1;
  }
  int seconds =FILESIZE_MB*2;
  ofstream outfile;
  outfile.exceptions ( ofstream::eofbit | ofstream::failbit | ofstream::badbit );
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
  outfile.open (path_file, ofstream::out | ofstream::trunc);

  gettimeofday(&tim, NULL);
  double t2=0;
  double elapsed_time=0;

  while((elapsed_time<seconds)
	&&(i<rounds)) {
       try{  
	 if (!outfile.is_open()){
	    cerr<<"error: can not create a test file for writing"<<endl;
	    removePath_file(path_file);
	    *result=0;
	    return 1;
	  }
	 outfile.write(buffer,sizeTab);
	 outfile.flush();
	 i++;
       }
       catch (ofstream::failure e) {
	 cout << "Exception writing file:"<<i<<endl;
	 removePath_file(path_file);
	 *result=0;
	 return 1;
    }
    gettimeofday (&tim, NULL);
    t2=tim.tv_sec+(tim.tv_usec/1000000.0);	
    elapsed_time= t2-t1;
   }
 outfile.close();
  
 double Mpersec;
 if(elapsed_time==0)
   return 1;
 Mpersec=sizeTab*sizeofchar*i/(elapsed_time*MEGABYTE);
 writespeed=Mpersec/COUNTPERBUFFER;
 removePath_file(path_file); 
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

  if (!is_Accessible(path_file)){ 
    delete []path_file;
    *result=0;
    return 1;
  }
  int seconds =FILESIZE_MB*2;
  ofstream outfile;
  outfile.exceptions ( ofstream::eofbit | ofstream::failbit | ofstream::badbit );
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
  outfile.open (path_file, ofstream::out | ofstream::trunc);

 while((!sigalarm)
	&&(i<rounds)) {
       try{  
	 if (!outfile.is_open()){
	    cerr<<"error: can not create a test file for writing"<<endl;
	    removePath_file(path_file);
	    *result=0;
	    return 1;
	  }
	 outfile.write(buffer,sizeTab);
	 outfile.flush();
	 i++;
       }
       catch (ofstream::failure e) {
	 cout << "Exception writing file:"<<i<<endl;
	 removePath_file(path_file);
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
  removePath_file(path_file); 
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
  path_file=new char[strlen(path)+6];
  *result=0;
  strcpy (path_file,path);
  strcat(path_file,"dummy");

  int seconds=3;
  int sizeofchar=sizeof(char);
  int sizeTab=BUFFSIZE/sizeofchar;
  
  if (create_file(path_file)){
    removePath_file(path_file);
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
    removePath_file(path_file);
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
  removePath_file(path_file);
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
  path_file=new char[strlen(path)+6];
  *result=0;
  strcpy (path_file,path);
  strcat(path_file,"dummy");

  int seconds=3;
  int sizeofchar=sizeof(char);
  int sizeTab=BUFFSIZE/sizeofchar;
  
  if (create_file(path_file)){
    removePath_file(path_file);
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
    removePath_file(path_file);
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
  removePath_file(path_file);
  *result=readspeed;
  return 0;
}

// /***************MAIN******************************/
// int main (int arc, char ** argv)
// {
//   Easy_Disk disk;
 
//   char * path  =argv[1];//"/home/peter/cori/start/dummy";


//   double result;
  
//   if (!disk.get_Write_Speed(path,&result)){
//   cout<<"write speed=" <<result<<" Mbyte/sec"<<endl;
//   }
//   else cerr<< " no info"<<endl;
//   if (!disk.get_Read_Speed(path, &result))
//   cout<<"read speed="<<result<<" Mbyte/sec"<<endl;
//   else cerr<< " no info"<<endl;
//   if (!disk.get_Total_DiskSpace(path,&result))
//   cout<<"total space=" <<result<<" Mbyte"<<endl;
//   else cerr<< " no info"<<endl;
  
//   if (!disk.get_Available_DiskSpace(path,&result))
//   cout<<"available space=" << result<< " Mbyte"<<endl;
// else cerr<< " no info"<<endl;
//   delete[]path;
// }
