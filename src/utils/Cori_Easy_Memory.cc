#include <DIET_config.h>
#include <unistd.h> //int getpagesize (void)
#ifdef HAVE_SYS_SYSINFO
#include <sys/sysinfo.h> // get_phys_pages (),get_avphys_pages ()
#endif
#include <stdio.h>
#include "Cori_Easy_Memory.hh"
#include <cstdlib> //system()

#include <iostream>
#include <fstream>	
#include <string>
//#include <string.h>//srtcmp
#include <string>
using namespace std;

int 
Easy_Memory::get_Total_Memory(double * result)
{
  double temp1,temp2,temp3;
  if (!get_Info_Memory_byProcMem(&temp1,0)){
    *result=temp1;
    return 0;
  }
  else 
  if (!get_Total_Memory_bysysinfo(&temp1)
      &&(!get_Avail_Memory_byvmstat(&temp2))
      &&(!get_Avail_Memory_bysysinfo(&temp3)))
  {
    cerr<<temp1<<temp2<<temp3;
    double pagesize= temp2/temp3;
    *result=temp1*pagesize;
    return 0;
  }
  else{
      *result=0;
      return 1;
    }
}
int 
Easy_Memory::get_Avail_Memory(double * result)
{
  double temp;
   if (!get_Info_Memory_byProcMem(&temp,1)){
    *result=temp;
    return 0;
  }
  else 
  if (!get_Avail_Memory_byvmstat(&temp)){
    *result=temp;
    return 0;
  }
  else{
      *result=0;
      return 1;
    }
}

/****************************************************************************/
/* Private methods                                                          */
/****************************************************************************/

int 
Easy_Memory::get_Total_Memory_bysysinfo(double * result)
{
  // using  get_phys_pages ()
  // returns the total number of physical pages the system has
  // and
  // int getpagesize (void)
  // Inquire about the virtual memory page size of the machine.
#if defined HAVE_GET_PHYS_PAGES && defined (HAVE_GETPAGESIZE)
  *result=get_phys_pages ()*getpagesize()/(1024*1024);
  return 0;
#else
    //chercher autre solutions
    return 1;
#endif
}

int 
Easy_Memory::get_Avail_Memory_bysysinfo(double * result)
{
  // using get_avphys_pages ()
  // returns the number of physical available pages the system has
  // and
  // int getpagesize (void)
  // Inquire about the virtual memory page size of the machine.
#if defined HAVE_GET_AVPHYS_PAGES && defined (HAVE_GETPAGESIZE)
  *result=get_avphys_pages ()*getpagesize()/(1024*1024);
  return  0;
#else
  //chercher autres sol
  return 1;
#endif
}

int 
Easy_Memory::get_Avail_Memory_byvmstat(double * result)
{
  remove ("memtmp.tmp");
  char *command ="vmstat >> memtmp.tmp";
  int returnval=1;
  if (system(command)!=-1){
    fstream myfile ("./memtmp.tmp"); 
    char word[256];    
    if ((myfile!=NULL)&&(myfile.is_open())){
   /*take the 4th word in line 3*/
      if (!myfile.eof()){
	  myfile.getline (word,256);
	  if (!myfile.eof()){
	    myfile.getline (word,256);
	    int i=0;
	    while ((!myfile.eof())&&(i<4)){
	      myfile>>word;  
	      i++;  
	    }    
	    if (i==4){
	      *result = atof ( word );  
	      returnval=0;
	    }     
	    myfile.close();
	  }
      }
    }
  }
  remove ("memtmp.tmp");
  return returnval;
}


/*
 cat /proc/meminfo*/
int 
Easy_Memory::get_Info_Memory_byProcMem(double* resultat, 
				       int freemem)
{
#ifdef HAVE_PROCMEM
       
  	char  word[256];
	char demanded[256];
	/* looking in the /proc/meminfo data file*/
	ifstream file ("/proc/meminfo");
	
	if (freemem)
	  strcpy (demanded,"MemFree:");	
	else strcpy (demanded,"MemTotal:");	

	 
	if (file.is_open())
	{ 	    	  	
	  while ( ! file.eof() )  //look at the whole file
	  {
	    if ( strcmp (demanded , word ) == 0 )
	    {
		file >> word;
		*resultat=atof(word)/1024;		
		return 0;			
	    }
	    file >> word;
	  }
	  file.close();
	}
  	else{
	  cerr<< "Error on reading file";
	  return 1;
	}
#endif //HAVE_PROCMEM   	
 return 1;	
	
}


// int main (){
//   Easy_Memory mem;
//   double maxmem;
//   double availmem;
//   if (!mem.get_Total_Memory(&maxmem))
//      printf("%d\n", maxmem);
//   else printf("no info\n");
  
//   if (!mem.get_Avail_Memory(&availmem))
//       printf("%d\n", availmem);
//   else printf("no info\n");
  
 

// }
