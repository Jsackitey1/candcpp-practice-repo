#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* --- Constants and data types ---
 * MAX_PROCESSES, SWITCH_TIME, and QUANTUM set the simulation limits.
 * Process holds everything about one job: its ID, priority, arrival/burst
 * times, how much work is left, and when it finished.
 * Policy is just which scheduler we're running.
 * Queue is a simple circular buffer used for Round Robin's ready queue.
 */

#define MAX_PROCESSES 1000
#define SWITCH_TIME 5
#define QUANTUM 20

typedef int64_t Time;

typedef struct {
  int64_t id, priority;
  Time arrival, burst, remaining, completion;
} Process;

typedef enum { FCFS, SJF, PRIORITY, RR } Policy;

typedef struct {
  int items[MAX_PROCESSES], head, count;
} Queue;

/* --- Error handling and safe math ---
 * fail() prints an error message and exits.
 * add_time() adds two time values but checks for overflow first.
 */

/* Prints a standardized error diagnostic to stderr and terminates execution. */
static void fail(const char *message) {
  fprintf(stderr, "CPUSim: %s\n", message);
  exit(EXIT_FAILURE);
}

/* Performs overflow-safe addition for 64-bit simulation millisecond timestamps.
 */
static Time add_time(Time a, Time b) {
  if (b > INT64_MAX - a)
    fail("simulation time exceeds the supported integer range");
  return a + b;
}

/* --- Reading and validating the input file ---
 * read_record() parses one line of space-separated integers.
 * read_processes() opens the file, reads all the process records, and
 * checks for problems like duplicate IDs, out-of-order arrivals,
 * negative times, zero bursts, etc.
 */

/* Reads and parses a single line record containing an exact count of 64-bit
 * integers. */
static void read_record(FILE *file, int64_t *values, int count) {
  char line[512], *cursor, *end;
  if (!fgets(line, sizeof line, file))
    fail("missing input record");
  if (!strchr(line, '\n') && !feof(file))
    fail("input record is too long");
  cursor = line;
  for (int i = 0; i < count; ++i) {
    while (isspace((unsigned char)*cursor))
      ++cursor;
    errno = 0;
    intmax_t value = strtoimax(cursor, &end, 10);
    if (cursor == end || errno == ERANGE || value < INT64_MIN ||
        value > INT64_MAX)
      fail("expected an integer within the signed 64-bit range");
    if (*end && !isspace((unsigned char)*end))
      fail("integers must be separated by whitespace");
    values[i] = (int64_t)value;
    cursor = end;
  }
  while (isspace((unsigned char)*cursor))
    ++cursor;
  if (*cursor)
    fail("too many fields in input record");
}

/* Opens input file, ingests process records, and validates simulation
 * preconditions. */
static int read_processes(const char *path, Process *processes) {
  FILE *file = fopen(path, "r");
  if (!file)
    fail("cannot open input file");
  int64_t fields[4];
  read_record(file, fields, 1);
  if (fields[0] < 1 || fields[0] > MAX_PROCESSES)
    fail("process count must be between 1 and 1000");
  int n = (int)fields[0];
  Time total = 0;
  for (int i = 0; i < n; ++i) {
    read_record(file, fields, 4);
    if (fields[2] < 0 || fields[3] <= 0)
      fail("arrival must be nonnegative and burst must be positive");
    if (i && fields[2] < processes[i - 1].arrival)
      fail("processes must be ordered by arrival time");
    for (int j = 0; j < i; ++j)
      if (fields[0] == processes[j].id)
        fail("duplicate process ID");
    processes[i] =
        (Process){fields[0], fields[1], fields[2], fields[3], fields[3], -1};
    total = add_time(total, fields[3]);
  }
  int ch;
  while ((ch = fgetc(file)) != EOF)
    if (!isspace((unsigned char)ch))
      fail("unexpected data after last process");
  if (ferror(file))
    fail("error reading input file");
  fclose(file);
  (void)add_time(processes[n - 1].arrival, total);
  return n;
}

/* --- Ready queue for Round Robin ---
 * A circular array that works as a FIFO queue.
 * enqueue() adds to the back, dequeue() takes from the front.
 */

/* Enqueues a process index at the tail of the circular ready queue. */
static void enqueue(Queue *q, int index) {
  assert(q->count < MAX_PROCESSES);
  q->items[(q->head + q->count) % MAX_PROCESSES] = index;
  ++q->count;
}

/* Removes and returns the process index at the head of the circular ready
 * queue. */
static int dequeue(Queue *q) {
  assert(q->count > 0);
  int result = q->items[q->head];
  q->head = (q->head + 1) % MAX_PROCESSES;
  --q->count;
  return result;
}

/* --- Admitting arrivals and picking the next process ---
 * admit() moves processes into the ready pool once the clock reaches
 * their arrival time. For RR it also puts them in the queue.
 * choose() scans admitted processes and picks the best one based on
 * the current policy (earliest arrival for FCFS, shortest remaining
 * burst for SJF, lowest priority number for Priority).
 * Ties go to whichever process arrived first / appears first in input.
 */

/* Discovers and admits processes whose arrival time is <= the current
 * simulation clock. */
static void admit(Process *p, int n, Time now, int *next, Queue *q,
                  Policy policy) {
  while (*next < n && p[*next].arrival <= now) {
    if (policy == RR)
      enqueue(q, *next);
    ++*next;
  }
}

/* Selects the highest-priority admitted candidate among incomplete processes.
 */
static int choose(const Process *p, int admitted, Policy policy) {
  int best = -1;
  for (int i = 0; i < admitted; ++i) {
    if (p[i].remaining == 0)
      continue;
    if (best == -1 || (policy == SJF && p[i].remaining < p[best].remaining) ||
        (policy == PRIORITY && p[i].priority < p[best].priority))
      best = i;
  }
  return best;
}

/* --- Main simulation loop ---
 * Runs the clock forward 1 ms at a time. Each tick does four things:
 *   1. Admit any processes that have arrived by now.
 *   2. Check if the running process should be preempted.
 *   3. If the CPU is free, pick the next process (with a 5 ms switch
 *      if we're changing from one process to another).
 *   4. Execute 1 ms of work and check if the process just finished.
 */

static void simulate(const Process *input, int n, Policy policy, int verbose) {
  static const char *names[] = {
      "First Come First Serve", "Preemptive Shortest Job First",
      "Priority (preemptive)", "Round Robin (quantum=20 ms)"};
  Process p[MAX_PROCESSES];
  memcpy(p, input, (size_t)n * sizeof *p);
  Queue queue = {{0}, 0, 0};
  Time now = 0, busy = 0, switching = 0, idle = 0;
  int next = 0, finished = 0, current = -1, previous = -1, slice = 0;

  while (finished < n) {
    /* Phase 6.1: Admit all processes that have arrived up to current time. */
    //-- Process Life Cycle - Admission:
    //-- When the simulation clock reaches a process's arrival time, the process
    //is admitted
    //-- into the system and transitions from unadmitted to the Ready state.
    //Under Round Robin,
    //-- the process is placed at the tail of the FIFO ready queue. Under FCFS,
    //SJF, and
    //-- Priority, the process enters the pool of admitted candidates where the
    //scheduler
    //-- can evaluate it based on arrival order, remaining burst, or priority
    //rank.
    admit(p, n, now, &next, &queue, policy);

    /* Phase 6.2: Preemption Evaluation */
    if (current >= 0) {
      int preempt = 0;
      if (policy == RR && slice == QUANTUM) {
        /* RR preemption: if other jobs are waiting, yield; otherwise continue
         * slice. */
        if (queue.count)
          preempt = 1;
        else
          slice = 0;
      } else if (policy == SJF || policy == PRIORITY) {
        int best = choose(p, next, policy);
        /* Preemption occurs only if a strictly superior candidate is ready (no
         * preemption on ties). */
        preempt =
            (policy == SJF && p[best].remaining < p[current].remaining) ||
            (policy == PRIORITY && p[best].priority < p[current].priority);
      }
      if (preempt) {
        //-- Process Life Cycle - Preemption:
        //-- An actively running process is forcibly interrupted before it has
        //finished its
        //-- total CPU burst. In Round Robin, preemption occurs when the process
        //reaches the
        //-- end of its 20 ms quantum while other processes are waiting; it
        //yields the CPU
        //-- and transitions from Running back to Ready at the end of the queue.
        //In SJF or
        //-- Priority, preemption occurs when another ready process has a
        //strictly shorter
        //-- remaining burst or strictly higher priority. The preempted process
        //yields the
        //-- CPU and transitions back to the Ready state until it is scheduled
        //again.
        previous = current;
        if (policy == RR)
          enqueue(&queue, current);
        current = -1;
      }
    }

    /* Phase 6.3: Candidate Selection, Context Switching & Idle Fast-Forward */
    if (current == -1) {
      int candidate = policy == RR
                          ? (queue.count ? queue.items[queue.head] : -1)
                          : choose(p, next, policy);
      if (candidate == -1) {
        /* No ready process: fast-forward clock directly to next process
         * arrival. */
        assert(next < n);
        idle = add_time(idle, p[next].arrival - now);
        now = p[next].arrival;
        previous =
            -1; /* Initial dispatch after idle is free of switch overhead. */
        continue;
      }
      /* Context switch overhead: switching between distinct processes costs
       * SWITCH_TIME ms. */
      if (previous >= 0 && previous != candidate) {
        //-- Process Life Cycle - Context Switching:
        //-- When control of the CPU transfers from one process to a different
        //process,
        //-- the system incurs a 5 ms context switch penalty. During these 5 ms,
        //the CPU
        //-- is occupied saving the outgoing process's context and loading the
        //incoming
        //-- process's context; neither process performs useful work or reduces
        //its remaining
        //-- burst. However, time advances, and any processes arriving during
        //the switch are
        //-- admitted into the Ready state. Dispatching after an idle period
        //incurs no switch.
        for (int ms = 0; ms < SWITCH_TIME; ++ms) {
          now = add_time(now, 1);
          switching = add_time(switching, 1);
          /* Arrivals occurring during switch time are admitted immediately. */
          admit(p, n, now, &next, &queue, policy);
        }
      }
      current = policy == RR ? dequeue(&queue) : choose(p, next, policy);
      previous = -1;
      slice = 0;
    }

    /* Phase 6.4: CPU Execution & Job Completion */
    //-- Process Life Cycle - CPU Execution:
    //-- The dispatched process enters the Running state and occupies the
    //processor. For each
    //-- 1 ms time unit of execution, the process makes forward progress: its
    //remaining CPU
    //-- burst is decremented by 1 ms, and in Round Robin its consumed time
    //slice increments.
    //-- Meanwhile, any other admitted but uncompleted processes wait in the
    //Ready state.
    now = add_time(now, 1);
    busy = add_time(busy, 1);
    --p[current].remaining;
    if (policy == RR)
      ++slice;
    if (p[current].remaining == 0) {
      //-- Process Life Cycle - Completion:
      //-- The process has reduced its remaining CPU burst to 0 and finishes
      //execution,
      //-- transitioning from Running to Terminated. Its completion timestamp is
      //permanently
      //-- recorded, which fixes its turnaround time (completion - arrival) and
      //wait time
      //-- (turnaround - burst). It leaves the active scheduling system,
      //releasing the CPU
      //-- for the next ready process with no trailing switch overhead after the
      //final job.
      /* Process finished execution: record completion timestamp. */
      p[current].completion = now;
      ++finished;
      previous = current;
      current = -1;
    }
  }

  /* --- Calculate and print the stats ---
   * turnaround = completion - arrival, wait = turnaround - burst.
   * Also computes CPU utilization, throughput, and averages.
   * Verbose mode prints each process's individual numbers too.
   */

  Time total_burst = 0;
  long double total_wait = 0, total_turnaround = 0;
  for (int i = 0; i < n; ++i) {
    Time turnaround = p[i].completion - p[i].arrival;
    assert(p[i].remaining == 0 && turnaround >= p[i].burst);
    total_burst = add_time(total_burst, p[i].burst);
    total_turnaround += turnaround;
    total_wait += turnaround - p[i].burst;
  }
  assert(busy == total_burst);
  assert(add_time(add_time(busy, switching), idle) == now);
  printf("%s\n", names[policy]);
  printf("Finish time: %" PRId64 " ms\n", now);
  printf("CPU utilization: %.6Lf%%\n", 100.0L * busy / now);
  printf("Throughput: %.6Lf processes/second\n", 1000.0L * n / now);
  printf("Average wait time: %.6Lf ms\n", total_wait / n);
  printf("Average turnaround time: %.6Lf ms\n", total_turnaround / n);
  if (verbose)
    for (int i = 0; i < n; ++i) {
      Time turnaround = p[i].completion - p[i].arrival;
      printf("%" PRId64 " %" PRId64 " %" PRId64 " %" PRId64 " %" PRId64 "\n",
             p[i].id, p[i].arrival, p[i].completion, turnaround,
             turnaround - p[i].burst);
    }
}

/* --- main ---
 * Checks args, reads the input file, and runs all four schedulers.
 */

int main(int argc, char **argv) {
  if (argc < 2 || argc > 3 || (argc == 3 && strcmp(argv[2], "-v") != 0)) {
    fprintf(stderr, "Usage: %s infile [-v]\n", argv[0]);
    return EXIT_FAILURE;
  }
  Process processes[MAX_PROCESSES];
  int n = read_processes(argv[1], processes);
  for (Policy policy = FCFS; policy <= RR; ++policy)
    simulate(processes, n, policy, argc == 3);
  return EXIT_SUCCESS;
}
