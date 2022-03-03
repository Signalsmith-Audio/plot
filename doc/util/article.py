"""
Plotting helpers for Signalsmith Audio graphs
"""

import os
import sys

import matplotlib
from matplotlib import pyplot
from matplotlib.colors import LinearSegmentedColormap
import numpy

# from html import escape  <-- doesn't exist in Python 2
def escape(text):
	return str(text).replace('&', '&amp;').replace('<', '&lt;').replace('>', '&gt;')

font_prop = None
matplotlib.rcParams['font.family'] = ['Arial', 'DejaVu Sans', 'Bitstream Vera Sans'];
matplotlib.rcParams['mathtext.fontset'] = 'stixsans'
matplotlib.rcParams['font.size'] = 12

rgb = [
	[0, 0, 0.9],
	[0.8, 0, 0],
	[0, 0.8, 0],
	[0.5, 0.4, 0],
	[0.9, 0.6, 0],
	[0.8, 0, 0.8],
]
colors = ["#%02X%02X%02X"%tuple([round(v*255) for v in c]) for c in rgb]

def common_style(figure, axes):
	result = {'strong-grid': False}
	return result

def set_style(figure, axes, color=True, dashesInterleaved=True, legend_loc=None, styleOffset=0):
	dashes = [(None, None), (1.2, 1.2), (2.8, 1.6), (5, 4), (4, 1, 1, 1, 1, 1), (10, 3), (4, 2, 1, 2)]
	lineWidths = [1, 1.3, 1.3, 1.3, 1.3, 1.3, 1.5]
	lineWidthMultiplier = 1 if color else 1.0
	lineWidthPower = 0.97 if color else 0.93

	def makeList(x):
		if x == None:
			return []
		if isinstance(x, tuple):
			return list(x)
		if isinstance(x, list):
			return x
		return [x]
		
	lines = axes.get_lines()
	for index in range(len(lines)):
		styleIndex = index + styleOffset
		line = lines[index]
		line.set_linewidth(1)

		if color:
			if (len(str(line.get_color())) < 3):
				c = rgb[styleIndex%len(rgb)]
				line.set_color(c)
		else:
			line.set_color("black")

		dashIndex = styleIndex%len(dashes) if dashesInterleaved else (index//len(rgb))%len(dashes)
		lineWidth = lineWidthMultiplier*(lineWidthPower**index)*lineWidths[dashIndex];
		line.set_linewidth(lineWidth)

		dash = dashes[dashIndex]
		if (dash[0] != None):
			dash = [x*lineWidth for x in dash]
		line.set_dashes(dash)

		zorder = 2.5 - index/float(len(lines))
		line.set_zorder(zorder)

	# By default, use 'best' legend for multi-line graphs
	if legend_loc == 0:
		legend_loc = 'best' if len(lines) > 1 else None
	if legend_loc:
		framealpha = 0.9 if color else 1
		axes.legend(framealpha=framealpha, loc=legend_loc, prop=font_prop)
		legend = axes.get_legend()
		if legend:
			legend.get_frame().set_edgecolor('k')
			legend.get_frame().set_lw(0.3)
			# Hide again if it's empty
			if len(legend.texts) == 0:
				legend.set_visible(False)

	style = common_style(figure, axes)
	if color:
		# Transparent figure, but solid inside axes
		figure.patch.set_fill(False)
		axes.patch.set_alpha(0.7)
		axes.grid(linewidth=0.5) # Used to be "lineWidth"
		if style['strong-grid']:
			axes.grid(alpha=0.4)
		else:
			axes.grid(alpha=0.15)
	else :
		figure.patch.set_fill(True)
		axes.patch.set_alpha(1)
		if style['strong-grid']:
			axes.grid(linewidth=0.1) # Used to be "lineWidth"
			axes.grid(alpha=1)
		else:
			axes.grid(alpha=0)

def save(prefix, figure, legend_loc=0, dpi=0, styleOffset=0):
	origDpi = dpi;
	if dpi <= 0:
		dpi = 90
		if "@2x.png" in prefix:
			dpi = 180
	dirname = os.path.dirname(prefix)
	if len(dirname) and not os.path.exists(dirname):
		os.makedirs(dirname)

	if len(figure.get_axes()) > 1:
		figure.set_tight_layout(True)

	# Accessible screen style (hybrid colours and dashes)
	for axes in figure.get_axes():
		set_style(figure, axes, color=True, dashesInterleaved=True, legend_loc=legend_loc, styleOffset=styleOffset)
	print(prefix)
	
	extension = prefix.split(".").pop().lower()
	if extension == "png":
		figure.savefig(prefix, bbox_inches='tight', dpi=dpi)
	elif extension == "svg":
		figure.savefig(prefix, bbox_inches='tight')
	else:
		figure.savefig(prefix + '.svg', bbox_inches='tight')
		figure.patch.set_fill(True)
		figure.savefig(prefix + '.png', bbox_inches='tight', dpi=90)
		figure.savefig(prefix + '@2x.png', bbox_inches='tight', dpi=180)

		# B+W print style (dashes only)
		for axes in figure.get_axes():
			set_style(figure, axes, color=False, dashesInterleaved=True, legend_loc=legend_loc)
		print(prefix + '.mono')
		figure.savefig(prefix + '.mono.svg', bbox_inches='tight')

	if ".png" in prefix and "@" not in prefix and origDpi == 0:
		save(prefix.replace(".png", "@2x.png"), figure, legend_loc=legend_loc, dpi=dpi*2, styleOffset=styleOffset)
	else:
		pyplot.close(figure)

def enhanceFigure(figure, nrows=1, ncols=1):
	def boundSave(prefix, legend_loc=0, dpi=90, styleOffset=0):
		return save(prefix, figure, legend_loc=legend_loc, dpi=dpi, styleOffset=styleOffset)
	def boundSubGrid(yx, size=(1, 1), **kwargs):
		pyplot.figure(figure.number);
		return pyplot.subplot2grid((nrows, ncols), yx, size[0], size[1], **kwargs)
	figure.save = boundSave
	figure.gridPlot = boundSubGrid
	return figure
	
rowStretch = 0.25

def small(nrows=1, ncols=1, stretch=True, *args, **kwargs):
	figure, axes = pyplot.subplots(nrows=nrows, ncols=ncols, *args, **kwargs)
	figure.set_size_inches(4.5, 3*(1 + (nrows - 1)*rowStretch*stretch))
	return enhanceFigure(figure, nrows, ncols), axes

def smallFigure(nrows=1, ncols=1, stretch=True, *args, **kwargs):
	figure = pyplot.figure(*args, **kwargs);
	figure.set_size_inches(4.5, 3*(1 + (nrows - 1)*rowStretch*stretch))
	return enhanceFigure(figure, nrows, ncols)

def medium(nrows=1, ncols=1, stretch=True, *args, **kwargs):
	figure, axes = pyplot.subplots(nrows=nrows, ncols=ncols, *args, **kwargs)
	figure.set_size_inches(6.5, 4*(1 + (nrows - 1)*rowStretch*stretch))
	return enhanceFigure(figure), axes

def mediumFigure(nrows=1, ncols=1, stretch=True, *args, **kwargs):
	figure = pyplot.figure(*args, **kwargs);
	figure.set_size_inches(6.5, 4*(1 + (nrows - 1)*rowStretch*stretch))
	return enhanceFigure(figure, nrows, ncols)

def tall(nrows=1, ncols=1, stretch=True, *args, **kwargs):
	figure, axes = pyplot.subplots(nrows=nrows, ncols=ncols, *args, **kwargs)
	figure.set_size_inches(4.5, 5.5*(1 + (nrows - 1)*rowStretch*stretch))
	return enhanceFigure(figure), axes

def tallFigure(nrows=1, ncols=1, stretch=True, *args, **kwargs):
	figure = pyplot.figure(*args, **kwargs);
	figure.set_size_inches(4.5, 5.5*(1 + (nrows - 1)*rowStretch*stretch))
	return enhanceFigure(figure, nrows, ncols)

def short(nrows=1, ncols=1, stretch=True, *args, **kwargs):
	figure, axes = pyplot.subplots(nrows=nrows, ncols=ncols, *args, **kwargs)
	figure.set_size_inches(7, 3*(1 + (nrows - 1)*rowStretch*stretch))
	return enhanceFigure(figure), axes

def shortFigure(nrows=1, ncols=1, stretch=True, *args, **kwargs):
	figure = pyplot.figure(*args, **kwargs);
	figure.set_size_inches(7, 3*(1 + (nrows - 1)*rowStretch*stretch))
	return enhanceFigure(figure, nrows, ncols)

def wide(nrows=1, ncols=1, stretch=True, *args, **kwargs):
	figure, axes = pyplot.subplots(nrows=nrows, ncols=ncols, *args, **kwargs)
	figure.set_size_inches(11, min(15, 4*(1 + (nrows - 1)*rowStretch*stretch)))
	return enhanceFigure(figure), axes

def wideFigure(nrows=1, ncols=1, stretch=True, *args, **kwargs):
	figure = pyplot.figure(*args, **kwargs);
	figure.set_size_inches(11, min(15, 4*(1 + (nrows - 1)*rowStretch*stretch)))
	return enhanceFigure(figure, nrows, ncols)

def full(nrows=1, ncols=1, *args, **kwargs):
	figure, axes = pyplot.subplots(nrows=nrows, ncols=ncols, *args, **kwargs)
	figure.set_size_inches(16, 10) # Doesn't stretch, because it's page-sized
	return enhanceFigure(figure), axes

def fullFigure(nrows=1, ncols=1, *args, **kwargs):
	figure = pyplot.figure(*args, **kwargs);
	figure.set_size_inches(16, 10) # Doesn't stretch, because it's page-sized
	return enhanceFigure(figure, nrows, ncols)

import random
import shutil
import subprocess
from math import ceil
def animate(outFile, func, fps, duration=0, frameCount=0, previewRatio=0):
	"""Use like:
	
		# Define a frame function
		def frame(file, frameNumber, time):
			figure, axes = article.small()
			
			phase = time*pi
			x = linspace(0, 1, 1000)
			axes.plot(x, sin(2*pi*x + phase))
			figure.save(file, dpi=180)

		animate("out/animation.mp4", frame, 30, 6)
		
		animate("out/short.mp4", frame, 30, frameCount=30)
	"""
	outdir = "frames-%08d"%random.randint(0, 1e8)
	os.makedirs(outdir)
	
	try:
		if frameCount == 0:
			frameCount = duration*fps
		framePattern = outdir + "/frame-" + ("%04d"%frameCount) + "-%04d.png"
		for frameNumber in range(frameCount):
			func(framePattern%frameNumber, frameNumber, float(frameNumber)/fps)

		previewIndex = int(frameCount*previewRatio)
		previewFrame = framePattern%previewIndex
		previewImage = pyplot.imread(previewFrame)
		frameHeight = previewImage.shape[0]
		frameWidth = previewImage.shape[1]
		# Make sure they're even
		frameHeight = ceil(frameHeight*0.5)*2
		frameWidth = ceil(frameWidth*0.5)*2
		
		previewPng = outFile + ".png"
		if os.path.exists(previewPng):
			os.unlink(previewPng)
		shutil.copyfile(previewFrame, previewPng)
		
		dirname = os.path.dirname(outFile)
		if len(dirname) and not os.path.exists(dirname):
			os.makedirs(dirname)
		
		cmd = ["ffmpeg", "-f", "lavfi", "-i", "color=white:s=%ix%i"%(frameWidth, frameHeight), "-r", str(fps), "-i", framePattern, "-shortest", "-filter_complex", "overlay", "-pix_fmt", "yuv420p", "-frames:v", str(frameCount), "-r", str(fps), "-t", str(float(frameCount)/fps), "-crf", "20", "-y", outFile]
		print(" ".join(cmd))
		subprocess.call(cmd)
		print("output video: %ix%i (half: %ix%i)"%(frameWidth, frameHeight, frameWidth/2, frameHeight/2))
		print("""<video src="%s" poster="%s" class="click-to-play" loop width="%i" height="%i"></video>"""%(outFile, previewPng, frameWidth/2, frameHeight/2))
	finally:
		shutil.rmtree(outdir)

def unwrapPhase(freq, phase):
	newFreq = [freq[0]]
	newPhase = [phase[0]]
	for i in range(1, len(freq)):
		if phase[i] < phase[i - 1] - numpy.pi:
			newFreq.append(freq[i])
			newPhase.append(phase[i] + 2*numpy.pi)
			newFreq.append(None)
			newPhase.append(None)
			newFreq.append(freq[i - 1])
			newPhase.append(phase[i - 1] - 2*numpy.pi)
		if phase[i] >= phase[i - 1] + numpy.pi:
			newFreq.append(freq[i])
			newPhase.append(phase[i] - 2*numpy.pi)
			newFreq.append(None)
			newPhase.append(None)
			newFreq.append(freq[i - 1])
			newPhase.append(phase[i - 1] + 2*numpy.pi)
		newFreq.append(freq[i])
		newPhase.append(phase[i])
	return newFreq, newPhase
	
def progress(message):
	sys.stdout.write("\033[K") # Clear the line
	print(message)
	sys.stdout.write("\033[F") # Move cursor up
def progressDone(message=""):
	sys.stdout.write("\033[K") # Clear the line
	print(message)

def csvStr(v):
	v = str(v)
	if "," in v or "\"" in v:
		v = "\"" + v.replace("\"", "\"\"") + "\""
	return v
class Table:
	def __init__(self, prefix, columns, cssClass=""):
		self.prefix = prefix
		self.cssClass = cssClass

		columns = list([[col] if isinstance(col, str) else col for col in columns])
		self.columns = columns
		self.rows = []
	def add(self, *args):
		self.rows.append(args)
	def __enter__(self):
		return self
	def __exit__(self, *args):
		html = "<table class=\"" + escape(self.cssClass) + "\">\n\t<thead>\n"
		csv = ",".join([" ".join(col) for col in self.columns]) + "\n"

		headerRows = max([len(c) for c in self.columns])
		for rowIndex in range(headerRows)[::-1]:
			html += "\t\t<tr>\n"
			colIndex = 0
			values = [col[rowIndex] if len(col) > rowIndex else "" for col in self.columns]
			scopedValues = [col[rowIndex:] for col in self.columns]
			while colIndex < len(self.columns):
				comparison = scopedValues[colIndex]
				header = values[colIndex]
				colspan = 1
				colIndex += 1
				while colIndex < len(self.columns) and scopedValues[colIndex] == comparison:
					colIndex += 1
					colspan += 1
				tag = "th" if len(header) else "td"
				html += "\t\t\t<%s colspan=\"%i\">"%(tag, colspan)
				html += escape(header) + "</%s>\n"%tag
			html += "\t\t</tr>\n"
		html += "\t</thead>\n\t<tbody>\n"
		for row in self.rows:
			html += "\t\t<tr>\n"
			for value in row:
				html += "\t\t\t<td>" + escape(str(value)) + "</td>\n"
			html += "\t\t</tr>\n"
			csv += ",".join([csvStr(value) for value in row]) + "\n"
		html += "\t</tbody>\n</table>"
		
		base = self.prefix
		with open(base+ ".csv", "w") as file:
			file.write(csv)
		with open(base+ ".html", "w") as file:
			file.write(html + "\n<script src=\"/style/article/fragment.js\"></script>")
		with open(base+ ".js", "w") as file:
			file.write("""(html=>{let scripts=document.querySelectorAll('script'),currentScript=scripts[scripts.length - 1],div=document.createElement('div');div.innerHTML = html;currentScript.parentNode.insertBefore(div, currentScript);})(`""" + html.replace("`", "\\`").replace("$", "\\$") + "`);")
			
# Just a little wrapper which optionally pops off the head, and rotates the results for easy plotting
import csv
def readCsv(filename, hasHeader=True):
	with open(filename, 'r') as csvFile:
		csvIter = csv.reader(csvFile, delimiter=",")
		header = None
		if hasHeader:
			header = csvIter.next()
		data = numpy.asarray([[float(v) for v in row] for row in csvIter])
		if hasHeader:
			return header, numpy.swapaxes(data, 0, 1);
		else:
			return numpy.swapaxes(data, 0, 1);

if __name__ == "__main__":
	figure, axis = medium()
	x = numpy.linspace(0, 1, 100)
	for power in range(1, 9):
		axis.plot(x, x**power, label="$y = x^%i$"%power)
	axis.set(xlabel="arbitrary units")

	# Saves to SVG
	save("example-plots/example", figure)

	with Table("example-plots/table", ["column1", "column2"], cssClass="small") as table:
		table.add("foo", "bar");
		table.add("baz", "bing");
	
	with Table("example-plots/headers",	[
		"A",
		["B", "Group 1"],
		["C", "Group 1"],
		["D", "Group 2"],
		["E", "Group 2"],
	]) as table:
		table.add(1, 2, 3, 4, 5);
		table.add("a", "b", "c", "d", "e");
