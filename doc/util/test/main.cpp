#include "../simple-args.h"
#include "../console-colours.h"

#include "tests.h"
#include <cstdlib> // srand, rand
#include <ctime> // time
#include <cstdlib> // getenv
#include <string>

TestList _globalTestList;

void Test::run(int depth, bool silent) {
	if (running) return fail("Re-entered test function");
	if (!silent) {
		std::cerr << Console::Dim;
		for (int i = 0; i < depth - 1; i++) {
			std::cerr << "  >  ";
		}
		std::cerr << Console::Cyan << "Test: "
			<< Console::Reset << Console::Cyan << testName
			<< Console::White << " (" << codeLocation << ")" << Console::Reset << std::endl;
	}
	running = true;

	runFn(*this);

	running = false;
}

void TestList::add(Test& test) {
	if (currentlyRunning.size() > 0) {
		Test *latest = currentlyRunning[0];
		if (!latest->success) return;
		// This is a sub-test, run it immediately instead of adding it
		currentlyRunning.push_back(&test);
		test.run(currentlyRunning.size(), currentlySilent);
		currentlyRunning.pop_back();
		return;
	}
	tests.push_back(test);
}

static void printFailure(const std::string &reason) {
	std::cerr << Console::Red << Console::Bright << "\nFailed: "
		<< Console::Reset << reason << "\n\n";
}

void TestList::fail(std::string reason) {
	for (auto testPtr : currentlyRunning) {
		testPtr->fail(reason);
	}
	if (exitOnFail) {
		printFailure(reason);
		std::exit(1);
	}
}

int TestList::run(int repeats) {
	currentlySilent = false;
	for (int repeat = 0; repeat < repeats; repeat++) {
		for (unsigned int i = 0; i < tests.size(); i++) {
			Test& test = tests[i];
			currentlyRunning = {&test};
			// Reset the random engine, so that we can reliably re-run individual tests
			randomEngine.seed(randomSeed + repeat);

			test.run(0, currentlySilent);

			if (!test.success) {
				printFailure(test.reason);
				return 1;
			}
		}
		currentlySilent = true;
	}
	currentlyRunning.resize(0);
	return 0;
}

#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
void errorHandler(int sig) {
	void *array[10];
	size_t size;

	// get void*'s for all entries on the stack
	size = backtrace(array, 10);

	// print out all the frames to stderr
	fprintf(stderr, "Error: signal %d:\n", sig);
	backtrace_symbols_fd(array, size, STDERR_FILENO);
	exit(1);
}

double defaultBenchmarkTime = 1;
int defaultBenchmarkDivisions = 5;

int main(int argc, char* argv[]) {
	signal(SIGSEGV, errorHandler);
	
	SimpleArgs args(argc, argv);
	args.helpFlag("help");

	int repeats = args.flag<int>("repeats", "loop the tests a certain number of times", 1);
	defaultBenchmarkTime = args.flag<double>("test-time", "target per-test duration for benchmarks (excluding setup)", 1);
	defaultBenchmarkDivisions = args.flag<double>("test-divisions", "target number of sub-divisions for benchmarks", 5);
	long long seed = time(NULL);
	if (const char *seedInt = std::getenv("SEED")) {
		seed = std::stoll(seedInt);
	}
	int randomSeed = args.flag<int>("seed", "random seed", seed);
	args.errorExit();

	srand(randomSeed);
	_globalTestList.setRandomSeed(randomSeed);
	std::cout << Console::Dim << "SEED=" << randomSeed << Console::Reset << "\n";
	return _globalTestList.run(repeats);
}
