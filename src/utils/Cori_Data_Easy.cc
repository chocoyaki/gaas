#include <iostream>
#include <fstream>	/*file reading*/
#include <cstdlib>
#include <string>
#include <math.h>
#include "Cori_Data_Easy.hh"
 
using namespace std;

void
print_Metric(estVector_t vector_v,diet_est_tag_t type_Info){

  double errorCode=0;
 switch (type_Info){
  
  case EST_ALLINFOS:{
 
	for (int i=0; i<diet_est_array_size_internal(vector_v,EST_CPUSPEED);i++){
	  cout << "CPU_frequence="<< diet_est_array_get_internal(vector_v,EST_CPUSPEED,i,errorCode)<< endl;
	  cout << "CPU_cache="<< diet_est_array_get_internal(vector_v,EST_CACHECPU,i,errorCode)<< endl;
	  cout << "CPU_Bogomips="<< diet_est_array_get_internal(vector_v,EST_CACHECPU,i,errorCode)<< endl;
	}

	cout << "CPU_num_processors="<< diet_est_get_internal(vector_v,EST_NBCPU,errorCode)<< endl;         
	cout << "Memory_total="<< diet_est_get_internal(vector_v,EST_TOTALMEM,errorCode) << endl;
	cout << "disk size = " << diet_est_get_internal(vector_v,EST_TOTALSIZEDISK,errorCode)<<" Mbyte(s)" << endl;
    	cout <<	"mem avail:"<<diet_est_get_internal(vector_v,EST_FREEMEM, errorCode) <<endl;
	cout << "cpu avg:"<<diet_est_get_internal(vector_v,EST_AVGFREECPU, errorCode)<<endl;
	cout << "diskspeedRead:"<<diet_est_get_internal(vector_v,EST_DISKACCESREAD,errorCode)<<" Mbyte/s"<<endl;
	cout << "diskspeedWrite:"<<diet_est_get_internal(vector_v,EST_DISKACCESWRITE,errorCode)<<" Mbyte/s"<<endl;
	cout <<	"disk size avail:"<<diet_est_get_internal(vector_v,EST_FREESIZEDISK,errorCode)<<" Mbyte(s)"<<endl;
	cout << "cpu free:"<<diet_est_get_internal(vector_v,EST_FREECPU, errorCode)<<" %" <<endl;
 }
    break;
  case EST_CPUSPEED: 
    for (int i=0; i<diet_est_array_size_internal(vector_v,EST_CPUSPEED);i++)
      cout << "CPU_frequence="<< diet_est_array_get_internal(vector_v,EST_CPUSPEED,i,errorCode)<< endl;
    break;
  case EST_CACHECPU:
    for (int i=0; i<diet_est_array_size_internal(vector_v,EST_CPUSPEED);i++)
      cout << "CPU_cache="<< diet_est_array_get_internal(vector_v,EST_CACHECPU,i,errorCode)<< endl;
    break;	
  case EST_BOGOMIPS:
    for (int i=0; i<diet_est_array_size_internal(vector_v,EST_CPUSPEED);i++)
      cout << "CPU_Bogomips="<< diet_est_array_get_internal(vector_v,EST_CACHECPU,i,errorCode)<< endl;
    break;   
  case EST_AVGFREECPU:  
    cout << "cpu avg:"<<diet_est_get_internal(vector_v,EST_AVGFREECPU, errorCode)<<endl;
    break;
  case EST_NBCPU:
    cout << "CPU_num_processors="<< diet_est_get_internal(vector_v,EST_NBCPU,errorCode)<< endl;     
   break;
 case EST_DISKACCESREAD:
    cout << "diskspeedRead:"<<diet_est_get_internal(vector_v,EST_DISKACCESREAD,errorCode)<<" Mbyte/s"<<endl;
    break;
 case EST_DISKACCESWRITE:
    cout << "diskspeedWrite:"<<diet_est_get_internal(vector_v,EST_DISKACCESWRITE,errorCode)<<" Mbyte/s"<<endl;
    break;
  case EST_TOTALSIZEDISK: 
    cout << "disk size = " << diet_est_get_internal(vector_v,EST_TOTALSIZEDISK,errorCode)<<" Mbyte(s)" << endl;
    break;
  case EST_FREESIZEDISK:
    cout <<	"disk size avail:"<<diet_est_get_internal(vector_v,EST_FREESIZEDISK,errorCode)<<" Mbyte(s)"<<endl;
    break;
  case EST_TOTALMEM: 
    cout << "Memory_total="<< diet_est_get_internal(vector_v,EST_TOTALMEM,errorCode) << endl;
    break;
  case EST_FREEMEM:
    cout << "mem avail:"<<diet_est_get_internal(vector_v,EST_FREEMEM, errorCode) <<endl;
    break;
  case EST_FREECPU: 
     cout << "cpu free:"<<diet_est_get_internal(vector_v,EST_FREECPU, errorCode)<<" %"  <<endl;
    break;
  default: {
    cerr<< "Tag " <<type_Info <<" unknown for information" <<endl;
  }	
 }
}
Cori_Data_Easy::Cori_Data_Easy(){
   // cpu=new Cori_Easy_CPU();
//    memory=new Cori_Easy_Memory();
//    disk=new Cori_Easy_Disk();
}

int 
Cori_Data_Easy::get_Information(diet_est_tag_t type_Info,       
				      estVector_t* info,
				      const void * data){
  char * path;
   vector<double> vect;
   int res=0;
   double temp=0;
   int minut;
   int print_it=1;
  switch (type_Info){
  
  case EST_ALLINFOS:{
    minut=15;
    res = get_Information(EST_CPUSPEED,info,NULL)||res;
    res = get_Information(EST_AVGFREECPU,info,&minut)||res;
    res = get_Information(EST_CACHECPU,info,NULL)||res;
    res = get_Information(EST_NBCPU,info,NULL)||res;
    res = get_Information(EST_BOGOMIPS,info,NULL)||res;
    res = get_Information(EST_DISKACCESREAD,info,"./")||res;
    res = get_Information(EST_DISKACCESWRITE,info,"./")||res;
    res = get_Information(EST_TOTALSIZEDISK,info,"./")||res;
    res = get_Information(EST_FREESIZEDISK,info,"./")||res;
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
    path= (char *) data;        
     res=disk->get_Read_Speed(path, &temp);
     convertSimple(temp, info,type_Info);
    break;
  case EST_DISKACCESWRITE:
     path=(char *) data;        
     res=disk->get_Write_Speed(path, &temp);
     convertSimple(temp, info,type_Info);
    break;
  case EST_TOTALSIZEDISK: 
     path= (char *) data;        
     res=disk->get_Total_DiskSpace(path,&temp);
     convertSimple(temp, info,type_Info);
    break;
  case EST_FREESIZEDISK:
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
  default: {
    cerr<< "Tag " <<type_Info <<" unknown for information" <<endl;
    res=1;
  }

}
  if (print_it) print_Metric(*info,type_Info); 
  return res;
}

/****************************************************************************/
/* Private methods                                                          */
/****************************************************************************/


 int  
Cori_Data_Easy::convertArray(vector <double> vect,
			     estVector_t * estvect,
			     diet_est_tag_t typeOfInfo ){
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
			      diet_est_tag_t typeOfInfo ){
  diet_est_set_internal(*estvect,typeOfInfo,value);
  return 0;
 
}


