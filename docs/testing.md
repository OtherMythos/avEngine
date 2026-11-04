# Testing

## Running the unit tests

The tests are a GoogleTest executable, the `avUnit` target, built when `BUILD_UNIT_TESTS`
is `ON` (the default). Build the target, then run the binary directly. The commands below
start from inside the build directory.

- **macOS**:
  ```sh
  cd "$AV_ENGINE_DIR/build"
  xcodebuild -project av.xcodeproj -target avUnit -configuration Debug build 2>&1 | grep -iE "error:|BUILD SUCCEEDED|BUILD FAILED" | head
  cd test/unit/Debug && ./avUnit
  ```
  (On Apple platforms there is no shared `avCore` library — the engine sources are compiled
  directly into both `av` and `avUnit`, so an `avUnit` build recompiles the engine sources.)
- **Linux**:
  ```sh
  cmake --build . --target avUnit
  cd test/unit && ./avUnit
  ```
- **Windows**:
  ```sh
  cmake --build . --target avUnit --config Debug
  .\test\unit\Debug\avUnit.exe
  ```

A healthy run ends with a line like `[  PASSED  ] 222 tests.`. Filter to just the summary
and failures with `./avUnit 2>&1 | grep -iE "FAILED|PASSED|OK \]" | tail`.

### Unit tests must be fast — 30ms is the hard ceiling

**No single unit test may take longer than 30ms**, and most should be under 1ms. The whole
`avUnit` binary is meant to run in a fraction of a second so it can be used constantly, as a
matter of course after any change, rather than being something you avoid running. A test that
needs seconds is either an integration test in the wrong place, or is sleeping when it could be
waiting. gtest prints each test's duration, so the offenders are easy to find:

```sh
./avUnit 2>&1 | grep "OK \]" | sed 's/^\[       OK \] //' | sort -t'(' -k2 -rn | head
```

Nearly always the cost is one of these, none of which is inherent to what's being tested:

- **Polling for something instead of blocking on it.** If production code signals a
  `std::condition_variable`, the test should wait on that same variable with a `wait_for`
  timeout — it wakes in microseconds *and* stays bounded, so a regression still fails rather
  than hanging the run. A `while(!done) sleep_for(1ms)` loop costs a full millisecond per
  iteration no matter how fast the thing being waited for actually is.
- **Spawning a thread per operation.** Thread creation dwarfs most operations under test. Wait
  on the real synchronisation primitive from the test thread instead, and reserve extra threads
  for the cases whose whole point is that two threads are involved.
- **A fixed `sleep_for` used as "wait long enough".** Yield in a bounded loop until the
  condition actually holds, or restructure so the case is deterministic.
- **Huge iteration counts on a probabilistic race test.** These have sharply diminishing
  returns; a race worth catching reproduces in hundreds of iterations, not tens of thousands.

If a test genuinely cannot be brought under 30ms, it does not belong in `avUnit` — move it to
the integration suite, where per-test cost is expected.

### Testing private members

Don't `#define private public` to reach engine internals; that hack was removed. Instead the
engine header moves the members a test needs from `private` to `protected`, and a test-only
subclass re-exposes them with `using`. Shared ones live in
[test/unit/src/TestAccessors.h](../test/unit/src/TestAccessors.h); one-off subclasses sit in the
test file itself (`InputManagerTests.cpp`). A subclass rather than a `friend` declaration keeps
production headers unaware of test names and layout.

## Running the integration test suite

The integration tests are a separate pair of repos, not part of this one:

- **[avTests](https://github.com/OtherMythos/avTests)** — one `avSetup.cfg` + `squirrelEntry.nut`
  pair per test case, grouped into plans (`integration/<Plan>/<TestCase>/`). Point at
  `avTestsIntegration.cfg` for routine runs — it covers everything except the `stress/` plan,
  which is slow load/lifetime testing you don't need for a normal check.
- **[avTools/testRunner](https://github.com/OtherMythos/avTools)** — `testRunner.py` drives
  the engine against each test case and collects `avTestFile.txt` (the pass/fail/still-running
  marker the engine writes on exit) into a summary, optionally as JUnit XML.

Run serially against a single engine window and this suite takes on the order of ten minutes
and puts a window on screen once per test case. Three independent levers bring that down to
under two minutes with nothing visible at all — always use `--headless` for any automated or
agent-driven run, for the same reasons as a single engine launch (see
[headless.md](headless.md)). The command that applies all three is in
[AGENTS.md's required verification workflow](../AGENTS.md#required-verification-workflow);
this is what each lever does:

- **`-f "--headless --noDebugger ..."`** — flags forwarded to every engine invocation.
  `--headless` is what stops 200+ windows flashing on screen; `--noDebugger` stops a script
  failure parking a headless engine in a debugger it can't show you, which would otherwise hang
  that test case until the wall-clock timeout. `-f`'s value needs at least two words, or
  `argparse` swallows a single leading `--...` as if it were its own flag — `-f "--headless"`
  alone fails with "expected one argument".
- **`--fixedUpdateRate <n> --physicsUpdateRate <n>`** (engine flags, both required together,
  clamp 1–240) — most test scripts wait on a tick count (a grace period, a retry loop, N
  updates of no change) rather than a fixed real-time duration, so raising the update rate
  compresses those waits proportionally. It does **not** speed up anything keyed to real
  time — animation playback still takes one real second per real second
  elapsed regardless of the rate, so a test dominated by one of those won't get faster this
  way — the per-test timing below is how you find which ones those are. **Both flags must move together**:
  physics steps once per script update only when `PhysicsUpdateRate / FixedUpdateRate == 1`,
  which is what every test asserting an exact collision-event count per update relies on —
  raising `FixedUpdateRate` alone breaks them. 240 is the ceiling both clamp at, so it's the
  natural fast setting rather than an arbitrary choice.
- **`-j <n>`** — how many test cases `testRunner.py` runs at once, as genuine concurrent OS
  processes (a `ThreadPoolExecutor` whose workers block in `subprocess.communicate()`, which
  releases the GIL, so this is real parallelism, not thread-scheduling inside one process). Set
  it to roughly your core count; each engine process pins close to one core, mostly in
  file/resource loading rather than CPU-bound work, so pushing far past the core count buys
  little. Running concurrently buffers each test's console output until it finishes (serial
  output still streams live), so nothing interleaves.

Every test case's wall time is timed to sub-millisecond precision and printed as it finishes
(`[i/total] TestName (Plan) - 1234.567ms`), and written into the JUnit `time` attribute (`-o`)
in fractional seconds. That's the quickest way to find what's actually slow after applying the
levers above — sort the XML descending:

```sh
python3 -c "
import xml.etree.ElementTree as ET
rows = []
for s in ET.parse('/tmp/av-integration-results.xml').getroot().iter('testsuite'):
    for tc in s.iter('testcase'):
        t = tc.get('time')
        if t: rows.append((float(t) * 1000.0, s.get('name'), tc.get('name')))
rows.sort(reverse=True)
for ms, plan, name in rows[:20]:
    print('%8.1fms  %-25s %s' % (ms, plan, name))
"
```

A test that's still slow after `--fixedUpdateRate`/`--physicsUpdateRate` are maxed out is
either waiting on real time (an animation) or doing genuinely heavy work
(chunk/resource loading) rather than idling on a tick count — worth shortening the test itself
rather than expecting the rate flags to help further.
