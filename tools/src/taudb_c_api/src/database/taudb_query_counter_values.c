#include "taudb_internal.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

TAUDB_COUNTER_VALUE* taudb_query_counter_values(TAUDB_CONNECTION* connection, TAUDB_TRIAL* trial) {
#ifdef TAUDB_DEBUG_DEBUG
  printf("Calling taudb_query_counter_values(%p)\n", trial);
#endif
  int nFields;
  int i, j;

  if (trial == NULL) {
    fprintf(stderr, "Error: trial parameter null. Please provide a valid trial.\n");
    return NULL;
  }

  // if the Trial already has the data, return it.
  if (trial->counter_values != NULL) {
    taudb_numItems = HASH_CNT(hh1,trial->counter_values);
    return trial->counter_values;
  }

  taudb_begin_transaction(connection);

  /*
   * Fetch rows from table_name, the system catalog of databases
   */
  char my_query[256];
  if (taudb_version == TAUDB_2005_SCHEMA) {
    sprintf(my_query,"select alp.* from atomic_location_profile alp inner join atomic_event ae on ae.id = alp.atomic_event where ae.trial = %d", trial->id);
  } else {
    sprintf(my_query,"select * from counter_value where trial = %d", trial->id);
  }
#ifdef TAUDB_DEBUG
  printf("'%s'\n",my_query);
#endif
  taudb_execute_query(connection, my_query);

  int nRows = taudb_get_num_rows(connection);
  taudb_numItems = nRows;
#ifdef TAUDB_DEBUG
  printf("'%d' rows returned\n",nRows);
#endif

  nFields = taudb_get_num_columns(connection);

  /* the rows */
  for (i = 0; i < taudb_get_num_rows(connection); i++)
  {
    TAUDB_COUNTER_VALUE* counter_value = taudb_create_counter_values(1);
    memset(&(counter_value->key), 0, sizeof(TAUDB_COUNTER_VALUE_KEY));
    /* the columns */
    int node = 0;
    int context = 0;
    int thread = 0;
    for (j = 0; j < nFields; j++) {
      if (strcmp(taudb_get_column_name(connection, j), "atomic_event") == 0) {
        counter_value->key.counter = taudb_get_counter_by_id(trial->counters_by_id, atoi(taudb_get_value(connection, i, j)));
      } else if (strcmp(taudb_get_column_name(connection, j), "counter") == 0) {
        counter_value->key.counter = taudb_get_counter_by_id(trial->counters_by_id, atoi(taudb_get_value(connection, i, j)));
      } else if (strcmp(taudb_get_column_name(connection, j), "timer_callpath") == 0) {
        counter_value->key.context = taudb_get_timer_callpath_by_id(trial->timer_callpaths_by_id, atoi(taudb_get_value(connection, i, j)));
      } else if (strcmp(taudb_get_column_name(connection, j), "node") == 0) {
        node = atoi(taudb_get_value(connection, i, j));
      } else if (strcmp(taudb_get_column_name(connection, j), "context") == 0) {
        context = atoi(taudb_get_value(connection, i, j));
      } else if (strcmp(taudb_get_column_name(connection, j), "thread") == 0) {
        thread = atoi(taudb_get_value(connection, i, j));
      } else if (strcmp(taudb_get_column_name(connection, j), "sample_count") == 0) {
        counter_value->sample_count = atoi(taudb_get_value(connection,i,j));
      } else if (strcmp(taudb_get_column_name(connection, j), "maximum_value") == 0) {
        counter_value->maximum_value = atof(taudb_get_value(connection,i,j));
      } else if (strcmp(taudb_get_column_name(connection, j), "minimum_value") == 0) {
        counter_value->minimum_value = atof(taudb_get_value(connection,i,j));
      } else if (strcmp(taudb_get_column_name(connection, j), "mean_value") == 0) {
        counter_value->mean_value = atof(taudb_get_value(connection,i,j));
      } else if (strcmp(taudb_get_column_name(connection, j), "standard_deviation") == 0) {
        counter_value->standard_deviation = atof(taudb_get_value(connection,i,j));
      } else {
        printf("Error: unknown column '%s'\n", taudb_get_column_name(connection, j));
        taudb_exit_nicely(connection);
      }
    } 
    int thread_index = node * trial->contexts_per_node * trial->threads_per_context;
    thread_index += context * trial->threads_per_context;
    thread_index += thread;
// make the key!
    counter_value->key.timestamp = NULL; // for now
    counter_value->key.thread = taudb_get_thread(trial->threads, thread_index);
    HASH_ADD(hh1, trial->counter_values, key, sizeof(TAUDB_COUNTER_VALUE_KEY), counter_value);
  }
  taudb_clear_result(connection);
  taudb_close_transaction(connection);

  return (trial->counter_values);
}

TAUDB_COUNTER_VALUE* taudb_get_counter_value(TAUDB_COUNTER_VALUE* counter_values, TAUDB_COUNTER* counter, TAUDB_THREAD* thread, TAUDB_TIMER_CALLPATH* context, char* timestamp) {
#ifdef TAUDB_DEBUG_DEBUG
  printf("Calling taudb_get_counter_value_by_id(%p,%p,%p,%p,%p)\n", counter_values, counter, thread, context, timestamp);
#endif
  if (counter_values == NULL) {
    fprintf(stderr, "Error: counter_value parameter null. Please provide a valid set of counter_values.\n");
    return NULL;
  }
  if (counter == NULL) {
    fprintf(stderr, "Error: counter parameter null. Please provide a valid counter.\n");
    return NULL;
  }

  if (thread == NULL) {
    fprintf(stderr, "Error: thread parameter null. Please provide a valid thread.\n");
    return NULL;
  }

  TAUDB_COUNTER_VALUE_KEY key;
  memset(&key, 0, sizeof(TAUDB_COUNTER_VALUE_KEY));
  key.counter = counter;
  key.thread = thread;
  key.context = context;
  key.timestamp = timestamp;
 
  TAUDB_COUNTER_VALUE* counter_value = NULL;
  HASH_FIND(hh1, counter_values, &key, sizeof(TAUDB_COUNTER_VALUE_KEY), counter_value);
#ifdef ITERATE_ON_FAILURE
  // HASH_FIND is not working so well... now we iterate. Sigh.
  if (counter_value == NULL) {
    TAUDB_COUNTER_VALUE *current, *tmp;
    HASH_ITER(hh1, counter_values, current, tmp) {
#ifdef TAUDB_DEBUG_DEBUG
      printf("COUNTER VALUE: (%p,%p,%p,%p)\n", current->key.counter, current->key.thread, current->key.context, current->key.timestamp);
#endif
      if ((current->key.counter->id == counter->id) && (current->key.thread == thread) && (current->key.context == context) && ((current->key.timestamp == NULL && timestamp == NULL) || (strcmp(current->key.timestamp, timestamp) == 0))) {
        return current;
      }
    }
  }
#endif
  return counter_value;
}


extern void taudb_save_counter_values(TAUDB_CONNECTION* connection, TAUDB_TRIAL* trial, boolean update) {
  printf("Counter values not supported yet.\n");
}
