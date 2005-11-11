/****************************************************************************
**			TAU Portable Profiling Package			   **
**			http://www.cs.uoregon.edu/research/tau	           **
*****************************************************************************
**    Copyright 1997  						   	   **
**    Department of Computer and Information Science, University of Oregon **
**    Advanced Computing Laboratory, Los Alamos National Laboratory        **
****************************************************************************/
/***************************************************************************
**	File 		: PthreadLayer.cpp				  **
**	Description 	: TAU Profiling Package RTS Layer definitions     **
**			  for supporting pthreads 			  **
**	Author		: Sameer Shende					  **
**	Contact		: sameer@cs.uoregon.edu sameer@acl.lanl.gov 	  **
**	Flags		: Compile with				          **
**			  -DPROFILING_ON to enable profiling (ESSENTIAL)  **
**			  -DPROFILE_STATS for Std. Deviation of Excl Time **
**			  -DSGI_HW_COUNTERS for using SGI counters 	  **
**			  -DPROFILE_CALLS  for trace of each invocation   **
**			  -DSGI_TIMERS  for SGI fast nanosecs timer	  **
**			  -DTULIP_TIMERS for non-sgi Platform	 	  **
**			  -DPOOMA_STDSTL for using STD STL in POOMA src   **
**			  -DPOOMA_TFLOP for Intel Teraflop at SNL/NM 	  **
**			  -DPOOMA_KAI for KCC compiler 			  **
**			  -DDEBUG_PROF  for internal debugging messages   **
**                        -DPROFILE_CALLSTACK to enable callstack traces  **
**	Documentation	: See http://www.cs.uoregon.edu/research/tau      **
***************************************************************************/


//////////////////////////////////////////////////////////////////////
// Include Files 
//////////////////////////////////////////////////////////////////////

//#define DEBUG_PROF
#ifdef TAU_DOT_H_LESS_HEADERS
#include <iostream>
using namespace std;
#else /* TAU_DOT_H_LESS_HEADERS */
#include <iostream.h>
#endif /* TAU_DOT_H_LESS_HEADERS */
#include "Profile/Profiler.h"

#ifdef TAU_CHARM
extern "C" {
#include <cpthreads.h>
}
#else 
#include <pthread.h>
#endif

/////////////////////////////////////////////////////////////////////////
// Member Function Definitions For class PthreadLayer
// This allows us to get thread ids from 0..N-1 instead of long nos 
// as generated by pthread_self() 
/////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////
// Define the static private members of PthreadLayer  
/////////////////////////////////////////////////////////////////////////

pthread_key_t PthreadLayer::tauPthreadId;
pthread_mutex_t PthreadLayer::tauThreadcountMutex;
pthread_mutexattr_t PthreadLayer::tauThreadcountAttr;
pthread_mutex_t     PthreadLayer::tauDBMutex;  
pthread_mutex_t     PthreadLayer::tauEnvMutex;  
pthread_mutexattr_t PthreadLayer::tauDBAttr;

int PthreadLayer::tauThreadCount = 0; 

////////////////////////////////////////////////////////////////////////
// RegisterThread() should be called before any profiling routines are
// invoked. This routine sets the thread id that is used by the code in
// FunctionInfo and Profiler classes. This should be the first routine a 
// thread should invoke from its wrapper. Note: main() thread shouldn't
// call this routine. 
////////////////////////////////////////////////////////////////////////
int PthreadLayer::RegisterThread(void)
{

  int *threadId = new int;

  pthread_mutex_lock(&tauThreadcountMutex);
  tauThreadCount ++;
  // A thread should call this routine exactly once. 
  *threadId = tauThreadCount;
  DEBUGPROFMSG("Thread id "<< tauThreadCount<< " Created! "<<endl;);

  pthread_mutex_unlock(&tauThreadcountMutex);
  pthread_setspecific(tauPthreadId, threadId);
  return 0;
}


////////////////////////////////////////////////////////////////////////
// GetThreadId returns an id in the range 0..N-1 by looking at the 
// thread specific data. Since a getspecific has to be preceeded by a 
// setspecific (that all threads besides main do), we get a null for the
// main thread that lets us identify it as thread 0. It is the only 
// thread that doesn't do a PthreadLayer::RegisterThread(). 
////////////////////////////////////////////////////////////////////////
int PthreadLayer::GetThreadId(void) 
{
#ifdef TAU_CHARM
  if (RtsLayer::myNode() == -1)
    return 0;
#endif

  int *id; 
  static int initflag = PthreadLayer::InitializeThreadData();
  // if its in here the first time, setup mutexes etc.

  id = (int *) pthread_getspecific(tauPthreadId);
  
  if (id == NULL)
  {
    return 0; // main() thread 
  } 
  else
  { 
    return *id;
  }

}

////////////////////////////////////////////////////////////////////////
// InitializeThreadData is called before any thread operations are performed. 
// It sets the default values for static private data members of the 
// PthreadLayer class.
////////////////////////////////////////////////////////////////////////
int PthreadLayer::InitializeThreadData(void)
{
  // Initialize the mutex
  pthread_key_create(&tauPthreadId, NULL);
  pthread_mutexattr_init(&tauThreadcountAttr);
  pthread_mutex_init(&tauThreadcountMutex, &tauThreadcountAttr);
  
  //cout << "PthreadLayer::Initialize() done! " <<endl;

  return 1;
}

////////////////////////////////////////////////////////////////////////
int PthreadLayer::InitializeDBMutexData(void)
{
  // For locking functionDB 
  pthread_mutexattr_init(&tauDBAttr);  
  pthread_mutex_init(&tauDBMutex, &tauDBAttr); 
  
  //cout <<" Initialized the functionDB Mutex data " <<endl;
  return 1;
}

////////////////////////////////////////////////////////////////////////
// LockDB locks the mutex protecting TheFunctionDB() global database of 
// functions. This is required to ensure that push_back() operation 
// performed on this is atomic (and in the case of tracing this is 
// followed by a GetFunctionID() ). This is used in 
// FunctionInfo::FunctionInfoInit().
////////////////////////////////////////////////////////////////////////
int PthreadLayer::LockDB(void)
{
  static int initflag=InitializeDBMutexData();
  // Lock the functionDB mutex
  pthread_mutex_lock(&tauDBMutex);
  return 1;
}

////////////////////////////////////////////////////////////////////////
// UnLockDB() unlocks the mutex tauDBMutex used by the above lock operation
////////////////////////////////////////////////////////////////////////
int PthreadLayer::UnLockDB(void)
{
  // Unlock the functionDB mutex
  pthread_mutex_unlock(&tauDBMutex);
  return 1;
}  


////////////////////////////////////////////////////////////////////////
int PthreadLayer::InitializeEnvMutexData(void)
{
  // For locking functionEnv 
  pthread_mutexattr_init(&tauDBAttr);  
  pthread_mutex_init(&tauEnvMutex, &tauDBAttr); 
  
  //cout <<" Initialized the functionEnv Mutex data " <<endl;
  return 1;
}

////////////////////////////////////////////////////////////////////////
// LockEnv locks the mutex protecting TheFunctionEnv() global database of 
// functions. This is required to ensure that push_back() operation 
// performed on this is atomic (and in the case of tracing this is 
// followed by a GetFunctionID() ). This is used in 
// FunctionInfo::FunctionInfoInit().
////////////////////////////////////////////////////////////////////////
int PthreadLayer::LockEnv(void)
{
  static int initflag=InitializeEnvMutexData();
  // Lock the functionEnv mutex
  pthread_mutex_lock(&tauEnvMutex);
  return 1;
}

////////////////////////////////////////////////////////////////////////
// UnLockEnv() unlocks the mutex tauEnvMutex used by the above lock operation
////////////////////////////////////////////////////////////////////////
int PthreadLayer::UnLockEnv(void)
{
  // Unlock the functionEnv mutex
  pthread_mutex_unlock(&tauEnvMutex);
  return 1;
}  
/***************************************************************************
 * $RCSfile: PthreadLayer.cpp,v $   $Author: amorris $
 * $Revision: 1.10 $   $Date: 2005/11/11 03:46:49 $
 * POOMA_VERSION_ID: $Id: PthreadLayer.cpp,v 1.10 2005/11/11 03:46:49 amorris Exp $
 ***************************************************************************/


