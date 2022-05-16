/** Signalsmith's Basic C++ Plots - https://signalsmith-audio.co.uk/code/plot/
@copyright Licensed as 0BSD.  If you need anything else, get in touch. */

#ifndef SIGNALSMITH_PLOT_H
#define SIGNALSMITH_PLOT_H

#include <fstream>
#include <memory>
#include <functional>
#include <vector>
#include <cmath>
#include <sstream>

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
	Colour/dash/hatch styles are defined as CSS classes, assigned to elements based on their integer style index.  CSS is written inline in the SVG, and can be extended/overridden with `.cssPrefix`/`.cssSuffix`.
 		\image html custom-2d.svg
	It generates CSS classes from `.colours` (`svg-plot-sN`/`svg-plot-fN`/`svg-plot-tN` for stroke/fill/text), `.dashes` (`svg-plot-dN`) and `.hatches` (`svg-plot-hN`), where `N` is the index - e.g. there are six colours by default, generating `svg-plot-s0` to `svg-plot-s5`.
*/
class PlotStyle {
public:
	double padding = 10;
	double lineWidth = 1.5, precision = 100;
	double tickH = 4, tickV = 5;
	// Text
	double labelSize = 12, valueSize = 10;
	double fontAspectRatio = 1; /// scales size estimates, if using a particularly wide font
	double textPadding = 5, lineHeight = 1.2;
	// Fills
	double fillOpacity = 0.3;
	double hatchWidth = 1, hatchSpacing = 3;

	std::string scriptHref = "", scriptSrc = "";
	std::string cssPrefix = "", cssSuffix = "";
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

	struct Counter {
		int colour, dash, hatch;
		Counter(int index=0) : colour(index), dash(index), hatch(index) {}

		/// Increment the counter, and return the previous value
		Counter bump() {
			Counter result = *this;
			++colour;
			++dash;
			++hatch;
			return result;
		}
	};
	std::string strokeClass(const Counter &counter) const {
		if (counter.colour < 0 || colours.size() == 0) return "";
		return "svg-plot-s" + std::to_string(counter.colour%(int)colours.size());
	}
	std::string fillClass(const Counter &counter) const {
		if (counter.colour < 0 || colours.size() == 0) return "";
		return "svg-plot-f" + std::to_string(counter.colour%(int)colours.size());
	}
	std::string textClass(const Counter &counter) const {
		if (counter.colour < 0 || colours.size() == 0) return "";
		return "svg-plot-t" + std::to_string(counter.colour%(int)colours.size());
	}
	std::string dashClass(const Counter &counter) const {
		if (counter.dash < 0 || dashes.size() == 0) return "";
		return "svg-plot-d" + std::to_string(counter.dash%(int)dashes.size());
	}
	std::string hatchClass(const Counter &counter) const {
		if (counter.hatch < 0 || hatches.size() == 0) return "";
		return "svg-plot-h" + std::to_string(counter.hatch%(int)hatches.size());
	}
	
	void css(std::ostream &o) const {
		o << cssPrefix;
		o << R"CSS(
			.svg-plot {
				stroke-linecap: butt;
				stroke-linejoin: round;
			}
			.svg-plot-bg {
				fill: none;
				stroke: none;
			}
			.svg-plot-axis {
				stroke: none;
				fill: #FFFFFFD9;
			}
			.svg-plot-legend {
				stroke: none;
				fill: #FFFFFFE4;
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
				stroke: #0000004D;
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
				stroke: #FFFFFF80;
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
				o << ".svg-plot-h" << i << "{opacity:" << (fillOpacity*(hatchWidth/hatchSpacing)) << "}\n";
			}
		}
		for (size_t i = 0; i < hatches.size(); ++i) {
			auto &h = hatches[i];
			if (h.lineScale != 1) {
				o << "#svg-plot-hatch" << i << "-pattern{stroke-width:" << hatchWidth*h.lineScale << "px}\n";
			}
		}
		o << cssSuffix;
	}
};

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
	Bounds & expandTo(const Bounds &other) {
		left = std::min(left, other.left);
		top = std::min(top, other.top);
		right = std::max(right, other.right);
		bottom = std::max(bottom, other.bottom);
		return *this;
	}
	Bounds pad(double hPad, double vPad) {
		return {left - hPad, right + hPad, top - vPad, bottom + vPad};
	}
	Bounds pad(double padding) {
		return pad(padding, padding);
	}
};

struct Point2D {
	double x, y;
};

/// Wrapper for slightly more semantic code when writing SVGs
class SvgWriter {
	std::ostream &output;
	std::vector<Bounds> clipStack;
	long idCounter = 0;
	double precision, invPrecision;
public:
	SvgWriter(std::ostream &output, Bounds bounds, double precision) : output(output), clipStack({bounds}), precision(precision), invPrecision(1.0/precision) {}

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
	SvgWriter & write(std::string str, Args &&...args) {
		return write(str.c_str(), args...);
	}
	
	template<class ...Args>
	SvgWriter & attr(const char *name, Args &&...args) {
		return raw(" ", name, "=\"").write(args...).raw("\"");
	}
	
	SvgWriter & pushClip(Bounds b, double dataCheckPadding) {
		clipStack.push_back(b.pad(dataCheckPadding));

		long clipId = idCounter++;
		tag("clipPath").attr("id", "clip", clipId);
		rect(b.left, b.top, b.width(), b.height());
		raw("</clipPath>");
		tag("g").attr("clip-path", "url(#clip", clipId, ")");
		return *this;
	}
	SvgWriter & popClip() {
		clipStack.resize(clipStack.size() - 1);
		return raw("</g>");
	}
	
	/// XML tag helper, closing the tag when it's destroyed
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
	
	char streak = 0; // Tracks streaks of points which are outside the clip
	Point2D prevPoint;
	void startPath() {
		streak = 0;
		prevPoint.x = prevPoint.y = -1e300;
	}
	void addPoint(double x, double y) {
		x = std::round(x*precision)*invPrecision;
		y = std::round(y*precision)*invPrecision;
		if (x == prevPoint.x && y == prevPoint.y) return;

		auto clip = clipStack.back();
		/// Bitmask indicating which direction(s) the point is outside the bounds
		char mask = (clip.left > x)
			| (2*(clip.right < x))
			| (4*(clip.top > y))
			| (8*(clip.bottom < y));
		char prevStreak = streak;
		streak &= mask;
		if (!streak) {
			if (prevStreak) { // we broke the streak - draw the last in-streak point
				if (!std::isnan(prevPoint.x) && !std::isnan(prevPoint.y)) {
					raw(" ", prevPoint.x, " ", prevPoint.y);
				}
			}
			if (!std::isnan(x) && !std::isnan(y)) {
				raw(" ", x, " ", y);
			}
			streak = mask;
		}
		prevPoint = {x, y};
	}
};

/** Any drawable element.
 	
	Each element can draw to three layers: fill, stroke and label.  Child elements are drawn in reverse order, so the earliest ones are drawn on top of each layer.
	
	Copy/assign is disabled, to prevent accidental copying when you should be holding a reference.
*/
class SvgDrawable {
	std::vector<std::unique_ptr<SvgDrawable>> children, layoutChildren;
	bool hasLayout = false;
protected:
	Bounds bounds;
	
	void invalidateLayout() {
		hasLayout = bounds.set = false;
		for (auto &c : children) c->invalidateLayout();
		layoutChildren.resize(0);
	}
	virtual void layout(const PlotStyle &style) {
		hasLayout = true;
		auto processChild = [&](std::unique_ptr<SvgDrawable> &child) {
			child->layoutIfNeeded(style);
			if (bounds.set) {
				if (child->bounds.set) bounds.expandTo(child->bounds);
			} else {
				bounds = child->bounds;
			}
		};
		for (auto &c : layoutChildren) processChild(c);
		for (auto &c : children) processChild(c);
	};
	/// These children are removed when the layout is invalidated
	void addLayoutChild(SvgDrawable *child) {
		layoutChildren.emplace_back(child);
	}
public:
	SvgDrawable() {}
	virtual ~SvgDrawable() {}
	SvgDrawable(const SvgDrawable &other) = delete;
	SvgDrawable & operator =(const SvgDrawable &other) = delete;

	Bounds layoutIfNeeded(const PlotStyle &style) {
		if (!hasLayout) this->layout(style);
		return bounds;
	}

	/// Takes ownership of the child
	void addChild(SvgDrawable *child) {
		children.emplace_back(child);
	}

	virtual void writeData(SvgWriter &svg, const PlotStyle &style) {
		for (int i = layoutChildren.size() - 1; i >= 0; --i) {
			layoutChildren[i]->writeData(svg, style);
		}
		for (int i = children.size() - 1; i >= 0; --i) {
			children[i]->writeData(svg, style);
		}
	}
	virtual void writeLabel(SvgWriter &svg, const PlotStyle &style) {
		for (int i = layoutChildren.size() - 1; i >= 0; --i) {
			layoutChildren[i]->writeLabel(svg, style);
		}
		for (int i = children.size() - 1; i >= 0; --i) {
			children[i]->writeLabel(svg, style);
		}
	}

};

/// Top-level objects which can generate SVG files
class SvgFileDrawable : public SvgDrawable {
public:
	virtual PlotStyle defaultStyle() const {
		return {};
	}

	void write(std::ostream &o, const PlotStyle &style) {
		this->invalidateLayout();
		this->layout(style);

		// Add padding
		auto bounds = this->bounds.pad(style.padding);
		
		SvgWriter svg(o, bounds, style.precision);
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
		const char *cPtr = css.c_str();
		// Strip whitespace that doesn't appear between letters/numbers
		bool letter = false, letterThenWhitespace = false;
		while (*cPtr) {
			char c = *(cPtr++);
			if (c == '\t' || c == '\n' || c == ' ') {
				letterThenWhitespace = letter;
			} else {
				letter = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '.';
				if (letterThenWhitespace && letter) o << ' ';
				letterThenWhitespace = false;
				o << c;
			}
		}
		svg.raw("</style>");
		if (style.scriptHref.size()) svg.tag("script", true).attr("href", style.scriptHref);
		if (style.scriptSrc.size()) svg.raw("<script>").write(style.scriptSrc).raw("</script>");
		svg.raw("</svg>");
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
	Tick(T v) : value(double(v)) {
		name = (std::stringstream() << value).str();
	}
};

/** A map from values to screen-space.

	Individual grid/ticks can be added with `.major()`/`.minor()`/`.tick()`.
	\code
		axis.major(4); // default label
		axis.major(5, "five"); // explicit label
	\endcode
	
	Multiple grids/ticks can be added using `.majors()`/`.minors()`/`.ticks()`, which accept a variable number of values:
	\code
		axis.majors(0, 10).minors(2, 4, 6, 8);
	\endcode
*/
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
	/// Not associated with a particular line by default, but can be
	PlotStyle::Counter styleIndex = -1;

	Axis(double drawLow, double drawHigh) : drawLow(drawLow), drawHigh(drawHigh) {
		linear(0, 1);
		autoScale = true;
		autoLabel = true;
	}
	explicit Axis(const Axis &other) = default;

	/// Register a value for the auto-scale
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
			if (autoLabel) minors(autoMin, autoMax);
		}
	}
	/// Prevent auto-labelling
	Axis & blank() {
		tickList.clear();
		autoLabel = false;
		return *this;
	}
	/// Clear the names from any existing labels
	Axis & blankLabels() {
		for (auto &t : tickList) t.name = "";
		return *this;
	}
	/// Whether the axis should draw on the non-default side (e.g. right/top)
	bool flipped = false;
	Axis & flip(bool flip=true) {
		flipped = flip;
		return *this;
	}
	
	/// Sets the label, and optionally style to match a particular line.
	Axis & label(std::string l, PlotStyle::Counter index=-1) {
		_label = l;
		styleIndex = index;
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
	Axis & range(double map(double)) {
		return range(std::function<double(double)>(map));
	}
	Axis & range(std::function<double(double)> map, double lowValue, double highValue) {
		double lowMapped = map(lowValue), highMapped = map(highValue);
		return range([=](double v) {
			double mapped = map(v);
			return (mapped - lowMapped)/(highMapped - lowMapped);
		});
	}
	Axis & range(double map(double), double lowValue, double highValue) {
		return range(std::function<double(double)>(map), lowValue, highValue);
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

	std::vector<Tick> tickList;

	template<class ...Args>
	Axis & major(Args &&...args) {
		Tick t(args...);
		autoValue(t.value);
		t.strength = Tick::Strength::major;
		tickList.push_back(t);
		autoLabel = false;
		return *this;
	}
	template<class ...Args>
	Axis & minor(Args &&...args) {
		Tick t(args...);
		autoValue(t.value);
		t.strength = Tick::Strength::minor;
		tickList.push_back(t);
		autoLabel = false;
		return *this;
	}
	template<class ...Args>
	Axis & tick(Args &&...args) {
		Tick t(args...);
		autoValue(t.value);
		t.strength = Tick::Strength::tick;
		tickList.push_back(t);
		autoLabel = false;
		return *this;
	}
	
	Axis &majors() {
		return *this;
	}
	template<class ...Args>
	Axis &majors(Tick tick, Args ...args) {
		return major(tick).majors(args...);
	}
	Axis &minors() {
		autoLabel = false;
		return *this;
	}
	template<class ...Args>
	Axis &minors(Tick tick, Args ...args) {
		return minor(tick).minors(args...);
	}
	Axis & ticks() {
		autoLabel = false;
		return *this;
	}
	template<class ...Args>
	Axis & ticks(Tick t, Args ...args) {
		return tick(t).ticks(args...);
	}
};

class TextLabel : public SvgDrawable {
	double textWidth = 0;
	void write(SvgWriter &svg, double fontSize) {
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
			ty -= fontSize*0.1; // Just a vertical alignment tweak
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
		textWidth = estimateUtf8Width(text.c_str())*fontSize*style.fontAspectRatio;

		if (vertical) {
			this->bounds = {x - fontSize*0.5, x + fontSize*0.5, y - textWidth*(alignment - 1)*0.5, y - textWidth*(alignment + 1)*0.5};
		} else {
			this->bounds = {x + textWidth*(alignment - 1)*0.5, x + textWidth*(alignment + 1)*0.5, y - fontSize*0.5, y + fontSize*0.5};
		}
		SvgDrawable::layout(style);
	}

public:
	TextLabel(Point2D at, double alignment, std::string text, std::string cssClass="svg-plot-label", bool vertical=false, bool isValue=false) : drawAt(at), alignment(alignment), text(text), cssClass(cssClass), vertical(vertical), isValue(isValue) {}
	
	void writeLabel(SvgWriter &svg, const PlotStyle &style) override {
		write(svg, isValue ? style.valueSize : style.labelSize);
	}
};

/** A line on a 2D plot, with fill and/or stroke
	\image html filled-circles.svg
*/
class Line2D : public SvgDrawable {
	bool _drawLine = true;
	bool _drawFill = false;
	bool hasFillToX = false, hasFillToY = false;
	Point2D fillToPoint;
	Line2D *fillToLine = nullptr;
	
	Axis &axisX, &axisY;
	std::vector<Point2D> points;
public:
	PlotStyle::Counter styleIndex;

	Line2D(Axis &axisX, Axis &axisY, PlotStyle::Counter styleIndex) : axisX(axisX), axisY(axisY), styleIndex(styleIndex) {}
	
	Line2D & add(double x, double y) {
		points.push_back({x, y});
		axisX.autoValue(x);
		axisY.autoValue(y);
		return *this;
	}

	template<class X, class Y>
	Line2D & addArray(X &&x, Y &&y, size_t size) {
		for (size_t i = 0; i < size; ++i) add(x[i], y[i]);
		return *this;
	}
	template<class X, class Y>
	Line2D & addArray(X &&x, Y &&y) {
		return addArray(std::forward<X>(x), std::forward<Y>(y), std::min<size_t>(x.size(), y.size()));
	}

	/// @{
	///@name Draw config

	Line2D & drawLine(bool draw=true) {
		_drawLine = draw;
		return *this;
	}
	Line2D & drawFill(bool draw=true) {
		_drawFill = draw;
		return *this;
	}
	/// Start/end the fill at a given Y value
	Line2D & fillToY(double y) {
		_drawFill = true;
		hasFillToX = false;
		hasFillToY = true;
		fillToPoint = {0, y};
		return *this;
	}
	/// Start/end the fill at a given X value
	Line2D & fillToX(double x) {
		_drawFill = true;
		hasFillToX = true;
		hasFillToY = false;
		fillToPoint = {x, 0};
		return *this;
	}
	Line2D & fillTo(Line2D &other) {
		_drawFill = true;
		hasFillToX = hasFillToY = false;
		fillToLine = &other;
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
		PlotStyle::Counter &styleIndex;
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
			this->cssClass = "svg-plot-label " + style.textClass(styleIndex);

			TextLabel::layout(style);
		}
	public:
		LineLabel(Axis &axisX, Axis &axisY, Point2D at, std::string name, double direction, double distance, PlotStyle::Counter &styleIndex) : TextLabel({0, 0}, 0, name), axisX(axisX), axisY(axisY), at(at), name(name), direction(direction), distance(distance), styleIndex(styleIndex) {}
		
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
			svg.startPath();
			for (auto &p : points) {
				svg.addPoint(axisX.map(p.x), axisY.map(p.y));
			}
			if (fillToLine) {
				auto &otherPoints = fillToLine->points;
				for (auto it = otherPoints.rbegin(); it != otherPoints.rend(); ++it) {
					svg.addPoint(fillToLine->axisX.map(it->x), fillToLine->axisY.map(it->y));
				}
			} else if (hasFillToX) {
				svg.addPoint(axisX.map(fillToPoint.x), axisY.map(points.back().y));
				svg.addPoint(axisX.map(fillToPoint.x), axisY.map(points[0].y));
			} else if (hasFillToY) {
				svg.addPoint(axisX.map(points.back().x), axisY.map(fillToPoint.y));
				svg.addPoint(axisX.map(points[0].x), axisY.map(fillToPoint.y));
			}
			svg.raw("\"/>");
		}
		if (_drawLine) {
			svg.raw("<path")
				.attr("class", "svg-plot-line ", style.strokeClass(styleIndex), " ", style.dashClass(styleIndex));
			svg.raw(" d=\"M");
			svg.startPath();
			for (auto &p : points) {
				svg.addPoint(axisX.map(p.x), axisY.map(p.y));
			}
			svg.raw("\"/>");
		}
		SvgDrawable::writeData(svg, style);
	}
};

class Legend : public SvgFileDrawable {
	SvgFileDrawable &ref;
	Bounds dataBounds;
	double rx, ry;
	Bounds location;
	struct Entry {
		PlotStyle::Counter style;
		std::string name;
		bool stroke, fill;
	};
	std::vector<Entry> entries;
public:
	Legend(SvgFileDrawable &ref, Bounds dataBounds, double rx, double ry) : ref(ref), dataBounds(dataBounds), rx(rx), ry(ry) {}
	
	void layout(const PlotStyle &style) override {
		Bounds refBounds = ref.layoutIfNeeded(style).pad(style.textPadding);
		double exampleLineWidth = style.labelSize*1.5; // 1.5em
		double longestLabel = 0;
		for (auto &e : entries) {
			longestLabel = std::max(longestLabel, estimateUtf8Width(e.name.c_str()));
		}
		double width = exampleLineWidth + style.textPadding*3 + longestLabel*style.labelSize;
		double height = style.textPadding*2 + entries.size()*style.labelSize*style.lineHeight;
		
		double extraW = dataBounds.width() - width;
		double extraH = dataBounds.height() - height;
		Point2D topLeft = {
			dataBounds.left + extraW*std::max(0.0, std::min(1.0, rx)),
			dataBounds.bottom - height - extraH*std::max(0.0, std::min(1.0, ry))
		};
		if (rx < 0) topLeft.x += (refBounds.left - width - topLeft.x)*-rx;
		if (rx > 1) topLeft.x += (refBounds.right - topLeft.x)*(rx - 1);
		if (ry < 0) topLeft.y += (refBounds.bottom - topLeft.y)*-ry;
		if (ry > 1) topLeft.y += (refBounds.top - height - topLeft.y)*(ry - 1);
		this->bounds = location = {topLeft.x, topLeft.x + width, topLeft.y, topLeft.y + height};
		
		for (size_t i = 0; i < entries.size(); ++i) {
			auto &entry = entries[i];
			double labelX = topLeft.x + style.textPadding*2 + exampleLineWidth;
			double labelY = location.top + style.textPadding + (i + 0.5)*style.labelSize*style.lineHeight;
			auto *label = new TextLabel({labelX, labelY}, 1, entry.name, "svg-plot-label", false, false);
			this->addLayoutChild(label);
		}
		SvgFileDrawable::layout(style);
	}
	Legend & line(PlotStyle::Counter style, std::string name, bool stroke=true, bool fill=false) {
		entries.push_back(Entry{style, name, stroke, fill});
		return *this;
	}
	Legend & line(const Line2D &line2D, std::string name) {
		return line(line2D.styleIndex, name, true, false);
	}
	Legend & fill(const Line2D &line2D, std::string name) {
		return line(line2D.styleIndex, name, false, true);
	}
	void writeLabel(SvgWriter &svg, const PlotStyle &style) override {
		svg.raw("<g>");
		svg.rect(location.left, location.top, location.width(), location.height())
			.attr("class", "svg-plot-legend");
		double lineX1 = location.left + style.textPadding;
		double lineX2 = lineX1 + style.labelSize*1.5; // 1.5em
		for (size_t i = 0; i < entries.size(); ++i) {
			auto &entry = entries[i];
			double lineY = location.top + style.textPadding + (i + 0.5)*style.labelSize*style.lineHeight;
			if (entry.fill) {
				double height = style.labelSize;
				svg.rect(lineX1, lineY - height*0.5, lineX2 - lineX1, height)
					.attr("class", "svg-plot-fill ", style.fillClass(entry.style), " ", style.hatchClass(entry.style));
			}
			if (entry.stroke) {
				svg.line(lineX1, lineY, lineX2, lineY)
					.attr("class", "svg-plot-line ", style.strokeClass(entry.style), " ", style.dashClass(entry.style));
			}
		}
		svg.raw("</g>");
		SvgFileDrawable::writeLabel(svg, style);
	}
};

class Plot2D : public SvgFileDrawable {
	std::string plotTitle;
	std::vector<std::unique_ptr<Axis>> xAxes, yAxes;
	Bounds size;
public:
	Axis &x, &y;
	/// Creates an X axis, covering some portion of the left/right side
	Axis & newX(double lowRatio=0, double highRatio=1) {
		Axis *x = new Axis(size.left + lowRatio*size.width(), size.left + highRatio*size.width());
		xAxes.emplace_back(x);
		return *x;
	}
	/// Creates a Y axis, covering some portion of the bottom/top side
	Axis & newY(double lowRatio=0, double highRatio=1) {
		Axis *y = new Axis(size.bottom - lowRatio*size.height(), size.bottom - highRatio*size.height());
		yAxes.emplace_back(y);
		return *y;
	}
	/// Style for the next auto-styled element
	PlotStyle::Counter styleCounter;

	Plot2D() : Plot2D(240, 130) {}
	Plot2D(double width, double height) : Plot2D({0, width}, {height, 0}) {}
	Plot2D(Axis ax, Axis ay) : size(ax.drawMin(), ax.drawMax(), ay.drawMin(), ay.drawMax()), x(*(new Axis(ax))), y(*(new Axis(ay))) {
		xAxes.emplace_back(&x); // created above, but we take ownership here
		yAxes.emplace_back(&y);
	}
	
	void writeData(SvgWriter &svg, const PlotStyle &style) override {
		svg.rect(size.left, size.top, size.width(), size.height())
			.attr("class", "svg-plot-axis");
		for (auto &x : xAxes) {
			for (auto &t : x->tickList) {
				if (t.strength != Tick::Strength::tick) {
					double screenX = x->map(t.value);
					bool isMajor = (t.strength == Tick::Strength::major);
					svg.line(screenX, size.top, screenX, size.bottom)
						.attr("class", "svg-plot-", isMajor ? "major" : "minor");
				}
			}
		}
		for (auto &y : yAxes) {
			for (auto &t : y->tickList) {
				if (t.strength != Tick::Strength::tick) {
					double screenY = y->map(t.value);
					bool isMajor = (t.strength == Tick::Strength::major);
					svg.line(size.left, screenY, size.right, screenY)
						.attr("class", "svg-plot-", isMajor ? "major" : "minor");
				}
			}
		}
		svg.pushClip(size.pad(style.lineWidth*0.5), style.lineWidth);
		SvgDrawable::writeData(svg, style);
		svg.popClip();
	}

	void writeLabel(SvgWriter &svg, const PlotStyle &style) override {
		svg.raw("<g>");
		SvgDrawable::writeLabel(svg, style);

		for (auto &x : xAxes) {
			double fromY = x->flipped ? size.top : size.bottom;
			double toY = fromY + (x->flipped ? -style.tickV : style.tickV);
			for (auto &t : x->tickList) {
				double screenX = x->map(t.value);
				if (t.name.size() && style.tickV != 0) {
					svg.line(screenX, fromY, screenX, toY)
						.attr("class", "svg-plot-tick");
				}
			}
		}
		for (auto &y : yAxes) {
			double fromX = y->flipped ? size.right : size.left;
			double toX = fromX + (y->flipped ? style.tickH : -style.tickV);
			for (auto &t : y->tickList) {
				if (t.name.size() && style.tickH != 0) {
					double screenY = y->map(t.value);
					svg.line(fromX, screenY, toX, screenY)
						.attr("class", "svg-plot-tick");
				}
			}
		}
		svg.raw("</g>");
	}

	void layout(const PlotStyle &style) override {
		// Auto-scale axes if needed
		for (auto &x : xAxes) x->autoSetup();
		for (auto &y : yAxes) y->autoSetup();

		double tv = std::max(style.tickV, 0.0), th = std::max(style.tickH, 0.0);

		// Add labels for axes
		for (auto &x : xAxes) {
			double alignment = (x->flipped ? -1 : 1), hasValues = x->tickList.size() ? 1 : 0;
			double screenY = (x->flipped ? size.top : size.bottom) + alignment*(tv + hasValues*(style.valueSize*0.5 + style.textPadding));
			for (auto &t : x->tickList) {
				if (t.name.size()) {
					double screenX = x->map(t.value);
					auto *label = new TextLabel({screenX, screenY}, 0, t.name, "svg-plot-value", false, true);
					this->addLayoutChild(label);
				}
			}
			if (x->label().size()) {
				double labelY = screenY + alignment*((style.labelSize + hasValues*style.valueSize)*0.5 + style.textPadding);
				double midX = (x->drawMax() + x->drawMin())*0.5;
				auto *label = new TextLabel({midX, labelY}, 0, x->label(), "svg-plot-label " + style.textClass(x->styleIndex), false, true);
				this->addLayoutChild(label);
			}
		}
		double longestLabelLeft = 0, longestLabelRight = 0;
		for (auto &y : yAxes) {
			double alignment = (y->flipped ? 1 : -1);
			double screenX = (y->flipped ? size.right : size.left) + alignment*(th + style.textPadding);
			for (auto &t : y->tickList) {
				if (t.name.size()) {
					double screenY = y->map(t.value);
					auto *label = new TextLabel({screenX, screenY}, alignment, t.name, "svg-plot-value", false, true);
					this->addLayoutChild(label);

					double &longestLabel = y->flipped ? longestLabelRight : longestLabelLeft;
					longestLabel = std::max(longestLabel, estimateUtf8Width(t.name.c_str()));
				}
			}
		}
		for (auto &y : yAxes) {
			double alignment = (y->flipped ? 1 : -1);
			double screenX = (y->flipped ? size.right : size.left) + alignment*(th + style.textPadding);
			if (y->label().size()) {
				double longestLabel = y->flipped ? longestLabelRight : longestLabelLeft;
				double labelX = screenX + alignment*(style.textPadding*1.5 + longestLabel*style.valueSize);
				double midY = (y->drawMax() + y->drawMin())*0.5;
				auto *label = new TextLabel({labelX, midY}, 0, y->label(), "svg-plot-label " + style.textClass(y->styleIndex), true, true);
				this->addLayoutChild(label);
			}
		}

		this->bounds = size.pad(th, tv);
		SvgDrawable::layout(style);
	};
	
	Line2D & line(Axis &x, Axis &y, PlotStyle::Counter styleIndex) {
		Line2D *line = new Line2D(x, y, styleIndex);
		this->addChild(line);
		return *line;
	}
	Line2D & line(Axis &x, Axis &y) {
		return line(x, y, styleCounter.bump());
	}
	Line2D & line(PlotStyle::Counter styleIndex) {
		return line(this->x, this->y, styleIndex);
	}
	Line2D & line() {
		return line(styleCounter.bump());
	}

	/// Convenience method, returns a line set to only fill.
	template<class ...Args>
	Line2D & fill(Args &&...args) {
		return line(args...).drawLine(false).drawFill(true);
	}
	
	/** Creates a legend at a given position.
	If `xRatio` and `yRatio` are in the range 0-1, the legend will be inside the plot.  Otherwise, it will move outside the plot (e.g. -1 will be left/below the axes, including any labels).
	*/
	Legend & legend(double xRatio, double yRatio) {
		Legend *legend = new Legend(*this, size, xRatio, yRatio);
		this->addChild(legend);
		return *legend;
	}
};

class Cell : public SvgFileDrawable {
protected:
	void layout(const PlotStyle &style) override {
		this->bounds = {0, 0, 0, 0};
		SvgFileDrawable::layout(style);
	}
public:
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
};

class Grid : public Cell {
	int _cols = 0, _rows = 0;
	struct Item {
		int column, row;
		int width, height;
		std::unique_ptr<Cell> cell;
		Point2D transpose = {0, 0};
		Item(int column, int row, int width, int height) : column(column), row(row), width(width), height(height), cell(new Cell()) {}
	};
	std::vector<Item> items;

	void writeItems(bool label, SvgWriter &svg, const PlotStyle &style) {
		for (auto &it : items) {
			svg.tag("g").attr("transform", "translate(", it.transpose.x, " ", it.transpose.y, ")");
			if (label) {
				it.cell->writeLabel(svg, style);
			} else {
				it.cell->writeData(svg, style);
			}
			svg.raw("</g>");
		}
	}
protected:
	void layout(const PlotStyle &style) override {
		struct Range {
			double min = 0, max = 0;
			double offset = 0;
			void include(double v) {
				min = std::min(v, min);
				max = std::max(v, max);
			}
		};
		std::vector<Range> colRange(_cols);
		std::vector<Range> rowRange(_rows);
		for (auto &it : items) {
			Bounds bounds = it.cell->layoutIfNeeded(style);
			colRange[it.column].include(bounds.left);
			colRange[it.column + it.width - 1].include(bounds.right);
			rowRange[it.row].include(bounds.top);
			rowRange[it.row + it.height - 1].include(bounds.bottom);
		}
		this->bounds = {0, 0, 0, 0};
		double offset = 0;
		for (auto &r : colRange) {
			r.offset = offset - r.min;
			offset += r.max - r.min + style.padding;
		}
		this->bounds.right = offset - style.padding;
		offset = 0;
		for (auto &r : rowRange) {
			r.offset = offset - r.min;
			offset += r.max - r.min + style.padding;
		}
		this->bounds.bottom = offset - style.padding;
		for (auto &it : items) {
			it.transpose = {colRange[it.column].offset, rowRange[it.row].offset};
		}
		SvgDrawable::layout(style);
	}

	void writeData(SvgWriter &svg, const PlotStyle &style) override {
		SvgDrawable::writeData(svg, style);
		writeItems(false, svg, style);
	}
	void writeLabel(SvgWriter &svg, const PlotStyle &style) override {
		SvgDrawable::writeLabel(svg, style);
		writeItems(true, svg, style);
	}
public:
	int rows() const {
		return _rows;
	}
	int columns() const {
		return _cols;
	}
	Cell & cell(int column, int row, int width=1, int height=1) {
		column = std::max(0, column);
		row = std::max(0, row);
		width = std::max(1, width);
		height = std::max(1, height);
		_cols = std::max(_cols, column + width);
		_rows = std::max(_rows, row + height);
		for (auto &it : items) {
			if (it.row == row && it.column == column && it.width == width && it.height == height) {
				return *it.cell;
			}
		}
		items.emplace_back(column, row, width, height);
		return *(items.back().cell);
	}
};

class Figure : public Grid {
public:
	PlotStyle style;
	PlotStyle defaultStyle() const override {
		return style;
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
