examples: out/examples
	cd examples && ../out/examples

out/examples: *.cpp ../*.h
	mkdir -p out
	g++ -std=c++11 -g -O3 \
 		-Wall -Wextra -Wfatal-errors -Wpedantic -pedantic-errors \
		examples.cpp -o out/examples

clean:
	rm -rf out


doxygen:
	doxygen Doxyfile-local

	cp examples.cpp html/examples.cpp

	mkdir -p html/doc
	mkdir -p html/doc/examples
	cp -r examples/* html/doc/examples/

publish: examples doxygen
	publish-signalsmith-raw /code/plot
	cd .. && publish-signalsmith-git /code/plot.git