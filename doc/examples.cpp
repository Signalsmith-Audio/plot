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
		signalsmith::plot::Plot2D plot(320, 80);
		
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

	{ // Multiple axes
		signalsmith::plot::Plot2D plot(200, 150);
		// Two axes, which occupy the top/bottom halves of the plot
		auto &yUp = plot.newY(0.5, 1);
		auto &yDown = plot.newY(0.5, 0).linear(0, 4);
		// Third axis on right-hand side
		auto &yComposite = plot.newY().flip();
		plot.x.flip(); // Draws on the non-default side (top)
		
		
		// Assign line
		auto &upLine = plot.line(plot.x, yUp).fillToY(0);
		// Explicit axes
		auto &downLine = plot.line(plot.x, yDown);
		// Explicit style index as well (doesn't increment the default style counter)
		auto &downLine2 = plot.line(plot.x, yDown, downLine.styleIndex);
		// Fill path returns back down the other line
		downLine.fillTo(downLine2);
		auto &compositeLine = plot.line(plot.x, yComposite);
		
		std::vector<double> xPoints = {0, 20, 25, 55, 80, 100};
		std::vector<double> upPoints = {100, 180, 150, 150, 220, 185};
		std::vector<double> downPoints = {1, 2, 2, 1, 1, 3, 2};
		std::vector<double> downPoints2 = {0, 0, 1, 0, 0, 1, 1};
		upLine.addArray(xPoints, upPoints, xPoints.size()); // Can set explicit size if data doesn't have `.size()`
		downLine.addArray(xPoints, downPoints); // Otherwise it can guess
		downLine2.addArray(xPoints, downPoints2);
		for (int i = 0; i < 6; ++i) {
			compositeLine.add(xPoints[i], upPoints[i]*0.35 - (downPoints[i] + downPoints2[i])*25);
		}
		// Label at a given X position
		compositeLine.label(80, "estimate");

		yUp.linear(0, 230).major(0).ticks(100, 200).label("bink", upLine.styleIndex);
		yDown.linear(0, 3).ticks(1, 2, 3).label("tork", downLine.styleIndex);
		yComposite.linear(-100, 100).ticks(-100, 0).tick(100, "+100").label("scrimbles (net)");
		plot.x.major(0).minor(100).label("day");

		plot.write("multiple-axes.svg");
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
