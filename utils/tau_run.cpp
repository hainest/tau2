/****************************************************************************
**			TAU Portable Profiling Package			   **
**			http://www.acl.lanl.gov/tau		           **
*****************************************************************************
**    Copyright 1999  						   	   **
**    Department of Computer and Information Science, University of Oregon **
**    Advanced Computing Laboratory, Los Alamos National Laboratory        **
*****************************************************************************
**    Description: tau_run is a utility program that spawns a user 	   **
**		   application and instruments it using DynInst package    **
**		   (from U. Maryland). 					   **
**		   Profile/trace data is generated as the program executes **
****************************************************************************/
 
#include <iostream.h>
#include <stdio.h>
#include <string.h>
#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)
#include <unistd.h>
#endif

#define FUNCNAMELEN 32*1024
#ifdef i386_unknown_nt4_0
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#include <string>
using namespace std;

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"
//#include "test_util.h"

int debugPrint = 0;

template class BPatch_Vector<BPatch_variableExpr*>;
void checkCost(BPatch_snippet snippet);

BPatch *bpatch;

// control debug printf statements
#define dprintf if (debugPrint) printf


#define NO_ERROR -1

int expectError = NO_ERROR;

//
// Error callback routine. 
//
void errorFunc(BPatchErrorLevel level, int num, const char **params)
{
    char line[256];

    const char *msg = bpatch->getEnglishErrorString(num);
    bpatch->formatErrorString(line, sizeof(line), msg, params);

    if (num != expectError) {
        printf("Error #%d (level %d): %s\n", num, level, line);

        // We consider some errors fatal.
        if (num == 101) {
            exit(-1);
        }
    }
}


//
// invokeRoutineInFunction calls routine "callee" with no arguments when 
// Function "function" is invoked at the point given by location
//
BPatchSnippetHandle *invokeRoutineInFunction(BPatch_thread *appThread,
	BPatch_image *appImage, BPatch_function *function, 
	BPatch_procedureLocation loc, BPatch_function *callee, 
	BPatch_Vector<BPatch_snippet *> *callee_args)
{

    // First create the snippet using the callee and the args 
    const BPatch_snippet *snippet = new BPatch_funcCallExpr(*callee, *callee_args);

    if (snippet == NULL) {
        fprintf(stderr, "Unable to create snippet to call callee\n");
        exit(1);
    }

    // Then find the points using loc (entry/exit) for the given function
    const BPatch_Vector<BPatch_point *> *points = function->findPoint(loc); 

    // Insert the given snippet at the given point 
    if (loc == BPatch_entry)
    { 
      appThread->insertSnippet(*snippet, *points, BPatch_callAfter);
    }
    else
    { // the default is BPatch_callBefore which is used for exit
      appThread->insertSnippet(*snippet, *points);
    }
    delete snippet; // free up space
      

}

// 
// Initialize calls TauInitCode, the initialization routine in the user
// application. It is executed exactly once, before any other routine.
//
void Initialize(BPatch_thread *appThread, BPatch_image *appImage, 
	BPatch_Vector<BPatch_snippet *>& initArgs)
{
  // Find the initialization function and call it
  BPatch_function *call_func = appImage->findFunction("TauInitCode");
  if (call_func == NULL) 
  {
    fprintf(stderr, "Unable to find function TauInitCode\n");
    exit(1);
  }

  BPatch_funcCallExpr call_Expr(*call_func, initArgs);
  // checkCost(call_Expr);
  appThread->oneTimeCode(call_Expr);
}

// FROM TEST3

// check that the cost of a snippet is sane.  Due to differences between
//   platforms, it is impossible to check this exactly in a machine independent
//   manner.
void checkCost(BPatch_snippet snippet)
{
    float cost;
    BPatch_snippet copy;

    // test copy constructor too.
    copy = snippet;

    cost = snippet.getCost();
    if (cost < 0.0) {
        printf("*Error*: negative snippet cost\n");
    } else if (cost == 0.0) {
        printf("*Warning*: zero snippet cost\n");
    } else if (cost > 0.01) {
        printf("*Error*: snippet cost of %f, exceeds max expected of 0.1",
            cost);
    }

}

int errorPrint = 0; // external "dyninst" tracing
void errorFunc1(BPatchErrorLevel level, int num, const char **params)
{
    if (num == 0) {
        // conditional reporting of warnings and informational messages
        if (errorPrint) {
            if (level == BPatchInfo)
              { if (errorPrint > 1) printf("%s\n", params[0]); }
            else
                printf("%s", params[0]);
        }
    } else {
        // reporting of actual errors
        char line[256];
        const char *msg = bpatch->getEnglishErrorString(num);
        bpatch->formatErrorString(line, sizeof(line), msg, params);
        
        if (num != expectError) {
            printf("Error #%d (level %d): %s\n", num, level, line);
        
            // We consider some errors fatal.
            if (num == 101) {
               exit(-1);
            }
        }
    }
}
// END OF TEST3 code
 
// Constraints for instrumentation 
int moduleConstraint(char *fname)
{ // fname is the name of module/file 

  if ((strcmp(fname, "DEFAULT_MODULE") == 0) ||
      (strcmp(fname, "LIBRARY_MODULE") == 0))
  {
    return true;
  }
  else
  {
    return false;
  }
}

// Constriant for routines 
int routineConstraint(char *fname)
{ // fname is the function name
  if ((strncmp(fname, "Tau", 3) == 0) ||
            (strncmp(fname, "Profiler", 8) == 0) ||
            (strstr(fname, "FunctionInfo") != 0) ||
            (strncmp(fname, "RtsLayer", 8) == 0) ||
            (strncmp(fname, "DYNINST", 7) == 0) ||
            (strncmp(fname, "PthreadLayer", 12) == 0) ||
            (strncmp(fname, "threaded_func", 13) == 0) ||
            (strncmp(fname, "The", 3) == 0)) 
  {
    return true;
  }
  else
  { 
    return false; 
  }
}

//
// entry point 
//
int main(int argc, char **argv)
{
  int i,j, answer;
  int instrumented=0;
  bool loadlib=false;
  char fname[FUNCNAMELEN], libname[FUNCNAMELEN];
  BPatch_thread *appThread;
  bpatch = new BPatch; // create a new version. 
  string functions;

  // parse the command line arguments 
  if ( argc < 2 )
  {
    fprintf (stderr, "usage: %s [-Xrun<Taulibrary> ] <application> [args]\n", argv[0]);
    fprintf (stderr, "%s instruments and executes <application> to generate performance data\n", argv[0]);
    fprintf (stderr, "e.g., \n");
    fprintf (stderr, "%%%s -XrunTAU a.out 100 \n", argv[0]);
    fprintf (stderr, "Loads libTAU.so from $LD_LIBRARY_PATH and executes a.out \n"); 
    exit (1);
  }
  else if ( strncasecmp (argv[1], "-Xrun", 5) == 0 )
  { // Load the library.
    loadlib = true;
    sprintf(libname,"lib%s.so", &argv[1][5]);
    fprintf(stderr, "%s> Loading %s ...\n", argv[0], libname);
    argv++;
  }


  // Register a callback function that prints any error messages
  bpatch->registerErrorCallback(errorFunc1);

  dprintf("Before createProcess\n");
  /* Specially added to disable Dyninst 2.0 feature of debug parsing. We were
     getting an assertion failure under Linux otherwise */
  bpatch->setDebugParsing(false);
  appThread = bpatch->createProcess(argv[1], &argv[1] , NULL);
  dprintf("After createProcess\n");
  if (!appThread)
  { 
    dprintf("tau_run> createProcess failed\n");
    exit(1);
  }
  BPatch_image *appImage = appThread->getImage();

  if (appThread == NULL)
  { 
    cout <<"create Process failed" << endl;
  }
  // Load the TAU library that has entry and exit routines.

  if (loadlib == true)
  {
    if (appThread->loadLibrary(libname) == true)
    {  
      dprintf("DSO loaded properly\n");
      bool found = false;
      BPatch_Vector<BPatch_module *> *m = appImage->getModules();
      for (i = 0; i < m->size(); i++) {
        char name[FUNCNAMELEN];
        (*m)[i]->getName(name, sizeof(name));
        if (strcmp(name, libname) == 0) {
  	found = true;
          break;
        } 
      }
      if (found) {
  	dprintf("%s loaded properly\n", libname);
      }
    }
    else
    {
      dprintf("%s not loaded properly\n", libname);
    }
  } // loadlib == true



  BPatch_Vector<BPatch_module *> *m = appImage->getModules();

  BPatch_function *inFunc;
  BPatch_function *enterstub = appImage->findFunction("TauRoutineEntry");
  BPatch_function *exitstub = appImage->findFunction("TauRoutineExit");
  BPatch_Vector<BPatch_snippet *> initArgs;
  
  char modulename[256];
  for (j=0; j < m->size(); j++) {
    sprintf(modulename, "Module %s\n", (*m)[j]->getName(fname, FUNCNAMELEN));
    BPatch_Vector<BPatch_function *> *p = (*m)[j]->getProcedures();
    dprintf("%s", modulename);



    if (moduleConstraint(fname)) 
    { // constraint 

      for (i=0; i < p->size(); i++) 
      {
        // For all procedures within the module, iterate  
        //memset(fname, 0x0, FUNCNAMELEN); 
        (*p)[i]->getName(fname, FUNCNAMELEN);
        dprintf("Name %s\n", fname);
        if (routineConstraint(fname))
        { // The above procedures shouldn't be instrumented
           dprintf("don't instrument %s\n", fname);
        } /* Put the constraints above */ 
        else 
        { // routines that are ok to instrument
         
	  functions.append("|");
	  functions.append(fname);

	}
      }
    } /* DEFAULT */
  } /* Module */

  // form the args to InitCode
  BPatch_constExpr funcName(functions.c_str());
  initArgs.push_back(&funcName);


  Initialize(appThread, appImage, initArgs);
  dprintf("Did Initialize\n");

  /* In our tests, the routines started execution concurrently with the 
     one time code. To avoid this, we first start the one time code and then
     iterate through the list of routines to select for instrumentation and
     instrument these. So, we need to iterate twice. */


  for (j=0; j < m->size(); j++) {
    sprintf(modulename, "Module %s\n", (*m)[j]->getName(fname, FUNCNAMELEN));
    BPatch_Vector<BPatch_function *> *p = (*m)[j]->getProcedures();
    dprintf("%s", modulename);


    if (moduleConstraint(fname)) 
    { // constraint

      for (i=0; i < p->size(); i++)
      {
        // For all procedures within the module, iterate
        //memset(fname, 0x0, FUNCNAMELEN);
        (*p)[i]->getName(fname, FUNCNAMELEN);
        dprintf("Name %s\n", fname);
  	if (routineConstraint(fname))
        { // The above procedures shouldn't be instrumented
           dprintf("don't instrument %s\n", fname);
        } /* Put the constraints above */
        else
        { // routines that are ok to instrument

 	  dprintf("Assigning id %d to %s\n", instrumented, fname);
          instrumented ++;
          BPatch_Vector<BPatch_snippet *> *callee_args = new BPatch_Vector<BPatch_snippet *>();
          BPatch_constExpr *constExpr = new BPatch_constExpr(instrumented);
	  // Don't pass name
          //BPatch_constExpr *constName = new BPatch_constExpr(fname);
          callee_args->push_back(constExpr);
          // callee_args->push_back(constName);
    
          inFunc = (*p)[i];
          invokeRoutineInFunction(appThread, appImage, inFunc, BPatch_entry, enterstub, callee_args);
          invokeRoutineInFunction(appThread, appImage, inFunc, BPatch_exit, exitstub, callee_args);

          delete callee_args;
          delete constExpr;
        } // routines that are ok to instrument
      } // for procedures 

    } // module constraint

  } // for modules


  dprintf("Executing...\n");
  appThread->continueExecution();
  
  while (!appThread->isTerminated())
        bpatch->waitForStatusChange();

  if (appThread->isTerminated()) {
        dprintf("End of application\n");
  }

  return 0;
}

// EOF tau_run.cpp
