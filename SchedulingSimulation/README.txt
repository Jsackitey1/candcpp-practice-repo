DEVELOPMENT PROCESS

I gave the AI the full assignment specification and asked it to propose a
plan before writing any code. The plan covered scheduling rules, tie-breaking,
context switch accounting, and edge cases. After I reviewed and approved it,
the AI generated the complete first implementation of CPUSim.c — all four
schedulers, input parsing, statistics, and output — in one pass. I then
worked with the AI iteratively: supplying the instructor's reference output,
asking it to explain differences, having it write tests, and adding the
//-- process lifecycle comments for submission. The approach was AI-writes-
first, then I review, test, and request targeted changes.

LLM USED

- OpenAI Codex (GPT-6), accessed through the Codex workspace/chat interface.
  Used for the initial plan, full C implementation, and test suite.
- Google Gemini (Gemini 3.8 Flash), accessed via the Google Antigravity IDE.
  Used for adding //-- lifecycle comments and final submission preparation.

IMPORTANT PROMPTS

1. "Help me implement this CPU Scheduling Simulation. Propose a plan
   covering scheduling rules, data structures, and edge cases. I have to
   approve the plan first before any code is written."

2. "Yes" — approving the plan so coding could begin.

3. "Write a Python test suite with hand-calculated fixtures that checks
   zero wait, simultaneous arrivals, ties in priority/burst, arrivals
   during a 5 ms switch, quantum boundary conditions, 1000 processes,
   and bad input handling."

4. "Compare our output with the instructor's testOut.txt. Analyze why
   finish times differ by 5 ms."

5. "Explain all these processes to me in detail and how each step works."

6. "Review my thematic comments to make sure that they accurately describe
   what the methods are doing in each case."

TESTING PROCESS

The code compiles with -std=c11 -Wall -Wextra -Wpedantic -O2 and produces
zero warnings. A Python test suite (tests/test_sim.py) runs 36 black-box
cases covering all four schedulers in default and verbose modes. Tests
include hand-calculated small workloads (single process, equal-priority
pairs, preemption scenarios, arrivals during switches, RR quantum boundaries,
1000 simultaneous processes) and invalid input rejection (bad counts, missing
fields, malformed integers, overflow, duplicate IDs, unsorted arrivals,
missing files, bad flags). UndefinedBehaviorSanitizer passed all 36 cases.
Runtime assertions inside CPUSim.c verify that busy + switching + idle
equals the final clock and that every process finishes with zero remaining
work and nonnegative wait time.
