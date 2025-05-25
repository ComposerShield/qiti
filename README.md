# qiti

![Your Project Logo](docs/logo.png)

Qiti is a lightweight C++20 library that brings profiling and instrumentation directly into your unit tests. 

By integrating seamlessly with your test framework of choice, Qiti lets you measure execution times, track custom metrics, and gather performance insights without ever leaving your test suite.

Qiti’s most powerful feature is its integration with Clang’s Thread Sanitizer: tests can be run in isolation under TSan, automatically detecting data races and other thread-safety issues. You can even enforce custom thread-safety behavior right from your test code, catching concurrency bugs early in CI.

## Requirements: 

- C++20
- Clang or Apple Clang (additional compiler support TBD)
- Your unit-test executable must be compiled with optimizations disabled (e.g. -O0) to ensure accurate profiling and sanitization.

## License

Qiti is licensed under the [MIT License](LICENSE).
