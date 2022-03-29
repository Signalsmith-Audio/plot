#ifndef SIGNALSMITH_SIG_PLOT_H
#define SIGNALSMITH_SIG_PLOT_H

#include <fstream>
#include <memory>
#include <functional>
#include <vector>
#include <array>

namespace signalsmith { namespace plot {



struct PlotStyle {
	static constexpr int colourCount = 6;
	static constexpr int dashCount = 7;
	static constexpr double padding = 30;
	static constexpr double fontSize = 10;
	static constexpr double lineWidth = 1.5;
	static void css(std::ostream &o) {
		o << R"CSS(
			@import "/style/article/dist.css";
			.svg-plot-line {
				stroke: blue;
				fill: none;
				stroke-width: 1.25px;
				stroke-linejoin: round;
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
			.svg-plot text, .svg-plot-label {
				font-size: 10px;
				font-family: inherit;
				stroke: rgba(255,255,255,0.7);
				stroke-width: 2px;
				paint-order: stroke fill;

				text-anchor: middle;
				dominant-baseline: central;
				alignment-baseline: baseline;
			}
			.svg-plot-label {
				fill: #000;
			}
			.svg-plot-bg {
				stroke: none;
				fill: #FFF;
				opacity: 0.85;
			}
			
			
			.svg-plot-s0 {
				stroke: #007AB0;
			}
			.svg-plot-s1 {
				stroke: #BB102B;
			}
			.svg-plot-s2 {
				stroke: #44A730;
			}
			.svg-plot-s3 {
				stroke: #87694F;
			}
			.svg-plot-s4 {
				stroke: #EDA720;
			}
			.svg-plot-s5 {
				stroke: #A64D99;
			}
			.svg-plot-f0 {
				fill: #007AB0;
			}
			.svg-plot-f1 {
				fill: #BB102B;
			}
			.svg-plot-f2 {
				fill: #44A730;
			}
			.svg-plot-f3 {
				fill: #87694F;
			}
			.svg-plot-f4 {
				fill: #EDA720;
			}
			.svg-plot-f5 {
				fill: #A64D99;
			}
			.svg-plot-d0 {
				stroke-width: 1.2px;
			}
			.svg-plot-d1 {
				stroke-dasharray: 1.8 1.8;
			}
			.svg-plot-d2 {
				stroke-dasharray: 4.2 2.4;
			}
			.svg-plot-d3 {
				stroke-dasharray: 7.5 6;
			}
			.svg-plot-d4 {
				stroke-dasharray: 6 1.5 1.5 1.5 1.5 1.5;
			}
			.svg-plot-d5 {
				stroke-dasharray: 15 4.5;
			}
			.svg-plot-d6 {
				stroke-dasharray: 6 3 1.5 3;
			}
		)CSS";
	}
};

class SvgDrawable {
	std::vector<std::unique_ptr<SvgDrawable>> children;
protected:
	void addChild(SvgDrawable *child) {
		children.emplace_back(child);
	}
public:
	SvgDrawable() {}
	virtual ~SvgDrawable() {}

	// Not copyable/movable
	SvgDrawable(const SvgDrawable &other) = delete;
	SvgDrawable & operator =(const SvgDrawable &other) = delete;
	
	virtual void write(std::ostream &o) {
		// Write in reverse order
		for (int i = children.size() - 1; i >= 0; --i) {
			children[i]->write(o);
		}
	}
	virtual void writeLabel(std::ostream &o) {
		// Write in reverse order
		for (int i = children.size() - 1; i >= 0; --i) {
			children[i]->writeLabel(o);
		}
	}
	virtual void bounds(double &left, double &right, double &top, double &bottom) {
		for (auto &c : children) {
			double l = left, r = right, t = top, b = bottom;
			c->bounds(l, r, t, b);
			left = std::min(left, l);
			top = std::min(top, t);
			right = std::max(right, r);
			bottom = std::max(bottom, b);
		}
	};

	static void escape(std::ostream &o, const char *str) {
		while (*str) {
			if (*str == '<') {
				o << "&lt;";
			} else if (*str == '&') {
				o << "&amp;";
			} else if (*str == '"') {
				o << "&quot;";
			} else {
				o << (*str);
			}
			++str;
		}
	}
	static void escape(std::ostream &o, const std::string &str) {
		escape(o, str.c_str());
	}
};

class Axis1D {
	double screenLow, screenHigh;
	std::function<double(double)> unitMap;
	bool hasAutoValue = false;
	double autoMin, autoMax;
public:
	bool autoScale, autoLabel;
	void autoValue(double v) {
		if (!hasAutoValue) {
			autoMin = autoMax = v;
			hasAutoValue = true;
		} else {
			autoMin = std::min(autoMin, v);
			autoMax = std::max(autoMax, v);
		}
	}

	Axis1D(double screenLow, double screenHigh) : screenLow(screenLow), screenHigh(screenHigh) {
		linear(0, 1);
		autoScale = true;
		autoLabel = true;
	}
	
	Axis1D & linear(double low, double high) {
		autoScale = false;
		unitMap = [=](double v) {
			return (v - low)/(high - low);
		};
		return *this;
	}
	
	double map(double v) {
		double unit = unitMap(v);
		return screenLow + unit*(screenHigh - screenLow);
	}

	struct Tick {
		double value;
		std::string name;
		int strength = 0;
		
		Tick(double value, std::string name) : value(value), name(name) {}

		template<typename T>
		Tick(T value) : value(double(value)), name(std::to_string(value)) {}
	};
	std::vector<Tick> ticks;
	
	Axis1D &major() {
		autoLabel = false;
		return *this;
	}
	template<class ...Args>
	Axis1D &major(Tick tick, Args ...args) {
		autoValue(tick.value);
		tick.strength = 2;
		ticks.push_back(tick);
		return major(args...);
	}
	template<typename T>
	Axis1D & majorRange(T start, T end, T step, bool useLabels=true) {
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
	Axis1D &minor() {
		autoLabel = false;
		return *this;
	}
	template<class ...Args>
	Axis1D &minor(Tick tick, Args ...args) {
		autoValue(tick.value);
		tick.strength = 1;
		ticks.push_back(tick);
		return minor(args...);
	}
	template<typename T>
	Axis1D & minorRange(T start, T end, T step, bool useLabels=true) {
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
	Axis1D &tick() {
		autoLabel = false;
		return *this;
	}
	template<class ...Args>
	Axis1D &tick(Tick t, Args ...args) {
		autoValue(t.value);
		t.strength = 0;
		ticks.push_back(t);
		return tick(args...);
	}
	template<typename T>
	Axis1D & tickRange(T start, T end, T step, bool useLabels=true) {
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

	void autoSetup() {
		if (hasAutoValue) {
			if (autoScale) linear(autoMin, autoMax);
			if (autoLabel) minor(autoMin, autoMax);
		}
	}
};

class Line2D;
class Axes2D : public SvgDrawable {
	double left, right, top, bottom;
	int styleIndex = 0;
public:
	Axis1D x, y;

	Axes2D(double left, double right, double top, double bottom) : left(left), right(right), top(top), bottom(bottom), x(left, right), y(bottom, top) {
	}
	
	void write(std::ostream &o) override {
		x.autoSetup();
		y.autoSetup();
		double padding = PlotStyle::lineWidth*0.5;
		long clipId = rand();
		o << "<clipPath id=\"clip" << clipId << "\"><rect x=\"" << (left - padding) << "\" y=\"" << (top - padding) << "\" width=\"" << (right - left + padding*2) << "\" height=\"" << (bottom  - top + padding*2) << "\" /></clipPath>";
		o << "<g clip-path=\"url(#clip" << clipId << "\">";
		SvgDrawable::write(o);
		o << "</g>";
	}

	void writeLabel(std::ostream &o) override {
		x.autoSetup();
		y.autoSetup();

		SvgDrawable::writeLabel(o);

		o << "<g>";
		for (auto &t : x.ticks) {
			double screenX = x.map(t.value);
			if (t.strength) {
				double extraTop = (t.strength == 2 && screenX == left && t.name.size()) ? PlotStyle::fontSize*0.35 : 0;
				o << "<line class=\"svg-plot-" << (t.strength == 2 ? "major" : "minor") << "\" y1=\"" << (top - extraTop) << "\" y2=\"" << bottom << "\" x1=\"" << screenX << "\" x2=\"" << screenX << "\"/>";
			}
			if (t.name.size()) {
				o << "<line class=\"svg-plot-tick\" y1=\"" << bottom << "\" y2=\"" << (bottom + PlotStyle::fontSize*0.5) << "\" x1=\"" << screenX << "\" x2=\"" << screenX << "\"/>";
				o << "<text class=\"svg-plot-label\" x=\"" << screenX << "\" y=\"" << (bottom + PlotStyle::fontSize*1.5) << "\">";
				escape(o, t.name);
				o << "</text>";
			}
		}
		for (auto &t : y.ticks) {
			double screenY = y.map(t.value);
			if (t.strength) {
				o << "<line class=\"svg-plot-" << (t.strength == 2 ? "major" : "minor") << "\" x1=\"" << left << "\" x2=\"" << right << "\" y1=\"" << screenY << "\" y2=\"" << screenY << "\"/>";
			}
			if (t.name.size()) {
				o << "<line class=\"svg-plot-tick\" x1=\"" << (left - PlotStyle::fontSize*0.3) << "\" x2=\"" << left << "\" y1=\"" << screenY << "\" y2=\"" << screenY << "\"/>";
				o << "<text class=\"svg-plot-label\" text-anchor=\"end\" y=\"" << screenY << "\" x=\"" << (left - PlotStyle::fontSize*0.8) << "\">";
				escape(o, t.name);
				o << "</text>";
			}
		}
		o << "</g>";
	}

	void bounds(double &l, double &r, double &t, double &b) override {
		l = left - 30;
		r = right + 30;
		t = top - PlotStyle::fontSize*0.75;
		b = bottom + PlotStyle::fontSize*2;
	};
	
	Line2D & line(int style);
	Line2D & line() {
		return line(styleIndex++);
	}
};

class Line2D : public SvgDrawable {
	Axes2D &axes;
	struct Point {
		double x, y;
	};
	std::vector<Point> points;
	struct Label {
		Point at;
		std::string name;
		// direction: 0=right, 1=up, 2=left, 3=down
		double direction, distance;
	};
	std::vector<Label> labels;
	int styleIndex = 0;
public:
	Line2D(Axes2D &axes, int styleIndex) : axes(axes), styleIndex(styleIndex) {}
	
	Line2D & add(double x, double y) {
		points.push_back({x, y});
		axes.x.autoValue(x);
		axes.y.autoValue(y);
		return *this;
	}

	Line2D & label(double x, double y, std::string name, double direction=0, double distance=0) {
		axes.x.autoValue(x);
		axes.y.autoValue(y);
		labels.push_back({Point{x, y}, name, direction, distance});
		return *this;
	}

	Line2D & label(std::string name, double direction=0, double distance=0) {
		Point latest = points.back();
		return label(latest.x, latest.y, name, direction, distance);
	}

	Line2D & label(double xIsh, std::string name, double direction=0, double distance=0) {
		size_t closest = 0;
		double closestError = -1;
		for (size_t i = 0; i < points.size(); ++i) {
			if (closestError < 0 || closestError > std::abs(points[i].x - xIsh)) {
				closest = i;
				closestError = std::abs(points[i].x - xIsh);
			}
		}
		Point latest = points[closest];
		return label(latest.x, latest.y, name, direction, distance);
	}
	
	void write(std::ostream &o) override {
		int strokeIndex = styleIndex%PlotStyle::colourCount;
		int dashIndex = styleIndex%PlotStyle::dashCount;
		o << "<path class=\"svg-plot-line svg-plot-s" << strokeIndex << " svg-plot-d" << dashIndex << "\" d=\"M";
		for (auto &p : points) {
			o << " " << axes.x.map(p.x) << " " << axes.y.map(p.y);
		}
		o << "\" />";

		SvgDrawable::write(o);
	}
	
	void writeLabel(std::ostream &o) override {
		int colourIndex = styleIndex%PlotStyle::colourCount;

		for (auto &label : labels) {
			double angle = label.direction*-0.5*3.14159265358979;
			double ax = std::cos(angle), ay = std::sin(angle);

			double sx = axes.x.map(label.at.x), sy = axes.y.map(label.at.y);
			double px = sx + label.distance*ax, py = sy + label.distance*ay;
			double tx = px, ty = py;
			ty -= PlotStyle::fontSize*0.1; // Just a vertical alignment tweak

			double space = PlotStyle::fontSize*0.25;
			double verticalWiggle = PlotStyle::fontSize*0.3;
			const char *anchor = "middle";
			if (ax < -0.7) {
				anchor = "end";
				tx -= space;
				ty += ay*verticalWiggle;
			} else if (ax > 0.7) {
				anchor = "start";
				tx += space;
				ty += ay*verticalWiggle;
			} else if (ay > 0) {
				ty += PlotStyle::fontSize*0.8;
				tx += ax*PlotStyle::fontSize;
			} else {
				ty -= PlotStyle::fontSize*0.8;
				tx += ax*PlotStyle::fontSize;
			}

			double distance = label.distance - space;
			if (distance > space) {
				o << "<line class=\"svg-plot-tick svg-plot-s" << colourIndex << "\" x1=\"" << (sx + ax*space) << "\" x2=\"" << px << "\" y1=\"" << (sy + ay*space) << "\" y2=\"" << py << "\"/>";
			}

			o << "<text dominant-baseline=\"baseline\" alignment-baseline=\"baseline\" style=\"text-anchor:" << anchor << "\" x=\"" << tx << "\" y=\"" << ty << "\" class=\"svg-plot-f" << colourIndex << "\">";
			escape(o, label.name);
			o << "</text>";
		}

		SvgDrawable::write(o);
	}
};
Line2D & Axes2D::line(int style) {
	Line2D *line = new Line2D(*this, style);
	this->addChild(line);
	return *line;
}

class Plot : public SvgDrawable {
	double width = 4.8*72*0.75;
	double height = 2.5*72*0.75;
public:

	Axes2D & axes() {
		Axes2D *axes = new Axes2D(0, width, 0, height);
		this->addChild(axes);
		return *axes;
	}

	void write(std::ostream &o) {
		double left = 0, right = width;
		double top = 0, bottom = height;
		this->bounds(left, right, top, bottom);
		o << "<svg class=\"svg-plot\" width=\"" << (right - left) << "pt\" height=\"" << (bottom - top) << "pt\" version=\"1.1\" viewBox=\"" << left << " " << top << " " << (right - left) << " " << (bottom - top) << "\" preserveAspectRatio=\"xMidYMid\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\">";

		SvgDrawable::write(o);
		SvgDrawable::writeLabel(o);

		o << "<style>";
		std::stringstream cssStream;
		PlotStyle::css(cssStream);
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
			if (*c == ':' && *(c + 1) == ' ') ++c;
			++c;
		}
		o << "</style></svg>";
	}

	void write(std::string svgFile) {
		std::ofstream s(svgFile);
		write(s);
	}

};

double estimateCharWidth(int c) {
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

}}; // namespace

#endif // include guard
