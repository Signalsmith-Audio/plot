# Basic C++ plots

This is minimal C++ plotting library.  It is header-only and dependency-free, but only outputs SVGs.

It is created and maintained mostly for internal Signalsmith use, so I'm adding features as I need them.

![example plot](doc/images/example.svg)

## Features / design choices

* SVG output only
* auto-styled lines and fills
	* simultaneous dash/colour sequences for accessibility
	* styling mostly done via CSS
	* customisable CSS, colour-/dash-sequences and various sizes 
* explicit labelling (no legends)
* values labelled with three levels: major/minor/tick
* no explicit axis-lines, use major gridlines instead

### Current limitations

* only 2D axes supported
* one axis per plot
* no automatic label placement or de-collision
* text lengths are heuristically estimated

## How to use

To set up a plot (after including `sigplot.h`):

```cpp
signalsmith::plot::Plot plot;
auto &axes = plot.axes();
```

To configure X/Y axes with major/minor grid lines and ticks:
```cpp
// Add major/minor lines, and a tick (with no grid-line)
axes.x.major(0, 10).tick(5).minor(3.5);
```

To set the range:
```cpp
axes.x.linear(0, 15);
// or a custom scale
axes.y.range(std::log, 1, 1000);
```

## Tests

There aren't any unit tests, just some smoke-test example plots.

Tests are `.cpp` files in `doc/tests/`, and are run (from `doc/`) with `make test`.  Individual sub-directories (`tests/foo`) can be run with `make test-foo`.

After compiling and running the tests, any Python scripts in the test directory are run as well.
