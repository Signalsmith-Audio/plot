# C++ utility template

This is a starting-point for developing a C++ library.  It includes a `doc/` directory which runs tests.

## Tests

Tests are `.cpp` files in `doc/tests/`, and are run (from `doc/`) with `make test`.  Individual sub-directories (`tests/foo`) can be run with `make test-foo`.

After compiling and running the tests, any Python scripts in the test directory are run as well.
