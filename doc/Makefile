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

	mkdir -p html/doc
	cp examples.cpp html/doc/examples.cpp

	mkdir -p html/doc/examples
	cp -r examples/* html/doc/examples/
