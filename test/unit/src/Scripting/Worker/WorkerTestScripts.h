#pragma once

/*
Squirrel fixtures for the script worker tests, held as source strings rather than as .nut files
on disk. ScriptWorker::prepareFromBuffer compiles these through sq_compilebuffer, so a test needs
nothing from the filesystem and the script it is asserting on sits next to the assertions.

R"NUT( ... )NUT" is used so the squirrel reads as squirrel; the delimiter avoids any clash with
parentheses in the script itself.
*/

namespace WorkerTestScripts{

    //Accumulates across runs, so a second dispatch proves the vm was not reset between them.
    static const char* COUNTER = R"NUT(
        function setup(){
            mTotal <- 0;
            mRuns <- 0;
        }

        function run(input){
            mRuns++;
            mTotal += input.amount;

            _workerSelf.setProgress(1.0);

            return {
                total = mTotal,
                runs = mRuns,
                echoed = input.amount
            };
        }
    )NUT";

    //Always fails, for the error reporting path.
    static const char* THROWER = R"NUT(
        function run(input){
            throw "deliberate worker failure";
        }
    )NUT";

    //Loops until the main thread cancels it, for cooperative cancellation and the shutdown drain.
    static const char* SPINNER = R"NUT(
        function run(input){
            local iterations = 0;
            while(!_workerSelf.isCancelled()){
                iterations++;
                //Something to do, so the loop is not reduced to a bare spin on the atomic.
                local unused = sqrt(iterations.tofloat());

                //Bounded, so a broken cancel fails the test rather than hanging the run.
                if(iterations > 200000000) break;
            }

            return { iterations = iterations, cancelled = _workerSelf.isCancelled() };
        }
    )NUT";

    //Declares no run(), which preparing must refuse.
    static const char* NO_RUN = R"NUT(
        function setup(){
            mValue <- 1;
        }
    )NUT";

    //Does not compile, for the compile failure path.
    static const char* SYNTAX_ERROR = R"NUT(
        function run(input){
            this is not squirrel
        }
    )NUT";

    //Reports what run() was given, for checking that the input crossed intact.
    static const char* ECHO = R"NUT(
        function run(input){
            return { received = input, type = typeof input };
        }
    )NUT";
}
