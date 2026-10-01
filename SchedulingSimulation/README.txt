CPU Scheduling Simulation
=========================

BUILD AND RUN

A C11 compiler and make are needed to build; Python 3 runs the tests.
From this project folder:

    make
    ./CPUSim tests/example.txt
    ./CPUSim tests/example.txt -v
    make test
    make sanitize

On the development Mac, the default SDK caused an "unknown architecture"
linker error. The installed macOS 15.2 SDK worked. Use this command here:

    make CC='cc -isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX15.2.sdk'

The same CC override can be added to make test and make sanitize. This is
a local toolchain workaround, not a dependency of the C source.
Alternatively compile CPUSim.c directly with a working C11 compiler:

    cc -std=c11 -Wall -Wextra -Wpedantic -O2 CPUSim.c -o CPUSim


FILES AND CODE GUIDE

CPUSim.c contains the complete simulator; no extra C libraries are needed.
Makefile builds the executable and runs the tests.
tests/example.txt contains the assignment's five-process input.
tests/testOut.txt preserves the instructor's reference output supplied later.
tests/test_sim.py contains small input fixtures, expected completion times,
output checks, and rejected-input cases.
tests/EXPECTATIONS.txt explains representative hand calculations.
PLAN.md records the plan approved before implementation.

Read the C code in this order:
1. Process: original input, remaining CPU time, and completion time.
2. read_processes: input validation and initial process state.
3. choose: selection for FCFS, shortest remaining time, and priority.
4. enqueue/dequeue/admit: FIFO bookkeeping and arrival admission.
5. simulate: arrivals, preemption, dispatch, execution, and completion.
6. Final statistics in simulate and command-line handling in main.

Each algorithm gets a fresh copy of the original processes. A running
process loses one millisecond of remaining work each loop iteration. When
remaining work reaches zero, its completion time is recorded. Waiting is
derived from completion rather than incrementing every ready process each
millisecond. RR uses a bounded circular queue; the other policies scan the
admitted processes. There are no real-time delays.

SCHEDULING AND ACCOUNTING DECISIONS

FCFS runs jobs to completion in arrival/input order.
Preemptive SJF compares remaining burst, not original burst.
Priority is preemptive, with smaller numbers indicating higher priority.
Round Robin uses a 20 ms quantum and a FIFO ready queue.

Ties use arrival time then input order. Equal SJF or priority keys never
preempt the running process. At a time boundary, completion is recorded
before admitting arrivals and deciding what runs next. RR arrivals at a
quantum boundary are queued before the unfinished running job is requeued.
Completion takes precedence over an expiring quantum.

A direct transfer between different processes costs 5 ms, including when
the old process completes and another is ready. Initial dispatch and
dispatch after idle are free. A lone RR process continues without switch
overhead when its quantum expires. The final completion has no trailing
switch. Switches run for all 5 ms without interruption. Arrivals during a
switch are admitted, and the next job is selected at the end of the switch.
Idle periods jump directly to the next arrival.

All simulation times are signed 64-bit integer milliseconds. Arithmetic
that advances the clock or sums time is checked for overflow. Long CPU
bursts can still take substantial execution time because execution uses
one simulated millisecond per iteration.

For each process:
    turnaround = completion - arrival
    wait = turnaround - original burst
Waiting includes ready time during switches. For each scheduler:
    finish = last completion (clock starts at zero)
    CPU utilization = 100 * total CPU burst / finish
    throughput = 1000 * process count / finish
    averages = sum of per-process values / process count
The denominator includes initial idle time and switching overhead.

Each scheduler prints six summary lines. Verbose mode follows its summary
with one line per process in input order:
    ID arrival completion turnaround wait
Those lines contain numbers only. Diagnostics go to stderr; invalid input
returns a nonzero status. Input accepts whitespace-separated integers,
requires one record per line, and rejects extra records with non-whitespace
content. IDs and priorities may be any signed 64-bit integers; arrivals
must be nonnegative and bursts positive.

DEVELOPMENT AND AI USE

This project was developed through an iterative AI-assisted workflow:

1. Tools and Access:
   - Initial Planning & Architecture Scaffolding: OpenAI Codex (based on GPT-6),
     accessed through the Codex workspace/chat interface.
   - Refinement, Submission Preparation, Lifecycle Comments & Verification:
     Google Gemini (Gemini 3.8 Flash), accessed via the Google Antigravity IDE.
   - Note on Campus Tools: Gettysburg College's Linc AI Hub was not used for this
     development; all work was performed via the direct workspace interfaces above.

2. Workflow Summary:
   The development followed a strict plan-first methodology:
   - The assignment requirements were supplied to formulate a formal design
     specification (PLAN.md), explicitly articulating scheduling edge cases,
     tie-breaking, queue policies, and dispatch accounting.
   - After student review and explicit plan approval, the initial C11
     discrete-event simulation engine, input validation, and statistical accounting
     were scaffolded.
   - Comprehensive black-box Python tests with independent hand-calculated
     fixtures were authored to validate all four algorithms.
   - When the instructor's testOut.txt reference became available, an in-depth
     comparative analysis was conducted to document the exact 5 ms dispatch delta.
   - For final submission, code was thoroughly reviewed, the required 20 ms
     quantum and 5 ms switch time were verified, and comprehensive //-- process
     lifecycle comments were added to CPUSim.c.

3. Important Prompts Written During Development and Testing:
   - Prompt 1 (Initial Planning & Design):
     "Create a project folder for the CPU Scheduling Simulation assignment.
     Review the assignment specification and propose a comprehensive development
     plan covering scheduling rules, data structures, and edge cases. I must
     approve the plan first before any code is written."
   - Prompt 2 (Plan Approval & Scaffolding):
     "Approved. Proceed to implement the discrete-event simulator in C11
     adhering to the approved design: FCFS, preemptive SJF, preemptive Priority,
     and Round Robin with 20 ms quantum and 5 ms context switch. Ensure strict
     input validation and 64-bit safe time math."
   - Prompt 3 (Test Suite Generation & Hand Calculations):
     "Write a Python test suite with independent hand-calculated fixtures that
     checks zero wait time, simultaneous arrivals, ties in priority/burst,
     arrivals during a 5 ms switch, boundary conditions at quantum expiration,
     1000 processes, and bad input handling."
   - Prompt 4 (Reference Output Comparison):
     "Compare our simulation output on tests/example.txt with the instructor's
     testOut.txt reference. Analyze why finish times differ by 5 ms and whether
     our free-initial-dispatch model is consistent with the assignment
     requirements."
   - Prompt 5 (Submission Preparation & Lifecycle Comments):
     "Document your development and testing in a file called README.txt,
     preparing the code for submission left these comments for the student to write.
     Before submission, add comments beginning with //-- or /*-- that explain
     the process life cycle in your own words. Useful locations are admission,
     preemption, context switching, CPU execution, and completion. Explain what
     happens to a process, rather than translating each C statement into English.
     Restore the required 20 ms quantum and 5 ms switch for submission. Which LLM
     did you use, and how did you access it? Include some important prompts you
     wrote while you developed and tested your code. Describe your testing process."


PROCESS LIFE CYCLE EXPLANATION

In CPUSim.c, comments starting with `//--` document the five key stages of the
process life cycle:

1. Admission (Phase 6.1):
   When the simulation clock reaches a process's arrival time, it transitions
   from the uncreated/unadmitted state into the Ready state. Under Round Robin,
   the process enters the tail of the circular FIFO queue. Under FCFS, SJF, and
   Priority, it enters the eligible pool of admitted processes for evaluation.
2. Preemption (Phase 6.2):
   An actively running process is forcibly interrupted before it has finished
   its burst. In Round Robin, preemption occurs when the process expends its
   20 ms quantum while other jobs wait; it yields the CPU and transitions from
   Running back to Ready at the queue's tail. In SJF or Priority, preemption
   occurs when a newly admitted process arrives with a strictly shorter remaining
   burst or strictly higher priority. The preempted process returns to the Ready
   state without losing its completed work.
3. Context Switching (Phase 6.3):
   When the CPU transfers control from one process to a different process, the
   operating system incurs a 5 ms switching penalty. During these 5 ms, the CPU
   is occupied by system overhead; no user process runs or makes computational
   progress. However, time advances, and any processes arriving during the switch
   are admitted into the Ready state. Initial dispatch after idle is free.
4. CPU Execution (Phase 6.4):
   The dispatched process enters the Running state and occupies the processor.
   For each 1 ms simulation tick, the process makes forward progress by
   decrementing its remaining burst time and incrementing its elapsed slice.
   All other admitted processes remain in the Ready state waiting.
5. Completion (Phase 6.4):
   When a running process reduces its remaining burst to 0, it finishes all
   required work and transitions from Running to Terminated. Its final completion
   time is recorded, fixing its turnaround and wait metrics permanently. The
   process leaves the active scheduling pool, and the CPU is immediately released
   for the next job without a trailing context switch penalty.



TESTING PROCESS

Our testing strategy employed a multi-tiered approach combining analytical
hand calculations, automated functional testing, boundary stress tests,
memory sanitizers, and comparative analysis against instructor reference data.

1. Automated Test Suite (tests/test_sim.py):
   A suite of 36 automated black-box tests executes against the compiled C binary.
   The suite validates both standard summary output and verbose mode (-v) across
   all four scheduling algorithms (FCFS, SJF, Priority, RR).

2. Hand-Calculated Scenarios & Edge Cases (tests/EXPECTATIONS.txt):
   Small, deterministic workloads were calculated by hand to serve as an
   independent oracle for millisecond-by-millisecond scheduler behavior:
   - Single Process Baseline: Verifies zero wait time, fast-forwarding over
     initial idle time, and Round Robin continuation past quantum boundaries
     without paying switch penalties when no competitors exist.
   - Two Identical Jobs: Verifies that switching between distinct processes
     incurs the exact 5 ms switch overhead and correctly shifts completion times.
   - Strict Preemption vs. Ties: Verifies that SJF and Priority preempt ONLY
     when a candidate is strictly superior (shorter remaining burst or lower
     priority integer). Equal keys never preempt the running job.
   - Arrivals During Context Switches: Verifies that processes arriving in the
     middle of an active 5 ms switch are admitted immediately, and the scheduler
     re-evaluates all candidates at the end of the switch without restarting it.
   - Round Robin Quantum Boundaries: Verifies that a newly arriving process at
     a quantum boundary is enqueued before the preempted running process is
     requeued, and that job completion takes precedence over quantum expiration.
   - Workload Scaling: A 1000-process stress test verifies circular queue
     capacity, 64-bit millisecond timestamp accumulation, and linear scaling.

3. Input Validation and Robustness:
   Negative testing ensures CPUSim safely fails with diagnostic messages to
   stderr and nonzero exit codes on invalid inputs:
   - Process count out of bounds (< 1 or > 1000).
   - Missing fields, extra fields, and trailing non-whitespace characters.
   - Malformed numbers, non-integer tokens, and integer overflow.
   - Negative arrival times and non-positive burst times.
   - Unsorted arrival times and duplicate process IDs.
   - Missing input files and invalid command-line flags.

4. Internal Consistency and Runtime Assertions:
   CPUSim includes active assertions validating core physical invariants:
   - Work conservation: Total CPU busy time exactly equals the sum of bursts.
   - Time partition: busy_time + switching_time + idle_time == final_clock.
   - Metric validity: turnaround >= burst, and wait_time >= 0 for every process.
   - Final state: All admitted processes have remaining == 0 at finish.
   - Summary consistency: Verbose mode process outputs mathematically agree
     with the summary averages, utilization, and throughput.

5. Compiler Warnings and Sanitizer Verification:
   - Built with strict warning flags: -std=c11 -Wall -Wextra -Wpedantic -O2.
     Clean build with zero warnings.
   - Memory and arithmetic safety verified using UndefinedBehaviorSanitizer
     (-fsanitize=undefined), which passed all 36 test cases cleanly.

6. Reference Output Comparison (tests/testOut.txt):
   Comparing CPUSim's results on tests/example.txt against the instructor's
   testOut.txt confirms identical scheduling decisions with an exact, consistent
   5 ms difference:
   - Our simulation finishes at 310 ms (FCFS), 320 ms (SJF), 310 ms (Priority),
     and 355 ms (RR).
   - The reference finishes at 315 ms, 325 ms, 315 ms, and 360 ms respectively.
   - This constant 5 ms shift occurs because the reference charges a 5 ms
     context switch for the very first dispatch at time 0/arrival, whereas our
     approved design treats initial dispatch from idle as free (costing 0 ms).
   - Once accounted for, all scheduling sequences, preemption points, burst
     consumptions, and relative intervals match the instructor's reference.

Submit CPUSim.c and this README.txt for grading.
