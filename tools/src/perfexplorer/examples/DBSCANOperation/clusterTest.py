from edu.uoregon.tau.perfexplorer.glue import *
from edu.uoregon.tau.perfexplorer.client import PerfExplorerModel
from java.util import *

tauData = ""

def getParameters():
	global tauData
	parameterMap = PerfExplorerModel.getModel().getScriptParameters()
	keys = parameterMap.keySet()
	tmp = parameterMap.get("tauData")
	if tmp != None:
		tauData = tmp
		print "Performance data: " + tauData
	else:
		print "TAU profile data path not specified... using current directory of profile.x.x.x files."

def loadFile(fileName):
	# load the trial
	files = []
	files.append(fileName)
	input = None
	if fileName.endswith("ppk"):
		input = DataSourceResult(DataSourceResult.PPK, files, False)
	else:
		input = DataSourceResult(DataSourceResult.TAUPROFILE, files, False)
	return input

def main():
	global filename
	print "--------------- JPython test script start ------------"
	print "doing cluster test"
	# get the parameters
	getParameters()
	# load the data
	result = loadFile(tauData)
	result.setIgnoreWarnings(True)

	# set the metric, type we are interested in
	metric = result.getTimeMetric()
	type = result.EXCLUSIVE
	
	# split communication and computation
	splitter = SplitCommunicationComputationOperation(result)
	outputs = splitter.processData()
	computation = outputs.get(SplitCommunicationComputationOperation.COMPUTATION)
	communication = outputs.get(SplitCommunicationComputationOperation.COMMUNICATION)
	#computation = result

	# do some basic statistics first
	stats = BasicStatisticsOperation(computation)
	means = stats.processData().get(BasicStatisticsOperation.MEAN)

	# then, using the stats, find the top X event names
	reducer = TopXEvents(means, metric, type, 10)
	reduced = reducer.processData().get(0)

	# then, extract those events from the actual data
	tmpEvents = ArrayList(reduced.getEvents())
	reducer = ExtractEventOperation(computation, tmpEvents)
	reduced = reducer.processData().get(0)

	# cluster the data - on calls!
	clusterer = DBSCANOperation(reduced, metric, type, 1.0)
	clusterResult = clusterer.processData()
	print "Estimated value for k:", str(clusterResult.get(0).getThreads().size())
	print "---------------- JPython test script end -------------"

if __name__ == "__main__":
	main()

