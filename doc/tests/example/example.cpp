#include <test/tests.h>
#include <csv-writer.h>

#include "../../sigplot.h"

#include <cmath>

using Plot = signalsmith::plot::Plot;

TEST("Example plot", example) {
	Plot plot;
	plot.style.prefix = "@import \"/style/article/dist.css\";";
	plot.style.suffix = R"CSS(
		.svg-plot-label, .svg-plot-value{
			font-family: inherit;
		}
	)CSS";

	auto &axes = plot.axes();
	axes.x.major(0).tick(10);
	axes.y.major(0).minor(-1, 1);
	
	auto &sin = axes.line(), &cos = axes.line();
	for (double x = 0; x < 10; x += 0.01) {
		sin.add(x, std::sin(x));
		cos.add(x, std::cos(x));
	}
	sin.label("sin(x)");
	cos.label("cos(x)");
	
	plot.write("example.svg");
	
	return test.pass();
}
