# Squirrel script profiler

Measures the engine's Squirrel execution: **how often each function is called**, **how
long it takes** inclusive and exclusive of its callees, **which function called it**,
**which frames were expensive**, and **which source lines** the vm spends its time on.

It is compiled in behind the `SCRIPT_PROFILER` build option and off until you ask for it,
like the debug server. Results come out two ways: a **report file**, and — when the debug
server is also running — **`/api/profiler` endpoints**.

## Enabling it

```sh
av /path/to/project/avSetup.cfg --profileScripts                    # log a summary at exit
av /path/to/project/avSetup.cfg --profileScripts /tmp/profile.json  # also dump the full report
av /path/to/project/avSetup.cfg --debugServer --profileScripts      # query it live as well
```

- `--profileScripts [path]` — collect from engine startup. The optional path is where the
  full report is written at shutdown; the format follows the extension (`.json` → JSON,
  anything else → the text table). With or without a path, a top-10 summary goes to the
  engine log at shutdown.
- `--profileScriptsLines <0|1>` — per line timing, on by default. This is the profiler's
  most expensive collector because it does work on every executed line rather than every
  call, so turn it off when measuring something call heavy.

As with `--debugServer`, put these **after** the positional `avSetup.cfg` path.

The report is written during engine shutdown, so let the engine close normally — kill it
and you get nothing. Over the debug server, `_shutdownEngine()` through `POST /api/eval`
closes it cleanly, or use `POST /api/profiler/dump` to write a report without exiting.

## Reading a report

```
==== Squirrel script profile ====
Stopped, collected for 19.73s over 1005 frames
Script time 0.112s (0.6% of wall) | 1156 calls | 18 functions | 42311 lines executed

   excl ms    incl ms        calls     avg us     max us  excl%  function
------------------------------------------------------------------------------------
     63.38      63.38          103     615.38    1161.17  56.4%  move    squirrelEntry.nut:75
     46.79     110.17         1034     106.55    1207.83  41.7%  update  squirrelEntry.nut:122
```

- **exclusive** — time in the function itself, *not* counting Squirrel functions it
  called. This is what you sort by to find the slow code.
- **inclusive** — time from entry to return, callees included. Only the outermost frame of
  a recursive chain contributes, so recursion is not multiply counted.
- The file dump also carries the call edges, the per line breakdown and the frame timeline.

## Endpoints

Available when the engine is run with both `--debugServer` and `--profileScripts`. Without
the latter they answer `{"enabled": false}` with a hint (`409` for the POSTs).

| Endpoint | Description |
|---|---|
| `GET /api/profiler?sort=<key>&max=<n>&minCalls=<n>&include=<sections>` | Top functions. `sort` is `exclusive` (default), `inclusive`, `calls` or `avg`; `max` defaults to 25. |
| `GET /api/profiler/function/<id>` | One function in full: callers, callees, hottest lines. |
| `GET /api/profiler/frames?max=<n>&worst=<bool>` | Per frame script time and call count, with each frame's most expensive root call. |
| `GET /api/profiler/lines?max=<n>` | Hottest source lines by self time. |
| `POST /api/profiler/start` \| `/stop` \| `/reset` | Control collection. **Mutating.** |
| `POST /api/profiler/dump?path=<file>&format=json\|text` | Write the full uncapped report to disk. **Mutating.** |

Bulk sections are opt-in on `/api/profiler`: pass `include=edges,lines,frames` for any
combination. The default response is just the function table.

**`reset` is the important one.** Collection starts at engine startup, so a session is
dominated by load-time work. To profile one specific thing, `POST /api/profiler/reset`,
do the thing, then read the report:

```sh
curl -s -X POST localhost:8788/api/profiler/reset
# ... play, inject input, eval something ...
curl -s "localhost:8788/api/profiler?sort=exclusive&max=15" | jq
curl -s localhost:8788/api/profiler/function/14 | jq          # drill into the top entry
```

Function `id`s are stable across a `reset`, so you can reset and keep drilling into the
same function.

## What it can and cannot see

- **Native engine functions are invisible.** Squirrel's debug hook does not fire for C++
  closures, so `_scene.*`, `_gui.*`, physics and rendering never appear. Their cost lands
  in the **exclusive** time of the Squirrel function that called them. High exclusive time
  means "slow itself **or** calling something expensive in C++" — check the line
  breakdown to tell which.
- **Expect Squirrel to run roughly 1.5–3× slower** while profiling. It is a measurement
  tool, not a monitor, and it perturbs what it measures: functions called very often with
  small bodies look relatively worse than they are.
- **In a `Debug` build the absolute per line numbers are inflated**, because the
  profiler's own per line work is a large share of what it measures. Relative ordering
  still holds. `SCRIPT_PROFILER` is a separate build option from `DEBUGGING_TOOLS`
  precisely so you can profile an optimised build.
- The profiler and the Squirrel debugger share the vm's single debug hook via
  `SquirrelHookDispatcher` ([src/Scripting/SquirrelHookDispatcher.h](../src/Scripting/SquirrelHookDispatcher.h)),
  so breakpoints still work with profiling on.
