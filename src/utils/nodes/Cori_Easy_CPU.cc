/****************************************************************************/
/* CoRI Collectors of ressource information                                 */
/*                                                                          */
/* Author(s):                                                               */
/*    - Frauenkron Peter (Peter.Frauenkron@ens-lyon.fr)                     */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.4  2011/01/20 23:56:12  bdepardo
 * Removed resource leak: psfile
 *
 * Revision 1.3  2010/12/17 09:48:02  kcoulomb
 * * Set diet to use the new log with forwarders
 * * Fix a CoRI problem
 * * Add library version remove DTM flag from ccmake because deprecated
 *
 * Revision 1.2  2010/03/31 21:15:41  bdepardo
 * Changed C headers into C++ headers
 *
 * Revision 1.1  2010/03/03 14:26:35  bdepardo
 * BEWARE!!!
 * Huge modifications to take into account CYGWIN.
 * Lots of files' directory have been changed.
 *
 * Revision 1.8  2007/04/30 13:53:22  ycaniou
 * Cosmetic changes (indentation) and small changes for Cori_Batch
 *
 * Revision 1.7  2006/11/16 09:55:55  eboix
 *   DIET_config.h is no longer used. --- Injay2461
 *
 * Revision 1.6  2006/10/31 22:51:01  ecaron
 * Collector for CPU Information (default mode)
 *
 ****************************************************************************/
#include <cstdlib>           // for getloadavg
#include <iostream>
#include <string>
#include <fstream> 
#include <cmath>             // for HUGE_VAL
#ifdef CORI_HAVE_SYS_TYPES
#include <sys/types.h>        // for sysctl on some systems
#endif
#ifdef CORI_HAVE_SYS_SYSCTL
#include <sys/sysctl.h>       // for sysctl on some (other) systems
#endif
#ifdef CORI_HAVE_SYS_SYSINFO
#include <sys/sysinfo.h>      // for get_nproc
#endif
#include "Cori_Easy_CPU.hh"
#include <cstdio>
#include "debug.hh"
using namespace std;

int 
Easy_CPU::get_CPU_Avg(int interval, 
		      double * resultat){
  double temp;
  if (!get_CPU_Avg_byGetloadavg(interval,&temp)){
    *resultat=temp;
    return 0;}
  else 
    *resultat=HUGE_VAL;
  return 1;
}


int 
Easy_CPU::get_CPU_Frequence(vector <double> * vlist)
{
 
  vector<double> temp;
 
  if (!get_CPU_Freq_From_Proc(&temp)||
      !get_CPU_Freq_for_FreeBSD(&temp)||
      !get_CPU_Freq_for_Darwin(&temp)||
      !get_CPU_Freq_for_NetBSD(&temp)
      ){
    
    //found it in a manner
    *vlist=temp;
    return 0;
  }
  else{
    //rechercher autres solutions...
    temp.push_back( 0 );
    *vlist=temp;
    return 1;
  }
}

int 
Easy_CPU::get_CPU_Cache(vector <double> * vlist){
  vector<double> temp;
  
  if (!get_CPU_Cache_From_Proc(&temp)){
    //using /proc succesful
    *vlist=temp;
    return 0;
  }
  else{
    //rechercher autres solutions...
    temp.push_back( 0 );
    *vlist=temp;
    return 1;
  }
}


int 
Easy_CPU::get_CPU_Bogomips(vector <double> * vlist){
  vector<double> temp;

  if (!get_Bogomips_From_Proc(&temp)){
    //using /proc/
    *vlist=temp;
    return 0;
  }
  else{
    //rechercher autres solutions...
    temp.push_back( 0 );
    *vlist=temp;
    return 1;
  }
}
  
int 
Easy_CPU::get_CPU_Number(double * result){
  if ((!get_CPU_Number_byget_nprocs(result))||
      (!get_CPU_Number_byNum_Proc(result)))
    return 0;
  return 1;
}



int 
Easy_CPU::get_CPU_ActualLoad(double * actualload){
  if (!get_CPU_ActualLoad_Byps(actualload))
    return 0;
      else {
	*actualload=1;
	return 1;
	}
}


/***********PRIVATE****************************************/
int 
Easy_CPU::get_CPU_Avg_byGetloadavg(int interval, 
				   double * resultat){
// int getloadavg (double loadavg[], int nelem) 
// This function gets the 1, 5 and 15 minute load 
//averages of the system. The values are placed in 
//loadavg. getloadavg will place at most nelem 
//elements into the array but never more than 
//three elements. The return value is the number
// of elements written to loadavg, or -1 on error.

// This function is declared in stdlib.h. 

#ifdef CORI_HAVE_getloadavg
  switch (interval){
  case 1: {
    double loadavg[1];
    getloadavg (loadavg, 1);
    *resultat =loadavg[0];
    break;
  }
   
  case 10: {
    double loadavg[2];
     getloadavg (loadavg, 2);
    *resultat = loadavg[1];
    break;
  } 
   
  case 15: {
    double loadavg[3];
    getloadavg (loadavg, 3);
    *resultat=loadavg[2];
    break;
  }
  default: {
    INTERNAL_WARNING("CoRI Easy: "<<interval<<" bad value for get_CPU_Avg(int x)!");
    return 1;
  }
  } 
  return 0;

#else 
return 1;
#endif
  }

int 
Easy_CPU::get_CPU_Number_byget_nprocs(double * result){
/* int get_nprocs (void) */
/* The get_nprocs function returns the number of available processors. */
/* For these two pieces of information the GNU C library also provides */
/* functions to get the information directly. The functions are        */
/* declared in sys/sysinfo.h This function is a GNU extension.         */
  fprintf (stderr, "TADAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n");
#ifdef CORI_HAVE_get_nprocs
  fprintf (stderr, "Getting nprocs : res=%lf \n", *result);
  *result=get_nprocs ();
  fprintf (stderr, "-> nprocs : res=%lf \n", *result);
  return 0;
  
#endif

 return 1;
}

int 
Easy_CPU::get_CPU_Number_byNum_Proc(double * result){
#undef NUM_PROC
// On each machine type the nomenclature for
//    flags to sysconf changes.  Map the correct
//    flag onto NUM_PROC


#if defined _SC_NPROCESSORS_ONLN //TARGET_ARCH_SOLARIS || TARGET_ARCH_AIX
# define NUM_PROC _SC_NPROCESSORS_ONLN

#elif defined _SC_NPROC_ONLN //TARGET_ARCH_IRIX
# define NUM_PROC _SC_NPROC_ONLN

#elif defined HW_AVAILCPU && defined CTL_HW //FREEBSD
# define NUM_PROC HW_AVAILCPU
# define SYSCTL2 1

#elif defined _SC_CRAY_NCPU //TARGET_ARCH_CRAYT3E
# define NUM_PROC _SC_CRAY_NCPU

#elif defined _SC_NPROCESSORS_CONF //TARGET_ARCH_OSF1
# define NUM_PROC _SC_NPROCESSORS_CONF

#endif

// #if defined(TARGET_ARCH_HPUX11)

// //  HPUX 11 does not provide sysinfo. pstat does not return
// //  proper cpu count information on HPUX 10

// #include <sys/param.h>
// #include <sys/pstat.h>
// #endif


// int main (void) {
// #if defined(TARGET_ARCH_HPUX11)
//   {
//     struct pst_dynamic buf;
//     int rc;
//     size_t elemsize=sizeof(struct pst_dynamic);
//     size_t elemcount=1U;
//     int index=0;

//     rc = pstat_getdynamic(&buf,
//                           elemsize,
//                           elemcount,
// 			  index);

//     printf("%d", (rc==-1) ? 1 : buf.psd_proc_cnt);
//   }
// #elif defined(TARGET_ARCH_HPUX)
//   {
//     FILE *pipe;
//     pipe = popen("sysinfo -cpu_count", "w");
//     if (pipe!=NULL) pclose (pipe); /\* command ran *\/
//     else fprintf (stdout, "1");
//   }


#if defined NUM_PROC && defined(CORI_HAVE_sysconf)
*result=sysconf(NUM_PROC);
 return 0;
#endif

#if defined SYSCTL2 && defined(CORI_HAVE_sysconf)
     int mib[2], maxproc;
           size_t len;

           mib[0] = CTL_HW;
           mib[1] = HW_AVAILCPU ;
           len = sizeof(maxproc);
           sysctl(mib, 2, &maxproc, &len, NULL, 0);
*result=maxproc;
 return  0;
#else
 
 return 1;
#endif
 }

int 
Easy_CPU::get_CPU_ActualLoad_Byps(double * actualload){
 
  FILE * psfile;
  char buffer [256];
  psfile=popen("ps -e -o pcpu","r");

  if (psfile==NULL){
    return 1;
  }
  else  
    fscanf (psfile,"%s",buffer);
  if (strcmp(buffer,"%CPU")!=0){
    pclose(psfile);
    return 1;
  }  
  float loadCPU=0;
  float tmp=0;
  while(!feof(psfile)){
    fscanf (psfile,"%f",&tmp);
    loadCPU+=tmp;
  }
  pclose(psfile);
  *actualload=loadCPU/100;
  if (*actualload<0) //error in what kind of manner
    *actualload=1;
  return 0;
}


int 
Easy_CPU::get_Bogomips_From_Proc(vector <double> * vlist){
  int     ret = 1;
#ifdef CORI_HAVE_PROCCPU 
  FILE    *fp;
  char    buf[128];
  double  val=0;
  

  if ((fp = fopen ("/proc/cpuinfo", "r")) != NULL)
    {
      while (fgets (buf, sizeof (buf), fp) != NULL)
        {
	  if (sscanf (buf, "bogomips : %lf\n", &val) == 1
              || sscanf (buf, "BogoMIPS : %lf\n", &val) == 1){        
              ret = 0;
	      vlist->push_back( val );
            }   
        }
      fclose (fp);
    }
#else 
  ret=1;
#endif
return ret;
}
int 
Easy_CPU::get_CPU_Cache_From_Proc(vector <double> * vlist){
 int     ret = 1;
#ifdef CORI_HAVE_PROCCPU
 FILE    *fp;
  char    buf[128];
  double  val=0;
 

  if ((fp = fopen ("/proc/cpuinfo", "r")) != NULL)
    {
      while (fgets (buf, sizeof (buf), fp) != NULL)
        {
	  if (sscanf (buf, "cache size : %lf\n", &val) == 1){        
              ret = 0;
	      vlist->push_back( val );
            }   
        }
      fclose (fp);
    }
#else 
  ret=1;
#endif
  return ret;
}
 
int 
Easy_CPU::get_CPU_Freq_From_Proc(vector <double> * vlist){
 int     ret = 1;
#ifdef CORI_HAVE_PROCCPU   
  FILE    *fp;
  char    buf[128];
  double  val=0;
 

  if ((fp = fopen ("/proc/cpuinfo", "r")) != NULL)
    {
      while (fgets (buf, sizeof (buf), fp) != NULL)
        {
          if (sscanf (buf, "cycle frequency [Hz]    : %lf", &val) == 1)
            {
             ret = 0;
             vlist->push_back( (val)/1000000 );
            }
          if (sscanf (buf, "cpu MHz : %lf\n", &val) == 1)
            {
	      ret = 0;    
	      vlist->push_back( val );           
            }
          if (sscanf (buf, "clock : %lfMHz\n", &val) == 1)
            {         
              ret = 0;
	      vlist->push_back( val );
            }   
        }
      fclose (fp);
    }
#else 
  ret=1;
#endif
  return ret;
}

int 
Easy_CPU::get_CPU_Freq_for_FreeBSD(vector <double> * vlist)
{
#if CORI_HAVE_sysctlbyname
  unsigned  val;
  size_t    size;
  size = sizeof(val);
  if (sysctlbyname ("machdep.i586_freq", &val, &size, NULL, 0) == 0
          && size == sizeof(val)&&(val!=0)
      ||sysctlbyname ("machdep.i586_freq", &val, &size, NULL, 0) == 0
          && size == sizeof(val)&&(val!=0)
      
      )
    {    
      vlist->push_back(val/1000000 );
      return 0;
    }
#endif
  return 1;
}



/* Apple powerpc Darwin 1.3 sysctl hw.cpufrequency is in hertz.  For some
   reason only seems to be available from sysctl(), not sysctlbyname().  */

int 
Easy_CPU::get_CPU_Freq_for_Darwin(vector <double> * vlist)
{
#if CORI_HAVE_sysctl && defined (CTL_HW) && defined (HW_CPU_FREQ)
  int       mib[2];
  unsigned  val;
  size_t    size;

  mib[0] = CTL_HW;
  mib[1] = HW_CPU_FREQ;
  size = sizeof(val);
  if (sysctl (mib, 2, &val, &size, NULL, 0) == 0)
    {
      vlist->push_back( val);   
      return 0;
    }
#endif
  return 1;
}


/* Alpha FreeBSD 4.1 and NetBSD 1.4 sysctl- hw.model string gives "Digital
   AlphaPC 164LX 599 MHz".  NetBSD 1.4 doesn't seem to have sysctlbyname, so
   sysctl() is used.  */

int
Easy_CPU::get_CPU_Freq_for_NetBSD(vector <double> * vlist)
{
#if CORI_HAVE_sysctl && defined (CTL_HW) && defined (HW_MODEL)
  int       mib[2];
  char      str[128];
  unsigned  val;
  size_t    size;
  char  *p;
  int   i;

  mib[0] = CTL_HW;
  mib[1] = HW_MODEL;
  size = sizeof(str);
  if (sysctl (mib, 2, str, &size, NULL, 0) == 0)
    {
      /* find the second last space */
      p = &str[size-1];
      for (i = 0; i < 2; i++)
        {
          for (;;)
            {
              if (p <= str)
                return 0;
              p--;
              if (*p == ' ')
                break;
            }
        }

      if (sscanf (p, "%u MHz", &val) != 1)
        return 1;

       vlist->push_back(val);
      return 0;
    }
#endif
  return 1;
}
