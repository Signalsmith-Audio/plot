(function(document, setTimeout, random) {
	var wobble = 2, wobbleInterval = 40, wobbleMs = 240, wobbleSync = true;
	var style = document.querySelector('style');
	style.textContent = "@import \"/style/article/dist.css\";" + style.textContent + ".svg-plot-value,.svg-plot-label{font-family:\"Geraint Dense\",Arial,sans-serif}";
	var updateFns = [];
	document.querySelectorAll("path").forEach(function (path) {
		function rand() {
			return (random() - 0.5)*wobble;
		}
		var d = path.getAttribute('d');

		function replacePath() {
			var counter = random()*wobbleInterval;
			var first = true;
			var fromX = rand(), toX = rand(), fromY = rand(), toY = rand();
			var prevX, prevY;
			var newD = d.replace(/([0-9\.]+) ([0-9\.]+)/g, function(p, x, y) {
				result = "";
				function addPoint(x, y) {
					x = parseFloat(x);
					y = parseFloat(y);
					if (!first) {
						var dx = x - prevX, dy = y - prevY;
						var d = Math.sqrt(dx*dx + dy*dy);
						if (d > wobbleInterval*0.5) { // Too long, split in half
							addPoint((prevX + x)*0.5, (prevY + y)*0.5);
							return addPoint(x, y);
						}
						counter += d;
						if (counter > wobbleInterval) {
							counter = 0;
							fromX = toX;
							fromY = toY;
							toX = rand();
							toY = rand();
						}
					}
					first = false;

					prevX = x;
					prevY = y;
					var r = counter/wobbleInterval;
					x += fromX + (toX - fromX)*r;
					y += fromY + (toY - fromY)*r;
					result += ' ' + x + ' ' + y;
				}
				addPoint(x, y);
				return result;
			});
			path.setAttribute('d', newD);
			
			if (!wobbleSync) setTimeout(replacePath, wobbleMs*(0.9 + 0*random()));
		}
		if (wobbleSync) {
			updateFns.push(replacePath);
		} else {
			setTimeout(replacePath, wobbleMs*random());
		}
	});
	if (wobbleSync) {
		function updates() {
			updateFns.forEach(function (f) {f()});
			setTimeout(updates, wobbleMs*(0.9 + 0.2*random()));
		}
		updates();
	}
})(document, setTimeout, Math.random);
