"""Black-box checks against hand-calculated completion times and metrics."""
import math
from pathlib import Path
import subprocess
import sys
import tempfile

# =============================================================================
# THEME 1: Subprocess Test Harness & Environment Setup
# =============================================================================
# Resolves the compiled simulator binary under test and provisions isolated
# temporary environments for capturing stdout, stderr, and process return codes.

PROGRAM = str(Path(sys.argv[1]).resolve())
checks = 0


def run(text, args=("-v",)):
    """Executes CPUSim against an isolated input file with specified CLI flags."""
    with tempfile.TemporaryDirectory() as directory:
        path = Path(directory) / "input.txt"
        path.write_text(text)
        return subprocess.run([PROGRAM, str(path), *args], capture_output=True,
                              text=True, timeout=15)


# =============================================================================
# THEME 2: Multi-Policy Oracle & Invariant Verification
# =============================================================================
# Executes workloads across all four scheduling policies (FCFS, SJF, Priority, RR),
# verifies exact completion milestones, and mathematically verifies scheduling
# invariant identities:
#   turnaround = completion - arrival
#   wait = turnaround - burst
#   CPU utilization = (sum(burst) / finish) * 100
#   throughput = (n / finish) * 1000
# Also asserts output conformity between default and verbose (-v) modes.

def verify(name, processes, expected=None):
    """Asserts format integrity, scheduling metrics, and expected completion times."""
    global checks
    text = str(len(processes)) + "\n" + "".join(
        " ".join(map(str, row)) + "\n" for row in processes)
    result = run(text)
    assert result.returncode == 0, (name, result.stderr)
    assert not result.stderr, (name, result.stderr)
    lines = result.stdout.splitlines()
    n = len(processes)
    assert len(lines) == 4 * (6 + n), (name, lines)
    summaries = []
    for policy in range(4):
        block = lines[policy * (6 + n):(policy + 1) * (6 + n)]
        summaries.extend(block[:6])
        rows = [list(map(int, line.split())) for line in block[6:]]
        assert all(len(row) == 5 for row in rows)
        completions = [row[2] for row in rows]
        if expected is not None:
            assert completions == expected[policy], (name, policy, completions, expected[policy])
        for original, row in zip(processes, rows):
            pid, _, arrival, burst = original
            assert row[0:2] == [pid, arrival]
            assert row[3] == row[2] - arrival
            assert row[4] == row[3] - burst and row[4] >= 0
        finish = max(completions)
        metrics = [float(line.split(": ")[1].split()[0].rstrip("%")) for line in block[1:6]]
        wanted = [finish, 100 * sum(p[3] for p in processes) / finish,
                  1000 * n / finish, sum(r[4] for r in rows) / n,
                  sum(r[3] for r in rows) / n]
        assert all(math.isclose(a, b, abs_tol=0.000001) for a, b in zip(metrics, wanted)), name
    default = run(text, ())
    assert default.returncode == 0 and not default.stderr
    assert default.stdout.splitlines() == summaries, name
    checks += 1


def same(values):
    """Helper mapping an identical completion vector across all 4 policies."""
    return [values] * 4


# =============================================================================
# THEME 3: Targeted Behavioral Test Fixtures & Edge Cases
# =============================================================================
# Hand-calculated scenarios designed to exercise critical boundary conditions:
# - Baseline: Single job execution.
# - Idle handling: Initial delays and idle gaps (switch cost waived).
# - Context switching: Overhead penalties on process handoffs (5 ms).
# - Preemption: Immediate preemption for strictly shorter remaining burst / higher priority.
# - Stability: Equal keys never preempt currently executing job.
# - Concurrent arrivals: Jobs arriving during active context switches.
# - Round Robin dynamics: Quantum expiration, rotation, and boundary orderings.
# - Scalability & Scale: 1,000 simultaneous processes and 64-bit timestamps.
# - Assignment workload: Official sample benchmark dataset.

# 3.1 Single process baseline
verify("single", [(1, 1, 0, 5)], same([5]))

# 3.2 Initial idle period; lone RR job continues across quanta without switch overhead
verify("initial idle and lone RR continuation", [(1, 1, 10, 45)], same([55]))

# 3.3 Context switch cost between two consecutive jobs
verify("one switch", [(1, 1, 0, 4), (2, 1, 0, 4)], same([4, 13]))

# 3.4 Idle gap between non-contiguous processes (switch overhead waived)
verify("idle gap", [(1, 1, 0, 2), (2, 1, 10, 3)], same([2, 13]))

# 3.5 Arrival coinciding exactly with previous job completion
verify("arrival at completion", [(1, 1, 0, 2), (2, 1, 2, 3)], same([2, 10]))

# 3.6 Preemption triggered by shorter remaining burst and higher priority
verify("shorter and higher-priority arrival", [(1, 5, 0, 10), (2, 1, 2, 2)],
       [[10, 17], [22, 9], [22, 9], [10, 17]])

# 3.7 Tie-breaking preserves running job when competing keys are equal
verify("equal key retains running job", [(1, 1, 0, 10), (2, 1, 2, 8)], same([10, 23]))

# 3.8 Arrival admitted during an ongoing 5 ms context switch
verify("arrival during switch", [(1, 3, 0, 10), (2, 2, 2, 4), (3, 1, 4, 1)],
       [[10, 19, 25], [30, 17, 8], [30, 17, 8], [10, 19, 25]])

# 3.9 Round Robin quantum rotation between equal-burst tasks
verify("RR rotation", [(1, 1, 0, 25), (2, 1, 0, 25)],
       [[25, 55], [25, 55], [25, 55], [55, 65]])

# 3.10 Arrival coinciding exactly with quantum expiration (arrival enqueued first)
verify("RR arrival at expiration", [(1, 1, 0, 21), (2, 1, 20, 1)],
       [[21, 27], [21, 27], [21, 27], [32, 26]])

# 3.11 Job completion taking precedence over quantum slice expiration
verify("completion beats expiration", [(1, 1, 0, 20), (2, 1, 20, 1)], same([20, 26]))

# 3.12 Numerical priority ordering overriding input file order
verify("priority beats input order", [(1, 3, 0, 4), (2, -1, 0, 4)],
       [[4, 13], [4, 13], [13, 4], [4, 13]])

# 3.13 Scalability stress test with maximum supported capacity (1000 processes)
verify("1000 simultaneous jobs", [(i, 1, 0, 1) for i in range(1000)],
       same([1 + 6 * i for i in range(1000)]))

# 3.14 Safe handling of wide 64-bit integer arrival timestamps
verify("wide arrival time", [(1, 1, 3000000000, 1)], same([3000000001]))

# 3.15 Coursework assignment reference fixture
example = Path(__file__).with_name("example.txt").read_text().splitlines()
verify("assignment example", [tuple(map(int, line.split())) for line in example[1:]])


# =============================================================================
# THEME 4: Negative Testing & Input Validation Rejection
# =============================================================================
# Ensures malformed input files and invalid CLI invocations are cleanly rejected
# with non-zero exit codes and descriptive error diagnostics on stderr.

invalid = ["", "0\n", "1001\n", "1 2\n", "1\n", "1\n1 1 -1 2\n",
           "1\n1 1 0 0\n", "1\n1 1 0 -2\n", "1\n1 x 0 2\n",
           "1\n1 1 0 2 5\n", "1\n1 1 0 2\nextra\n", "1\n1+1 0 2\n",
           "2\n1 1 0 2\n1 2 1 3\n", "2\n1 1 2 2\n2 2 1 3\n",
           "1\n1 1 9223372036854775808 1\n",
           "1\n1 1 9223372036854775807 1\n",
           "2\n1 1 9223372036854775800 1\n2 1 9223372036854775800 2\n"]
for text in invalid:
    result = run(text)
    assert result.returncode != 0 and result.stderr and not result.stdout, text
    checks += 1

# Invalid command-line options
for args in [("--verbose",), ("-v", "extra")]:
    result = run("1\n1 1 0 1\n", args)
    assert result.returncode != 0 and result.stderr and not result.stdout
    checks += 1

# Missing file and non-existent input path
for args in [[], ["/nonexistent/CPUSim-input"]]:
    result = subprocess.run([PROGRAM, *args], capture_output=True, text=True)
    assert result.returncode != 0 and result.stderr and not result.stdout
    checks += 1

print(f"PASS: {checks} test cases (valid cases check all four schedulers and both output modes)")
