#include "../plot.h"

#include <cmath>
#include <algorithm>

signalsmith::plot::PlotStyle customStyle();

int main() {
	{ // Basic example
		signalsmith::plot::Plot2D plot;

		// Customise the axes
		plot.x.major(0).tick(10).label("time");
		plot.y.major(0).minors(-1, 1).label("signal");

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
		signalsmith::plot::Plot2D plot(360, 80);
		
		// Prevent it from adding default ticks
		plot.y.blank();
		
		// Add a filled triangle for each one
		for (int i = 0; i < 10; ++i) {
			auto &line = plot.line().fillToY(0);
			line.add(i - 0.5, 0);
			line.add(i + 0.5, 1);

			plot.x.tick(i);
		}
		
		// Remove bottom ticks
		auto style = plot.defaultStyle();
		style.tickV = 0;
		plot.write("style-sequence.svg", style);
	}

	{ // Custom style, using a figure
		signalsmith::plot::Figure figure;

		auto &plot = figure.plot();

		// Customise the axes
		plot.x.major(0).tick(10).label("time");
		plot.y.major(0).minors(-1, 1).label("signal");

		// Add some data with `.add(x, y)`
		auto &sin = plot.line().fillToY(0), &cos = plot.line().fillToY(0);
		for (double x = 0; x < 10; x += 0.01) {
			sin.add(x, std::sin(x));
			cos.add(x, std::cos(x));
		}
		sin.label("sin(x)");
		cos.label("cos(x)");

		figure.style = customStyle();
		figure.write("custom-2d.svg");
	}

	{ // Filled circles
		signalsmith::plot::Plot2D plot(200, 200);
		// No ticks or grid
		plot.x.blank();
		plot.y.blank();

		auto circle = [&](double x, double y, double r) -> signalsmith::plot::Line2D & {
			auto &line = plot.fill();
			for (double a = 0; a < 2*M_PI + 0.05; a += 0.05) {
				line.add(x + std::cos(a)*r, y + std::sin(a)*r);
			}
			return line;
		};
		circle(0, 0, 1.25).label(-0.5, -0.5, "A");
		circle(0, 1, 1.25).label(-0.5, 1.5, "B");
		circle(1, 1, 1.25).label(1.5, 1.5, "C");
		circle(1, 0, 1.25).label(1.5, -0.5, "D");
		
		plot.styleCounter.dash = 0; // Those were just fills, so we reset the dash counter
		circle(0.5, 0.5, 2).drawFill(false).drawLine(true).label(0.5, -1.5, "outer boundary", 90);
		
		plot.write("filled-circles.svg");
	}
}

signalsmith::plot::PlotStyle customStyle() {
	signalsmith::plot::PlotStyle style;
	style.lineWidth = 2;
	style.valueSize = 9;
	style.fontAspectRatio = 1.1;
	style.fillOpacity = 0.6;
	style.tickH = style.tickV = 0;
	
	// Swap the first two colours, the second two dashes, and the 1st/3rd hatches
	std::swap(style.colours[0], style.colours[1]);
	std::swap(style.dashes[1], style.dashes[2]);
	std::swap(style.hatches[0], style.hatches[2]);
	
	style.cssSuffix = R"CSS(
		.svg-plot-value, .svg-plot-label {
			font-family: Verdana,sans-serif;
		}
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
	return style;
}
