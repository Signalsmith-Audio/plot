import article
import numpy;

figure, axes = article.medium()

x = numpy.linspace(0, 10, 1000);
axes.plot(x, numpy.sin(x), label="sin(x)");
axes.plot(x, numpy.cos(x), label="cos(x)");
axes.spines['bottom'].set(position=('data', 0))

figure.save("python.svg");
