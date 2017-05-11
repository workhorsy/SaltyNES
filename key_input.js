
let g_get_key_up = 0;
let g_get_key_down = 0;
let g_get_key_left = 0;
let g_get_key_right = 0;
let g_get_key_b = 0;
let g_get_key_a = 0;
let g_get_key_start = 0;

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
	}
}

document.addEventListener('keydown', onKeyDown, false);
document.addEventListener('keyup', onKeyUp, false);

console.log('key_input imported ...');
