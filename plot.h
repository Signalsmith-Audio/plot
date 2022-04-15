/** Signalsmith's Basic C++ Plots
@copyright Licensed as 0BSD.  If you need anything else, get in touch. */

#ifndef SIGNALSMITH_PLOT_H
#define SIGNALSMITH_PLOT_H

#include <fstream>
#include <memory>
#include <functional>
#include <vector>
#include <array>
#include <cmath>
#include <sstream>

//#include <iostream>
//#define LOG_EXPR(x) std::cout << #x << " = " << (x) << "\n";

namespace signalsmith { namespace plot {

/**	@defgroup Plots Plots
	@brief Basic C++ plotting
	
	To use, set up a `Figure` or `Plot2D`, add elements to it, and then write with `.write("output.svg")`.
	\image html default-2d.svg An example plot
	
	Elements are drawn hierarchically, but generally in reverse order, so you should add your most important elements first.

	Elements can have a "style index" which simultaneously loops through colour/dash/hatch sequences, for increased greyscale/colourblind support.
 		\image html style-sequence.svg

	@{
	@file
**/

static double estimateUtf8Width(const char *utf8Str);

/** Plotting style, used for both layout and SVG rendering.
	The baseline CSS style is produced using various sizes, plus `.colours`, `.dashes` and `.hatches`.  You can also add your own CSS with `.prefix`/`.suffix`.
 		\image html custom-2d.svg
*/
class PlotStyle {
public:
	double padding = 10;
	double labelSize = 12;
	double valueSize = 10;
	double lineWidth = 1.5;
	double fillOpacity = 0.25;
	double hatchWidth = 1;
	double hatchSpacing = 3;
	// Use this scale the text-size estimates if you use a particularly wide font
	double textAspect = 1;
	double tickH = 4, tickV = 5;
	double textPadding = 5;

	/// Extra CSS
	std::string prefix = "", suffix = "";

	std::vector<std::string> colours = {"#0073E6", "#CC0000", "#00B300", "#806600", "#E69900", "#CC00CC"};
	std::vector<std::vector<double>> dashes = {{}, {1.2, 1.2}, {2.8, 1.6}, {5, 4}, {4, 1, 1, 1, 1, 1}, {10, 3}, {4, 2, 1, 2}};

	struct Hatch {
		std::vector<double> angles;
		double lineScale = 1, spaceScale=1;
		Hatch() {}
		Hatch(double angle) : angles({angle}) {}
		Hatch(std::vector<double> angles, double scale=1) : angles(angles), lineScale(scale), spaceScale(scale) {}
		Hatch(std::vector<double> angles, double lineScale, double spaceScale) : angles(angles), lineScale(lineScale), spaceScale(spaceScale) {}
	};
	std::vector<Hatch> hatches = {{}, {-50}, {{30}, 0.9, 0.8}, {{8, 93}, 0.7, 1}};

	std::string strokeClass(int styleIndex) const {
		if (styleIndex < 0 || colours.size() == 0) return "";
		return "svg-plot-s" + std::to_string(styleIndex%(int)colours.size());
	}
	std::string fillClass(int styleIndex) const {
		if (styleIndex < 0 || colours.size() == 0) return "";
		return "svg-plot-f" + std::to_string(styleIndex%(int)colours.size());
	}
	std::string dashClass(int styleIndex) const {
		if (styleIndex < 0 || dashes.size() == 0) return "";
		return "svg-plot-d" + std::to_string(styleIndex%(int)dashes.size());
	}
	std::string hatchClass(int styleIndex) const {
		if (styleIndex < 0 || hatches.size() == 0) return "";
		return "svg-plot-h" + std::to_string(styleIndex%(int)hatches.size());
	}
	
	void css(std::ostream &o) const {
		o << prefix;
		o << R"CSS(
			.svg-plot {
				stroke-linecap: butt;
			}
			.svg-plot-bg {
				fill: none;
				stroke: none;
			}
			.svg-plot-axis {
				stroke: none;
				fill: rgba(255,255,255,0.85);
			}
			.svg-plot-line {
				stroke: blue;
				fill: none;
				stroke-width: )CSS" << lineWidth << R"CSS(px;
				stroke-linejoin: round;
			}
			.svg-plot-fill {
				stroke: none;
				opacity: )CSS" << fillOpacity << R"CSS(;
			}
			.svg-plot-major {
				stroke: #000;
				stroke-width: 1px;
				stroke-linecap: square;
				fill: none;
			}
			.svg-plot-minor {
				stroke: rgba(0,0,0,0.3);
				stroke-width: 0.5px;
				stroke-dasharray: 0.5 1.5;
				stroke-linecap: round;
				fill: none;
			}
			.svg-plot-tick {
				stroke: #000;
				fill: none;
				stroke-width: 1px;
				stroke-linecap: butt;
			}
			.svg-plot-value, .svg-plot-label {
				font-family: Arial,sans-serif;
				fill: #000;
				stroke: rgba(255,255,255,0.7);
				stroke-width: 2px;
				paint-order: stroke fill;

				text-anchor: middle;
				dominant-baseline: central;
				alignment-baseline: baseline;
			}
			.svg-plot-label {
				font-size: )CSS" << labelSize << R"CSS(px;
			}
			.svg-plot-value {
				font-size: )CSS" << valueSize << R"CSS(px;
			}
			.svg-plot-hatch {
				stroke: #FFF;
				stroke-width: )CSS" << hatchWidth << R"CSS(px;
			}
		)CSS";
		
		for (size_t i = 0; i < colours.size(); ++i) {
			o << ".svg-plot-s" << i << "{stroke:" << colours[i] << "}\n";
			o << ".svg-plot-f" << i << ",.svg-plot-t" << i << "{fill:" << colours[i] << "}\n";
		}
		for (size_t i = 0; i < dashes.size(); ++i) {
			auto &d = dashes[i];
			if (d.size() == 0) {
				o << ".svg-plot-d" << i << "{stroke-width:" << (0.9*lineWidth) << "px}\n";
			} else {
				o << ".svg-plot-d" << i << "{stroke-dasharray:";
				for (auto &v : d) o << " " << (v*lineWidth);
				o << "}\n";
			}
		}
		for (size_t i = 0; i < hatches.size(); ++i) {
			auto &h = hatches[i];
			if (h.angles.size()) {
				o << ".svg-plot-h" << i << "{mask:url(#svg-plot-hatch" << i << ")}\n";
			} else {
				// Compensate for the fact that it's not hatched
				o << ".svg-plot-h" << i << "{opacity:" << (fillOpacity*std::sqrt(hatchWidth/hatchSpacing)) << "}\n";
			}
		}
		for (size_t i = 0; i < hatches.size(); ++i) {
			auto &h = hatches[i];
			if (h.lineScale != 1) {
				o << "#svg-plot-hatch" << i << "-pattern{stroke-width:" << hatchWidth*h.lineScale << "px}\n";
			}
		}
		o << suffix;
	}
};

/// Wrapper for slightly more semantic code when writing SVGs
class SvgWriter {
	std::ostream &output;
public:
	SvgWriter(std::ostream &output) : output(output) {}

	SvgWriter & raw() {
		return *this;
	}
	template<class First, class ...Args>
	SvgWriter & raw(First &&first, Args &&...args) {
		output << first;
		return raw(args...);
	}

	SvgWriter & write() {
		return *this;
	}
	template<class First, class ...Args>
	SvgWriter & write(First &&v, Args &&...args) {
		// Only strings get escaped
		return raw(v).write(args...);
	}
	template<class ...Args>
	SvgWriter & write(const char *str, Args &&...args) {
		while (*str) {
			if (*str == '<') {
				output << "&lt;";
			} else if (*str == '&') {
				output << "&amp;";
			} else if (*str == '"') {
				output << "&quot;";
			} else {
				output << (*str);
			}
			++str;
		}
		return write(args...);
	}
	template<class ...Args>
	SvgWriter & write(const std::string &str, Args &&...args) {
		return write(str.c_str(), args...);
	}
	
	template<class ...Args>
	SvgWriter & attr(const char *name, Args &&...args) {
		return raw(" ", name, "=\"").write(args...).raw("\"");
	}
	
	/// XML tag helper, should only ever exist as rvalue reference
	struct Tag {
		SvgWriter &writer;
		bool active = true;
		bool selfClose;

		Tag(SvgWriter &writer, bool selfClose=false) : writer(writer), selfClose(selfClose) {}
		// Move-construct only
		Tag(Tag &&other) : writer(other.writer), selfClose(other.selfClose) {
			other.active = false;
		}
		~Tag() {
			if (active) writer.raw(selfClose ? "/>" : ">");
		}

		template<class ...Args>
		Tag & attr(const char *name, Args &&...args) & {
			writer.attr(name, args...);
			return *this;
		}
		template<class ...Args>
		Tag && attr(const char *name, Args &&...args) && {
			writer.attr(name, args...);
			return std::move(*this);
		}
	};
	Tag tag(const char *name, bool selfClose=false) {
		raw("<", name);
		return Tag(*this, selfClose);
	}
	Tag line(double x1, double y1, double x2, double y2) {
		return tag("line", true).attr("x1", x1).attr("x2", x2).attr("y1", y1).attr("y2", y2);
	}
	Tag rect(double x, double y, double w, double h) {
		return tag("rect", true).attr("x", x).attr("y", y).attr("width", w).attr("height", h);
	}
};

/** Any drawable element.
	Not copyable/assignable because that's usually a mistake.
	
	There are two layers: data and labels.  The last-registered elements are drawn first.
*/
class SvgDrawable {
	std::vector<std::unique_ptr<SvgDrawable>> children;
	bool hasLayout = false;
protected:
	struct Bounds {
		double left = 0, right = 0, top = 0, bottom = 0;
		bool set = false;
		Bounds() {}
		Bounds(double left, double right, double top, double bottom) : left(left), right(right), top(top), bottom(bottom), set(true) {}
		double width() const {
			return right - left;
		}
		double height() const {
			return bottom - top;
		}
	};
	Bounds bounds;

	virtual void layout(const PlotStyle &style) {
		if (hasLayout) {
			assert(false); // shouldn't do this more than once
		};
		hasLayout = true;
		for (auto &c : children) {
			c->layout(style);
			if (bounds.set) {
				if (c->bounds.set) {
					bounds.left = std::min(bounds.left, c->bounds.left);
					bounds.top = std::min(bounds.top, c->bounds.top);
					bounds.right = std::max(bounds.right, c->bounds.right);
					bounds.bottom = std::max(bounds.bottom, c->bounds.bottom);
				}
			} else {
				bounds = c->bounds;
			}
		}
	};
	void layoutIfNeeded(const PlotStyle &style) {
		if (!hasLayout) this->layout(style);
	}
public:
	SvgDrawable() {}
	virtual ~SvgDrawable() {}

	void addChild(SvgDrawable *child) {
		children.emplace_back(child);
	}

	SvgDrawable(const SvgDrawable &other) = delete;
	SvgDrawable & operator =(const SvgDrawable &other) = delete;

	virtual void writeData(SvgWriter &svg, const PlotStyle &style) {
		// Write in reverse order
		for (int i = children.size() - 1; i >= 0; --i) {
			children[i]->writeData(svg, style);
		}
	}
	virtual void writeLabel(SvgWriter &svg, const PlotStyle &style) {
		// Write in reverse order
		for (int i = children.size() - 1; i >= 0; --i) {
			children[i]->writeLabel(svg, style);
		}
	}

};

/// Top-level objects which can generate SVG files
class SvgFileDrawable : public SvgDrawable {
public:
	virtual PlotStyle defaultStyle() {
		return {};
	}

	void write(std::ostream &o, const PlotStyle &style) {
		this->layoutIfNeeded(style);

		// Add padding
		auto bounds = this->bounds;
		bounds.left -= style.padding;
		bounds.right += style.padding;
		bounds.top -= style.padding;
		bounds.bottom += style.padding;
		
		SvgWriter svg(o);
		svg.raw("<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"no\"?>\n<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n");
		svg.tag("svg").attr("version", "1.1").attr("class", "svg-plot")
			.attr("xmlns", "http://www.w3.org/2000/svg")
			.attr("width", bounds.width(), "pt").attr("height", bounds.height(), "pt")
			.attr("viewBox", bounds.left, " ", bounds.top, " ", bounds.width(), " ", bounds.height())
			.attr("preserveAspectRatio", "xMidYMid");

		svg.rect(this->bounds.left, this->bounds.top, this->bounds.width(), this->bounds.height())
			.attr("class", "svg-plot-bg");
		this->writeData(svg, style);
		this->writeLabel(svg, style);

		int maxBounds = std::ceil(std::max(
			std::max(std::abs(this->bounds.left), std::abs(this->bounds.right)),
			std::max(std::abs(this->bounds.top), std::abs(this->bounds.bottom))
		)*std::sqrt(2));
		svg.raw("<defs>");
		for (size_t i = 0; i < style.hatches.size(); ++i) {
			auto &hatch = style.hatches[i];
			if (!hatch.angles.size()) continue;
			svg.tag("mask").attr("id", "svg-plot-hatch", i);
			for (double angle : hatch.angles) {
				svg.rect(-maxBounds, -maxBounds, 2*maxBounds, 2*maxBounds)
					.attr("fill", "url(#svg-plot-hatch", i, "-pattern)")
					.attr("style", "transform:rotate(", angle, "deg)");
			}
			svg.raw("</mask>");
			double spacing = style.hatchSpacing*hatch.spaceScale;
			svg.tag("pattern").attr("patternUnits", "userSpaceOnUse")
				.attr("id", "svg-plot-hatch", i, "-pattern").attr("class", "svg-plot-hatch")
				.attr("x", 0).attr("y", 0).attr("width", 10).attr("height", spacing)
				.attr("stroke", "#FFF").attr("fill", "none");
			svg.line(-1, spacing*0.5, 11, spacing*0.5);
			svg.raw("</pattern>");
		}
		svg.raw("</defs>");

		svg.raw("<style>");
		std::stringstream cssStream;
		style.css(cssStream);
		std::string css = cssStream.str();
		const char *c = css.c_str();
		// Strip tabs and newlines;
		while (*c) {
			if (*c != '\t' && *c != '\n') {
				// skip space before {
				if (*c != ' ' || *(c + 1) != '{') {
					o << (*c);
				}
			}
			// skip space after :
			if ((*c == ':' || *c == ',') && *(c + 1) == ' ') ++c;
			++c;
		}
		svg.raw("</style></svg>");
	}
	void write(std::string svgFile, const PlotStyle &style) {
		std::ofstream s(svgFile);
		write(s, style);
	}
	// If we aren't given a style, use the default one
	void write(std::ostream &o) {
		this->write(o, this->defaultStyle());
	}
	void write(std::string svgFile) {
		write(svgFile, this->defaultStyle());
	}
};

/// A labelled point on an axis.
struct Tick {
	double value;
	std::string name;

	enum class Strength {major, minor, tick};
	Strength strength = Strength::tick;
	
	Tick(double value, std::string name) : value(value), name(name) {}

	template<typename T>
	Tick(T value) : value(double(value)), name(std::to_string(value)) {}
};

/// A map from values to screen-space
class Axis {
	std::function<double(double)> unitMap;
	double autoMin, autoMax;
	bool hasAutoValue = false;
	bool autoScale, autoLabel;
	std::string _label = "";
public:
	double drawLow, drawHigh;
	double drawMin() const {
		return std::min(drawLow, drawHigh);
	}
	double drawMax() const {
		return std::max(drawLow, drawHigh);
	}
	double drawSize() const {
		return std::abs(drawHigh - drawLow);
	}

	void autoValue(double v) {
		if (!autoScale) return;
		if (!hasAutoValue) {
			autoMin = autoMax = v;
			hasAutoValue = true;
		} else {
			autoMin = std::min(autoMin, v);
			autoMax = std::max(autoMax, v);
		}
	}
	void autoSetup() {
		if (hasAutoValue) {
			if (autoScale) linear(autoMin, autoMax);
			if (autoLabel) minor(autoMin, autoMax);
		}
	}

	Axis(double drawLow, double drawHigh) : drawLow(drawLow), drawHigh(drawHigh) {
		linear(0, 1);
		autoScale = true;
		autoLabel = true;
	}
	
	Axis & label(std::string l) {
		_label = l;
		return *this;
	}
	const std::string & label() {
		return _label;
	}

	Axis & range(std::function<double(double)> valueToUnit) {
		autoScale = false;
		unitMap = valueToUnit;
		return *this;
	}

	Axis & range(std::function<double(double)> map, double lowValue, double highValue) {
		double lowMapped = map(lowValue), highMapped = map(highValue);
		return range([=](double v) {
			double mapped = map(v);
			return (mapped - lowMapped)/(highMapped - lowMapped);
		});
	}

	Axis & linear(double low, double high) {
		return range([=](double v) {
			return (v - low)/(high - low);
		});
	}
	
	double map(double v) {
		double unit = unitMap(v);
		return drawLow + unit*(drawHigh - drawLow);
	}

	std::vector<Tick> ticks;
	
	Axis &major() {
		autoLabel = false;
		return *this;
	}
	template<class ...Args>
	Axis &major(Tick tick, Args ...args) {
		autoValue(tick.value);
		tick.strength = Tick::Strength::major;
		ticks.push_back(tick);
		return major(args...);
	}
	template<typename T>
	Axis & majorRange(T start, T end, T step, bool useLabels=true) {
		step = std::abs(step);
		if (start <= end) {
			for (T t = start; t <= end; t += step) {
				useLabels ? major(t) : major({double(t), ""});
			}
		} else {
			for (T t = end; t >= start; t -= step) {
				useLabels ? major(t) : major({double(t), ""});
			}
		}
		return *this;
	}
	Axis &minor() {
		autoLabel = false;
		return *this;
	}
	template<class ...Args>
	Axis &minor(Tick tick, Args ...args) {
		autoValue(tick.value);
		tick.strength = Tick::Strength::minor;
		ticks.push_back(tick);
		return minor(args...);
	}
	template<typename T>
	Axis & minorRange(T start, T end, T step, bool useLabels=true) {
		step = std::abs(step);
		if (start <= end) {
			for (T t = start; t <= end; t += step) {
				useLabels ? minor(t) : minor({double(t), ""});
			}
		} else {
			for (T t = end; t >= start; t -= step) {
				useLabels ? minor(t) : minor({double(t), ""});
			}
		}
		return *this;
	}
	Axis &tick() {
		autoLabel = false;
		return *this;
	}
	template<class ...Args>
	Axis &tick(Tick t, Args ...args) {
		autoValue(t.value);
		t.strength = Tick::Strength::tick;
		ticks.push_back(t);
		return tick(args...);
	}
	template<typename T>
	Axis & tickRange(T start, T end, T step, bool useLabels=true) {
		step = std::abs(step);
		if (start <= end) {
			for (T t = start; t <= end; t += step) {
				useLabels ? tick(t) : tick({double(t), ""});
			}
		} else {
			for (T t = end; t >= start; t -= step) {
				useLabels ? tick(t) : tick({double(t), ""});
			}
		}
		return *this;
	}
};

struct Point2D {
	double x, y;
};

class TextLabel : public SvgDrawable {
	double textWidth = 0;
	void write(SvgWriter &svg) {
		{
			auto text = svg.tag("text").attr("class", cssClass);
			double tx = drawAt.x, ty = drawAt.y;
			if (alignment > 0.5) {
				text.attr("style", "text-anchor:start");
				tx += textWidth*(alignment - 1);
			} else if (alignment < -0.5) {
				text.attr("style", "text-anchor:end");
				tx += textWidth*(alignment + 1);
			} else {
				tx += textWidth*alignment;
			}
			if (vertical) {
				text.attr("x", 0).attr("y", 0)
					.attr("transform", "rotate(-90) translate(", -ty, " ", tx, ")");
			} else {
				text.attr("x", tx).attr("y", ty);
			}
		}
		svg.write(text);
		svg.raw("</text>");
	}
protected:
	Point2D drawAt;
	double alignment = 0; // 0=centre, 1=left, -1=right
	std::string text, cssClass;
	bool vertical, isValue;

	void layout(const PlotStyle &style) override {
		double x = drawAt.x, y = drawAt.y;
		double fontSize = isValue ? style.valueSize : style.labelSize;

		// Assume all text/labels are UTF-8
		textWidth = estimateUtf8Width(text.c_str())*fontSize*style.textAspect;

		if (vertical) {
			this->bounds = {x - fontSize*0.5, x + fontSize*0.5, y - textWidth*(alignment - 1)*0.5, y - textWidth*(alignment + 1)*0.5};
		} else {
			this->bounds = {x + textWidth*(alignment - 1)*0.5, x + textWidth*(alignment + 1)*0.5, y - fontSize*0.5, y + fontSize*0.5};
		}
	}

public:
	TextLabel(Point2D at, double alignment, std::string text, std::string cssClass="svg-plot-label", bool vertical=false, bool isValue=false) : drawAt(at), alignment(alignment), text(text), cssClass(cssClass), vertical(vertical), isValue(isValue) {}
	
	void writeLabel(SvgWriter &svg, const PlotStyle &) override {
		write(svg);
	}
};

/** A line on a 2D plot, with fill and/or stroke
	\image html filled-circles.svg
*/
class Line2D : public SvgDrawable {
	bool _drawLine = true;
	bool _drawFill = false;
	bool hasFillToX = false, hasFillToY = false;
	Point2D fillTo;
	
	Axis &axisX, &axisY;
	std::vector<Point2D> points;
	int styleIndex = 0;
public:
	Line2D(Axis &axisX, Axis &axisY, int styleIndex) : axisX(axisX), axisY(axisY), styleIndex(styleIndex) {}
	
	Line2D & add(double x, double y) {
		points.push_back({x, y});
		axisX.autoValue(x);
		axisY.autoValue(y);
		return *this;
	}
	
	/// @{
	///@name Draw config

	Line2D & drawLine(bool draw) {
		_drawLine = draw;
		return *this;
	}
	Line2D & drawFill(bool draw) {
		_drawFill = draw;
		return *this;
	}
	/// Start/end the fill at a given Y value
	Line2D & fillToY(double y) {
		_drawFill = true;
		hasFillToX = false;
		hasFillToY = true;
		fillTo = {0, y};
		return *this;
	}
	/// Start/end the fill at a given X value
	Line2D & fillToX(double x) {
		_drawFill = true;
		hasFillToX = true;
		hasFillToY = false;
		fillTo = {x, 0};
		return *this;
	}
	/// @}
	
	class LineLabel : public TextLabel {
		Axis &axisX, &axisY;
		Point2D at;
		std::string name;
		// direction: 0=right, 1=up, 2=left, 3=down
		double direction, distance;
		Point2D drawLineFrom{0, 0}, drawLineTo{0, 0};
		int styleIndex;
	protected:
		void layout(const PlotStyle &style) override {
			double sx = axisX.map(at.x), sy = axisY.map(at.y);
			if (distance < 0) {
				this->alignment = 0;
				this->drawAt = {sx, sy};
			} else {
				double angle = direction*3.14159265358979/180;
				double ax = std::cos(angle), ay = std::sin(angle);

				double px = sx + distance*ax, py = sy + distance*ay;
				double tx = px, ty = py;
				double fontSize = style.labelSize;
				double letterHeight = fontSize*0.8;
				ty -= fontSize*0.1; // Just a vertical alignment tweak

				double space = fontSize*0.25;
				double verticalWiggle = fontSize*0.3;
				if (ax < -0.7) {
					this->alignment = -1;
					tx -= space;
					ty += ay*verticalWiggle;
				} else if (ax > 0.7) {
					this->alignment = 1;
					tx += space;
					ty += ay*verticalWiggle;
				} else if (ay > 0) {
					ty += letterHeight;
					tx += ax*fontSize;
					this->alignment = ax;
				} else {
					ty -= letterHeight;
					tx += ax*fontSize;
					this->alignment = ax;
				}
				
				double lineDistance = distance - space;
				drawLineFrom = drawLineTo = {px, py};
				if (lineDistance > space) {
					drawLineTo = {sx + ax*space, sy + ay*space};
				}

				this->drawAt = {tx, ty};
			}
			this->cssClass = "svg-plot-label " + style.fillClass(styleIndex);

			TextLabel::layout(style);
		}
	public:
		LineLabel(Axis &axisX, Axis &axisY, Point2D at, std::string name, double direction, double distance, int styleIndex) : TextLabel({0, 0}, 0, name), axisX(axisX), axisY(axisY), at(at), name(name), direction(direction), distance(distance), styleIndex(styleIndex) {}
		
		void writeLabel(SvgWriter &svg, const PlotStyle &style) override {
			if (drawLineTo.x != drawLineFrom.x || drawLineTo.y != drawLineFrom.y) {
				svg.line(drawLineFrom.x, drawLineFrom.y, drawLineTo.x, drawLineTo.y)
					.attr("class", "svg-plot-tick ", style.strokeClass(styleIndex));
			}
			TextLabel::writeLabel(svg, style);
		}
	};

	Line2D & label(double valueX, double valueY, std::string name) {
		return label(valueX, valueY, name, 0, -1);
	}

	Line2D & label(double valueX, double valueY, std::string name, double degrees, double distance=0) {
		axisX.autoValue(valueX);
		axisY.autoValue(valueY);
		this->addChild(new LineLabel(axisX, axisY, {valueX, valueY}, name, degrees, distance, styleIndex));
		return *this;
	}

	Line2D & label(std::string name, double degrees=0, double distance=0) {
		Point2D latest = points.back();
		return label(latest.x, latest.y, name, degrees, distance);
	}

	Line2D & label(double xIsh, std::string name, double degrees=0, double distance=0) {
		size_t closest = 0;
		double closestError = -1;
		for (size_t i = 0; i < points.size(); ++i) {
			if (closestError < 0 || closestError > std::abs(points[i].x - xIsh)) {
				closest = i;
				closestError = std::abs(points[i].x - xIsh);
			}
		}
		Point2D latest = points[closest];
		return label(latest.x, latest.y, name, degrees, distance);
	}
	
	void writeData(SvgWriter &svg, const PlotStyle &style) override {
		if (_drawFill) {
			svg.raw("<path")
				.attr("class", "svg-plot-fill ", style.fillClass(styleIndex), " ", style.hatchClass(styleIndex));
			svg.raw(" d=\"M");
			for (auto &p : points) {
				svg.raw(" ", axisX.map(p.x), " ", axisY.map(p.y));
			}
			if (hasFillToX) {
				svg.raw(" ", axisX.map(fillTo.x), " ", axisY.map(points.back().y));
				svg.raw(" ", axisX.map(fillTo.x), " ", axisY.map(points[0].y));
			} else if (hasFillToY) {
				svg.raw(" ", axisX.map(points.back().x), " ", axisY.map(fillTo.y));
				svg.raw(" ", axisX.map(points[0].x), " ", axisY.map(fillTo.y));
			}
			svg.raw("\"/>");
		}

		if (_drawLine) {
			svg.raw("<path")
				.attr("class", "svg-plot-line ", style.strokeClass(styleIndex), " ", style.dashClass(styleIndex));
			svg.raw(" d=\"M");
			for (auto &p : points) {
				svg.raw(" ", axisX.map(p.x), " ", axisY.map(p.y));
			}
			svg.raw("\"/>");
		}

		SvgDrawable::writeData(svg, style);
	}
};

class Plot2D : public SvgFileDrawable {
	int styleIndex = 0;
public:
	Axis x, y;

	Plot2D() : Plot2D(240, 130) {}
	Plot2D(double width, double height) : Plot2D({0, width}, {height, 0}) {}
	Plot2D(Axis x, Axis y) : x(x), y(y) {}
	
	void writeData(SvgWriter &svg, const PlotStyle &style) override {
		double padding = style.lineWidth*0.5;
		long clipId = rand();

		svg.rect(x.drawMin(), y.drawMin(), x.drawSize(), y.drawSize())
			.attr("class", "svg-plot-axis");
		for (auto &t : x.ticks) {
			if (t.strength != Tick::Strength::tick) {
				double screenX = x.map(t.value);
				bool isMajor = (t.strength == Tick::Strength::major);
				bool isLeftBorder = std::abs(screenX - x.drawMin()) < 0.01; // 1% of a pixel is close enough
				double extraTop = (isMajor && isLeftBorder && t.name.size()) ? style.tickH : 0; // Extend using horizontal tick even though it's vertical
				
				svg.line(screenX, y.drawMin() - extraTop, screenX, y.drawMax())
					.attr("class", "svg-plot-", isMajor ? "major" : "minor");
			}
		}
		for (auto &t : y.ticks) {
			if (t.strength != Tick::Strength::tick) {
				double screenY = y.map(t.value);
				bool isMajor = (t.strength == Tick::Strength::major);
				svg.line(x.drawMin(), screenY, x.drawMax(), screenY)
					.attr("class", "svg-plot-", isMajor ? "major" : "minor");
			}
		}

		svg.tag("clipPath").attr("id", "clip", clipId);
		svg.rect(x.drawMin() - padding, y.drawMin() - padding, x.drawSize() + padding*2, y.drawSize() + padding*2);
		svg.raw("</clipPath>");
		svg.tag("g").attr("clip-path", "url(#clip", clipId, ")");
		SvgDrawable::writeData(svg, style);
		svg.raw("</g>");
	}

	void writeLabel(SvgWriter &svg, const PlotStyle &style) override {
		svg.raw("<g>");
		SvgDrawable::writeLabel(svg, style);

		for (auto &t : x.ticks) {
			double screenX = x.map(t.value);
			if (t.name.size()) {
				svg.line(screenX, y.drawMax(), screenX, y.drawMax() + style.tickV)
					.attr("class", "svg-plot-tick");
			}
		}
		for (auto &t : y.ticks) {
			if (t.name.size()) {
				double screenY = y.map(t.value);
				svg.line(x.drawMin() - style.tickH, screenY, x.drawMin(), screenY)
					.attr("class", "svg-plot-tick");
			}
		}
		svg.raw("</g>");
	}

	void layout(const PlotStyle &style) override {
		// Auto-scale axes if needed
		x.autoSetup();
		y.autoSetup();

		// Add labels for axes
		double screenY = y.drawMax() + style.tickV + style.valueSize*0.5 + style.textPadding;
		for (auto &t : x.ticks) {
			if (t.name.size()) {
				double screenX = x.map(t.value);
				auto *label = new TextLabel({screenX, screenY}, 0, t.name, "svg-plot-value", false, true);
				this->addChild(label);
			}
		}
		if (x.label().size()) {
			double midX = (x.drawMax() + x.drawMin())*0.5;
			auto *label = new TextLabel({midX, screenY + (style.labelSize + style.valueSize)*0.5}, 0, x.label(), "svg-plot-label", false, true);
			this->addChild(label);
		}
		double screenX = x.drawMin() - style.tickH - style.textPadding;
		double longestLabel = 0;
		for (auto &t : y.ticks) {
			if (t.name.size()) {
				double screenY = y.map(t.value);
				auto *label = new TextLabel({screenX, screenY}, -1, t.name, "svg-plot-value", false, true);
				longestLabel = std::max(longestLabel, estimateUtf8Width(t.name.c_str()));
				this->addChild(label);
			}
		}
		if (y.label().size()) {
			double midY = (y.drawMax() + y.drawMin())*0.5;
			auto *label = new TextLabel({screenX - style.textPadding*1.5 - longestLabel*style.valueSize, midY}, 0, y.label(), "svg-plot-label", true, true);
			this->addChild(label);
		}

		this->bounds = {
			x.drawMin() - style.tickH,
			x.drawMax() + style.tickH,
			y.drawMin() - style.tickV,
			y.drawMax() + style.tickV
		};

		SvgDrawable::layout(style);
	};
	
	Line2D & line(int styleIndex) {
		Line2D *line = new Line2D(this->x, this->y, styleIndex);
		this->addChild(line);
		return *line;
	}
	Line2D & line() {
		return line(styleIndex++);
	}

	Line2D & fill(int styleIndex) {
		return line(styleIndex).drawLine(false).drawFill(true);
	}
	Line2D & fill() {
		return fill(styleIndex++);
	}
};

class Figure : SvgFileDrawable {
public:
	PlotStyle style;

	Plot2D & plot(double widthPt, double heightPt) {
		Plot2D *axes = new Plot2D({0, widthPt}, {heightPt, 0});
		this->addChild(axes);
		return *axes;
	}
	Plot2D & plot() {
		Plot2D *axes = new Plot2D();
		this->addChild(axes);
		return *axes;
	}
	
	// Convenience for creating certain types
	template<class ...Args>
	Tick tick(Args ...args) {
		return Tick(args...);
	}
		
	void layout(const PlotStyle &style) override {
		this->bounds = {0, 0, 0, 0};
		SvgFileDrawable::layout(style);
	}
	
	void write(std::ostream &o) {
		SvgFileDrawable::write(o, style);
	}
	void write(std::string svgFile) {
		SvgFileDrawable::write(svgFile, style);
	}
};

static double estimateCharWidth(int c) {
	// measured experimentally, covering basic Latin (no accents) and Greek
	if (c >= 32 && c < 127) {
		static char w[95] = {31, 36, 45, 70, 61, 95, 77, 29, 39, 39, 40, 72, 31, 39, 31, 44, 61, 54, 58, 59, 59, 58, 59, 58, 59, 59, 38, 38, 74, 100, 74, 54, 97, 69, 66, 71, 76, 64, 62, 76, 77, 41, 53, 69, 57, 89, 76, 78, 63, 80, 68, 64, 62, 75, 67, 96, 69, 64, 64, 41, 46, 41, 68, 59, 54, 57, 59, 52, 59, 56, 38, 58, 58, 29, 33, 53, 30, 87, 58, 57, 59, 59, 43, 49, 38, 58, 53, 77, 54, 53, 50, 47, 46, 47, 69};
		return w[c - 32]*0.01;
	} else if (c == 168) {
		return 0.53;
	} else if (c == 183) {
		return 0.33;
	} else if (c == 697) {
		return 0.26;
	} else if (c >= 880 && c < 884) {
		static char w[4] = {42, 31, 64, 52};
		return w[c - 880]*0.01;
	} else if (c >= 885 && c < 888) {
		static char w[3] = {40, 66, 48};
		return w[c - 885]*0.01;
	} else if (c >= 890 && c < 894) {
		static char w[4] = {33, 52, 52, 52};
		return w[c - 890]*0.01;
	} else if (c == 895) {
		return 0.33;
	} else if (c == 900) {
		return 0.52;
	} else if (c >= 913 && c < 930) {
		static char w[17] = {75, 71, 63, 73, 71, 71, 82, 82, 45, 77, 75, 94, 81, 70, 83, 85, 67};
		return w[c - 913]*0.01;
	} else if (c >= 931 && c < 938) {
		static char w[7] = {69, 65, 70, 82, 80, 85, 84};
		return w[c - 931]*0.01;
	} else if (c >= 945 && c < 970) {
		static char w[25] = {61, 58, 57, 57, 49, 50, 58, 60, 29, 57, 55, 59, 53, 51, 57, 63, 59, 50, 59, 48, 58, 72, 56, 76, 76};
		return w[c - 945]*0.01;
	} else if (c >= 975 && c < 979) {
		static char w[4] = {47, 66, 74, 66};
		return w[c - 975]*0.01;
	} else if (c >= 981 && c < 1024) {
		static char w[43] = {80, 86, 56, 79, 63, 68, 67, 57, 53, 60, 53, 75, 85, 86, 85, 69, 56, 70, 53, 69, 69, 61, 61, 75, 56, 43, 37, 59, 63, 46, 29, 79, 55, 55, 62, 63, 71, 87, 75, 75, 75, 75, 75};
		return w[c - 981]*0.01;
	} else if (c == 65291) {
		return 1;
	}
	return 0.85;
}

/// Estimates string width encoded with UTF-8
static double estimateUtf8Width(const char *utf8Str) {
	/// TODO: UTF-8 decoding!
	double total = 0;
	while (*utf8Str) {
		total += estimateCharWidth(*utf8Str);
		++utf8Str;
	}
	return total;
}

/// @}
}}; // namespace

#endif // include guard
