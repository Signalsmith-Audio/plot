#ifndef _TEST_FRAMEWORK_BENCHMARKS_H
#define _TEST_FRAMEWORK_BENCHMARKS_H

#include "./tests.h"

#include "../csv-writer.h"
#include "../stopwatch.h"
#include <chrono>

#include <memory>

template<class... Args>
class Benchmark {
	CsvWriter csv;
	
	struct VirtualRunner {
		std::string name;
		VirtualRunner(std::string name) : name(name) {}
		virtual ~VirtualRunner() {}
		virtual double run(Args ...args) = 0;
	};
	std::vector<std::unique_ptr<VirtualRunner>> runners;
	
	template<class Impl>
	struct Runner : public VirtualRunner {
		Benchmark &benchmark;
		Runner(Benchmark &benchmark, std::string name) : VirtualRunner(name), benchmark(benchmark) {}
		
		double run(Args ...args) override {
			// Set things up
			Impl impl(std::forward<Args>(args)...);
			
			double totalSeconds = 0;
			
			Stopwatch stopwatch;
			int repeats = 1;
			double seconds = benchmark.testSeconds;
			double splitSeconds = std::min(seconds, benchmark.splitSeconds);
			while (repeats < 1e8) {
				auto begin = std::chrono::high_resolution_clock::now();
				stopwatch.start();
				for (int r = 0; r < repeats; ++r) {
					impl.run();
				}
				stopwatch.lap();
				auto end = std::chrono::high_resolution_clock::now();
				auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
				totalSeconds = elapsed.count()*1e-9;
				
				if (totalSeconds >= splitSeconds) break;
				repeats *= 2;
			};

			int split = 1; // our first split (while figuring out the repeats) counts
			int maxSplits = std::ceil(seconds/splitSeconds*10);
			while (split < maxSplits && totalSeconds < seconds) {
				auto begin = std::chrono::high_resolution_clock::now();
				stopwatch.startLap();
				for (int r = 0; r < repeats; ++r) {
					impl.run();
				}
				stopwatch.lap();
				auto end = std::chrono::high_resolution_clock::now();
				auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
				totalSeconds += elapsed.count()*1e-9;
				++split;
			}
			return stopwatch.optimistic()/split/repeats;
		}
	};
		
public:
	template<class... Columns>
	Benchmark(std::string name, Columns... columns) : csv(name) {
		static_assert(sizeof...(Args) <= sizeof...(Columns), "missing some column names");
		static_assert(sizeof...(Args) >= sizeof...(Columns), "too many column names");
		csv.write(columns...);
	}
	
	double testSeconds = 1;
	double splitSeconds = 0.1;
	bool verbose = false;
	
	void reset(std::string name) {
		csv = CsvWriter(name);
	}
	
	template<class Impl>
	void add(std::string name) {
		VirtualRunner *runner = new Runner<Impl>(*this, name);
		runners.push_back(std::unique_ptr<VirtualRunner>(runner));
		csv.write(name);
	}
	
	void run(Args ...args) {
		run(args..., 1);
	}
	void run(Args ...args, double referenceComplexity) {
		csv.line(); // Finish the declaration
		csv.write(args...);
		for (auto &runner : runners) {
			double speed = runner->run(args...)/referenceComplexity;
			csv.write(speed);
		}
	}
};

#endif
