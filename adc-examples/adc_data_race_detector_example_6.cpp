
/******************************************************************************
 * Data Race Detector - Summary
 ******************************************************************************/

// 1. Compile with -fsanitize=thread -g -fno-omit-frame-pointer

// 2. Configure TSan log location via __tsan_default_options()

// 3. Run potentially racy code in forked process

// 4. Parse TSan log for "data race" in parent process

// 5. Test pass/fail based on result
