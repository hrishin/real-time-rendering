// global variables
var canvas = null;
var gl = null; // webgl context
var bFullscreen = false;
var canvas_original_width;
var canvas_original_height;

const WebGLMacros = // when whole 'WebGLMacros' is 'const', all inside it are automatically 'const'
{
VDG_ATTRIBUTE_VERTEX:0,
VDG_ATTRIBUTE_COLOR:1,
VDG_ATTRIBUTE_NORMAL:2,
VDG_ATTRIBUTE_TEXTURE0:3,
};

var vertexShaderObject;
var fragmentShaderObject;
var shaderProgramObject;

var vaoSqaure;
var vboPosition;
var vboNormals;

var mvpUniform;

var perspectiveProjectionMatrix;

var gModelViewMatrixUniform, gProjectionMatrixUniform;
var gLdUniform, gKdUniform, gLightPositionUniform;
var gLKeyPressedUniform;

var angleCube = 0.0;

var gbAnimate;
var gbLight;

// To start animation : To have requestAnimationFrame() to be called "cross-browser" compatible
var requestAnimationFrame =
window.requestAnimationFrame ||
window.webkitRequestAnimationFrame ||
window.mozRequestAnimationFrame ||
window.oRequestAnimationFrame ||
window.msRequestAnimationFrame;

// To stop animation : To have cancelAnimationFrame() to be called "cross-browser" compatible
var cancelAnimationFrame =
window.cancelAnimationFrame ||
window.webkitCancelRequestAnimationFrame || window.webkitCancelAnimationFrame ||
window.mozCancelRequestAnimationFrame || window.mozCancelAnimationFrame ||
window.oCancelRequestAnimationFrame || window.oCancelAnimationFrame ||
window.msCancelRequestAnimationFrame || window.msCancelAnimationFrame;

// onload function
function main()
{
    // get <canvas> element
    canvas = document.getElementById("amc");
    if(!canvas) {
        console.error("Obtaining Canvas Failed\n");
        return;
    } 
    console.log("Obtaining Canvas Succeeded\n");
    

    canvas_original_width = canvas.width;
    canvas_original_height = canvas.height;
    
    // register keyboard's keydown event handler
    window.addEventListener("keydown", keyDown, false);
    window.addEventListener("click", mouseDown, false);
    window.addEventListener("resize", resize, false);

    // initialize WebGL
    init();
    
    // start drawing here as warming-up
    resize();
    draw();
}

function init()
{
    // code
    // get WebGL 2.0 context
    gl = canvas.getContext("webgl2");
    if(gl == null) // failed to get context
    {
        console.error("Failed to get the rendering context for WebGL");
        return;
    } 
    console.log("obtained context successfully");
    
    gl.viewportWidth = canvas.width;
    gl.viewportHeight = canvas.height;
    
    // vertex shader
    var vertexShaderSourceCode =
    "#version 300 es" +
    "\n" +
    "precision highp float;" +
    "in vec4 vPosition;" +
    "in vec3 vNormal;" +
    "uniform mat4 u_model_view_matrix;" +
    "uniform mat4 u_projection_matrix;" +
    "uniform float u_LKeyPressed;" +
    "uniform vec3 u_Ld;" +
    "uniform vec3 u_Kd;" +
    "uniform vec4 u_light_position;" +
    "out vec3 diffuse_light;" +
    "void main(void)" +
    "{" +
        "if (u_LKeyPressed > 1.0)" +
        "{" +
            "vec4 eyeCoordinates = u_model_view_matrix * vPosition;"+
            "vec3 tnorm = normalize(mat3(u_model_view_matrix) * vNormal);" +
            "vec3 s = normalize(vec3(u_light_position - eyeCoordinates));" +
            "diffuse_light = u_Ld * u_Kd * max(dot(s, tnorm), 0.0);" +
        "}" +
        "gl_Position = u_projection_matrix * u_model_view_matrix * vPosition;" +
    "}";
    vertexShaderObject = gl.createShader(gl.VERTEX_SHADER);
    gl.shaderSource(vertexShaderObject, vertexShaderSourceCode);
    gl.compileShader(vertexShaderObject);
    if(gl.getShaderParameter(vertexShaderObject, gl.COMPILE_STATUS) == false)
    {
        var error = gl.getShaderInfoLog(vertexShaderObject);
        if(error.length > 0)
        {
            alert("Vertex" + error);
            uninitialize();
        }
    }
    
    // fragment shader
    var fragmentShaderSourceCode =
    "#version 300 es" +
    "\n" +
    "precision highp float;" +
    "in vec3 diffuse_light;" +
    "out vec4 FragColor;" +
    "uniform float u_LKeyPressed;" +
    "void main(void)" +
    "{" +
        "vec4 color;" +
        "if (u_LKeyPressed > 1.0)" +
        "{" +
            "color = vec4(diffuse_light, 1.0);" +
        "}" +
        "else" +
        "{" +
            "color = vec4(1.0, 1.0, 1.0, 1.0);" +
        "}" +
        "FragColor = color;" +
    "}";
    fragmentShaderObject = gl.createShader(gl.FRAGMENT_SHADER);
    gl.shaderSource(fragmentShaderObject, fragmentShaderSourceCode);
    gl.compileShader(fragmentShaderObject);
    if(gl.getShaderParameter(fragmentShaderObject, gl.COMPILE_STATUS) == false)
    {
        var error = gl.getShaderInfoLog(fragmentShaderObject);
        if(error.length > 0)
        {
            alert("Fragment: " + error);
            uninitialize();
        }
    }
    
    // shader program
    shaderProgramObject=gl.createProgram();
    gl.attachShader(shaderProgramObject, vertexShaderObject);
    gl.attachShader(shaderProgramObject, fragmentShaderObject);
    
    // pre-link binding of shader program object with vertex shader attributes
    gl.bindAttribLocation(shaderProgramObject, WebGLMacros.VDG_ATTRIBUTE_VERTEX, "vPosition");
    gl.bindAttribLocation(shaderProgramObject,WebGLMacros.VDG_ATTRIBUTE_NORMAL,"vNormal");

    // linking
    gl.linkProgram(shaderProgramObject);
    if (!gl.getProgramParameter(shaderProgramObject, gl.LINK_STATUS))
    {
        var error=gl.getProgramInfoLog(shaderProgramObject);
        if(error.length > 0)
        {
            alert(error);
            uninitialize();
        }
    }


    // get MVP uniform location
    gModelViewMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_model_view_matrix");
    gProjectionMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_projection_matrix");
    
    gLKeyPressedUniform = gl.getUniformLocation(shaderProgramObject, "u_LKeyPressed");
    
    gLdUniform = gl.getUniformLocation(shaderProgramObject, "u_Ld");
    gKdUniform = gl.getUniformLocation(shaderProgramObject, "u_Kd");
    
    gLightPositionUniform = gl.getUniformLocation(shaderProgramObject, "u_light_position");
    
    // vertices, colors, shader attribs, vbo, vao initializations
    var squareVertices = new Float32Array([
        1.0, 1.0, -1.0, // TOP
        - 1.0, 1.0, -1.0,
        - 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0,
        1.0, -1.0, -1.0, // bootom
        - 1.0, -1.0, -1.0,
        - 1.0, -1.0, 1.0,
        1.0, -1.0, 1.0,
        1.0, 1.0, 1.0, // ront
        - 1.0, 1.0, 1.0,
        - 1.0, -1.0, 1.0,
        1.0, -1.0, 1.0,
        1.0, 1.0, -1.0, // back
        - 1.0, 1.0, -1.0,
        - 1.0, -1.0, -1.0,
        1.0, -1.0, -1.0,
        1.0, 1.0, -1.0, // right
        1.0, 1.0, 1.0,
        1.0, -1.0, 1.0,
        1.0, -1.0, -1.0,
        -1.0, 1.0, -1.0, // let
        - 1.0, 1.0, 1.0,
        - 1.0, -1.0, 1.0,
        - 1.0, -1.0, -1.0
    ]);

    vaoSqaure = gl.createVertexArray();
    gl.bindVertexArray(vaoSqaure);
    
    vboPosition = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vboPosition);
    gl.bufferData(gl.ARRAY_BUFFER, squareVertices, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.VDG_ATTRIBUTE_VERTEX,
                           3, 
                           gl.FLOAT,
                           false, 0 ,0);
    gl.enableVertexAttribArray(WebGLMacros.VDG_ATTRIBUTE_VERTEX);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
   
    // vertices, colors, shader attribs, vbo, vao initializations
    var squareNormals = new Float32Array([
        // top
        0.0, 1.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 1.0, 0.0,
        
        // bottom
        0.0, -1.0, 0.0,
        0.0, -1.0, 0.0,
        0.0, -1.0, 0.0,
        0.0, -1.0, 0.0,
        
        // front
        0.0, 0.0, 1.0,
        0.0, 0.0, 1.0,
        0.0, 0.0, 1.0,
        0.0, 0.0, 1.0,
        
        // back
        0.0, 0.0, -1.0,
        0.0, 0.0, -1.0,
        0.0, 0.0, -1.0,
        0.0, 0.0, -1.0,
        
        // left
        -1.0, 0.0, 0.0,
        -1.0, 0.0, 0.0,
        -1.0, 0.0, 0.0,
        -1.0, 0.0, 0.0,
        
        // right
        1.0, 0.0, 0.0,
        1.0, 0.0, 0.0,
        1.0, 0.0, 0.0,
        1.0, 0.0, 0.0
    ])
    
    vboNormals = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vboNormals);
    gl.bufferData(gl.ARRAY_BUFFER, squareNormals, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.VDG_ATTRIBUTE_NORMAL,
                            3, 
                            gl.FLOAT,
                            false, 0 ,0);
    gl.enableVertexAttribArray(WebGLMacros.VDG_ATTRIBUTE_NORMAL);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    gl.bindVertexArray(null);  
    
    gbAnimate = false;
    gbLight = false;

    // set clear color
    gl.clearColor(0.0, 0.0, 0.0, 1.0); // black
    
    // Depth test will always be enabled
    gl.enable(gl.DEPTH_TEST);

    gl.depthFunc(gl.LEQUAL);
    
    // We will always cull back faces for better performance
    //gl.enable(gl.CULL_FACE);
    
    // initialize projection matrix
    perspectiveProjectionMatrix = mat4.create();
}

function resize()
{
    // code
    if(bFullscreen == true)
    {
        canvas.width = window.innerWidth;
        canvas.height = window.innerHeight;
    }
    else
    {
        canvas.width = canvas_original_width;
        canvas.height = canvas_original_height;
    }
   
    // set the viewport to match
    gl.viewport(0, 0, canvas.width, canvas.height);
    mat4.perspective(perspectiveProjectionMatrix, 45.0, parseFloat(canvas.width) / parseFloat(canvas.height), 0.1, 100.0);
}

function draw()
{
    // code
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
    
    gl.useProgram(shaderProgramObject);

    if(gbLight) {
        gl.uniform1f(gLKeyPressedUniform, 2.0);
        
        gl.uniform3f(gLdUniform, 1.0, 1.0, 1.0);
        gl.uniform3f(gKdUniform, 0.5, 0.5, 0.5);
        
        var lightPosition = [0.0, 0.0, 2.0, 1.0];
        gl.uniform4fv(gLightPositionUniform, lightPosition);
    } else {
        gl.uniform1f(gLKeyPressedUniform, 2.0);
    }
    
    // cube
    modelViewMatrix = mat4.create();
    modelViewProjectionMatrix = mat4.create();

    mat4.translate(modelViewMatrix, modelViewMatrix, [0.0, 0.0, -5.0]);
    mat4.rotateX(modelViewMatrix ,modelViewMatrix, degToRad(angleCube));
    mat4.rotateY(modelViewMatrix ,modelViewMatrix, degToRad(angleCube));
    mat4.rotateZ(modelViewMatrix ,modelViewMatrix, degToRad(angleCube));
    
    
    gl.uniformMatrix4fv(gModelViewMatrixUniform, false, modelViewMatrix);
    gl.uniformMatrix4fv(gProjectionMatrixUniform, false, perspectiveProjectionMatrix);

    gl.bindVertexArray(vaoSqaure);
    gl.drawArrays(gl.TRIANGLE_FAN, 0, 4);
    gl.drawArrays(gl.TRIANGLE_FAN, 4, 4);
    gl.drawArrays(gl.TRIANGLE_FAN, 8, 4);
    gl.drawArrays(gl.TRIANGLE_FAN, 12, 4);
    gl.drawArrays(gl.TRIANGLE_FAN, 16, 4);
    gl.drawArrays(gl.TRIANGLE_FAN, 20, 4);
    gl.bindVertexArray(null);
    
    gl.useProgram(null);

    if(gbAnimate) {
        angleCube = angleCube+1.0;
        if(angleCube >= 360.0)
            angleCube = angleCube - 360.0;
    }
    
    // animation loop
    requestAnimationFrame(draw, canvas);
}

function uninitialize()
{
    if(vaoSqaure)
    {
        gl.deleteVertexArray(vaoSqaure);
        vaoSqaure = null;
    }
    
    if(vboPosition)
    {
        gl.deleteBuffer(vboPosition);
        vboPosition = null;
    }

    if(vboNormals)
    {
        gl.deleteBuffer(vboNormals);
        vboNormals = null;
    }

    if(shaderProgramObject)
    {
        if(fragmentShaderObject)
        {
            gl.detachShader(shaderProgramObject, fragmentShaderObject);
            gl.deleteShader(fragmentShaderObject);
            fragmentShaderObject = null;
        }
        
        if(vertexShaderObject)
        {
            gl.detachShader(shaderProgramObject, vertexShaderObject);
            gl.deleteShader(vertexShaderObject);
            vertexShaderObject = null;
        }
        
        gl.deleteProgram(shaderProgramObject);
        shaderProgramObject = null;
    }
}

function toggleFullScreen()
{
    // code
    var fullscreen_element =
    document.fullscreenElement ||
    document.webkitFullscreenElement ||
    document.mozFullScreenElement ||
    document.msFullscreenElement ||
    null;

    // if not fullscreen
    if(fullscreen_element == null)
    {
        if(canvas.requestFullscreen)
            canvas.requestFullscreen();
        else if(canvas.mozRequestFullScreen)
            canvas.mozRequestFullScreen();
        else if(canvas.webkitRequestFullscreen)
            canvas.webkitRequestFullscreen();
        else if(canvas.msRequestFullscreen)
            canvas.msRequestFullscreen();
        bFullscreen = true;
    }
    else // if already fullscreen
    {
        if(document.exitFullscreen)
            document.exitFullscreen();
        else if(document.mozCancelFullScreen)
            document.mozCancelFullScreen();
        else if(document.webkitExitFullscreen)
            document.webkitExitFullscreen();
        else if(document.msExitFullscreen)
            document.msExitFullscreen();
        bFullscreen=false;
    }
}

function keyDown(event)
{
    // code
    switch(event.keyCode)
    {
        case 27: // Escape
            // uninitialize
            uninitialize();
            // close our application's tab
            window.close(); // may not work in Firefox but works in Safari and chrome
            break;

        case 65:
            gbAnimate = !gbAnimate;
            break;

        case 76:
            gbLight = !gbLight;
            break;

        case 70: // for 'F' or 'f'
            toggleFullScreen();
            break;
    }
}

function mouseDown()
{
    // code
}

function degToRad(degrees)
{
    // code
    return(degrees * Math.PI / 180);
}