# Testing AutoBleem

## Quick Start

```bash
# Build and run all tests
make sys
make test

# Or run manually with CTest
cd build_sys
ctest --output-on-failure
```

## Running Specific Tests

```bash
cd build_sys/tests

# Run a specific test executable
./<test_name>

# Run specific test suite (Google Test)
./<test_name> --gtest_filter=SuiteName.*

# Run single test
./<test_name> --gtest_filter=SuiteName.TestName

# List available tests
./<test_name> --gtest_list_tests
```

## Test Data

Test data files are included in `tests/test_data/` and checked into the repository.
All tests should pass out of the box.

## Troubleshooting

**Tests not building?**
```bash
cd build_sys
cmake .. -DBUILD_TESTS=OFF
make
```

**Wrong working directory?**
Always run tests from `build_sys/` or `build_sys/tests/`:
```bash
cd build_sys
./tests/chd_reader_test
```

## Notes

- Tests only build for x86_64 (local development)
- ARM builds (`make arm`) skip tests automatically
- Google Test is downloaded automatically on first build
