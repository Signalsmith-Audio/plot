#include <test/tests.h>
#include <csv-writer.h>

#include "../../sigplot.h"

#include <cmath>

using Plot = signalsmith::plot::Plot;

TEST("Example plot", example) {
	Plot plot;

	auto &axes = plot.axes();
	axes.x.major(0).tick(10);
	axes.y.major(0).minor(-1, 1);
	
	auto &sin = axes.line(), &cos = axes.line();
	auto &fill = axes.fill(0);
	for (double x = 0; x < 10; x += 0.01) {
		sin.add(x, std::sin(x));
		cos.add(x, std::cos(x));
		fill.add(x, std::sin(x));
	}
	fill.add(10, 0);
	sin.label("sin(x)");
	cos.label("cos(x)");

	plot.write("example.svg");
	
	return test.pass();
}

TEST("Custom styles", custom_styles) {
	Plot plot;
	plot.style.prefix = "@import \"/style/article/dist.css\";";
	plot.style.suffix = R"CSS(
		.svg-plot-label, .svg-plot-value {
			font-family: inherit;
		}
		.svg-plot-fill:hover {
			opacity: 18%;
		}
		.svg-plot-fill.svg-plot-f0 {
			fill: #005AB0;
		}
		text.svg-plot-f0 {
			fill: #006AA0;
		}
		.svg-plot-fill.svg-plot-f4 {
			fill: #EDB410;
		}
	)CSS";
	plot.style.colours = {"#007AB0", "#BB102B", "#44A730", "#87694F", "#EDA720", "#A64D99"};

	auto &axes = plot.axes();
	axes.x.major(0).tick(10);
	axes.y.major(0).minor(-1, plot.tick(-0.5, ""), plot.tick(0.5, ""), 1);
	
	auto &sin = axes.line(), &cos = axes.line();
	auto &fill = axes.fill(0);
	for (double x = 0; x < 10; x += 0.01) {
		sin.add(x, std::sin(x));
		cos.add(x, std::cos(x));
		fill.add(x, std::sin(x));
	}
	fill.add(10, 0);
	sin.label("sin(x)");
	cos.label("cos(x)");

	plot.write("custom.svg");
	
	return test.pass();
}

TEST("Custom styles (2)", custom_styles2) {
	Plot plot;
	plot.style.lineWidth = 2;
	plot.style.valueSize = 9;
	plot.style.textAspect = 1.1;
	plot.style.suffix = R"CSS(
		.svg-plot-axis {
			fill: #EEE;
		}
		.svg-plot-tick {
			stroke: #666;
			stroke-width: 0.75px;
		}
		.svg-plot-value {
			fill: #666;
			opacity: 0.8;
			font-weight: bold;
		}
		.svg-plot-major {
			stroke: #FFF;
			stroke-width: 1.5px;
		}
		.svg-plot-minor {
			stroke: #FFF;
			stroke-width: 0.75px;
			stroke-dasharray: none;
		}
	)CSS";

	auto &axes = plot.axes();
	axes.x.major(0).tick(10);
	axes.y.major(0).minor(-1, plot.tick(-0.5, ""), plot.tick(0.5, ""), 1);
	
	auto &sin = axes.line(), &cos = axes.line();
	auto &fill = axes.fill(0);
	for (double x = 0; x < 10; x += 0.01) {
		sin.add(x, std::sin(x));
		cos.add(x, std::cos(x));
		fill.add(x, std::sin(x));
	}
	fill.add(10, 0);
	sin.label("sin(x)");
	cos.label("cos(x)");

	plot.write("custom2.svg");
	
	return test.pass();
}

TEST("Colour/dash patterns", colour_dash_patterns) {
	Plot plot;
	auto &axes = plot.axes();
	axes.x.major(0).tick(10);
	axes.y.major(0).minor(-1, 1);

	int n = 20;
	for (int i = 0; i < n; ++i) {
		auto &line = axes.line();
		double phase = 2*M_PI*i/n;
		for (double x = 0; x < 10; x += 0.01) {
			line.add(x, std::sin(x + phase));
		}
	}
	
	plot.write("patterns.svg");
	
	return test.pass();
}
