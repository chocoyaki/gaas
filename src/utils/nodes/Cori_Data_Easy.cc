/****************************************************************************/
/* CoRI Collectors of ressource information (Data part)                     */
/*                                                                          */
/* Author(s):                                                               */
/*    - Frauenkron Peter (Peter.Frauenkron@ens-lyon.fr)                     */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.2  2010/03/31 21:15:41  bdepardo
 * Changed C headers into C++ headers
 *
 * Revision 1.1  2010/03/03 14:26:35  bdepardo
 * BEWARE!!!
 * Huge modifications to take into account CYGWIN.
 * Lots of files' directory have been changed.
 *
 * Revision 1.14  2008/05/11 16:19:51  ycaniou
 * Check that pathToTmp and pathToNFS exist
 * Check and eventually correct if pathToTmp or pathToNFS finish or not by '/'
 * Rewrite of the propagation of the request concerning job parallel_flag
 * Implementation of Cori_batch system
 * Numerous information can be dynamically retrieved through batch systems
 *
 * Revision 1.13  2008/04/07 12:19:12  ycaniou
 * Except for the class Parsers (someone to re-code it? :)
 *   correct "deprecated conversion from string constant to 'char*'" warnings
 *
 * Revision 1.12  2007/04/30 13:53:22  ycaniou
 * Cosmetic changes (indentation) and small changes for Cori_Batch
 *
 * Revision 1.11  2007/04/16 22:43:44  ycaniou
 * Make all necessary changes to have the new option HAVE_ALT_BATCH operational.
 * This is indented to replace HAVE_BATCH.
 *
 * First draw to manage batch systems with a new Cori plug-in.
 *
 * Revision 1.10  2006/12/06 21:57:15  ecaron
 * Header bug fix
 *
 * Revision 1.9  2006/12/04 21:59:27  ecaron
 * Full header from cvs log
 *
 *
 * Revision 1.7 2006/01/25 20:55:37 pfrauenk
 * CoRI : printing approved
 * 
 * Revision 1.6 2006/01/19 21:45:55 pfrauenk
 * CoRI : small bugs fixed - Cori in FAST-Mode now with EST_COMMTIME
 * 
 * Revision 1.5 2006/01/15 15:37:07 pfrauenk
 * CoRI: debug.hh is used now, cleanup, minor changes
 * 
 * Revision 1.4 2005/12/20 21:53:28 pfrauenk
 * CoRI print function improved
 *
 * Revision 1.3 2005/12/16 07:31:34 pfrauenk
 * CoRI bug fixes
 * 
 * Revision 1.2 2005/12/15 14:00:37 pfrauenk
 * Cori fixes
 *
 * Revision 1.1 2005/12/15 10:12:17 pfrauenk
 * CoRI functionality added by Peter Frauenkron
 *
 ****************************************************************************/

#include "Cori_Data_Easy.hh"

#include "debug.hh"
#include <iostream>
#include <fstream>	/*file reading*/
#include <cstdlib>
#include <string>
#include <cmath>

 
using namespace std;

void
print_Metric(estVector_t vector_v,int type_Info){

  double errorCode=0;
 switch (type_Info){
  
  case EST_ALLINFOS:
     break;
  case EST_CPUSPEED: 
    for (int i=0; i<diet_est_array_size_internal(vector_v,EST_CPUSPEED);i++)
      cout << "CPU "<<i<<" frequence : "<< diet_est_array_get_internal(vector_v,EST_CPUSPEED,i,errorCode)<<" Mhz"<< endl;
    break;
  case EST_CACHECPU:
    for (int i=0; i<diet_est_array_size_internal(vector_v,EST_CACHECPU);i++)
      cout << "CPU "<<i<<" cache : "<< diet_est_array_get_internal(vector_v,EST_CACHECPU,i,errorCode)<<" Kb"<< endl;
    break;	
  case EST_BOGOMIPS:
    for (int i=0; i<diet_est_array_size_internal(vector_v,EST_BOGOMIPS);i++)
      cout << "CPU "<<i<<" Bogomips : "<< diet_est_array_get_internal(vector_v,EST_BOGOMIPS,i,errorCode)<< endl;
    break;   
  case EST_AVGFREECPU:  
    cout << "cpu average load : "<<diet_est_get_internal(vector_v,EST_AVGFREECPU, errorCode)<<endl;
    break;
  case EST_NBCPU:
    cout << "number of processors : " << diet_est_get_internal(vector_v,EST_NBCPU,errorCode)<< endl;     
   break;
 case EST_DISKACCESREAD:
    cout << "diskspeed in reading : "<<diet_est_get_internal(vector_v,EST_DISKACCESREAD,errorCode)<<" Mbyte/s"<<endl;
    break;
 case EST_DISKACCESWRITE:
    cout << "diskspeed in writing : "<<diet_est_get_internal(vector_v,EST_DISKACCESWRITE,errorCode)<<" Mbyte/s"<<endl;
    break;
  case EST_TOTALSIZEDISK: 
    cout << "total disk size : " << diet_est_get_internal(vector_v,EST_TOTALSIZEDISK,errorCode)<<" Mb" << endl;
    break;
  case EST_FREESIZEDISK:
    cout <<	"available disk size  :"<<diet_est_get_internal(vector_v,EST_FREESIZEDISK,errorCode)<<" Mb"<<endl;
    break;
  case EST_TOTALMEM: 
    cout << "total memory : "<< diet_est_get_internal(vector_v,EST_TOTALMEM,errorCode)<<" Mb" << endl;
    break;
  case EST_FREEMEM:
    cout << "available memory : "<<diet_est_get_internal(vector_v,EST_FREEMEM, errorCode)<<" Mb" <<endl;
    break;
  case EST_FREECPU: 
     cout << "free cpu: "<<diet_est_get_internal(vector_v,EST_FREECPU, errorCode)<<endl;
    break;
  default: {
    INTERNAL_WARNING( "CoRI: Tag " <<type_Info <<" for printing info");
  }	
 }
}
Cori_Data_Easy::Cori_Data_Easy(){
   // cpu=new Cori_Easy_CPU();
//    memory=new Cori_Easy_Memory();
//    disk=new Cori_Easy_Disk();
}

int 
Cori_Data_Easy::get_Information(int type_Info,       
				estVector_t* info,
				const void * data){
  const char * path;
  vector<double> vect;
  int res=0;
  double temp=0;
  int minut;
  const char * tmp="./";
  switch (type_Info){
 
  case EST_ALLINFOS:{
    minut=15;
    res = get_Information(EST_CPUSPEED,info,NULL)||res;
    res = get_Information(EST_AVGFREECPU,info,&minut)||res;
    res = get_Information(EST_CACHECPU,info,NULL)||res;
    res = get_Information(EST_NBCPU,info,NULL)||res;
    res = get_Information(EST_BOGOMIPS,info,NULL)||res;
    res = get_Information(EST_DISKACCESREAD,info,tmp)||res;
    res = get_Information(EST_DISKACCESWRITE,info,tmp)||res;
    res = get_Information(EST_TOTALSIZEDISK,info,tmp)||res;
    res = get_Information(EST_FREESIZEDISK,info,tmp)||res;
    res = get_Information(EST_TOTALMEM,info,NULL)||res;
    res = get_Information(EST_FREEMEM,info,NULL)||res; 
    //set all other TAGS to bad values
    diet_est_set_internal(*info, EST_TCOMP, HUGE_VAL);
  }
  case EST_CPUSPEED:
    res =cpu->get_CPU_Frequence(&vect); 
    convertArray(vect,info,type_Info);
    break;
  case EST_AVGFREECPU:  
    if (data==NULL){
      minut=15;
    }
    else
      minut= *((int*) data);
    res=cpu->get_CPU_Avg(minut,&temp);
    convertSimple(temp,info,type_Info);
    break;
  case EST_CACHECPU:
     res=cpu->get_CPU_Cache(&vect);
     convertArray(vect,info,type_Info);
     break;
  case EST_NBCPU:
     res=cpu->get_CPU_Number(&temp);
     convertSimple(temp, info,type_Info);
    break;
  case EST_BOGOMIPS:
     res=cpu->get_CPU_Bogomips(&vect);
     convertArray(vect,info,type_Info);
    break;
  case EST_DISKACCESREAD: 
    if (data==NULL)
      path="./";
    else
      path= (char *) data;        
     res=disk->get_Read_Speed(path, &temp);
     convertSimple(temp, info,type_Info);
    break;
  case EST_DISKACCESWRITE:
    if (data==NULL)
      path="./";
    else
     path=(char *) data;       
    res=disk->get_Write_Speed(path, &temp);
    convertSimple(temp, info,type_Info);
    break;
  case EST_TOTALSIZEDISK: 
    if (data==NULL)
      path="./";
    else
     path= (char *) data;        
    res=disk->get_Total_DiskSpace(path,&temp);
    convertSimple(temp, info,type_Info);
    break;
  case EST_FREESIZEDISK:
    if (data==NULL)
      path="./";
    else
      path= (char *) data;        
    res=disk->get_Available_DiskSpace(path,&temp);
    convertSimple(temp, info,type_Info);
    break;
  case EST_TOTALMEM:
    res=memory->get_Total_Memory(&temp);
    convertSimple(temp, info,type_Info);
    break;
  case EST_FREEMEM: 
    res=memory->get_Avail_Memory(&temp);
    convertSimple(temp, info,type_Info);
    break;
  case EST_FREECPU:
    res=cpu->get_CPU_ActualLoad(&temp);
    temp=1-temp;
    convertSimple(temp, info,type_Info);
    break;
  default: {
    INTERNAL_WARNING("CoRI: Tag " <<type_Info <<" unknown for collecting info");
    res=1;
  }
  }
  if (TRACE_LEVEL>=TRACE_ALL_STEPS)
    print_Metric(*info,type_Info); 
  return res;
}

/****************************************************************************/
/* Private methods                                                          */
/****************************************************************************/


 int  
Cori_Data_Easy::convertArray(vector <double> vect,
			     estVector_t * estvect,
			     int typeOfInfo ){
  vector<double>::iterator iter1;
  iter1 = vect.begin();
  int i=0;
  while( iter1 != vect.end()) {
    diet_est_array_set_internal(*estvect,typeOfInfo,i,*iter1);
    i++; 
    iter1++;
  }
  return 0;
}

int  
Cori_Data_Easy::convertSimple(double value,
			      estVector_t * estvect,
			      int typeOfInfo ){
  diet_est_set_internal(*estvect,typeOfInfo,value);
  return 0;
}


