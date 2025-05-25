# qiti

![Your Project Logo](docs/logo.png)

Qiti is a lightweight C++20 library that brings profiling and instrumentation directly into your unit tests. 

By integrating seamlessly with your test framework of choice, Qiti lets you measure execution times, track custom metrics, and gather performance insights without ever leaving your test suite.

Qiti’s most powerful feature is its integration with Clang’s Thread Sanitizer: tests can be run in isolation under TSan, automatically detecting data races and other thread-safety issues. You can even enforce custom thread-safety behavior right from your test code, catching concurrency bugs early in CI.

## Requirements: 

- C++20
- Clang or Apple Clang (additional compiler support TBD)
- Your unit-test executable must be compiled with optimizations disabled (e.g. -O0) to ensure accurate profiling and sanitization.

## CMake Integration

To integrate Qiti into your CMake-based project, add Qiti as a subdirectory and link against the `qiti_lib` target provided by the library:

```cmake
# Add Qiti to your project
add_subdirectory(path/to/qiti)

# Link your test executable with Qiti
add_executable(my_tests
    tests/test_my_component.cpp
    # etc.
)

target_link_libraries(my_tests
    PRIVATE
        qiti_lib                # Qiti library target
        Catch2::Catch2WithMain  # or your chosen test framework
        # etc.
)
```

By linking against `qiti_lib`, Qiti automatically propagates:

- **Include directories**: `${CMAKE_CURRENT_SOURCE_DIR}/include`
- **Compiler flags** (via `INTERFACE`):
  - `-finstrument-functions`       (enable function instrumentation)
  - `-fno-omit-frame-pointer`     (preserve frame pointers)
  - `-fsanitize=thread`            (enable Thread Sanitizer)
  - `-g`                           (generate debug symbols)
  - `-O0`                          (disable optimizations)
- **Linker flags** (via `INTERFACE`):
  - `-fsanitize=thread`

You do not need to add these flags yourself—just ensure you're using Clang with C++20 and building your test executable with `-O0` (or Debug-only).


## License

Qiti is licensed under the [MIT License](LICENSE).
