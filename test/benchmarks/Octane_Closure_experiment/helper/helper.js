////////////////////////////////////////////////////////////////////////////////
// Runner
////////////////////////////////////////////////////////////////////////////////

var success = true;

function NotifyStart(name) {
}

function NotifyError(name, error) {
    WScript.Echo(name + " : ERROR : " + error.stack);
    success = false;
}

function NotifyResult(name, score) {
    if (success) {
        WScript.Echo("### SCORE:", score);
    }
}

function NotifyScore(score) {
}

BenchmarkSuite.RunSuites({
    NotifyStart: NotifyStart,
    NotifyError: NotifyError,
    NotifyResult: NotifyResult,
    NotifyScore: NotifyScore
});