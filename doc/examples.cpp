#include "../plot.h"
#include "../sigplot.h"

#include <cmath>
#include <algorithm>

/** @file */

int main() {
	{ // Basic example
		signalsmith::plot::Plot2D plot;

		// Customise the axes
		plot.x.major(0).tick(10).label("time");
		plot.y.major(0).minor(-1, 1).label("signal");

		// Add some data with `.add(x, y)`
		auto &sin = plot.line(), &cos = plot.line();
		for (double x = 0; x < 10; x += 0.01) {
			sin.add(x, std::sin(x));
			cos.add(x, std::cos(x));
		}
		sin.label("sin(x)");
		cos.label("cos(x)");

		plot.write("default-2d.svg");
	}
	
	{ // Demonstrating default colour/dash sequence
		signalsmith::plot::Plot2D plot(320, 100);
		
		// It will add default ticks unless you specify something, even a blank list
		plot.y.minor();
		
		// Add a filled triangle for each one
		for (int i = 0; i < 10; ++i) {
			auto &line = plot.line().fillToY(0);
			line.add(i - 0.5, 0);
			line.add(i + 0.5, 1);

			plot.x.tick(i);
		}
		
		plot.write("style-sequence.svg");
	}

	{ // Custom style, using a figure
		signalsmith::plot::Figure figure;
		figure.style.lineWidth = 2;
		figure.style.valueSize = 9;
		figure.style.textAspect = 1.1;
		// Swap the first two colours, the second two dashes, and the 1st/3rd hatches
		std::swap(figure.style.colours[0], figure.style.colours[1]);
		std::swap(figure.style.dashes[1], figure.style.dashes[2]);
		std::swap(figure.style.hatches[0], figure.style.hatches[2]);
		figure.style.suffix = R"CSS(
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

		auto &plot = figure.plot();

		// Customise the axes
		plot.x.major(0).tick(10).label("time");
		plot.y.major(0).minor(-1, 1).label("signal");

		// Add some data with `.add(x, y)`
		auto &sin = plot.line().fillToY(0), &cos = plot.line().fillToY(0);
		for (double x = 0; x < 10; x += 0.01) {
			sin.add(x, std::sin(x));
			cos.add(x, std::cos(x));
		}
		sin.label("sin(x)");
		cos.label("cos(x)");

		figure.write("custom-2d.svg");
	}

	{ // Filled lines
		signalsmith::plot::Plot2D plot(200, 200);
		// No ticks or grid
		plot.x.major();
		plot.y.major();

		auto circle = [&](double x, double y, double r) -> signalsmith::plot::Line2D & {
			auto &line = plot.fill();
			for (double a = 0; a < 2*M_PI; a += 0.05) {
				line.add(x + std::cos(a)*r, y + std::sin(a)*r);
			}
			return line;
		};
		circle(0, 0, 1.25).label(-0.5, -0.5, "A");
		circle(0, 1, 1.25).label(-0.5, 1.5, "B");
		circle(1, 1, 1.25).label(1.5, 1.5, "C");
		circle(1, 0, 1.25).label(1.5, -0.5, "D");
		
		plot.write("filled-lines.svg");
	}
}
