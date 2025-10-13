
/******************************************************************************
 * TSan Log Location
 ******************************************************************************/

extern "C"
{
    const char* __tsan_default_options()
    {
        return "log_path=/tmp/tsan_report.txt";
    }
}

// Other useful options:
// "halt_on_error=1"           Stop execution on first error
// "exitcode=42"               Return specific exit code on detection
// "verbosity=2"               Increase diagnostic output
// "log_path=stdout"           Write to stdout instead of file
