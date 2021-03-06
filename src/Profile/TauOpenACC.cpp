/****************************************************************************
 **			TAU Portable Profiling Package			   **
 **			http://www.cs.uoregon.edu/research/tau	           **
 *****************************************************************************
 **    Copyright 1997-2015 	          			   	   **
 **    Department of Computer and Information Science, University of Oregon **
 **    Advanced Computing Laboratory, Los Alamos National Laboratory        **
 ****************************************************************************/
/***************************************************************************
 **	File 		: TauOpenACC.cpp				  **
 **	Description 	: TAU Profiling Package				  **
 **	Contact		: tau-bugs@cs.uoregon.edu 		 	  **
 **	Documentation	: See http://www.cs.uoregon.edu/research/tau      **
 ***************************************************************************/

//////////////////////////////////////////////////////////////////////
// Include Files
//////////////////////////////////////////////////////////////////////



#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <openacc.h>
#include "pgi_acc_prof.h"

#ifdef CUPTI
#include <cupti.h>
#include <cuda.h>
#endif

#include <Profile/Profiler.h>
#include <Profile/TauOpenACC.h>
#include <Profile/TauGpuAdapterOpenACC.h>
#include <Profile/TauGpu.h>

#define TAU_SET_EVENT_NAME(event_name, str) strcpy(event_name, str)
////////////////////////////////////////////////////////////////////////////

// strings for OpenACC parent constructs; based on enum acc_construct_t
const char* acc_constructs[] = {
	"parallel",
	"kernels",
	"loop",
	"data",
	"enter data",
	"exit data",
	"host data",
	"atomic",
	"declare",
	"init",
	"shutdown",
	"set",
	"update",
	"routine",
	"wait",
	"runtime api",
	"serial",
};

extern "C" static void
Tau_openacc_launch_callback(acc_prof_info* prof_info, acc_event_info* event_info, acc_api_info* api_info)
{
	acc_launch_event_info* launch_event = &(event_info->launch_event);
	char file_name[256];
	char event_name[256];
	char event_data[256];
	int start = -1; // 0 = stop timer, 1 = start timer, -1 = something else; trigger event

	switch(prof_info->event_type) {
		// note: these don't correspond to when kernels are actually run, just when they're put in the
		// execution queue on the device
		case acc_ev_enqueue_launch_start:
			start = 1;
			sprintf(event_name, "OpenACC enqueue launch");
			break;
		case acc_ev_enqueue_launch_end:
			start = 0;
			break;
		default:
			start = -1;
			sprintf(event_name, "UNKNOWN OPENACC LAUNCH EVENT");
			fprintf(stderr, "ERROR: Unknown launch event passed to OpenACC launch event callback.");
	}

	// if this is an end event, short circuit by grabbing the FunctionInfo pointer out of tool_info
	// and stopping that timer; if the pointer is NULL something bad happened, print warning and kill
	// whatever timer is on top of the stack
	if (start == 0) {
		if (launch_event->tool_info == NULL) {
			fprintf(stderr, "WARNING: OpenACC launch end event has bad matching start event.");
			Tau_global_stop();
		}
		else {
			Tau_lite_stop_timer(launch_event->tool_info);
		}
		return;
	}

	sprintf(file_name, "%s:%s-%s",
			prof_info->src_file,
			(prof_info->line_no > 0) ? std::to_string(prof_info->line_no).c_str() : "?",
			(prof_info->end_line_no > 0) ? std::to_string(prof_info->end_line_no).c_str() : "?");


	sprintf(event_data, " kernel name = %s %s; parent construct = %s ; gangs=%zu, workers=%zu, vector lanes=%zu (%s)",
	//                             name ^  ^ (implicit?)                                       file and line no. ^
			(launch_event->kernel_name) ? launch_event->kernel_name : "unknown kernel",
			(launch_event->implicit) ? "(implicit)" : "",
			(launch_event->parent_construct < 9999) ? acc_constructs[launch_event->parent_construct] : "unknown construct",
			launch_event->num_gangs,
			launch_event->num_workers,
			launch_event->vector_length,
			file_name);


	strcat(event_name, event_data);

	// if this is a start event, get the FunctionInfo and put it in tool_info so the end event will
	// get it to stop the timer
	if (start == 1) {
		void* func_info = Tau_get_function_info(event_name, "", TAU_USER, "TAU_OPENACC");
		launch_event->tool_info = func_info;
		Tau_lite_start_timer(func_info, 0);
	}
	else {
		TAU_TRIGGER_EVENT(&event_name[0], 0);
	}
}

extern void Tau_pure_start_task_string(const std::string name, int tid);
static std::string acc_ev_enqueue_upload_str("OpenACC enqueue data transfer (HtoD)");
static std::string acc_ev_enqueue_download_str("OpenACC enqueue data transfer (DtoH)");

extern "C" static void
Tau_openacc_data_callback_signal_safe( acc_prof_info* prof_info, acc_event_info* event_info, acc_api_info* api_info )
{
	switch(prof_info->event_type) {
		case acc_ev_enqueue_upload_start:
            Tau_pure_start_task_string(acc_ev_enqueue_upload_str, Tau_get_thread());
			break;
		case acc_ev_enqueue_download_start:
            Tau_pure_start_task_string(acc_ev_enqueue_download_str, Tau_get_thread());
			break;
		default:
			Tau_global_stop();
			break;
	}
}

#if 0

extern "C" static void
Tau_openacc_data_callback( acc_prof_info* prof_info, acc_event_info* event_info, acc_api_info* api_info )
{
	acc_data_event_info* data_event = &(event_info->data_event);
	char file_name[256];
	char event_name[256];
	char event_data[256];
	int start = -1;

	switch(prof_info->event_type) {
		case acc_ev_enqueue_upload_start:
			start = 1;
			sprintf(event_name, "OpenACC enqueue data transfer (HtoD)");
			break;
		case acc_ev_enqueue_upload_end:
			start = 0;
			break;
		case acc_ev_enqueue_download_start:
			start = 1;
			sprintf(event_name, "OpenACC enqueue data transfer (DtoH)");
			break;
		case acc_ev_enqueue_download_end:
			start = 0;
			break;
		case acc_ev_create:
			start = -1;
			sprintf(event_name, "OpenACC device data create");
			break;
		case acc_ev_delete:
			start = -1;
			sprintf(event_name, "OpenACC device data delete");
			break;
		case acc_ev_alloc:
			start = -1;
			sprintf(event_name, "OpenACC device alloc");
			break;
		case acc_ev_free:
			start = -1;
			sprintf(event_name, "OpenACC device free");
			break;
		default:
			start = -1;
			sprintf(event_name, "UNKNOWN OPENACC DATA EVENT");
			fprintf(stderr, "ERROR: Unknown data event passed to OpenACC data event callback.");
	}

	// if this is an end event, short circuit by grabbing the FunctionInfo pointer out of tool_info
	// and stopping that timer; if the pointer is NULL something bad happened, print warning and kill
	// whatever timer is on top of the stack
	if (start == 0) {
		if (data_event->tool_info == NULL) {
			fprintf(stderr, "WARNING: OpenACC launch end event has bad matching start event.");
			Tau_global_stop();
		}
		else {
			Tau_lite_stop_timer(data_event->tool_info);
		}
		return;
	}

	sprintf(file_name, "%s:%s-%s",
			prof_info->src_file,
			(prof_info->line_no > 0) ? std::to_string(prof_info->line_no).c_str() : "?",
			(prof_info->end_line_no > 0) ? std::to_string(prof_info->end_line_no).c_str() : "?");

	sprintf(event_data, " ; variable name = %s %s; parent construct = %s (%s)",
	//                               name ^  ^ (implicit move?)         ^ file and line no.
			(data_event->var_name) ? data_event->var_name : "unknown variable",
			(data_event->implicit) ? "(implicit move)" : "",
			(data_event->parent_construct < 9999) ? acc_constructs[data_event->parent_construct] : "unknown construct",
			file_name);

	strcat(event_name, event_data);

	// if this is a start event, get the FunctionInfo and put it in tool_info so the end event will
	// get it to stop the timer
	if (start == 1) {
		void* func_info = Tau_get_function_info(event_name, "", TAU_USER, "TAU_OPENACC");
		data_event->tool_info = func_info;
		Tau_lite_start_timer(func_info, 0);
		TAU_TRIGGER_EVENT(&event_name[0], data_event->bytes);
	}
	else {
		TAU_TRIGGER_EVENT(&event_name[0], data_event->bytes);
	}
}

#endif

extern "C" static void
Tau_openacc_other_callback( acc_prof_info* prof_info, acc_event_info* event_info, acc_api_info* api_info )
{
	acc_other_event_info* other_event = &(event_info->other_event);
	char file_name[256];
	char event_name[256];
	char event_data[256];
	int start = -1;

	switch(prof_info->event_type) {
		case acc_ev_device_init_start:
			start = 1;
			sprintf(event_name, "OpenACC device init");
			break;
		case acc_ev_device_init_end:
			start = 0;
			break;
		case acc_ev_device_shutdown_start:
			start = 1;
			sprintf(event_name, "OpenACC device shutdown");
			break;
		case acc_ev_device_shutdown_end:
			start = 0;
			break;
		case acc_ev_runtime_shutdown:
			start = -1;
			sprintf(event_name, "OpenACC runtime shutdown");
			break;
		case acc_ev_enter_data_start:
			start = 1;
			sprintf(event_name, "OpenACC enter data");
			break;
		case acc_ev_enter_data_end:
			start = 0;
			break;
		case acc_ev_exit_data_start:
			start = 1;
			sprintf(event_name, "OpenACC exit data");
			break;
		case acc_ev_exit_data_end:
			start = 0;
			break;
		case acc_ev_update_start:
			start = 1;
			sprintf(event_name, "OpenACC update");
			break;
		case acc_ev_update_end:
			start = 0;
			break;
		case acc_ev_compute_construct_start:
			start = 1;
			sprintf(event_name, "OpenACC compute construct");
			break;
		case acc_ev_compute_construct_end:
			start = 0;
			break;
		case acc_ev_wait_start:
			start = 1;
			sprintf(event_name, "OpenACC wait");
			break;
		case acc_ev_wait_end:
			start = 0;
			break;
		default:
			start = -1;
			sprintf(event_name, "UNKNOWN OPENACC OTHER EVENT");
			fprintf(stderr, "ERROR: Unknown other event passed to OpenACC other event callback.");
	}

	// if this is an end event, short circuit by grabbing the FunctionInfo pointer out of tool_info
	// and stopping that timer; if the pointer is NULL something bad happened, print warning and kill
	// whatever timer is on top of the stack
	if (start == 0) {
		if (other_event->tool_info == NULL) {
			fprintf(stderr, "WARNING: OpenACC launch end event has bad matching start event.");
			Tau_global_stop();
		}
		else {
			Tau_lite_stop_timer(other_event->tool_info);
		}
		return;
	}

	sprintf(file_name, "%s:%s-%s",
			prof_info->src_file,
			(prof_info->line_no > 0) ? std::to_string(prof_info->line_no).c_str() : "?",
			(prof_info->end_line_no > 0) ? std::to_string(prof_info->end_line_no).c_str() : "?");

	sprintf(event_data, " %s; parent construct = %s (%s)",
	//                      ^ (implicit?)              ^ file and line no.
			(other_event->implicit) ? "(implicit)" : "",
			(other_event->parent_construct < 9999) ? acc_constructs[other_event->parent_construct] : "unknown construct",
			file_name);

	//printf("parent construct %d\n", other_event->parent_construct);

	strcat(event_name, event_data);

#ifdef DEBUG_OPENACC
	printf("%s\n", event_name);
#endif

	// if this is a start event, get the FunctionInfo and put it in tool_info so the end event will
	// get it to stop the timer
	if (start == 1) {
		void* func_info = Tau_get_function_info(event_name, "", TAU_USER, "TAU_OPENACC");
		other_event->tool_info = func_info;
		Tau_lite_start_timer(func_info, 0);
	}
	else {
		TAU_TRIGGER_EVENT(&event_name[0], 0);
	}
}



#define CUPTI_CALL(call)                                                \
  do {                                                                  \
    CUptiResult _status = call;                                         \
    if (_status != CUPTI_SUCCESS) {                                     \
      const char *errstr;                                               \
      cuptiGetResultString(_status, &errstr);                           \
      fprintf(stderr, "%s:%d: error: function %s failed with error %s.\n", \
              __FILE__, __LINE__, #call, errstr);                       \
          exit(-1);                                                     \
    }                                                                   \
  } while (0)

#define BUF_SIZE (32 * 1024)
#define ALIGN_SIZE (8)
#define ALIGN_BUFFER(buffer, align)                                            \
  (((uintptr_t) (buffer) & ((align)-1)) ? ((buffer) + (align) - ((uintptr_t) (buffer) & ((align)-1))) : (buffer))


//array enumerating CUpti_OpenAccEventKind strings
const char* openacc_event_names[] = {
		"CUPTI_OPENACC_EVENT_KIND_INVALD",
		"CUPTI_OPENACC_EVENT_KIND_DEVICE_INIT",
		"CUPTI_OPENACC_EVENT_KIND_DEVICE_SHUTDOWN",
		"CUPTI_OPENACC_EVENT_KIND_RUNTIME_SHUTDOWN",
		"CUPTI_OPENACC_EVENT_KIND_ENQUEUE_LAUNCH",
		"CUPTI_OPENACC_EVENT_KIND_ENQUEUE_UPLOAD",
		"CUPTI_OPENACC_EVENT_KIND_ENQUEUE_DOWNLOAD",
		"CUPTI_OPENACC_EVENT_KIND_WAIT",
		"CUPTI_OPENACC_EVENT_KIND_IMPLICIT_WAIT",
		"CUPTI_OPENACC_EVENT_KIND_COMPUTE_CONSTRUCT",
		"CUPTI_OPENACC_EVENT_KIND_UPDATE",
		"CUPTI_OPENACC_EVENT_KIND_ENTER_DATA",
		"CUPTI_OPENACC_EVENT_KIND_EXIT_DATA",
		"CUPTI_OPENACC_EVENT_KIND_CREATE",
		"CUPTI_OPENACC_EVENT_KIND_DELETE",
		"CUPTI_OPENACC_EVENT_KIND_ALLOC",
		"CUPTI_OPENACC_EVENT_KIND_FREE"
	};

static size_t openacc_records = 0;
#ifdef CUPTI
static void
printActivity(CUpti_Activity *record)
{
	GpuEventAttributes* map;
	int map_size;
  switch (record->kind) {
        case CUPTI_ACTIVITY_KIND_OPENACC_DATA:
				{
					CUpti_ActivityOpenAccData *oacc_data = (CUpti_ActivityOpenAccData*) record;
					if (oacc_data->deviceType != acc_device_nvidia) {
            printf("Error: OpenACC device type is %u, not %u (acc_device_nvidia)\n", oacc_data->deviceType, acc_device_nvidia);
            exit(-1);
          }

					map_size = 2;
					map = (GpuEventAttributes *) malloc(sizeof(GpuEventAttributes) * map_size);

					static TauContextUserEvent* bytes;
					Tau_get_context_userevent((void**) &bytes, "Bytes transfered");
					map[0].userEvent = bytes;
					map[0].data = oacc_data->bytes;
					break;
				}
        case CUPTI_ACTIVITY_KIND_OPENACC_LAUNCH:
				{
					CUpti_ActivityOpenAccLaunch *oacc_launch = (CUpti_ActivityOpenAccLaunch*) record;
					if (oacc_launch->deviceType != acc_device_nvidia) {
            printf("Error: OpenACC device type is %u, not %u (acc_device_nvidia)\n", oacc_launch->deviceType, acc_device_nvidia);
            exit(-1);
          }

					map_size = 4;
					map = (GpuEventAttributes *) malloc(sizeof(GpuEventAttributes) * map_size);

					static TauContextUserEvent* gangs;
					Tau_get_context_userevent((void**) &gangs, "Num gangs");
					map[0].userEvent = gangs;
					map[0].data = oacc_launch->numGangs;

					static TauContextUserEvent* workers;
					Tau_get_context_userevent((void**) &workers, "Num workers");
					map[1].userEvent = workers;
					map[1].data = oacc_launch->numWorkers;

					static TauContextUserEvent* vector;
					Tau_get_context_userevent((void**) &vector, "Vector lanes");
					map[2].userEvent = vector;
					map[2].data = oacc_launch->vectorLength;

					break;
				}
        case CUPTI_ACTIVITY_KIND_OPENACC_OTHER:
        {
					CUpti_ActivityOpenAccData *oacc_other = (CUpti_ActivityOpenAccData*) record;
					if (oacc_other->deviceType != acc_device_nvidia) {
            printf("Error: OpenACC device type is %u, not %u (acc_device_nvidia)\n", oacc_other->deviceType, acc_device_nvidia);
            exit(-1);
          }

					map_size = 1;
					map = (GpuEventAttributes *) malloc(sizeof(GpuEventAttributes) * map_size);
        break;
        }

		default:
      ;
  }

	CUpti_ActivityOpenAcc* oacc = (CUpti_ActivityOpenAcc*) record;
	// TODO: are we guaranteed to only get openacc events? I don't know. Guess we'll find out.
	// always add duration at the end
	uint32_t context = oacc->cuContextId;
	uint32_t device = oacc->cuDeviceId;
	uint32_t stream = oacc->cuStreamId;
	uint32_t corr_id = oacc->externalId; // pretty sure this is right
	uint64_t start = oacc->start;
	uint64_t end = oacc->end;

	int task = oacc->cuThreadId;

	static TauContextUserEvent* duration;
	Tau_get_context_userevent((void**) &duration, "Duration");
	map[map_size-1].userEvent = duration;
	map[map_size-1].data = (double)(end - start) / (double)1e6;

	//TODO: could do better but this'll do for now
	const char* name = openacc_event_names[oacc->eventKind];

	Tau_openacc_register_gpu_event(name, device, stream, context, task, corr_id, map, map_size, start/1e3, end/1e3);

	openacc_records++;
}

void CUPTIAPI bufferRequested(uint8_t **buffer, size_t *size, size_t *maxNumRecords)
{
  uint8_t *bfr = (uint8_t *) malloc(BUF_SIZE + ALIGN_SIZE);
  if (bfr == NULL) {
    printf("Error: out of memory\n");
    exit(-1);
  }

  *size = BUF_SIZE;
  *buffer = ALIGN_BUFFER(bfr, ALIGN_SIZE);
  *maxNumRecords = 0;
}

void CUPTIAPI bufferCompleted(CUcontext ctx, uint32_t streamId, uint8_t *buffer, size_t size, size_t validSize)
{
  CUptiResult status;
  CUpti_Activity *record = NULL;

  if (validSize > 0) {
    do {
      status = cuptiActivityGetNextRecord(buffer, validSize, &record);
      if (status == CUPTI_SUCCESS) {
        printActivity(record);
      }
      else if (status == CUPTI_ERROR_MAX_LIMIT_REACHED)
        break;
      else {
        CUPTI_CALL(status);
      }
    } while (1);

    // report any records dropped from the queue
    size_t dropped;
    CUPTI_CALL(cuptiActivityGetNumDroppedRecords(ctx, streamId, &dropped));
    if (dropped != 0) {
      printf("Dropped %u activity records\n", (unsigned int) dropped);
    }
  }

  free(buffer);
}
#endif
void finalize()
{
#ifdef CUPTI
  cuptiActivityFlushAll(0);
#endif
  printf("Found %llu OpenACC records\n", (long long unsigned) openacc_records);
}

////////////////////////////////////////////////////////////////////////////
extern "C" void
acc_register_library(acc_prof_reg reg, acc_prof_reg unreg, acc_prof_lookup lookup)
{
    TAU_VERBOSE("Inside acc_register_library\n");

		// Launch events
    reg( acc_ev_enqueue_launch_start,      Tau_openacc_launch_callback, acc_reg );
    reg( acc_ev_enqueue_launch_end,        Tau_openacc_launch_callback, acc_reg );

/* The data events aren't signal safe, for some reason.  So, that means we can't
 * allocate any memory, which limits what we can do.  For that reason, we only
 * handle some events, and with static timer names. */
		// Data events
    reg( acc_ev_enqueue_upload_start,      Tau_openacc_data_callback_signal_safe, acc_reg );
    reg( acc_ev_enqueue_upload_end,        Tau_openacc_data_callback_signal_safe, acc_reg );
    reg( acc_ev_enqueue_download_start,    Tau_openacc_data_callback_signal_safe, acc_reg );
    reg( acc_ev_enqueue_download_end,      Tau_openacc_data_callback_signal_safe, acc_reg );
#if 0
    reg( acc_ev_create,                    Tau_openacc_data_callback, acc_reg );
    reg( acc_ev_delete,                    Tau_openacc_data_callback, acc_reg );
    reg( acc_ev_alloc,                     Tau_openacc_data_callback, acc_reg );
    reg( acc_ev_free,                      Tau_openacc_data_callback, acc_reg );
#endif
		// Other events
		reg( acc_ev_device_init_start,         Tau_openacc_other_callback, acc_reg );
    reg( acc_ev_device_init_end,           Tau_openacc_other_callback, acc_reg );
    reg( acc_ev_device_shutdown_start,     Tau_openacc_other_callback, acc_reg );
    reg( acc_ev_device_shutdown_end,       Tau_openacc_other_callback, acc_reg );
		reg( acc_ev_runtime_shutdown,					 Tau_openacc_other_callback, acc_reg );
		reg( acc_ev_enter_data_start,					 Tau_openacc_other_callback, acc_reg );
		reg( acc_ev_enter_data_end,						 Tau_openacc_other_callback, acc_reg );
		reg( acc_ev_exit_data_start,					 Tau_openacc_other_callback, acc_reg );
		reg( acc_ev_exit_data_end,						 Tau_openacc_other_callback, acc_reg );
    reg( acc_ev_update_start,              Tau_openacc_other_callback, acc_reg );
    reg( acc_ev_update_end,                Tau_openacc_other_callback, acc_reg );
    reg( acc_ev_compute_construct_start,   Tau_openacc_other_callback, acc_reg );
    reg( acc_ev_compute_construct_end,     Tau_openacc_other_callback, acc_reg );
    reg( acc_ev_wait_start,                Tau_openacc_other_callback, acc_reg );
    reg( acc_ev_wait_end,                  Tau_openacc_other_callback, acc_reg );


#ifdef CUPTI
    if (cuptiOpenACCInitialize(reg, unreg, lookup) != CUPTI_SUCCESS) {
        printf("ERROR: failed to initialize CUPTI OpenACC support\n");
    }

    printf("Initialized CUPTI for OpenACC\n");

    CUptiResult cupti_err = CUPTI_SUCCESS;

    cupti_err = cuptiActivityEnable(CUPTI_ACTIVITY_KIND_OPENACC_DATA);
    cupti_err = cuptiActivityEnable(CUPTI_ACTIVITY_KIND_OPENACC_LAUNCH);
    cupti_err = cuptiActivityEnable(CUPTI_ACTIVITY_KIND_OPENACC_OTHER);

    if (cupti_err != CUPTI_SUCCESS) {
        printf("ERROR: unable to enable some OpenACC CUPTI measurements\n");
    }

    cupti_err = cuptiActivityRegisterCallbacks(bufferRequested, bufferCompleted);

    if (cupti_err != CUPTI_SUCCESS) {
        printf("ERROR: unable to register buffers with CUPTI\n");
    }
#endif
    atexit(finalize);

} // acc_register_library

