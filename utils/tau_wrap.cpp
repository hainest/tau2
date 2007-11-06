/****************************************************************************
**                      TAU Portable Profiling Package                     **
**                      http://www.cs.uoregon.edu/research/paracomp/tau    ** 
*****************************************************************************
**    Copyright 2007                                                       **
**    Department of Computer and Information Science, University of Oregon **
**    Advanced Computing Laboratory, Los Alamos National Laboratory        **
****************************************************************************/
/***************************************************************************
**      File            : tau_wrap.cpp                                    **
**      Description     : Generates a wrapper library for external pkgs   **
**                        for instrumentation with TAU.                   **
**      Author          : Sameer Shende                                   **
**      Contact         : sameer@cs.uoregon.edu sameer@paratools.com      **
**      Documentation   :                                                 **
***************************************************************************/ 

/* Headers */
#include <stdio.h>
#include <ctype.h>
#include <string.h> 
#include <stdlib.h>
#if (!defined(TAU_WINDOWS))
#include <unistd.h>
#endif //TAU_WINDOWS

#ifdef _OLD_HEADER_
# include <fstream.h>
# include <set.h>
# include <algo.h>
# include <sstream.h>
# include <deque.h>
#else
# include <fstream> 
# include <algorithm>
# include <set> 
# include <list>
# include <string>
# include <sstream>
# include <deque>
using namespace std;
#endif
#include "pdbAll.h"
#include "tau_datatypes.h"



/* defines */
#ifdef TAU_WINDOWS
#define TAU_DIR_CHARACTER '\\' 
#else
#define TAU_DIR_CHARACTER '/' 
#endif /* TAU_WINDOWS */
 



//#define DEBUG 1
/* For selective instrumentation */
extern int processInstrumentationRequests(char *fname);
extern bool instrumentEntity(const string& function_name);
extern bool processFileForInstrumentation(const string& file_name);
extern bool isInstrumentListEmpty(void);
/* Prototypes for selective instrumentation */
extern bool addFileInstrumentationRequests(PDB& p, pdbFile *file, vector        <itemRef *>& itemvec);



/* Globals */
bool memory_flag = false;   /* by default, do not insert malloc.h in instrumented C/C++ files */
const int INBUF_SIZE = 65536;


///////////////////////////////////////////////////////////////////////////



/* -------------------------------------------------------------------------- */
/* -- Fuzzy Match. Allows us to match files that don't quite match properly, 
 * but infact refer to the same file. For e.g., /home/pkg/foo.cpp and ./foo.cpp
 * or foo.cpp and ./foo.cpp. This routine allows us to match such files! 
 * -------------------------------------------------------------------------- */
bool fuzzyMatch(const string& a, const string& b)
{ /* This function allows us to match string like ./foo.cpp with
     /home/pkg/foo.cpp */
  if (a == b)
  { /* the two files do match */
#ifdef DEBUG
    cout <<"fuzzyMatch returns true for "<<a<<" and "<<b<<endl;
#endif /* DEBUG */
    return true;
  }
  else 
  { /* fuzzy match */
    /* Extract the name without the / character */
    int loca = a.find_last_of(TAU_DIR_CHARACTER);
    int locb = b.find_last_of(TAU_DIR_CHARACTER);

    /* truncate the strings */
    string trunca(a,loca+1);
    string truncb(b,locb+1);
    /*
    cout <<"trunca = "<<trunca<<endl;
    cout <<"truncb = "<<truncb<<endl;
    */
    if (trunca == truncb) 
    {
#ifdef DEBUG
      cout <<"fuzzyMatch returns true for "<<a<<" and "<<b<<endl;
#endif /* DEBUG */
      return true;
    }
    else
    {
#ifdef DEBUG
      cout <<"fuzzyMatch returns false for "<<a<<" and "<<b<<endl;
#endif /* DEBUG */
      return false;
    }
  }
}
///////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////

/* -------------------------------------------------------------------------- */
/* -- Get a list of instrumentation points for a C program ------------------ */
/* -------------------------------------------------------------------------- */
/* Create a vector of items that need action: such as BODY_BEGIN, RETURN etc.*/
void getCReferencesForWrapper(vector<itemRef *>& itemvec, PDB& pdb, pdbFile *file) {

  /* moved selective instrumentation file processing here */
  if (!isInstrumentListEmpty()) 
  { /* there are finite instrumentation requests, add requests for this file */
    addFileInstrumentationRequests(pdb, file, itemvec);
  }
}

#ifdef OLD
{
  /* we used to keep the selective instrumentation file processing at the
     entry. But, when a routine is specified as a phase, we need to annotate
     its itemRef accordingly. This needs the entry/exit records to be created
     prior to processing the selective instrumentation file. N/A for wrappers
     as there are no entry/exit records created.*/

  PDB::croutinevec routines = pdb.getCRoutineVec();
  for (PDB::croutinevec::const_iterator rit=routines.begin();
       rit!=routines.end(); ++rit)
  {
    pdbRoutine::locvec retlocations = (*rit)->returnLocations();
    if ( (*rit)->location().file() == file && !(*rit)->isCompilerGenerated()
	 && (instrumentEntity((*rit)->fullName())) )
    {
        itemvec.push_back(new itemRef(*rit, BODY_BEGIN,
                (*rit)->bodyBegin().line(), (*rit)->bodyBegin().col()));
#ifdef DEBUG
        cout <<" Location begin: "<< (*rit)->location().line() << " col "
             << (*rit)->location().col() <<endl;
        cout <<" Location head Begin: "<< (*rit)->headBegin().line() << " col "             << (*rit)->headBegin().col() <<endl;
        cout <<" Location head End: "<< (*rit)->headEnd().line() << " col "
             << (*rit)->headEnd().col() <<endl;
        cout <<" Location body Begin: "<< (*rit)->bodyBegin().line() << " col "             << (*rit)->bodyBegin().col() <<endl;
        cout <<" Location body End: "<< (*rit)->bodyEnd().line() << " col "
             << (*rit)->bodyEnd().col() <<endl;
#endif /* DEBUG */
        for(pdbRoutine::locvec::iterator rlit = retlocations.begin();
           rlit != retlocations.end(); rlit++)
        {
#ifdef DEBUG 
          cout <<" Return Locations : "<< (*rlit)->line() << " col "
             << (*rlit)->col() <<endl;
#endif /* DEBUG */
          itemvec.push_back(new itemRef(*rit, RETURN,
                (*rlit)->line(), (*rlit)->col()));
        }
        itemvec.push_back(new itemRef(*rit, BODY_END,
                (*rit)->bodyEnd().line(), (*rit)->bodyEnd().col()));
#ifdef DEBUG 
        cout <<" Return type: " << (*rit)->signature()->returnType()->name()<<endl;
        cout <<" Routine name: "<<(*rit)->name() <<" Signature: " <<
                (*rit)->signature()->name() <<endl;
#endif /* DEBUG */

	/* See if the current routine calls exit() */
	pdbRoutine::callvec c = (*rit)->callees(); 
    }
  }

  /* All instrumentation requests are in. Sort these now and remove duplicates */
#ifdef DEBUG
  for(vector<itemRef *>::iterator iter = itemvec.begin(); iter != itemvec.end();
   iter++)
  {
    cout <<"Before SORT: Items ("<<(*iter)->line<<", "<<(*iter)->col<<")"
	 <<"snippet = "<<(*iter)->snippet<<endl;
  }
#endif /* DEBUG */
#ifdef DEBUG
  for(vector<itemRef *>::iterator iter = itemvec.begin(); iter != itemvec.end();
   iter++)
  {
    cout <<"Items ("<<(*iter)->line<<", "<<(*iter)->col<<")"
	 <<"snippet = "<<(*iter)->snippet<<endl;
  }
#endif /* DEBUG */
}
#endif /* OLD - delete */

bool isReturnTypeVoid(pdbRoutine *r)
{
  if (strcmp(r->signature()->returnType()->name().c_str(), "void") == 0)
  {
#ifdef DEBUG
     cout <<"Return type is void for "<<r->name()<<endl;
#endif /* DEBUG */
     return true;
  }
  else 
     return false;
}

void  printRoutineInOutputFile(pdbRoutine *r, ofstream& header, ofstream& impl, string& group_name)
{
  string macro("#define ");
  string func(r->name());
  string proto(r->name());
  func.append("(");
  proto.append("(");
  impl<<r->signature()->returnType()->name()<<"  tau_"; /* put in return type */
  impl<<func;
#ifdef DEBUG
  cout <<"Examining "<<r->name()<<endl;
  cout <<"Return type :"<<r->signature()->returnType()->name()<<endl;
#endif /* DEBUG */
  pdbType::argvec av = r->signature()->arguments();
  int argcount = 1;
  bool isVoid = isReturnTypeVoid(r);
  for(pdbType::argvec::const_iterator argsit = av.begin();
      argsit != av.end(); argsit++, argcount++)
  {
    char number[256];
#ifdef DEBUG
    cout <<"Argument "<<(*argsit).name()<<" Type "<<(*argsit).type()->name()<<endl;
#endif /* DEBUG */
    if (argcount != 1) { /* not a startup */
      func.append(", ");
      proto.append(", ");
      impl<<", ";
    }
    sprintf(number, "%d", argcount);
    proto.append((*argsit).type()->name());
    proto.append(" ");
    proto.append(string("a")+string(number));
    func.append(string("a")+string(number));
    impl<<(*argsit).type()->name()<<" ";
    impl<<"a"<<number;
  }
  func.append(")");
  proto.append(")");
  impl<<") {" <<endl<<endl;
  if (!isVoid)
  {
    impl<<"  "<<r->signature()->returnType()->name()<< " retval;"<<endl;
  }
  /* Now put in the body of the routine */
  impl<<"  TAU_PROFILE_TIMER(t,\""<<r->fullName()<<"\", \"\", "
      <<group_name<<");"<<endl;
  impl<<"  TAU_PROFILE_START(t);"<<endl<<endl;
  if (!isVoid)
  {
    impl<<"  retval  =";
  }
  impl<<"  "<<func<<";"<<endl;
  impl<<"  TAU_PROFILE_STOP(t);"<<endl;
  if (!isVoid)
  {
    impl<<"  return retval;"<<endl;
  }
  impl<<endl;

  impl<<"}"<<endl<<endl;
  macro.append(" "+func+" " +"tau_"+func);
#ifdef DEBUG
  cout <<"macro = "<<macro<<endl;
  cout <<"func = "<<func<<endl;
#endif /* DEBUG */

  /* The macro goes in header file, the implementation goes in the other file */
  header <<macro<<endl;  
  header <<"extern "<<r->signature()->returnType()->name()<<" tau_"<<proto<<";"<<endl<<endl;


}
/* -------------------------------------------------------------------------- *//* -- Instrumentation routine for a C program ------------------------------- *//* -------------------------------------------------------------------------- */bool instrumentCFile(PDB& pdb, pdbFile* f, ofstream& header, ofstream& impl, string& group_name, string& header_file)
{
  //static int firsttime=0;
  int inbufLength, i, j, space;
  string file(f->name());
  static char inbuf[INBUF_SIZE]; // to read the line
  string exit_expression;
  bool abort_used = false;
  char newline;
  newline = '\n'; /* for C \ processing in return statements */
  // open outfile for instrumented version of source file
  
  // open source file
  ifstream istr(file.c_str());
  if (!istr) {
    cerr << "Error: Cannot open '" << file << "'" << endl;
    return false;
  }
#ifdef DEBUG
  cout << "Processing " << file << " in instrumentCFile..." << endl;
#endif

  header <<"#ifndef _TAU_"<<file<<"_H_"<<endl;
  header <<"#define _TAU_"<<file<<"_H_"<<endl<<endl;
  header <<"#include <../"<<file<<">"<<endl<<endl;
  memset(inbuf, INBUF_SIZE, 0); // reset to zero
  // initialize reference vector
  vector<itemRef *> itemvec;
  getCReferencesForWrapper(itemvec, pdb, f);
  PDB::croutinevec routines = pdb.getCRoutineVec();
  for (PDB::croutinevec::const_iterator rit=routines.begin();
       rit!=routines.end(); ++rit)   {
    pdbRoutine::locvec retlocations = (*rit)->returnLocations();
    if ( (*rit)->location().file() == f && !(*rit)->isCompilerGenerated()
         && (instrumentEntity((*rit)->fullName())) )
    {
       printRoutineInOutputFile(*rit, header, impl, group_name);

    }
  }
  header <<"#endif /*  _TAU_"<<file<<"_H_ */"<<endl;


}

/* -------------------------------------------------------------------------- */
/* -- Instrument the program using C, C++ or F90 instrumentation routines --- */
/* -------------------------------------------------------------------------- */
int main(int argc, char **argv)
{
  string outFileName("out.ins.C");
  string group_name("TAU_USER"); /* Default: if nothing else is defined */
  string header_file("Profile/Profiler.h");
  bool retval;
        /* Default: if nothing else is defined */

  if (argc < 3)
  {
    cout <<"Usage : "<<argv[0] <<" <pdbfile> <sourcefile> [-o <outputfile>] [-g groupname] [-i headerfile] [-c|-c++|-fortran] [-f <instr_req_file> ]"<<endl;
    cout<<"----------------------------------------------------------------------------------------------------------"<<endl;
  }
  PDB p(argv[1]); if ( !p ) return 1;
  /* setGroupName(p, group_name); */
  bool outFileNameSpecified = false;
  int i;
  const char *filename;
  for(i=0; i < argc; i++)
  {
    switch(i) 
    {
      case 0:
#ifdef DEBUG
        printf("Name of pdb file = %s\n", argv[1]);
#endif /* DEBUG */
        break;
      case 1:
#ifdef DEBUG
        printf("Name of source file = %s\n", argv[2]);
#endif /* DEBUG */
        filename = argv[2];
        break;
      default:
        if (strcmp(argv[i], "-o")== 0)
        {
          ++i;
#ifdef DEBUG
          printf("output file = %s\n", argv[i]);
#endif /* DEBUG */
          outFileName = string(argv[i]);
          outFileNameSpecified = true;
        }

        if (strcmp(argv[i], "-g") == 0)
        {
          ++i;
          group_name = string("TAU_GROUP_")+string(argv[i]);
#ifdef DEBUG
          printf("Group %s\n", group_name.c_str());
#endif /* DEBUG */
        }
        if (strcmp(argv[i], "-i") == 0)
        {
          ++i;
          header_file = string(argv[i]);
#ifdef DEBUG
          printf("Header file %s\n", header_file.c_str());
#endif /* DEBUG */
        }
        if (strcmp(argv[i], "-f") == 0)
        {
          ++i;
          processInstrumentationRequests(argv[i]);
#ifdef DEBUG
          printf("Using instrumentation requests file: %s\n", argv[i]);
#endif /* DEBUG */
        }
        break; /* end of default case */
    }
  }

  if (!outFileNameSpecified)
  { /* if name is not specified on the command line */
    outFileName = string(filename + string(".ins"));
  }
  /* should we make a directory and put it in there? */
  system("mkdir -p wrapper");
  ofstream impl(string("wrapper/"+outFileName).c_str()); /* actual implementation goes in here */
  ofstream header(string("wrapper/"+string(filename)).c_str()); /* use the same file name as the original */
  if (!impl) {
    cerr << "Error: Cannot open '" << outFileName << "'" << endl;
    return false;
  }
  if (!header) {
    cerr << "Error: Cannot open wrapper/" <<filename  << "" << endl;
    return false;
  }
  /* files created properly */
  //header <<"#include <"<<filename<<">"<<endl;
  impl <<"#include <"<<filename<<">"<<endl;
  impl <<"#include <"<<header_file<<">"<<endl; /* Profile/Profiler.h */

  bool instrumentThisFile;
  bool fuzzyMatchResult;
  bool fileInstrumented = false;
  for (PDB::filevec::const_iterator it=p.getFileVec().begin();
       it!=p.getFileVec().end(); ++it)
  {
     /* reset this variable at the beginning of the loop */
     instrumentThisFile = false;

     /* for headers, we should only use the processFileForInstrumentation check */
/*
     if ((fuzzyMatchResult = fuzzyMatch((*it)->name(), string(filename))) &&
         (instrumentThisFile = processFileForInstrumentation(string(filename))))     
*/
     if (instrumentThisFile = processFileForInstrumentation(string(filename)))
     { /* should we instrument this file? Yes */
       instrumentCFile(p, *it, header, impl, group_name, header_file);
     }
  }

} /* end of main */






///////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////

/* EOF */

/***************************************************************************
 * $RCSfile: tau_wrap.cpp,v $   $Author: sameer $
 * $Revision: 1.6 $   $Date: 2007/11/06 22:39:49 $
 * VERSION_ID: $Id: tau_wrap.cpp,v 1.6 2007/11/06 22:39:49 sameer Exp $
 ***************************************************************************/

