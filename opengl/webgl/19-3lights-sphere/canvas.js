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
var gLightSpecular = [1.0,1.0,1.0];
var gLightAmbience = [0.0,0.0,0.0,1.0];
var gRedLightDiffuse = [1.0,0.0,0.0];
var gGreenLightDiffuse = [0.0,1.0,0.0];
var gBlueLightDiffuse = [0.0,0.0,1.0];
var gLightSpecular = [1.0,1.0,1.0];
var gRedLightPosition = [0.0,0.0,0.0,1.0];
var gGreenLightPosition = [0.0,0.0,0.0,1.0];
var gBlueLightPosition = [0.0,0.0,0.0, 1.0];
var gMaterialAmbient = [0.0,0.0,0.0];
var gMaterialDiffuse = [1.0,1.0,1.0];
var gMaterialSpecular = [1.0,1.0,1.0];
var gMaterialShininess = 50.0;

var sphere = null;

var perspectiveProjectionMatrix;

var gModelMatrixUniform, gViewMatrixUniform, gProjectionMatrixUniform;
var gLaUniform, gRedLdUniform, gLsUniform, gRedLightPositionUniform;
var gGreenLdUniform, gGreenLightPositionUniform;
var gBlueLdUniform, gBlueLightPositionUniform;
var gKaUniform, gKdUniform, gKsUniform, gMaterialShininessUniform;
var gLKeyPressedUniform, gPerVertexUniform;

var gbLight;
var gbPerVertex;

var angleRedLight;
var angleGreenLight;
var angleBlueLight;

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
    "uniform float u_lighting_enabled;" +
    "uniform float u_per_vertex;" +
    "uniform mat4 u_model_matrix;" +
    "uniform mat4 u_view_matrix;" +
    "uniform mat4 u_projection_matrix;" +
    "uniform vec3 u_La;" +
    "uniform vec3 u_r_Ld;" +
    "uniform vec3 u_g_Ld;" +
    "uniform vec3 u_b_Ld;" +
    "uniform vec3 u_Ls;" +
    "uniform vec4 u_r_light_position;" +
    "uniform vec4 u_g_light_position;" +
    "uniform vec4 u_b_light_position;" +
    "uniform vec3 u_Ka;" +
    "uniform vec3 u_Kd;" +
    "uniform vec3 u_Ks;" +
    "uniform float u_material_shininess;" +
    "out vec3 phong_ads_color_out;" +
    "out vec3 trasnformed_normals_out;" +
    "out vec3 light_r_direction_out;" +
    "out vec3 light_g_direction_out;" +
    "out vec3 light_b_direction_out;" +
    "out vec3 viewer_vector_out;" +
    "void main(void)" +
    "{" +
    "if (u_lighting_enabled > 1.0)" +
    "{" +
    "if (u_per_vertex > 1.0)" +
    "{" +
    "vec4 eye_coordinates = u_view_matrix * u_model_matrix * vPosition;" +
    "vec3 trasnformed_normals = normalize(mat3(u_view_matrix * u_model_matrix) * vNormal);" +
    "vec3 r_light_direction = normalize(vec3(u_r_light_position) - eye_coordinates.xyz);" +
    "vec3 g_light_direction = normalize(vec3(u_g_light_position) - eye_coordinates.xyz);" +
    "vec3 b_light_direction = normalize(vec3(u_b_light_position) - eye_coordinates.xyz);" +
    "float tn_dot_r_ld = max(dot(trasnformed_normals, r_light_direction), 0.0);" +
    "float tn_dot_g_ld = max(dot(trasnformed_normals, g_light_direction), 0.0);" +
    "float tn_dot_b_ld = max(dot(trasnformed_normals, b_light_direction), 0.0);" +
    "vec3 ambient = u_La * u_Ka;" +
    "vec3 r_diffuse = u_r_Ld * u_Kd * tn_dot_r_ld;" +
    "vec3 g_diffuse = u_g_Ld * u_Kd * tn_dot_g_ld;" +
    "vec3 b_diffuse = u_b_Ld * u_Kd * tn_dot_b_ld;" +
    "vec3 r_reflection_vector = reflect(-r_light_direction, trasnformed_normals);" +
    "vec3 g_reflection_vector = reflect(-g_light_direction, trasnformed_normals);" +
    "vec3 b_reflection_vector = reflect(-b_light_direction, trasnformed_normals);" +
    "vec3 viewer_vector = normalize(-eye_coordinates.xyz);" +
    "vec3 r_specular = u_Ls * u_Ks * pow(max(dot(r_reflection_vector, viewer_vector) , 0.0) , u_material_shininess);" +
    "vec3 g_specular = u_Ls * u_Ks * pow(max(dot(g_reflection_vector, viewer_vector) , 0.0) , u_material_shininess);" +
    "vec3 b_specular = u_Ls * u_Ks * pow(max(dot(b_reflection_vector, viewer_vector) , 0.0) , u_material_shininess);" +
    "phong_ads_color_out = ambient + r_diffuse + r_specular + g_diffuse + g_specular + b_diffuse + b_specular;" +
    "}" +
    "else" +
    "{" +
    "vec4 eye_coordinates = u_view_matrix * u_model_matrix * vPosition;" +
    "trasnformed_normals_out = mat3(u_view_matrix * u_model_matrix) * vNormal;" +
    "light_r_direction_out = vec3(u_r_light_position) - eye_coordinates.xyz;" +
    "light_g_direction_out = vec3(u_g_light_position) - eye_coordinates.xyz;" +
    "light_b_direction_out = vec3(u_b_light_position) - eye_coordinates.xyz;" +
    "viewer_vector_out = -eye_coordinates.xyz;" +
    "}" +
    "}" +
    "else" +
    "{" +
    "phong_ads_color_out = vec3(1.0, 1.0, 1.0);" +
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
    "#version 300 es"  +
    "\n"  +
    "precision highp float;"  +
    "in vec3 phong_ads_color_out;"  +
    "in vec3 trasnformed_normals_out;"  +
    "in vec3 light_r_direction_out;"  +
    "in vec3 light_g_direction_out;"  +
    "in vec3 light_b_direction_out;"  +
    "in vec3 viewer_vector_out;"  +
    "uniform float u_per_vertex;"  +
    "uniform float u_lighting_enabled;"  +
    "uniform vec3 u_La;"  +
    "uniform vec3 u_r_Ld;"  +
    "uniform vec3 u_g_Ld;"  +
    "uniform vec3 u_b_Ld;"  +
    "uniform vec3 u_Ls;"  +
    "uniform vec3 u_Ka;"  +
    "uniform vec3 u_Kd;"  +
    "uniform vec3 u_Ks;"  +
    "uniform float u_material_shininess;"  +
    "out vec4 FragColor;"  +
    "void main(void)"  +
    "{"  +
    "if (u_per_vertex > 1.0)"  +
    "{"  +
    "FragColor = vec4(phong_ads_color_out, 1.0);"  +
    "}"  +
    "else"  +
    "{"  +
    "vec3 phong_ads_color;"  +
    "if (u_lighting_enabled > 1.0)"  +
    "{"  +
    "vec3 normalized_trasnformed_normals = normalize(trasnformed_normals_out);"  +
    "vec3 normalized_r_light_direction = normalize(light_r_direction_out);"  +
    "vec3 normalized_g_light_direction = normalize(light_g_direction_out);"  +
    "vec3 normalized_b_light_direction = normalize(light_b_direction_out);"  +
    "float tn_dot_r_ld = max(dot(normalized_trasnformed_normals, normalized_r_light_direction), 0.0);"  +
    "float tn_dot_g_ld = max(dot(normalized_trasnformed_normals, normalized_g_light_direction), 0.0);"  +
    "float tn_dot_b_ld = max(dot(normalized_trasnformed_normals, normalized_b_light_direction), 0.0);"  +
    "vec3 r_reflection_vector = reflect(-normalized_r_light_direction, normalized_trasnformed_normals);"  +
    "vec3 g_reflection_vector = reflect(-normalized_g_light_direction, normalized_trasnformed_normals);"  +
    "vec3 b_reflection_vector = reflect(-normalized_b_light_direction, normalized_trasnformed_normals);"  +
    "vec3 normalized_viewer_vector = normalize(viewer_vector_out);"  +
    "vec3 r_specular = u_Ls * u_Ks * pow(max(dot(r_reflection_vector, normalized_viewer_vector) , 0.0) , u_material_shininess);"  +
    "vec3 g_specular = u_Ls * u_Ks * pow(max(dot(g_reflection_vector, normalized_viewer_vector) , 0.0) , u_material_shininess);"  +
    "vec3 b_specular = u_Ls * u_Ks * pow(max(dot(b_reflection_vector, normalized_viewer_vector) , 0.0) , u_material_shininess);"  +
    "vec3 ambient = u_La * u_Ka;"  +
    "vec3 r_diffuse = u_r_Ld * u_Kd * tn_dot_r_ld;"  +
    "vec3 g_diffuse = u_g_Ld * u_Kd * tn_dot_g_ld;"  +
    "vec3 b_diffuse = u_b_Ld * u_Kd * tn_dot_b_ld;"  +
    "phong_ads_color = ambient + r_diffuse + r_specular + g_diffuse + g_specular + b_diffuse + b_specular;"  +
    "}"  +
    "else"  +
    "{"  +
    "phong_ads_color = vec3(1.0, 1.0, 1.0);"  +
    "}"  +
    "FragColor = vec4(phong_ads_color, 1.0);"  +
    "}" +
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
    gPerVertexUniform = gl.getUniformLocation(shaderProgramObject,"u_per_vertex");
    
    gLaUniform = gl.getUniformLocation(shaderProgramObject,"u_La");
    gRedLdUniform = gl.getUniformLocation(shaderProgramObject,"u_r_Ld");
    gLsUniform = gl.getUniformLocation(shaderProgramObject,"u_Ls");
    gRedLightPositionUniform = gl.getUniformLocation(shaderProgramObject,"u_r_light_position");

    gGreenLdUniform = gl.getUniformLocation(shaderProgramObject,"u_g_Ld");
    gGreenLightPositionUniform = gl.getUniformLocation(shaderProgramObject,"u_g_light_position");

    gBlueLdUniform = gl.getUniformLocation(shaderProgramObject,"u_b_Ld");
    gBlueLightPositionUniform = gl.getUniformLocation(shaderProgramObject,"u_b_light_position");
    
    gKaUniform = gl.getUniformLocation(shaderProgramObject,"u_Ka");
    gKdUniform = gl.getUniformLocation(shaderProgramObject,"u_Kd");
    gKsUniform = gl.getUniformLocation(shaderProgramObject,"u_Ks");
    gMaterialShininessUniform = gl.getUniformLocation(shaderProgramObject,"u_material_shininess");
    
    // *** vertices, colors, shader attribs, vbo, vao initializations ***
    sphere = new Mesh();
    makeSphere(sphere, 2.0, 30, 30);
    
    gbLight = false;
    gbPerVertex = true;

    angleRedLight = 0.0;
    angleGreenLight = 0.0;
    angleBlueLight = 0.0;

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
    // code
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
    
    gl.useProgram(shaderProgramObject);

    if(gbPerVertex) {
        // setting light enabled uniform
        gl.uniform1f(gPerVertexUniform, 2.0);
    } else {
        gl.uniform1f(gPerVertexUniform, 0.0);
    }

    if(gbLight) {
        // setting light enabled uniform
        gl.uniform1f(gLKeyPressedUniform, 2.0);

        // setting light properties uniform
        gl.uniform3fv(gLaUniform, gLightAmbient);
        gl.uniform3fv(gRedLdUniform, gRedLightDiffuse);
        gl.uniform3fv(gLsUniform, gLightSpecular);
        gRedLightPosition[0] = 0.0;
        gRedLightPosition[1] = Math.sin(angleRedLight) * 20.0;
        gRedLightPosition[2] = Math.cos(angleRedLight) * 20.0;
        gl.uniform4fv(gRedLightPositionUniform, gRedLightPosition);
        
        gl.uniform3fv(gGreenLdUniform, gGreenLightDiffuse);
        gGreenLightPosition[0] = Math.sin(angleGreenLight) * 20.0;
        gGreenLightPosition[1] = 0.0;
        gGreenLightPosition[2] = Math.cos(angleGreenLight) * 20.0;
        gl.uniform4fv(gGreenLightPositionUniform, gGreenLightPosition);
        
        gl.uniform3fv(gBlueLdUniform, gBlueLightDiffuse);
        gBlueLightPosition[0] = Math.sin(angleBlueLight) * 20.0;
        gBlueLightPosition[1] = Math.cos(angleBlueLight) * 20.0;
        gBlueLightPosition[2] = 0.0;
        gl.uniform4fv(gBlueLightPositionUniform, gBlueLightPosition);

        gl.uniform3fv(gKaUniform, gMaterialAmbient);
        gl.uniform3fv(gKdUniform, gMaterialDiffuse);
        gl.uniform3fv(gKsUniform, gMaterialSpecular);
        gl.uniform1f(gMaterialShininessUniform, gMaterialShininess);
    } else {
        gl.uniform1f(gLKeyPressedUniform, 0.0);
    }
    
    // cube
    modelMatrix = mat4.create();
    viewMatrix = mat4.create();

    mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -5.0]);
    
    gl.uniformMatrix4fv(gModelMatrixUniform, false, modelMatrix);
    gl.uniformMatrix4fv(gViewMatrixUniform, false, viewMatrix);
    gl.uniformMatrix4fv(gProjectionMatrixUniform, false, perspectiveProjectionMatrix);

    sphere.draw();

    gl.useProgram(null);
    
    angleRedLight = angleRedLight > 360 ? 0.0 : angleRedLight + 0.10;
    angleGreenLight = angleGreenLight > 360 ? 0.0 : angleGreenLight + 0.10;
    angleBlueLight = angleBlueLight > 360 ? 0.0 : angleBlueLight + 0.10;

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

        case 86:
            gbPerVertex = true;
            break;
        
        case 76:
            gbLight = !gbLight;
            break;

        case 70: // for 'F' or 'f'
            gbPerVertex = false;
            break;

        case 83: // for 'F' or 'f'
            toggleFullScreen();
            break;            
    }
}

function mouseDown()
{
    // code
}