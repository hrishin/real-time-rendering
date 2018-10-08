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

var gLightAmbient = [0.0,0.0,0.0];
var gLightDiffuse = [1.0,1.0,1.0];
var gLightSpecular = [1.0,1.0,1.0];
var gLightPosition = [100.0,100.0,100.0,1.0];

var gMaterialAmbient = [ 
[0.021500, 0.174500, 0.021500],
[0.135000, 0.222500, 0.157500],
[0.053750, 0.050000, 0.066250],
[0.250000, 0.207250, 0.207250],
[0.174500, 0.011750, 0.011750],
[0.100000, 0.187250, 0.174500],
[0.329412, 0.223529, 0.027451],
[0.212500, 0.127500, 0.054000],
[0.250000, 0.250000, 0.250000],
[0.191250, 0.073500, 0.022500],
[0.247250, 0.199500, 0.074500],
[0.192250, 0.192250, 0.192250],
[0.000000, 0.000000, 0.000000],
[0.000000, 0.100000, 0.060000],
[0.000000, 0.000000, 0.000000],
[0.000000, 0.000000, 0.000000],
[0.000000, 0.000000, 0.000000],
[0.000000, 0.000000, 0.000000],
[0.020000, 0.020000, 0.020000],
[0.000000, 0.050000, 0.050000],
[0.000000, 0.050000, 0.000000],
[0.050000, 0.000000, 0.000000],
[0.050000, 0.050000, 0.050000],
[0.050000, 0.050000, 0.000000],
]; 
var gMaterialDiffuse = [ 
[0.075680, 0.614240, 0.075680],
[0.540000, 0.890000, 0.630000],
[0.182750, 0.170000, 0.225250],
[1.000000, 0.829000, 0.829000],
[0.614240, 0.041360, 0.041360],
[0.396000, 0.741510, 0.691020],
[0.780392, 0.568627, 0.113725],
[0.714000, 0.428400, 0.181440],
[0.400000, 0.400000, 0.400000],
[0.703800, 0.270480, 0.082800],
[0.751640, 0.606480, 0.226480],
[0.507540, 0.507540, 0.507540],
[0.010000, 0.010000, 0.010000],
[0.000000, 0.509804, 0.509804],
[0.100000, 0.350000, 0.100000],
[0.500000, 0.000000, 0.000000],
[0.550000, 0.550000, 0.550000],
[0.500000, 0.500000, 0.000000],
[0.010000, 0.010000, 0.010000],
[0.400000, 0.500000, 0.500000],
[0.400000, 0.500000, 0.400000],
[0.500000, 0.400000, 0.400000],
[0.500000, 0.500000, 0.500000],
[0.500000, 0.500000, 0.400000],
];
var gMaterialSpecular = [ 
[0.633000, 0.727811, 0.633000],
[0.316228, 0.316228, 0.316228],
[0.332741, 0.328634, 0.346435],
[0.296648, 0.296648, 0.296648],
[0.727811, 0.626959, 0.626959],
[0.297254, 0.308290, 0.306678],
[0.992157, 0.941176, 0.807843],
[0.393548, 0.271906, 0.166721],
[0.774597, 0.774597, 0.774597],
[0.256777, 0.137622, 0.086014],
[0.628281, 0.555802, 0.366065],
[0.508273, 0.508273, 0.508273],
[0.500000, 0.500000, 0.500000],
[0.501961, 0.501961, 0.501961],
[0.450000, 0.550000, 0.450000],
[0.700000, 0.600000, 0.600000],
[0.700000, 0.700000, 0.700000],
[0.600000, 0.600000, 0.500000],
[0.400000, 0.400000, 0.400000],
[0.040000, 0.700000, 0.700000],
[0.040000, 0.700000, 0.040000],
[0.700000, 0.040000, 0.040000],
[0.700000, 0.700000, 0.700000],
[0.700000, 0.700000, 0.040000],
];
var gMaterialShininess = [ 
[0.600000, 128.000000],
[0.100000, 128.000000],
[0.300000, 128.000000],
[0.088000, 128.000000],
[0.600000, 128.000000],
[0.100000, 128.000000],
[0.217949, 128.000000],
[0.200000, 128.000000],
[0.600000, 128.000000],
[0.100000, 128.000000],
[0.400000, 128.000000],
[0.400000, 128.000000],
[0.250000, 128.000000],
[0.250000, 128.000000],
[0.250000, 128.000000],
[0.250000, 128.000000],
[0.250000, 128.000000],
[0.250000, 128.000000],
[0.078125, 128.000000],
[0.078125, 128.000000],
[0.078125, 128.000000],
[0.078125, 128.000000],
[0.078125, 128.000000],
[0.078125, 128.000000],
];  

var sphere = null;

var perspectiveProjectionMatrix;

var gModelMatrixUniform, gViewMatrixUniform, gProjectionMatrixUniform;
var gLaUniform, gLdUniform, gLlsUniform, gLightPositionUniform;
var gKaUniform, gKdUniform, gKsUniform, gMaterialShininessUniform;
var gLKeyPressedUniform;

var gbLight;
var gLightRotationAxis;
    
var angleXLight;
var angleYLight;
var angleZLight;
   
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
    "uniform mat4 u_model_matrix;" +
    "uniform mat4 u_view_matrix;" +
    "uniform mat4 u_projection_matrix;" +
    "uniform float u_lighting_enabled;" +
    "uniform vec4 u_light_position;" +
    "out vec3 trasnformed_normals;" +
    "out vec3 light_direction ;" +
    "out vec3 viewer_vector;" +
    "void main(void)" +
    "{" +
    "if (u_lighting_enabled >= 1.0)" +
    "{" +
    "vec4 eye_coordinates = u_view_matrix * u_model_matrix * vPosition;" +
    "trasnformed_normals = mat3(u_view_matrix * u_model_matrix) * vNormal;" +
    "light_direction = vec3(u_light_position) - eye_coordinates.xyz;" +
    "viewer_vector = -eye_coordinates.xyz;" +
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
            alert("Vertex" + error);
            uninitialize();
        }
    }
    
    // fragment shader
    var fragmentShaderSourceCode =
    "#version 300 es" +
    "\n" +
    "precision highp float;" +
    "in vec3 trasnformed_normals;" +
    "in vec3 light_direction;" +
    "in vec3 viewer_vector;" +
    "out vec4 FragColor;" +
    "uniform float u_lighting_enabled;" +
    "uniform vec3 u_La;" +
    "uniform vec3 u_Ld;" +
    "uniform vec3 u_Ls;" +
    "uniform vec3 u_Ka;" +
    "uniform vec3 u_Kd;" +
    "uniform vec3 u_Ks;" +
    "uniform float u_material_shininess;" +
    "void main(void)" +
    "{" +
    "vec3 phong_ads_color;" +
    "if (u_lighting_enabled >= 1.0)" +
    "{" +
    "vec3 normalized_trasnformed_normals = normalize(trasnformed_normals);" +
    "vec3 normalized_light_direction = normalize(light_direction);" +
    "float tn_dot_ld = max(dot(normalized_trasnformed_normals, normalized_light_direction), 0.0);" +
    "vec3 reflection_vector = reflect(-normalized_light_direction, normalized_trasnformed_normals);" +
    "vec3 normalized_viewer_vector = normalize(viewer_vector);" +
    "vec3 specular = u_Ls * u_Ks * pow(max(dot(reflection_vector, normalized_viewer_vector) , 0.0) , u_material_shininess);" +
    "vec3 ambient = u_La * u_Ka;" +
    "vec3 diffuse = u_Ld * u_Kd * tn_dot_ld;" +
    "phong_ads_color = ambient + diffuse + specular;" +
    "}" +
    "else" +
    "{" +
    "phong_ads_color = vec3(1.0, 1.0, 1.0);" +
    "}" +
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

    gModelMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_model_matrix");
    gViewMatrixUniform = gl.getUniformLocation(shaderProgramObject,"u_view_matrix");
    gProjectionMatrixUniform = gl.getUniformLocation(shaderProgramObject,"u_projection_matrix");
    
    // get single tap detecting uniform
    gLKeyPressedUniform = gl.getUniformLocation(shaderProgramObject,"u_lighting_enabled");
    
    gLaUniform = gl.getUniformLocation(shaderProgramObject,"u_La");
    gLdUniform = gl.getUniformLocation(shaderProgramObject,"u_Ld");
    gLsUniform = gl.getUniformLocation(shaderProgramObject,"u_Ls");
    gLightPositionUniform = gl.getUniformLocation(shaderProgramObject,"u_light_position");
    
    gKaUniform = gl.getUniformLocation(shaderProgramObject,"u_Ka");
    gKdUniform = gl.getUniformLocation(shaderProgramObject,"u_Kd");
    gKsUniform = gl.getUniformLocation(shaderProgramObject,"u_Ks");
    gMaterialShininessUniform = gl.getUniformLocation(shaderProgramObject,"u_material_shininess");
    
    // *** vertices, colors, shader attribs, vbo, vao initializations ***
    sphere = new Mesh();
    makeSphere(sphere, 0.7, 30, 30);
    
    gbLight = false;
    gLightRotationAxis = 'x';
    
    angleXLight = 0.0;
    angleYLight = 0.0;
    angleZLight = 0.0;

    // set clear color
    gl.clearColor(0.0, 0.0, 0.0, 1.0); // black
    
    // Depth test will always be enabled
    gl.enable(gl.DEPTH_TEST);

    gl.depthFunc(gl.LEQUAL);
    
    // We will always cull back faces for better performance
    gl.enable(gl.CULL_FACE);
    
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
    var index;
    var row, column;
    var xPos, yPos;

    // code
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
    
    gl.useProgram(shaderProgramObject);

    if(gbLight) 
    {
        // setting light enabled uniform
        gl.uniform1f(gLKeyPressedUniform, 2.0);

        // setting light properties uniform
        gl.uniform3fv(gLaUniform, gLightAmbient);
        gl.uniform3fv(gLdUniform, gLightDiffuse);
        gl.uniform3fv(gLsUniform, gLightSpecular);
        gl.uniform4fv(gLightPositionUniform, gLightPosition);

        if (gLightRotationAxis == 'x')
		{
			gLightPosition[0] = Math.sin(angleXLight) * 20.0;
			gLightPosition[1] = 0.0;
			gLightPosition[2] = Math.cos(angleXLight) * 20.0;
			gl.uniform4fv(gLightPositionUniform, gLightPosition);
		}
		else if (gLightRotationAxis == 'y')
		{
			gLightPosition[0] = 0.0;
			gLightPosition[1] = Math.sin(angleYLight) * 20.0;
			gLightPosition[2] = Math.cos(angleYLight) * 20.0;
			gl.uniform4fv(gLightPositionUniform, gLightPosition);
		}
		else if (gLightRotationAxis == 'z')
		{
			gLightPosition[0] = Math.sin(angleZLight) * 20.0;
			gLightPosition[1] = Math.cos(angleZLight) * 20.0;
			gLightPosition[2] = 0.0;
			gl.uniform4fv(gLightPositionUniform, gLightPosition);
		}
    } else 
    {
        gl.uniform1f(gLKeyPressedUniform, 0.0);
    }
    
    // cube
    viewMatrix = mat4.create();
    
    gl.uniformMatrix4fv(gViewMatrixUniform, false, viewMatrix);
    gl.uniformMatrix4fv(gProjectionMatrixUniform, false, perspectiveProjectionMatrix);
    index = 0;
    yPos = 3.5;
    for (row = 0; row < 6; row++) 
    {
        xPos = -2.5;
        for (column = 0; column < 4; column++) 
        {
            modelMatrix = mat4.create();
            mat4.translate(modelMatrix, modelMatrix, [xPos, yPos, -12.0]);
            gl.uniformMatrix4fv(gModelMatrixUniform, false, modelMatrix);

            if(gbLight) 
            {
                gl.uniform3fv(gKaUniform, gMaterialAmbient[index]);
                gl.uniform3fv(gKdUniform, gMaterialDiffuse[index]);
                gl.uniform3fv(gKsUniform, gMaterialSpecular[index]);
                gl.uniform1f(gMaterialShininessUniform, gMaterialShininess[index][0] * gMaterialShininess[index][1] );
            }

            sphere.draw();

            xPos = xPos + 1.5;
            index ++;
        }

        yPos = yPos - 1.5;
    }


    gl.useProgram(null);
    
    angleXLight = angleXLight + 0.1;
    angleYLight = angleYLight + 0.1;
    angleZLight = angleZLight + 0.1;

    // animation loop
    requestAnimationFrame(draw, canvas);
}

function uninitialize()
{
    if(sphere)
    {
        sphere.deallocate();
        sphere = null;
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

        case 88:
            gLightRotationAxis = 'x';
            break;
            
        case 89:
            gLightRotationAxis = 'y';
            break;
        
        case 90:
            gLightRotationAxis = 'z';
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