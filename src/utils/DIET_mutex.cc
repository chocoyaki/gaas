/****************************************************************************/
/* $Id$ */
/* DIET mutex interface for multi-threaded server applications (source code)*/
/*                                                                          */
/*  Author(s):                                                              */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*    - Bert VAN HEUKELOM (Bert.Van-Heukelom@ens-lyon.fr)                   */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/*
 * $Log$
 * Revision 1.3  2003/02/19 09:03:40  cpera
 * Add headers include for gcc 2.95.3 compatibility under SunOS.
 *
 * Revision 1.2  2003/02/04 10:08:22  pcombes
 * Apply Coding Standards
 *
 * Revision 1.1  2002/12/03 19:08:24  pcombes
 * Update configure, update to FAST 0.3.15, clean CVS logs in files.
 * Put main Makefile in root directory.
 ****************************************************************************/

#include <stdio.h>
#include <iostream>
using namespace std;
#include <stdlib.h>
//#include <string.h>
//#include <math.h>
#include "omnithread.h"
#include "DIET_mutex.h"


static omni_mutex** MUTEX_FIELD = NULL;
static int MUTEXCOUNT  = 0;
static int INITIALIZED = 0;

//diet_mutex_t M[100];


void
diet_mutex_initialize()
{
  int i;
  MUTEX_FIELD=(omni_mutex**)malloc(10*sizeof(omni_mutex));
  for(i=0; i<10; i++){
    MUTEX_FIELD[i]=NULL;
  }
  MUTEXCOUNT=10;
  INITIALIZED = 1;
}

void
diet_mutex_create(int* ret)
{
  int i;
  omni_mutex **temp;
  
  if(!INITIALIZED){
    printf("diet_mutex_create Error: diet_mutex_initialize has not been called\n");
    exit(2);
  }

  for(i=0; i<MUTEXCOUNT; i++){
    if(MUTEX_FIELD[i]==NULL){
      MUTEX_FIELD[i]=new omni_mutex();
      *ret = i;
      return;
    }
  }
  
  //printf("adding space %d \n", MUTEXCOUNT);
  temp = (omni_mutex**)malloc((10+MUTEXCOUNT)*sizeof(omni_mutex));
  for(i=0; i<MUTEXCOUNT; i++){
    temp[i]=MUTEX_FIELD[i];
  }
  for(i=0; i<10; i++){
    temp[i+MUTEXCOUNT]=NULL;
  }
  free(MUTEX_FIELD);
  MUTEX_FIELD=temp;
  
  MUTEX_FIELD[MUTEXCOUNT]=new omni_mutex();
  *ret = MUTEXCOUNT;
  
  MUTEXCOUNT+=10;
}
  
void
diet_mutex_free(int* i)
{
  if(!INITIALIZED){
    printf("diet_mutex_free Error: diet_mutex_initialize has not been called\n");
    exit(2);
  }
  if((*i)>=MUTEXCOUNT){
    printf("diet_mutex_free Error: invalid mutex\n");
    exit(2);
  }    

  delete(MUTEX_FIELD[*i]);

  MUTEX_FIELD[*i]=NULL;
  *i=0;

}

void
diet_mutex_lock(int i){
   if(!INITIALIZED){
    printf("diet_mutex_lock Error: diet_mutex_initialize has not been called\n");
    exit(2);
  }
  if((i)>=MUTEXCOUNT){
    printf("diet_mutex_lock Error: invalid mutex\n");
    exit(2);
  }    
  MUTEX_FIELD[i]->lock();
}

void
diet_mutex_unlock(int i){
  if(!INITIALIZED){
    printf("diet_mutex_unlock Error: diet_mutex_initialize has not been called\n");
    exit(2);
  }
  if((i)>=MUTEXCOUNT){
    printf("diet_mutex_unlock Error: invalid mutex\n");
    exit(2);
  }    
  MUTEX_FIELD[i]->unlock();
}

void
diet_mutex_finalize()
{
  if(!INITIALIZED){
    printf("diet_mutex_finalize Error: diet_mutex_initialize has not been called\n");
    exit(2);
  }
  free(MUTEX_FIELD);
  INITIALIZED=0;
}

void
diet_thread_sleep(int m, int n)
{
  omni_thread *myThread = NULL;
  myThread = omni_thread::self();
  myThread->sleep(m, n);
}

void
diet_thread_yield()
{
  omni_thread *myThread = NULL;
  myThread = omni_thread::self();
  myThread->yield();
}

int
diet_thread_id()
{
  omni_thread *myThread = NULL;
  myThread = omni_thread::self();
  return myThread->id();
}

