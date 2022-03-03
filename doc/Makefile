all: test

clean:
	# Tests and analysis
	rm -rf out

############## Testing ##############
#
# make test
#	builds all tests (.cpp files) in tests/, excluding directories starting with "_"
#
# make test-foo
#	builds all tests in tests/foo/, excluding directories starting with "_"
#
# make benchmark-foo
#	builds all tests in benchmarks/foo/, excluding directories starting with "_"
#
# For any of these, you can specify SEED=??? in the environment to reproduce randomised tests

# Used for plots and stuff
export PYTHONPATH=$(shell cd util && pwd)

test: out/test
	mkdir -p out/analysis
	cd out/analysis && ../test
	cd out/analysis && find ../../tests -iname \*.py -print0 | xargs -0 -n1 python

out/test: $(shell find .. -iname "*.h") $(shell find tests -iname "*.cpp")
	@TEST_CPP_FILES=$$(find tests -iname "*.cpp" -not -path "*/_*/*" | sort) ;\
	echo "Building tests:" ;\
	echo "$${TEST_CPP_FILES}" | sed 's/^/     /' ;\
	mkdir -p out ;\
	time g++ -std=c++11 -Wall -Wextra -Wfatal-errors -g -O3 -ffast-math \
 		-Wpedantic -pedantic-errors \
		"util/test/main.cpp" -I "util" \
		-I tests/ $${TEST_CPP_FILES} \
		-I "../" \
		-o out/test

## Individual tests

test-% : out/test-%
	mkdir -p out/analysis
	cd out/analysis && ../test-$*
	cd out/analysis && find ../../tests/$* -iname \*.py -print0 | xargs -0 -n1 python

python-%:
	cd out/analysis && find ../../tests/$* -iname \*.py -print0 | xargs -0 -n1 python

out/test-%: $(shell find .. -iname "*.h") $(shell find tests/$* -iname "*.cpp")
	TEST_CPP_FILES=$$(find tests/$* -iname "*.cpp" -not -path "*/_*/*" | sort) ;\
	echo "Building tests:" ;\
	echo "$${TEST_CPP_FILES}" | sed 's/^/     /' ;\
	mkdir -p out ;\
	g++ -std=c++11 -Wall -Wextra -Wfatal-errors -g -O3 -ffast-math \
 		-Wpedantic -pedantic-errors \
		"util/test/main.cpp" -I "util" \
		-I tests/ $${TEST_CPP_FILES} \
		-I "../" \
		-o out/test-$*
