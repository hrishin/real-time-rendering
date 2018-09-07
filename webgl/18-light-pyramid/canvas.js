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

var vaoTriangle;
var vboPosition;
var vboPyramidNormal;

var gModelMatrixUniform, gViewMatrixUniform, gProjectionMatrixUniform;
var gLaUniform, gL1dUniform, gL2dUniform, gLsUniform, gLight1PositionUniform, gLight2PositionUniform;
var gKaUniform, gKdUniform, gKsUniform, gMaterialShinessUniform;
var gLKeyPressedUniform;

var gLightAmbient = [0.0, 0.0, 0.0];
var gLight1Diffuse = [1.0, 0.0, 0.0];
var gLightSpecular = [1.0, 1.0, 1.0];
var gLight1Position = [200.0, 100.0, 100.0, 1.0];

var gLight2Diffuse = [0.0, 0.0, 1.0];
var gLight2Position = [-200.0,100.0,100.0,1.0];

var gMaterialAmbient = [0.0,0.0,0.0];
var gMaterialDiffuse = [1.0,1.0,1.0];
var gMaterialSpecular = [1.0,1.0,1.0];
var gMaterialShininess = 50.0;

var perspectiveProjectionMatrix;

var gbLight;
var gbAnimate;

var anglePyramid =0.0;

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
    "in vec4 vPosition;" +
    "in vec3 vNormal;" +
    "uniform mat4 u_model_matrix;" +
    "uniform mat4 u_view_matrix;" +
    "uniform mat4 u_projection_matrix;" +
    "uniform float u_lighting_enabled;" +
    "uniform vec3 u_La;" +
    "uniform vec3 u_L1d;" +
    "uniform vec3 u_L2d;" +
    "uniform vec3 u_Ls;" +
    "uniform vec4 u_light1_position;" +
    "uniform vec4 u_light2_position;" +
    "uniform vec3 u_Ka;" +
    "uniform vec3 u_Kd;" +
    "uniform vec3 u_Ks;" +
    "uniform float u_material_shininess;" +
    "out vec3 phong_ads_color;" +
    "void main(void)" +
    "{" +
        "if (u_lighting_enabled > 1.0)" +
        "{" +
            "vec4 eye_coordinates = u_view_matrix * u_model_matrix * vPosition;" +
            "vec3 trasnformed_normals = normalize(mat3(u_view_matrix * u_model_matrix) * vNormal);" +
            "vec3 light1_direction = normalize(vec3(u_light1_position) - eye_coordinates.xyz);" +
            "vec3 light2_direction = normalize(vec3(u_light2_position) - eye_coordinates.xyz);" +
            "float tn_dot_ld1 = max(dot(trasnformed_normals, light1_direction), 0.0);" +
            "float tn_dot_ld2 = max(dot(trasnformed_normals, light2_direction), 0.0);" +
            "vec3 ambient = u_La * u_Ka;" +
            "vec3 diffuse1 = u_L1d * u_Kd * tn_dot_ld1;" +
            "vec3 diffuse2 = u_L2d * u_Kd * tn_dot_ld2;" +
            "vec3 reflection_vector1 = reflect(-light1_direction, trasnformed_normals);" +
            "vec3 reflection_vector2 = reflect(-light2_direction, trasnformed_normals);" +
            "vec3 viewer_vector = normalize(-eye_coordinates.xyz);" +
            "vec3 specular1 = u_Ls * u_Ks * pow(max(dot(reflection_vector1, viewer_vector) , 0.0) , u_material_shininess);" +
            "vec3 specular2 = u_Ls * u_Ks * pow(max(dot(reflection_vector2, viewer_vector) , 0.0) , u_material_shininess);" +
            "phong_ads_color = ambient + diffuse1 + specular1 + diffuse2 + specular2;" +
        "}" +
        "else" +
        "{" +
            "phong_ads_color = vec3(1.0, 1.0, 1.0);" +
        "}" +
        "gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vPosition;" +
    "}";
    vertexShaderObject = gl.createShader(gl.VERTEX_SHADER);
    gl.shaderSource(vertexShaderObject, vertexShaderSourceCode);
    gl.compileShader(vertexShaderObject);
    if(gl.getShaderParameter(vertexShaderObject, gl.COMPILE_STATUS) == false)
    {
        var error = gl.getShaderInfoLog(vertexShaderObject);
        if(error.length > 0)
        {
            alert(error);
            uninitialize();
        }
    }
    
    // fragment shader
    var fragmentShaderSourceCode =
    "#version 300 es" +
    "\n" +
    "precision highp float;" +
    "in vec3 phong_ads_color;" +
    "out vec4 FragColor;" +
    "void main(void)" +
    "{" +
        "FragColor = vec4(phong_ads_color, 1.0);" +
    "}";
    fragmentShaderObject = gl.createShader(gl.FRAGMENT_SHADER);
    gl.shaderSource(fragmentShaderObject, fragmentShaderSourceCode);
    gl.compileShader(fragmentShaderObject);
    if(gl.getShaderParameter(fragmentShaderObject, gl.COMPILE_STATUS) == false)
    {
        var error = gl.getShaderInfoLog(fragmentShaderObject);
        if(error.length > 0)
        {
            alert(error);
            uninitialize();
        }
    }
    
    // shader program
    shaderProgramObject=gl.createProgram();
    gl.attachShader(shaderProgramObject, vertexShaderObject);
    gl.attachShader(shaderProgramObject, fragmentShaderObject);
    
    // pre-link binding of shader program object with vertex shader attributes
    gl.bindAttribLocation(shaderProgramObject, WebGLMacros.VDG_ATTRIBUTE_VERTEX, "vPosition");
    gl.bindAttribLocation(shaderProgramObject, WebGLMacros.VDG_ATTRIBUTE_NORMAL, "vNormal");

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
    gModelMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_model_matrix");
    gViewMatrixUniform = gl.getUniformLocation(shaderProgramObject,"u_view_matrix");
    gProjectionMatrixUniform = gl.getUniformLocation(shaderProgramObject,"u_projection_matrix");

    gLKeyPressedUniform = gl.getUniformLocation(shaderProgramObject,"u_lighting_enabled");

    gLaUniform = gl.getUniformLocation(shaderProgramObject,"u_La");
    gL1dUniform = gl.getUniformLocation(shaderProgramObject,"u_L1d");
    gLsUniform = gl.getUniformLocation(shaderProgramObject,"u_Ls");
    gLight1PositionUniform = gl.getUniformLocation(shaderProgramObject,"u_light1_position");
    
    gL2dUniform = gl.getUniformLocation(shaderProgramObject,"u_L2d");
    gLight2PositionUniform = gl.getUniformLocation(shaderProgramObject,"u_light2_position");

    gKaUniform = gl.getUniformLocation(shaderProgramObject,"u_Ka");
    gKdUniform = gl.getUniformLocation(shaderProgramObject,"u_Kd");
    gKsUniform = gl.getUniformLocation(shaderProgramObject,"u_Ks");
    gMaterialShininessUniform = gl.getUniformLocation(shaderProgramObject,"u_material_shininess");


    vaoTriangle = gl.createVertexArray();
    gl.bindVertexArray(vaoTriangle);

    // vertices, colors, shader attribs, vbo, vao initializations
    var triangleVertices = new Float32Array([
        0.0, 1.0, 0.0, // front
        -1.0, -1.0, 1.0,
        1.0, -1.0, 1.0,
        
        0.0, 1.0, 0.0, // right
        1.0, -1.0, 1.0,
        1.0, -1.0, -1.0,
        
        0.0, 1.0, 0.0, // back
        1.0, -1.0, -1.0,
        -1.0, -1.0, -1.0,
        
        0.0, 1.0, 0.0, // left
        -1.0, -1.0, -1.0,
        -1.0, -1.0, 1.0
    ]);
    
    vboPosition = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vboPosition);
    gl.bufferData(gl.ARRAY_BUFFER, triangleVertices, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.VDG_ATTRIBUTE_VERTEX,
                           3, 
                           gl.FLOAT,
                           false, 0 ,0);
    gl.enableVertexAttribArray(WebGLMacros.VDG_ATTRIBUTE_VERTEX);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    // vertices, colors, shader attribs, vbo, vao initializations
    var triangleNormals = new Float32Array([
        0.0, 0.447214, 0.894427,
        0.0, 0.447214, 0.894427,
        0.0, 0.447214, 0.894427,

        0.447214, 0.894427, 0.0,
        0.447214, 0.894427, 0.0,
        0.447214, 0.894427, 0.0,

        0.0, 0.447214, -0.894427,
        0.0, 0.447214, -0.894427,
        0.0, 0.447214, -0.894427,

        -0.447214, 0.894427, 0.0,
        -0.447214, 0.894427, 0.0,
        -0.447214, 0.894427, 0.0,
    ]);
    
    vboPyramidNormal = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vboPyramidNormal);
    gl.bufferData(gl.ARRAY_BUFFER, triangleNormals, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.VDG_ATTRIBUTE_NORMAL,
                           3, 
                           gl.FLOAT,
                           false, 0 ,0);
    gl.enableVertexAttribArray(WebGLMacros.VDG_ATTRIBUTE_NORMAL);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    gl.bindVertexArray(null);

    // set clear color
    gl.clearColor(0.0, 0.0, 0.0, 1.0); // black
    
    // Depth test will always be enabled
    gl.enable(gl.DEPTH_TEST);
    
    gbLight = false;
    gbAnimate = false;

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
        // setting light enabled uniform
        gl.uniform1f(gLKeyPressedUniform, 2.0);

        // setting light properties uniform
        gl.uniform3fv(gLaUniform, gLightAmbient);
        gl.uniform3fv(gL1dUniform, gLight1Diffuse);
        gl.uniform3fv(gLsUniform, gLightSpecular);
        gl.uniform4fv(gLight1PositionUniform, gLight1Position);
        
        gl.uniform3fv(gL2dUniform, gLight2Diffuse);
        gl.uniform4fv(gLight2PositionUniform, gLight2Position);

        gl.uniform3fv(gKaUniform, gMaterialAmbient);
        gl.uniform3fv(gKdUniform, gMaterialDiffuse);
        gl.uniform3fv(gKsUniform, gMaterialSpecular);
        gl.uniform1f(gMaterialShininessUniform, gMaterialShininess);
    } else {
        gl.uniform1f(gLKeyPressedUniform, 0.0);
    }
    
    // traingle
    modelMatrix = mat4.create();
    viewMatrix = mat4.create();

    mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -5.0]);
    mat4.rotateY(modelMatrix ,modelMatrix, degToRad(anglePyramid));    
    gl.uniformMatrix4fv(gModelMatrixUniform, false, modelMatrix);
    gl.uniformMatrix4fv(gViewMatrixUniform, false, viewMatrix);
    gl.uniformMatrix4fv(gProjectionMatrixUniform, false, perspectiveProjectionMatrix);

    gl.bindVertexArray(vaoTriangle);
    gl.drawArrays(gl.TRIANGLES, 0, 12);
    gl.bindVertexArray(null);
    
    gl.useProgram(null);

    if(gbAnimate) {
        anglePyramid = anglePyramid >= 360.0 ? 0 : anglePyramid + 1.0
    }

    // animation loop
    requestAnimationFrame(draw, canvas);
}

function uninitialize()
{
    // code
    if(vaoTriangle)
    {
        gl.deleteVertexArray(vaoTriangle);
        vaoTriangle = null;
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
            
        case 76: // for 'F' or 'f'
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