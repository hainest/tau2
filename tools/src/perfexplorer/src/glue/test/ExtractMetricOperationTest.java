package glue.test;

import edu.uoregon.tau.perfdmf.Trial;
import glue.ExtractMetricOperation;
import glue.PerformanceAnalysisOperation;
import glue.PerformanceResult;
import glue.TrialMeanResult;
import glue.Utilities;

import java.util.List;
import java.util.ArrayList;
import junit.framework.TestCase;

public class ExtractMetricOperationTest extends TestCase {

	public final void testProcessData() {
		Utilities.setSession("perfdmf_test");
		Trial trial = Utilities.getTrial("GTC_s_PAPI", "VN XT3", "004");
		PerformanceResult result = new TrialMeanResult(trial);
		String metric = result.getTimeMetric();
		List<String> metrics = new ArrayList<String>();
		metrics.add(metric);
		PerformanceAnalysisOperation operation = new ExtractMetricOperation(result, metrics);
		List<PerformanceResult> outputs = operation.processData();
		PerformanceResult output = outputs.get(0);
		assertNotNull(output);
		assertEquals(output.getThreads().size(), 1);
		assertEquals(output.getEvents().size(), 2721);
		assertEquals(output.getMetrics().size(), 1);
		
		for (String event : output.getEvents()) {
			for (Integer thread : output.getThreads()) {
				assertEquals(output.getExclusive(thread, event, metric), 
						result.getExclusive(thread, event, metric));
				assertEquals(output.getInclusive(thread, event, metric), 
						result.getInclusive(thread, event, metric));
				assertEquals(output.getCalls(thread, event), 
						result.getCalls(thread, event));
				assertEquals(output.getSubroutines(thread, event), 
						result.getSubroutines(thread, event));
			}
		}
	}
}