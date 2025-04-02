#include "../plot.h"
#include "../heatmap.h"

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
	
	{ // Demonstrating default colour/dash/hatch/marker sequence
		signalsmith::plot::Plot2D plot(320, 80);
		
		// Prevent it from adding default ticks
		plot.y.blank();
		
		// Add a filled triangle for each one
		for (int i = 0; i < 10; ++i) {
			auto &line = plot.line().fillToY(0);
			line.add(i - 0.5, 0);
			line.add(i + 0.5, 1);
			line.marker(i, 0.5);

			plot.x.tick(i);
		}
		
		// Remove bottom ticks
		auto style = signalsmith::plot::PlotStyle::defaultStyle().copy();
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

	{
		auto prevDefault = signalsmith::plot::PlotStyle::defaultStyle().copy();
		// Change the default style
		signalsmith::plot::PlotStyle::defaultStyle() = customStyle();
		
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

		figure.write("custom-default-2d.svg");
		
		// Put the default back
		signalsmith::plot::PlotStyle::defaultStyle() = prevDefault;
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

	{ // Legend
		signalsmith::plot::Plot2D plot(250, 150);

		auto &fillFast = plot.fill(1); // Add the most important things first, so they're on top
		auto &line = plot.line(0), &fillSlow = plot.fill(0);

		for (double d = 0; d < 10; d += 0.05) {
			double v = std::sin(d);
			double r = (10 - d)/10;
			line.add(d, v);
			fillSlow.add(d, v*r);
			fillFast.add(d, v*r*r);
		}
		auto &legend = plot.legend(0, 0); // to left of axis, vertically centered
		legend.line(line, "signal").fill(fillSlow, "slow decay").fill(fillFast, "fast decay");
		
		plot.y.major(0).ticks(-1, 1);
		plot.x.major(0, "min").minor(10, "max").label("time");
		
		auto style = signalsmith::plot::PlotStyle::defaultStyle().copy();
		style.fillOpacity = 0.6;
		style.cssSuffix = ".svg-plot-legend{fill: none}";
		plot.write("legend.svg", style);
	}

	{ // Grid
		signalsmith::plot::Figure figure;
		// Draws when this goes out of scope
		auto scheduledWrite = figure.writeLater("grid.svg");

		// Cell access with (col, row)
		auto &mainPlot = figure(0, 0).plot(100, 100);
		auto &topPlot = figure(0, -1).plot(100, 30);
		auto &leftPlot = figure(-1, 0).plot(30, 100);

		{
			auto &line = mainPlot.line();
			for (double a = 0; a < 2.1*M_PI; a += 0.0001) {
				line.add(std::cos(a), std::sin(a));
			}
			mainPlot.x.linear(-1, 1).minors(-1, 0, 1).blankLabels();
			mainPlot.y.linear(-1, 1).flip().minors(-1, 0, 1).blankLabels();
		}
		{
			topPlot.x.linear(-1, 1).minors(-1, 0, 1).blankLabels();
			topPlot.y.minors(0, 1).flip();
			auto &line = topPlot.line().fillToY(0);
			for (double x = -1; x < 1; x += 0.0001) {
				double x2 = std::sin(x*M_PI/2);
				line.add(x2, std::sqrt(1 - x2*x2));
			}
		}
		{
			leftPlot.y.linear(-1, 1).minors(-1, 0, 1).blankLabels();
			leftPlot.x.linear(1, 0).minors(0, 1);
			auto &line = leftPlot.line().fillToX(0);
			for (double x = -1; x < 1; x += 0.0001) {
				double x2 = std::sin(x*M_PI/2);
				line.add(std::sqrt(1 - x2*x2), x2);
			}
		}
	}
	
	{ // Animation
		signalsmith::plot::Plot2D plot;
		plot.x.linear(0, 10).major(0).minor(10).minor(5);
		plot.y.linear(-1, 1).major(0).minors(-1, 1);
		auto &legend = plot.legend(2, 1);
		{
			auto &line = plot.line();
			line.styleIndex.colour = 5;
			for (double p = 0; p < 2*M_PI; p += 1) {
				for (double x = 0; x < 10; x += 0.01) {
					line.add(x, std::sin(x + p));
				}
				line.marker(5, std::sin(5 + p));
				line.toFrame(p);
			}
			line.loopFrame(2*M_PI);
			legend.add(line, "1 frame/rad");
		}
		{
			auto &line = plot.line();
			line.styleIndex.colour = 3;
			for (double p = 0; p < 2*M_PI; p += .1) {
				for (double x = 0; x < 10; x += 0.01) {
					line.add(x, std::sin(x + p));
				}
				line.toFrame(p);
			}
			line.loopFrame(2*M_PI);
			line.fillToY(0);
			legend.add(line, "10 frames/rad", true, true);
		}
		plot.write("animation.svg");
	}

	{ // Embedded image
		signalsmith::plot::Plot2D plot(100, 100);
		plot.x.linear(-1, 1).major(0).minors(-1, 1);
		plot.y.copyFrom(plot.x);
		plot.image({-1, 1, 1, -1}, "https://signalsmith-audio.co.uk/style/images/logo-v3/square@2x.png");
		plot.write("embedded-image.svg");
	}

	{ // Embedded heat-map
		signalsmith::plot::Plot2D plot(200, 150);
		plot.x.linear(-1, 1).major(0).minors(-1, 1);
		plot.y.copyFrom(plot.x);
		
		auto &line = plot.line(-1);
		line.marker(0, 0).label("label", 0, 10);
		line.marker(-0.4, -0.25).label("angled label", 30, 15);
		
		signalsmith::plot::HeatMap heatMap(101, 101);
		heatMap.scale.linear(2, 0);
		heatMap.addTo(plot);
		for (int x = 0; x <= 100; ++x) {
			for (int y = 0; y <= 100; ++y) {
				double sx = x/100.0;
				double sy = y/100.0;
				heatMap(x, y) = sx + sy;
			}
		}
		heatMap.write("heat-map.png");

		plot.write("embedded-heat-map.svg");
	}

	for (int i = 0; i < 2; ++i){ // Heat-map with automatically added scale
		signalsmith::plot::HeatMap heatMap(201, 201);
		heatMap.light = (i == 1);
		heatMap.scale.linear(0, 1).minors(0, 0.5, 1);

		signalsmith::plot::Figure figure;
		auto &plot = heatMap.addTo(figure, 120, 120);
		plot.x.linear(-1, 1).blank();
		plot.y.copyFrom(plot.x);
		
		for (int x = 0; x <= 200; ++x) {
			for (int y = 0; y <= 200; ++y) {
				double sx = x/100.0 - 1;
				double sy = y/100.0 - 1;
				
				double d2 = sx*sx + 2*sy*sy - 2*sx*sy;
				heatMap(x, y) = std::exp(-2*d2);
			}
		}
		figure.write("embedded-heat-map-with-scale" + std::string(heatMap.light ? "-light" : "") + ".svg");
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
	// Minified version of `examples/wiggle.js`
	// This JS won't run inside an <img> tag - view the image itself, or embed it as <object>.
	style.scriptSrc = R"JS(!function(t,a,o){Array.from(t.querySelectorAll("style")).pop().textContent+=' .svg-plot-value,.svg-plot-label{font-family:"Geraint Dense","Comic Sans MS",Arial,sans-serif}';t.querySelectorAll("path").forEach(function(e){function v(){return 2*(o()-.5)}var r=e.getAttribute("d");function n(){var l,i,u=40*o(),f=!0,s=v(),c=v(),p=v(),m=v(),t=r.replace(/(-?[0-9\.]+) (-?[0-9\.]+)/g,function(t,e,r){let o="";return function t(e,r){if(e=parseFloat(e),r=parseFloat(r),!f){var n=e-l,a=r-i;if(20<(n=Math.sqrt(n*n+a*a)))return t(.5*(l+e),.5*(i+r)),t(e,r);40<(u+=n)&&(u=0,s=c,p=m,c=v(),m=v())}f=!1,l=e,i=r;a=u/40;e+=s+(c-s)*a,r+=p+(m-p)*a,o+=" "+e+" "+r}(e,r),o});e.setAttribute("d",t),a(n,240*(.9+0*o()))}a(n,240*o())})}(document,setTimeout,Math.random);)JS";
	return style;
}
