function main() {
	var canvas = document.getElementById("AMC");

	if(!canvas) {
		console.error("Obtaininng canvas element failed\n");
		return;
	} else {
		console.log("Obtaininng canvas element succeeded\n");
	}

	console.log("Canvas width : "+ canvas.width +" height : " + canvas.height);

	// GET 2D CONTEXT
	var context = canvas.getContext("2d");
	if(!context) {
		console.error("Obtaining 2D context failed\n");
		return;
	} else {
		console.log("Obtaining 2D context succeeded\n");
	}

	// fill canvas with color
	context.fillStyle = "black";
	context.fillRect(0, 0, canvas.width, canvas.height);

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