import article

figure, axes = article.small()

columns, data = article.readCsv("example.csv")
axes.plot(data[0], data[1]);

figure.save("example.svg")
