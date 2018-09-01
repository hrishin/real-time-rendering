var canvas = null;
var context = null;

function main() {

	canvas = document.getElementById("AMC");

	if(!canvas) {
		console.error("Obtaininng canvas element failed\n");
		return;
	} else {
		console.log("Obtaininng canvas element succeeded\n");
	}

	console.log("Canvas width : "+ canvas.width +" height : " + canvas.height);

	// GET 2D CONTEXT
	context = canvas.getContext("2d");
	if(!context) {
		console.error("Obtaining 2D context failed\n");
		return;
	} else {
		console.log("Obtaining 2D context succeeded\n");
	}

	// fill canvas with color
	context.fillStyle = "black";
	context.fillRect(0, 0, canvas.width, canvas.height);

	drawText("Hello World !!!");

	// register keyboard keydown event handler
	window.addEventListener("keydown", keyDown, false);
	window.addEventListener("click", mouseDown, false);
}

function drawText(text) {
	// center the text
	context.textAlign = "center";
	context.textBaseline = "middle";

	// text
	var str = "Hello World!!!";

	// text font
	context.font = "48px sans-serif";

	// text color
	context.fillStyle = "white"; //"#FFFFFF"

	// display text in center
	context.fillText(str, canvas.width/2, canvas.height/2);
}

function toggleFullScreen() {
	var fullscreen_element = document.fullscreenElement || 
	                         document.webkitFullscreenElement || 
	                         document.mozFullScreenElement || 
	                         document.msFullscreenElement ||
	                         null;

	if(fullscreen_element == null) {
		if(canvas.requestFullscreen)
			canvas.requestFullscreen();
		else if(canvas.mozRequestFullScreen)
			canvas.mozRequestFullScreen();
		else if(canvas.webkitRequestFullscreen)
			canvas.webkitRequestFullscreen();
		else if(canvas.mozRequestFullScreen)
			canvas.msRequestFullscreen();
	} else {
		if(document.exitFullscreen)
			document.exitFullscreen();
		else if(document.mozCancelFullScreen)
			document.mozCancelFullScreen();
		else if(document.webkitExitFullscreen)
			document.webkitExitFullscreen();
		else if(document.msExitFullscreen)
			document.msExitFullscreen();
	}
}

function keyDown(event) {
	console.log("key is pressed");
	switch(event.keyCode) {
		case 70:
			toggleFullScreen();

			//repaint
			drawText("Hello World !!!");

			break;
	}
}

function mouseDown(event) {
	console.log("A mouse is clicked");
}