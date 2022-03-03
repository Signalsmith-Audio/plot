#include <test/tests.h>

#include <csv-writer.h>

TEST("Example test", example) {
	double r = test.random(-1, 1);

	// Some assertions
	TEST_ASSERT(r >= -1);
	TEST_ASSERT(r < 1);
	// Prints indented
	test.log("OK");
	
	// CSV data
	CsvWriter csv("example");
	csv.line("x", "x^2");
	for (double x = 0; x < 1; x += 0.01) {
		csv.line(x, x*x);
		// can also use csv.write(...) to avoid newlines
	}
}
