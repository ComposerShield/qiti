# qiti

![Your Project Logo](docs/logo.png)

Qiti is a lightweight C++20 library that brings profiling and instrumentation directly into your unit tests. 

By integrating seamlessly with your test framework of choice, Qiti lets you track custom metrics and gather performance insights without ever leaving your test suite.

Qiti’s most powerful feature is its wrapping of Clang’s Thread Sanitizer: tests can be run in isolation under TSan, automatically detecting data races and other thread-safety issues. You can even enforce custom thread-safety behavior right from your test code, catching concurrency bugs early in CI.

## Requirements

- macOS (14 or 15, x86_64 and/or arm64)
- Linux (tested on Ubuntu, Debian, and Fedora; x86_64)
- Clang or Apple Clang (additional compiler support TBD)
- C++20
- CMake
- Your unit-test executable must be compiled with optimizations disabled (e.g. -O0) to ensure accurate profiling and sanitization. 
  - This will be done automatically when linking qiti_lib; however, you must ensure your settings do not override these changes (see "CMake Settings" below).

## CMake Integration

To integrate Qiti into your CMake-based project, add Qiti as a subdirectory and link against the `qiti_lib` target provided by the library:

```cmake
# Add Qiti to your project
add_subdirectory(path/to/qiti)

# Create your unit test executable (e.g. Catch2)
add_executable(my_tests
    tests/test_my_component.cpp
    # etc.
)

# Link any intermediary libraries you build (and wish to profile/instrument) with qiti_lib
target_link_libraries(my_lib
    PRIVATE
        qiti_lib                # General Qiti library target
        Catch2::Catch2WithMain  # (or your chosen test framework)
        # etc.
)

# Link your final unit test executable with qiti_lib and qiti_tests_client
target_link_libraries(my_tests
    PRIVATE
        qiti_lib                # General Qiti library target
        qiti_tests_client       # Qiti target specific to final executable
        Catch2::Catch2WithMain  # (or your chosen test framework)
        # etc.
)
```

## CMake Settings

By linking against `qiti_lib`, Qiti automatically propagates:

- **Include directories**: `./include`
- **Compiler flags** (via `INTERFACE`):
  - `-finstrument-functions`       (enable function instrumentation)
  - `-fno-omit-frame-pointer`     (preserve frame pointers)
  - `-fsanitize=thread`            (enable Thread Sanitizer)
  - `-g`                           (generate debug symbols)
  - `-O0`                          (disable optimizations)
- **Linker flags** (via `INTERFACE`):
  - `-fsanitize=thread`

You do not need to add these flags yourself—just ensure you are using Clang with C++20 and building your test executable with `-O0` (or in Debug mode).

In addition, by linking your unit test executable with `qiti_tests_client`, Qiti automatically propagates:

- **Object File**: `./source/client/qiti_tests_client.cpp`
- **Linker flags** (via `INTERFACE`):
  - `-rdynamic`

## Deployment Target (macOS)

- When compiling on macOS, the deployment target must be 10.15 (Catalina) or later.

## Documentation

Qiti uses Doxygen (via a custom CMake `doxygen` target) to generate HTML API documentation. To build the docs, ensure Doxygen is installed on your system and then run the following from your project root:

```bash
cmake -B build-docs .
cmake --build build-docs --target doxygen
rm -fd build-docs
open docs/html/index.html 
```

This will invoke the `doxygen` target defined in `CMakeLists.txt` (which calls `Doxygen(source docs)`) and generate HTML output under `docs/html`. Once complete, open `docs/html/index.html` in your browser to view the documentation.

If Doxygen is not found, the build will fail with a "Doxygen not found" message—please install Doxygen to enable documentation generation.

### Installing Doxygen

- **macOS (via Homebrew)**  
  ```bash
  brew install doxygen
  ```

- **Ubuntu / Debian**  
  ```bash
  sudo apt-get update
  sudo apt-get install doxygen
  ```

- **Fedora**  
  ```bash
  sudo dnf install doxygen
  ```


## Quick Start

In your unit test file, add
```c++
#include "qiti_include.hpp"
```

Then in each unit test you wish to use Qiti, add a qiti::ScopedQitiTest at the top of the test. For example:
```c++
TEST_CASE("Example Test")
{
    qiti::ScopedQitiTest test;
    
    // your test code here
}
```
Even if you don't call any functions of ScopedQitiTest directly, instantiating it still enables most of the functionality of Qiti and cleans up the state once the test ends.

### Basic Tests
  
Number of times called.
```c++
TEST_CASE("Example Test")
{
    qiti::ScopedQitiTest test;
        
    // Profile one of your functions
    auto funcData = qiti::FunctionData::getFunctionData<&myFunc>();
    REQUIRE(funcData != nullptr);
    
    // Call twice
    myFunc();
    myFunc();

    // Enforce that it was indeed called twice
    CHECK(funcData->getNumTimesCalled() == 2);
}
```
Enforce number of heap allocations.
```c++
TEST_CASE("Example Test")
{    
    qiti::ScopedQitiTest test;
    
    // Profile one of your functions
    auto funcData = qiti::FunctionData::getFunctionData<&myFunc>();
    REQUIRE(funcData != nullptr);
    
    // Call function
    funcData();

    // Get information on last function call
    auto lastFunctionCall = funcData->getLastFunctionCall();

    // Enforce that this function does not heap allocate
    REQUIRE(lastFunctionCall.getNumHeapAllocations() == 0);
}
```
Enforce length of test.
```c++
TEST_CASE("Example Test")
{    
    qiti::ScopedQitiTest test;
    
    // test code here

    // Ensure that the test does not take too long
    REQUIRE(test.getLengthOfTest_ms() < 10);
}
```

### Thread Sanitizer Tests
Detect data races.
```c++
TEST_CASE("Example Test")
{
    qiti::ScopedQitiTest test;

    auto dataRaceDetector = qiti::ThreadSanitizer::createDataRaceDetector();
    auto codeToTest = []()
    {
        std::thread t([]
        {
            functionToRunOnThread0();
        });
        functionToRunOnThread1();
        t.join();
    };
    dataRaceDetector->run(codeToTest);
    REQUIRE(dataRaceDetector->passed()); // No data races detected
}
```

Please refer to the documentation for a full overview of all available features.

## License

Qiti is licensed under the [MIT License](LICENSE).
