/****************************************************************************/
/* $Id$ */
/* DIET mutex interface for multi-threaded server applications (source code)*/
/*                                                                          */
/*  Author(s):                                                              */
/*    - Bert VAN HEUKELOM         - LIP ENS-Lyon (France)                   */
/*    - Philippe COMBES           - LIP ENS-Lyon (France)                   */
/*                                                                          */
/*  This is part of DIET software.                                          */
/*  Copyright (C) 2002 ReMaP/INRIA                                          */
/*                                                                          */
/****************************************************************************/
/*
 * $Log$
 * Revision 1.1  2002/12/03 19:08:24  pcombes
 * Update configure, update to FAST 0.3.15, clean CVS logs in files.
 * Put main Makefile in root directory.
 *
 ****************************************************************************/


#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "omnithread.h"
#include "DIET_mutex.h"


omni_mutex **mutex_field = NULL;
int mutexCount  = 0;
int initialized = 0;

diet_mutex_t m[100];


void diet_mutex_initialize(){
  int i;
  mutex_field=(omni_mutex**)malloc(10*sizeof(omni_mutex));
  for(i=0; i<10; i++){
    mutex_field[i]=NULL;
  }
  mutexCount=10;
  initialized = 1;
}

void diet_mutex_create(int *ret){
  int i;
  omni_mutex **temp;
  
  if(!initialized){
    printf("diet_mutex_create Error: diet_mutex_initialize has not been called\n");
    exit(2);
  }

  for(i=0; i<mutexCount; i++){
    if(mutex_field[i]==NULL){
      mutex_field[i]=new omni_mutex();
      *ret = i;
      return;
    }
  }
  
  //printf("adding space %d \n", mutexCount);
  temp = (omni_mutex**)malloc((10+mutexCount)*sizeof(omni_mutex));
  for(i=0; i<mutexCount; i++){
    temp[i]=mutex_field[i];
  }
  for(i=0; i<10; i++){
    temp[i+mutexCount]=NULL;
  }
  free(mutex_field);
  mutex_field=temp;
  
  mutex_field[mutexCount]=new omni_mutex();
  *ret = mutexCount;
  
  mutexCount+=10;
}
  
void diet_mutex_free(int *i){

  if(!initialized){
    printf("diet_mutex_free Error: diet_mutex_initialize has not been called\n");
    exit(2);
  }
  if((*i)>=mutexCount){
    printf("diet_mutex_free Error: invalid mutex\n");
    exit(2);
  }    

  delete(mutex_field[*i]);

  mutex_field[*i]=NULL;
  *i=0;

}

void
diet_mutex_lock(int i){
   if(!initialized){
    printf("diet_mutex_lock Error: diet_mutex_initialize has not been called\n");
    exit(2);
  }
  if((i)>=mutexCount){
    printf("diet_mutex_lock Error: invalid mutex\n");
    exit(2);
  }    
  mutex_field[i]->lock();
}

void
diet_mutex_unlock(int i){
  if(!initialized){
    printf("diet_mutex_unlock Error: diet_mutex_initialize has not been called\n");
    exit(2);
  }
  if((i)>=mutexCount){
    printf("diet_mutex_unlock Error: invalid mutex\n");
    exit(2);
  }    
  mutex_field[i]->unlock();
}

void diet_mutex_finalize(){
  if(!initialized){
    printf("diet_mutex_finalize Error: diet_mutex_initialize has not been called\n");
    exit(2);
  }
  free(mutex_field);
  initialized=0;
}

void diet_thread_sleep(int m, int n){
  omni_thread *myThread = NULL;
  myThread = omni_thread::self();
  myThread->sleep(m, n);
}
void diet_thread_yield(){
  omni_thread *myThread = NULL;
  myThread = omni_thread::self();
  myThread->yield();
}
int diet_thread_id(){
  omni_thread *myThread = NULL;
  myThread = omni_thread::self();
  return myThread->id();
}

