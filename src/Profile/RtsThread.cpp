/****************************************************************************
**			TAU Portable Profiling Package			   **
**			http://www.acl.lanl.gov/tau		           **
*****************************************************************************
**    Copyright 1997  						   	   **
**    Department of Computer and Information Science, University of Oregon **
**    Advanced Computing Laboratory, Los Alamos National Laboratory        **
****************************************************************************/
/***************************************************************************
**	File 		: RtsThread.cpp				  **
**	Description 	: TAU Profiling Package RTS Layer definitions     **
**			  for supporting threads 			  **
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
**	Documentation	: See http://www.acl.lanl.gov/tau	          **
***************************************************************************/


//////////////////////////////////////////////////////////////////////
// Include Files 
//////////////////////////////////////////////////////////////////////

#ifdef TAU_DOT_H_LESS_HEADERS
#include <iostream>
using namespace std;
#else /* TAU_DOT_H_LESS_HEADERS */
#include <iostream.h>
#endif /* TAU_DOT_H_LESS_HEADERS */
#include "Profile/Profiler.h"
#include "Profile/OpenMPLayer.h"

#ifdef TRACING_ON
#define PCXX_EVENT_SRC
#include "Profile/pcxx_events.h"
void TraceCallStack(int tid, Profiler *current);
#endif // TRACING_ON


//////////////////////////////////////////////////////////////////////
// myNode() returns the current node id (0..N-1)
//////////////////////////////////////////////////////////////////////
int RtsLayer::myNode(void)
{
  return TheNode();
}


//////////////////////////////////////////////////////////////////////
// myContext() returns the current context id (0..N-1)
//////////////////////////////////////////////////////////////////////
int RtsLayer::myContext(void)
{
  return TheContext(); 
}

//////////////////////////////////////////////////////////////////////
// myNode() returns the current node id (0..N-1)
//////////////////////////////////////////////////////////////////////
int RtsLayer::myThread(void)
{
#ifdef PTHREADS
  return PthreadLayer::GetThreadId();
#elif  TAU_SPROC
  return SprocLayer::GetThreadId();
#elif  TAU_WINDOWS
  return WindowsThreadLayer::GetThreadId();
#elif  TULIPTHREADS
  return TulipThreadLayer::GetThreadId();
#elif JAVA
  return JavaThreadLayer::GetThreadId(); 
	// C++ app shouldn't use this unless there's a VM
#elif TAU_OPENMP
  return OpenMPLayer::GetThreadId();
#else  // if no other thread package is available 
  return 0;
#endif // PTHREADS
}


//////////////////////////////////////////////////////////////////////
// RegisterThread is called before any other profiling function in a 
// thread that is spawned off
//////////////////////////////////////////////////////////////////////
void RtsLayer::RegisterThread()
{ /* Check the size of threads */
  LockEnv();
  static int numthreads = 1;
  numthreads ++;
  if (numthreads >= TAU_MAX_THREADS)
  {
    fprintf(stderr, "TAU: RtsLayer: Max thread limit (%d) exceeded. Please re-configure TAU with -useropt=-DTAU_MAX_THREADS=<higher limit>\n", numthreads);
  }
  UnLockEnv();

#ifdef PTHREADS
  PthreadLayer::RegisterThread();
#elif TAU_SPROC
  SprocLayer::RegisterThread();
#elif  TAU_WINDOWS
  WindowsThreadLayer::RegisterThread();
#elif  TULIPTHREADS
  TulipThreadLayer::RegisterThread();
#elif TAU_OPENMP
  OpenMPLayer::RegisterThread();
#endif // PTHREADS
// Note: Java thread registration is done at the VM layer in TauJava.cpp
  return;
}


//////////////////////////////////////////////////////////////////////
// RegisterFork is called before any other profiling function in a 
// process that is forked off (child process)
//////////////////////////////////////////////////////////////////////
void RtsLayer::RegisterFork(int nodeid, enum TauFork_t opcode)
{
#ifdef PROFILING_ON
  vector<FunctionInfo*>::iterator it;
  Profiler *current;
#endif // PROFILING_ON

  TAU_PROFILE_SET_NODE(nodeid);
  // First, set the new id 
  if (opcode == TAU_EXCLUDE_PARENT_DATA)
  {
  // If opcode is TAU_EXCLUDE_PARENT_DATA then we clear out the 
  // previous values in the TheFunctionDB()

  // Get the current time
#ifndef TAU_MULTIPLE_COUNTERS
     double CurrentTimeOrCounts = getUSecD(myThread());
#else //TAU_MULTIPLE_COUNTERS
     double CurrentTimeOrCounts[MAX_TAU_COUNTERS];
     for(int i=0;i<MAX_TAU_COUNTERS;i++){
       CurrentTimeOrCounts[i]=0;
     }
     getUSecD(myThread(), CurrentTimeOrCounts);
#endif//TAU_MULTIPLE_COUNTERS
     for (int tid = 0; tid < TAU_MAX_THREADS; tid++)
     { // For each thread of execution 
#ifdef PROFILING_ON
       for(it=TheFunctionDB().begin(); it!=TheFunctionDB().end(); it++)
       { // Iterate through each FunctionDB item 
	 // Clear all values 
	 (*it)->SetCalls(tid, 0);
	 (*it)->SetSubrs(tid, 0);
#ifndef TAU_MULTIPLE_COUNTERS
	 (*it)->SetExclTime(tid, 0);
	 (*it)->SetInclTime(tid, 0);
#else //TAU_MULTIPLE_COUNTERS
         (*it)->SetExclTimeZero(tid);
         (*it)->SetInclTimeZero(tid);
#endif//TAU_MULTIPLE_COUNTERS
#ifdef PROFILE_STATS
	 (*it)->SetSumExclSqr(tid, 0);
#endif // PROFILE_STATS 
	/* Do we need to change AlreadyOnStack? No*/
	DEBUGPROFMSG("FI Zap: Inside "<< (*it)->GetName() <<endl;);
       }
       // Now that the FunctionDB is cleared, we need to add values to it 
       //	corresponding to the present state.
       current = Profiler::CurrentProfiler[tid];
       while (current != 0) 
       { // Iterate through each profiler on the callstack and 
	 // fill Values in it 
	 DEBUGPROFMSG("P Correct: Inside "<< current->ThisFunction->GetName() 
			<<endl;);
	 current->ThisFunction->IncrNumCalls(tid);
	 if (current->ParentProfiler != 0)
	 { // Increment the number of called functions in its parent
	   current->ParentProfiler->ThisFunction->IncrNumSubrs(tid);
	 }
#ifndef TAU_MULTIPLE_COUNTERS
	 current->StartTime = CurrentTimeOrCounts;
#else //TAU_MULTIPLE_COUNTERS
	 for(int j=0;j<MAX_TAU_COUNTERS;j++){
	     current->StartTime[j] = CurrentTimeOrCounts[j];
	 }
#endif//TAU_MULTIPLE_COUNTERS
#if ( defined(PROFILE_CALLS) || defined(PROFILE_STATS) || defined(PROFILE_CALLSTACK) )
	 current->ExclTimeThisCall = 0;
#endif   //  PROFILE_CALLS || PROFILE_STATS || PROFILE_CALLSTACK
	 current = current->ParentProfiler;
       } // Until the top of the stack
#endif   // PROFILING_ON
#ifdef TRACING_ON    
       DEBUGPROFMSG("Tracing Correct: "<<endl;);
       TraceUnInitialize(tid); // Zap the earlier contents of the trace buffer  
       TraceCallStack(tid, Profiler::CurrentProfiler[tid]); 
#endif   // TRACING_ON
     } // for tid loop
     // DONE! 
   }
   // If it is TAU_INCLUDE_PARENT_DATA then there's no need to do anything.
   // fork would copy over all the parent data as it is. 
}
	 
	
     
//////////////////////////////////////////////////////////////////////
// This ensure that the FunctionDB (global) is locked while updating
//////////////////////////////////////////////////////////////////////

void RtsLayer::LockDB(void)
{
#ifdef PTHREADS
  PthreadLayer::LockDB();
#elif TAU_SPROC
  SprocLayer::LockDB();
#elif  TAU_WINDOWS
  WindowsThreadLayer::LockDB();
#elif  TULIPTHREADS
  TulipThreadLayer::LockDB();
#elif  JAVA
  JavaThreadLayer::LockDB();
#elif TAU_OPENMP
  OpenMPLayer::LockDB();
#endif // PTHREADS
  return ; // do nothing if threads are not used
}


//////////////////////////////////////////////////////////////////////
// This ensure that the FunctionDB (global) is locked while updating
//////////////////////////////////////////////////////////////////////
void RtsLayer::UnLockDB(void)
{
#ifdef PTHREADS
  PthreadLayer::UnLockDB();
#elif TAU_SPROC
  SprocLayer::UnLockDB();
#elif  TAU_WINDOWS
  WindowsThreadLayer::UnLockDB();
#elif  TULIPTHREADS
  TulipThreadLayer::UnLockDB();
#elif JAVA
  JavaThreadLayer::UnLockDB();
#elif TAU_OPENMP
  OpenMPLayer::UnLockDB();
#endif // PTHREADS
  return;
}

//////////////////////////////////////////////////////////////////////
// This ensure that the FunctionEnv (global) is locked while updating
//////////////////////////////////////////////////////////////////////

void RtsLayer::LockEnv(void)
{
#ifdef PTHREADS
  PthreadLayer::LockEnv();
#elif TAU_SPROC
  SprocLayer::LockEnv();
#elif  TAU_WINDOWS
  WindowsThreadLayer::LockEnv();
#elif  TULIPTHREADS
  TulipThreadLayer::LockEnv();
#elif  JAVA
  JavaThreadLayer::LockEnv();
#elif TAU_OPENMP
  OpenMPLayer::LockEnv();
#endif // PTHREADS
  return ; // do nothing if threads are not used
}


//////////////////////////////////////////////////////////////////////
// This ensure that the FunctionEnv (global) is locked while updating
//////////////////////////////////////////////////////////////////////
void RtsLayer::UnLockEnv(void)
{
#ifdef PTHREADS
  PthreadLayer::UnLockEnv();
#elif TAU_SPROC
  SprocLayer::UnLockEnv();
#elif  TAU_WINDOWS
  WindowsThreadLayer::UnLockEnv();
#elif  TULIPTHREADS
  TulipThreadLayer::UnLockEnv();
#elif JAVA
  JavaThreadLayer::UnLockEnv();
#elif TAU_OPENMP
  OpenMPLayer::UnLockEnv();
#endif // PTHREADS
  return;
}


/***************************************************************************
 * $RCSfile: RtsThread.cpp,v $   $Author: sameer $
 * $Revision: 1.19 $   $Date: 2005/01/08 01:01:23 $
 * VERSION: $Id: RtsThread.cpp,v 1.19 2005/01/08 01:01:23 sameer Exp $
 ***************************************************************************/


