/* 
   ParaProfApplication.java

   Title:      ParaProf
   Author:     Robert Bell
   Description:  
*/

package paraprof;

import java.util.*;
import javax.swing.tree.*;
import dms.dss.*;

public class ParaProfApplication extends Application implements ParaProfTreeNodeUserObject{

    public ParaProfApplication(){
	super();
	this.setID(-1);
	this.setName("");
	this.setVersion("");
	this.setDescription("");
	this.setLanguage("");
	this.setParaDiag("");
	this.setUsage("");
	this.setExecutableOptions("");
	this.setUserData("");
    }

    public ParaProfApplication(Application application){
	super();
	this.setID(application.getID());
	this.setName(application.getName());
	this.setVersion(application.getVersion());
	this.setDescription(application.getDescription());
	this.setLanguage(application.getLanguage());
	this.setParaDiag(application.getParaDiag());
	this.setUsage(application.getUsage());
	this.setExecutableOptions(application.getExecutableOptions());
	this.setUserData(application.getUserData());
    }
  
    public void setDMTN(DefaultMutableTreeNode defaultMutableTreeNode){
	this.defaultMutableTreeNode = defaultMutableTreeNode;}
  
    public DefaultMutableTreeNode getDMTN(){
	return defaultMutableTreeNode;}

    public void setTreePath(TreePath treePath){
	this.treePath = treePath;}

    public TreePath getTreePath(){
	return treePath;}
  
    public void setDBApplication(boolean dBApplication){
	this.dBApplication = dBApplication;}
  
    public boolean dBApplication(){
	return dBApplication;}

    public ParaProfExperiment getExperiment(int experimentID){
	return (ParaProfExperiment) experiments.elementAt(experimentID);}
  
    public Vector getExperiments(){
	return experiments;}

    public ListIterator getExperimentList(){
	return new DataSessionIterator(experiments);}
  
    public ParaProfExperiment addExperiment(){
	ParaProfExperiment experiment = new ParaProfExperiment();
	experiment.setApplication(this);
	experiment.setID((experiments.size()));
	experiments.add(experiment);
	return experiment;
    }
  
    public void removeParaProfExperiment(ParaProfExperiment experiment){
	experiments.remove(experiment);}
  
    public boolean isExperimentPresent(String name){
	for(Enumeration e = experiments.elements(); e.hasMoreElements() ;){
	    ParaProfExperiment exp = (ParaProfExperiment) e.nextElement();
	    if(name.equals(exp.getName()))
		return true;
	}
	//If we make it here, the experiment run name is not present.  Return false.
	return false;
    }

    public String getIDString(){
	return Integer.toString(this.getID());}
  
    public String toString(){ 
	return super.getName();}

    //####################################
    //Interface code.
    //####################################
    
    //######
    //ParaProfTreeUserObject
    //######
    public void clearDefaultMutableTreeNodes(){
	this.setDMTN(null);}
    //######
    //End - ParaProfTreeUserObject
    //######

    //####################################
    //End - Interface code.
    //####################################

    //####################################
    //Instance data.
    //####################################
    private DefaultMutableTreeNode defaultMutableTreeNode = null;
    private TreePath treePath = null;
    private boolean dBApplication = false;
    private Vector experiments = new Vector();
    //####################################
    //End - Instance data.
    //####################################
}
