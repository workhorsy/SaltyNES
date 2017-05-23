/*
Copyright (c) 2012-2017 Matthew Brennan Jones <matthew.brennan.jones@gmail.com>
A NES emulator in WebAssembly. Based on vNES.
Licensed under GPLV3 or later
Hosted at: https://github.com/workhorsy/SaltyNES
*/

let g_zoom = 1;
let g_mouse_move_timeout = null;

let statusElement = $('#status');
let progressElement = $('#progress');

if (! ('WebAssembly' in window)) {
	alert('This browser does not support WebAssembly.');
}

var Module = {
	preRun: [],
	postRun: [],
	print: (function() {
		let element = $('#output');
		if (element) element.value = ''; // clear browser cache
		return function(text) {
			if (arguments.length > 1) text = Array.prototype.slice.call(arguments).join(' ');
			// These replacements are necessary if you render to raw HTML
			//text = text.replace(/&/g, "&amp;");
			//text = text.replace(/</g, "&lt;");
			//text = text.replace(/>/g, "&gt;");
			//text = text.replace('\n', '<br>', 'g');
			console.log(text);
			if (element) {
				element.value += text + "\n";
				element.scrollTop = element.scrollHeight; // focus on bottom
			}
		};
	})(),
	printErr: function(text) {
		if (arguments.length > 1) text = Array.prototype.slice.call(arguments).join(' ');
		if (0) { // XXX disabled for safety typeof dump == 'function') {
			dump(text + '\n'); // fast, straight to the real console
		} else {
			console.error(text);
		}
	},
	canvas: (function() {
		let screen = $('#screen');

		// As a default initial behavior, pop up an alert when webgl context is lost. To make your
		// application robust, you may want to override this behavior before shipping!
		// See http://www.khronos.org/registry/webgl/specs/latest/1.0/#5.15.2
		screen.addEventListener("webglcontextlost", function(e) { alert('WebGL context lost. You will need to reload the page.'); e.preventDefault(); }, false);

		return screen;
	})(),
	setStatus: function(text) {
		if (!Module.setStatus.last) Module.setStatus.last = { time: Date.now(), text: '' };
		if (text === Module.setStatus.text) return;
		let m = text.match(/([^(]+)\((\d+(\.\d+)?)\/(\d+)\)/);
		let now = Date.now();
		if (m && now - Date.now() < 30) return; // if this is a progress update, skip it if too soon
		if (m) {
			text = m[1];
			progressElement.value = parseInt(m[2])*100;
			progressElement.max = parseInt(m[4])*100;
			progressElement.hidden = false;
		} else {
			progressElement.value = null;
			progressElement.max = null;
			progressElement.hidden = true;
		}
		statusElement.innerHTML = text;
	},
	totalDependencies: 0,
	monitorRunDependencies: function(left) {
		this.totalDependencies = Math.max(this.totalDependencies, left);
		Module.setStatus(left ? 'Preparing... (' + (this.totalDependencies-left) + '/' + this.totalDependencies + ')' : 'All downloads complete.');
	}
};

window.onerror = function(event) {
	// TODO: do not warn on ok events like simulating an infinite loop or exitStatus
	Module.setStatus('Exception thrown, see JavaScript console');
	Module.setStatus = function(text) {
		if (text) Module.printErr('[post-exception status] ' + text);
	};
};

document.addEventListener('mousemove', function() {
	// Show the cursor when the mouse moves
	document.body.style.cursor = '';
	$('#screen').style.cursor = '';

	// Just return if not fullscreen
	const is_full_screen = (
		document.fullscreen ||
		document.webkitIsFullScreen ||
		document.mozFullScreen
	);
	if (! is_full_screen) {
		return;
	}

	// Clear the previous mouse timeout
	if (g_mouse_move_timeout) {
		clearTimeout(g_mouse_move_timeout);
		g_mouse_move_timeout = null;
	}

	// Hide the cursor after 3 seconds
	g_mouse_move_timeout = setTimeout(function() {
		document.body.style.cursor = 'none';
		$('#screen').style.cursor = 'none';
		g_mouse_move_timeout = null;
	}, 3000);
}, false);

$('#button_toggle_console').addEventListener('click', function() {
	if (is_hidden('#output')) {
		show('#output');
		this.value = "Console On";
	} else {
		hide('#output');
		this.value = "Console Off";
	}
}, false);

$('#button_toggle_sound').addEventListener('click', function() {
	let toggle_sound = Module.cwrap('toggle_sound', null, []);
	let is_on = (toggle_sound() == 1);
	if (is_on) {
		this.value = "Sound On";
	} else {
		this.value = "Sound Off";
	}
}, false);

$('#button_full_screen').addEventListener('click', function() {
	Module.requestFullscreen(
		false,
		false
	);
	g_zoom = 1;
}, false);

$('#button_zoom_in').addEventListener('click', function() {
	g_zoom++;
	let style = $('#screen').style;
	style.width = (g_zoom * 256) + 'px';
	style.height = (g_zoom * 240) + 'px';
}, false);

$('#button_zoom_out').addEventListener('click', function() {
	if (g_zoom > 1) g_zoom--;
	let style = $('#screen').style;
	style.width = (g_zoom * 256) + 'px';
	style.height = (g_zoom * 240) + 'px';
}, false);

$('#select_game').addEventListener('change', function(event) {
	// Save the file name in the module args
	Module.setStatus('Downloading ...');
	Module.arguments = [ $('#select_game').value ];

	// Load the wasm boot strapper
	let script = document.createElement('script');
	script.setAttribute('src', 'static/index.js');
	document.head.appendChild(script);

	// Hide the game selector, and enabled the full screen button
	$('#button_full_screen').disabled = false;
	$('#button_zoom_out').disabled = false;
	$('#button_zoom_in').disabled = false;
	$('#button_toggle_sound').disabled = false;
	hide('#select_game');
}, false);

$('#screen').addEventListener('contextmenu', function(event) {
	event.preventDefault();
}, false);

documentOnReady(() => {
	Module.setStatus('Ready ...');
});
