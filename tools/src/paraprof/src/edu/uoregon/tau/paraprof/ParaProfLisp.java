/* 
   ParaProfLisp.java

   Title:      ParaProf
   Author:     Robert Bell
   Description:  This class handles all lisp activity with help from Jatha.
   Ref: http://jatha.sourceforge.net/
*/

/*
  To do: 
  1) Testing complete.  Now need to integrate the system.
*/

package edu.uoregon.tau.paraprof;

import org.jatha.Jatha;
import org.jatha.dynatype.*;
import org.jatha.compile.*;
import edu.uoregon.tau.dms.dss.*;

public class ParaProfLisp{
    public ParaProfLisp(boolean debug){
	this.lisp = new Jatha(false, false);
	this.lisp.init();
	this.lisp.start();
	this.debug = debug;
    }

    public void register(LispPrimitive lispPrimitive){
	lisp.COMPILER.Register(lispPrimitive);}

    public void registerParaProfPrimitives(){
	DssIterator l = ParaProfLispPrimitives.getPrimitiveList(lisp, this.debug());
	while(l.hasNext()){
	    this.register((LispPrimitive)l.next());}
    }

    public String eval(String s){
	String value = null;
	try{
	    LispValue input = this.lisp.parse(s);
	    LispValue result = this.lisp.eval(input);
	    value = result.toString();
	}
	catch(Exception e){
	    System.err.println("LISP Exception: " + e);
	}
	return value;
    }

    public void setDebug(boolean debug){
	this.debug = debug;}
    
    public boolean debug(){
	return debug;}

    //####################################
    //Instance Data
    //####################################
    Jatha lisp = null;
    private boolean debug = false; //Off by default.
    //####################################
    //Instance Data
    //####################################
}
