
let g_get_key_up = 0;
let g_get_key_down = 0;
let g_get_key_left = 0;
let g_get_key_right = 0;
let g_get_key_b = 0;
let g_get_key_a = 0;
let g_get_key_start = 0;
let g_get_key_select = 0;

function onKeyDown(event) {
	let code = event.keyCode || event.which;
	//console.log(code);

	switch (code) {
		case 65: g_get_key_left = 1; break;
		case 68: g_get_key_right = 1; break;
		case 87: g_get_key_up = 1; break;
		case 83: g_get_key_down = 1; break;
		case 74: g_get_key_b = 1; break;
		case 75: g_get_key_a = 1; break;
		case 13: g_get_key_start = 1; break;
		case 16: g_get_key_select = 1; break;
	}
}

function onKeyUp(event) {
	let code = event.keyCode || event.which;
	//console.log(code);

	switch (code) {
		case 65: g_get_key_left = 0; break;
		case 68: g_get_key_right = 0; break;
		case 87: g_get_key_up = 0; break;
		case 83: g_get_key_down = 0; break;
		case 74: g_get_key_b = 0; break;
		case 75: g_get_key_a = 0; break;
		case 13: g_get_key_start = 0; break;
		case 16: g_get_key_select = 0; break;
	}
}

let statusElement = document.getElementById('status');
let progressElement = document.getElementById('progress');
let spinnerElement = document.getElementById('spinner');

var Module = {
	preRun: [],
	postRun: [],
	print: (function() {
		let element = document.getElementById('output');
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
		let screen = document.getElementById('screen');

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
			spinnerElement.hidden = false;
		} else {
			progressElement.value = null;
			progressElement.max = null;
			progressElement.hidden = true;
			if (!text) spinnerElement.style.display = 'none';
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
	spinnerElement.style.display = 'none';
	Module.setStatus = function(text) {
		if (text) Module.printErr('[post-exception status] ' + text);
	};
};

document.addEventListener('keydown', onKeyDown, false);
document.addEventListener('keyup', onKeyUp, false);

document.getElementById('button_full_screen').addEventListener('click', function() {
	Module.requestFullscreen(
		document.getElementById('pointerLock').checked,
		document.getElementById('resize').checked
	);
}, false);

document.getElementById('button_play').addEventListener('click', function() {
	Module.setStatus('Downloading ...');

	// Load the wasm boot strapper
	let script = document.createElement('script');
	script.setAttribute('src', 'index.js');
	document.head.appendChild(script);
}, false);

document.getElementById('screen').addEventListener('contextmenu', function(event) {
	event.preventDefault();
}, false);

documentOnReady(() => {
	Module.setStatus('Ready ...');
});
