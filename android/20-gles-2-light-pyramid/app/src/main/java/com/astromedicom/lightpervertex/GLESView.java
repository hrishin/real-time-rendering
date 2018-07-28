package com.astromedicom.lightpervertex;

import android.content.Context;
import android.opengl.GLES30;
import android.opengl.GLSurfaceView;
import android.opengl.Matrix;
import android.util.Log;
import android.view.GestureDetector;
import android.view.MotionEvent;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * Created by Hrishikesh on 28-01-2018.
 */

class GLESView extends GLSurfaceView implements GLSurfaceView.Renderer, GestureDetector.OnGestureListener, GestureDetector.OnDoubleTapListener {
    private final GestureDetector gestureDetector;
    private final Context context;

    private int vertexShaderObject;
    private int fragmentShaderObject;
    private int shaderProgramObject;

    private int modelMatrixUniform, viewMatrixUniform, projectionMatrixUniform;
    private int laUniform, ld1Uniform, ld2Uniform, lsUniform, light1PositionUniform, light2PositionUniform;
    private int kaUniform, kdUniform, ksUniform, materialShinessUniform;
    private int enableLightUniform;

    private int[] vaoPyramid = new int[1];
    private int[] vboPyramidPosition = new int[1];
    private int[] vboPyramidNormal = new int[1];

    private float[] perspectiveProjectionMatrix = new float[16];

    private float lightAmbient[] = {0.0f, 0.0f, 0.0f, 1.0f};
    private float lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    private float light1Diffuse[] = { 1.0f, 0.0f, 0.0f, 1.0f };
    private float light1Position[] = { 200.0f, 100.0f, 0.0f, 1.0f };
    private float light2Diffuse[] = { 0.0f, 0.0f, 1.0f, 1.0f };
    private float light2Position[] = { -200.0f, 100.0f, 0.0f, 1.0f };

    private float materialAmbient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    private float materialDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    private float materialSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    private float materialShininess = 50.0f;

    private boolean isLight;
    private float pyramidAngle;

    public GLESView(Context context) {
        super(context);
        this.context = context;
        setEGLContextClientVersion(3);

        // set the renderer for drawing on the GLSurfaceView
        setRenderer(this);

        // render the view only when there is change in drawing data
        setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);

        gestureDetector = new GestureDetector(context, this, null, false);
        gestureDetector.setOnDoubleTapListener(this);
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig eglConfig) {
        String gleVersion = gl.glGetString(GL10.GL_VERSION);
        Log.d("VERSION", "GLE version " + gleVersion);
        String glSLVersion = gl.glGetString(GLES30.GL_SHADING_LANGUAGE_VERSION);
        Log.d("VERSION", "GL Shader Language version " + glSLVersion);
        initialize(gl);
    }

    private void initialize(GL10 gl) {
        /*VERTEX SHADER*/
        // create shader
        vertexShaderObject = GLES30.glCreateShader(GLES30.GL_VERTEX_SHADER);

        // vertex shader source code
        final String vertexShaderSourceCode = String.format(
                "#version 300 es"+ 
                "\n" +
                "in vec4 vPosition;" +
                "in vec3 vNormal;" +
                "uniform mat4 u_model_matrix;" +
                "uniform mat4 u_view_matrix;" +
                "uniform mat4 u_projection_matrix;" +
                "uniform int u_lighting_enabled;" +
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
                "if (u_lighting_enabled == 1)" +
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
                "}"
        );

        // set shader source
        GLES30.glShaderSource(vertexShaderObject, vertexShaderSourceCode);
        GLES30.glCompileShader(vertexShaderObject);

        // get vertex  shader compilation error message
        int[] iShaderCompiledStatus = new int[1];
        GLES30.glGetShaderiv(vertexShaderObject, GLES30.GL_COMPILE_STATUS, iShaderCompiledStatus, 0);
        if (iShaderCompiledStatus[0] == GLES30.GL_FALSE) {
            int[] iShaderLength = new int[1];
            GLES30.glGetShaderiv(vertexShaderObject, GLES30.GL_INFO_LOG_LENGTH, iShaderLength, 0);
            if(iShaderLength[0] > 0) {
                String logMsg = GLES30.glGetShaderInfoLog(vertexShaderObject);
                Log.d("COMPILATION", "Vertex Shader compilation log :" + logMsg);
                unInitialize();
                System.exit(0);
            }
        }

        /*FRAGMENT SHADER*/
        // create shader
        fragmentShaderObject = GLES30.glCreateShader(GLES30.GL_FRAGMENT_SHADER);

        // fragment shader source code
        final String fragmentShaderSourceCode = String.format(
                "#version 300 es"+
                "\n" +
                "in vec3 phong_ads_color;" +
                "out vec4 FragColor;" +
                "void main(void)" +
                "{" +
                "FragColor = vec4(phong_ads_color, 1.0);" +
                "}"
        );

        // set shader source
        GLES30.glShaderSource(fragmentShaderObject, fragmentShaderSourceCode);
        GLES30.glCompileShader(fragmentShaderObject);

        // get fragment shader compilation error message
        iShaderCompiledStatus[0] = 0;
        GLES30.glGetShaderiv(fragmentShaderObject, GLES30.GL_COMPILE_STATUS, iShaderCompiledStatus, 0);
        if (iShaderCompiledStatus[0] == GLES30.GL_FALSE) {
            int[] iShaderLength = new int[1];
            GLES30.glGetShaderiv(fragmentShaderObject, GLES30.GL_INFO_LOG_LENGTH, iShaderLength, 0);
            if(iShaderLength[0] > 0) {
                String logMsg = GLES30.glGetShaderInfoLog(fragmentShaderObject);
                Log.d("COMPILATION", "Fragment Shader compilation log :" + logMsg);
                unInitialize();
                System.exit(0);
            }
        }

        // shader program object
        shaderProgramObject = GLES30.glCreateProgram();

        GLES30.glAttachShader(shaderProgramObject, vertexShaderObject);

        GLES30.glAttachShader(shaderProgramObject, fragmentShaderObject);

        // prelink of dynamic data for vertex position attribute
        GLES30.glBindAttribLocation(shaderProgramObject, GLESMacros.VDG_ATTRIBUTE_VERTEX, "vPosition");

        // prelink of dynamic data for texture attribute
        GLES30.glBindAttribLocation(shaderProgramObject, GLESMacros.VDG_ATTRIBUTE_NORMAL, "vNormal");

        // link shader program
        GLES30.glLinkProgram(shaderProgramObject);

        int[] iShaderProgamLinkStatus = new int[1];
        GLES30.glGetProgramiv(shaderProgramObject, GLES30.GL_LINK_STATUS, iShaderProgamLinkStatus, 0);
        if(iShaderProgamLinkStatus[0] == GLES30.GL_FALSE) {
            int[] iInfoLogLength = new int[1];
            GLES30.glGetProgramiv(shaderProgramObject,GLES30.GL_INFO_LOG_LENGTH, iInfoLogLength, 0);
            if(iInfoLogLength[0] > 0) {
                String logMsg = GLES30.glGetProgramInfoLog(shaderProgramObject);
                Log.d("COMPILATION", "Shader program link log :" + logMsg);
                unInitialize();
                System.exit(0);
            }
        }

        // get MVP uniform location
        modelMatrixUniform = GLES30.glGetUniformLocation(shaderProgramObject,"u_model_matrix");
        viewMatrixUniform = GLES30.glGetUniformLocation(shaderProgramObject,"u_view_matrix");
        projectionMatrixUniform = GLES30.glGetUniformLocation(shaderProgramObject,"u_projection_matrix");

        enableLightUniform = GLES30.glGetUniformLocation(shaderProgramObject,"u_lighting_enabled");

        laUniform = GLES30.glGetUniformLocation(shaderProgramObject,"u_La");
        ld1Uniform = GLES30.glGetUniformLocation(shaderProgramObject,"u_L1d");
        ld2Uniform = GLES30.glGetUniformLocation(shaderProgramObject,"u_L2d");
        lsUniform = GLES30.glGetUniformLocation(shaderProgramObject,"u_Ls");
        light1PositionUniform = GLES30.glGetUniformLocation(shaderProgramObject,"u_light1_position");
        light2PositionUniform = GLES30.glGetUniformLocation(shaderProgramObject,"u_light2_position");

        kaUniform = GLES30.glGetUniformLocation(shaderProgramObject,"u_Ka");
        kdUniform = GLES30.glGetUniformLocation(shaderProgramObject,"u_Kd");
        ksUniform = GLES30.glGetUniformLocation(shaderProgramObject,"u_Ks");
        materialShinessUniform = GLES30.glGetUniformLocation(shaderProgramObject,"u_material_shininess");

        // get pyramid vertices, normals, textures, elements
        final float pyramidVertices[] =
        {
                0.0f, 1.0f, 0.0f, // front
                -1.0f, -1.0f, 1.0f,
                1.0f, -1.0f, 1.0f,

                0.0f, 1.0f, 0.0f, // right
                1.0f, -1.0f, 1.0f,
                1.0f, -1.0f, -1.0f,

                0.0f, 1.0f, 0.0f, // back
                1.0f, -1.0f, -1.0f,
                -1.0f, -1.0f, -1.0f,

                0.0f, 1.0f, 0.0f, // left
                -1.0f, -1.0f, -1.0f,
                -1.0f, -1.0f, 1.0f
        };

        final float pyramidNormals[] =
        {
                0.0f, 0.447214f, 0.894427f,
                0.0f, 0.447214f, 0.894427f,
                0.0f, 0.447214f, 0.894427f,

                0.447214f, 0.894427f, 0.0f,
                0.447214f, 0.894427f, 0.0f,
                0.447214f, 0.894427f, 0.0f,

                0.0f, 0.447214f, -0.894427f,
                0.0f, 0.447214f, -0.894427f,
                0.0f, 0.447214f, -0.894427f,

                -0.447214f, 0.894427f, 0.0f,
                -0.447214f, 0.894427f, 0.0f,
                -0.447214f, 0.894427f, 0.0f,
        };

        GLES30.glGenVertexArrays(1, vaoPyramid, 0);
        GLES30.glBindVertexArray(vaoPyramid[0]);

        // position
        GLES30.glGenBuffers(1, vboPyramidPosition, 0);
        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, vboPyramidPosition[0]);
        ByteBuffer byteBuffer=ByteBuffer.allocateDirect(pyramidVertices.length * 4);
        byteBuffer.order(ByteOrder.nativeOrder());
        FloatBuffer verticesBuffer=byteBuffer.asFloatBuffer();
        verticesBuffer.put(pyramidVertices);
        verticesBuffer.position(0);
        GLES30.glBufferData(GLES30.GL_ARRAY_BUFFER, pyramidVertices.length * 4, verticesBuffer, GLES30.GL_STATIC_DRAW);
        GLES30.glVertexAttribPointer(GLESMacros.VDG_ATTRIBUTE_VERTEX, 3, GLES30.GL_FLOAT, false,0,0);
        GLES30.glEnableVertexAttribArray(GLESMacros.VDG_ATTRIBUTE_VERTEX);
        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER,0);

        // normal vbo
        GLES30.glGenBuffers(1, vboPyramidNormal,0);
        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, vboPyramidNormal[0]);
        byteBuffer=ByteBuffer.allocateDirect(pyramidNormals.length * 4);
        byteBuffer.order(ByteOrder.nativeOrder());
        verticesBuffer=byteBuffer.asFloatBuffer();
        verticesBuffer.put(pyramidNormals);
        verticesBuffer.position(0);
        GLES30.glBufferData(GLES30.GL_ARRAY_BUFFER,pyramidNormals.length * 4, verticesBuffer, GLES30.GL_STATIC_DRAW);
        GLES30.glVertexAttribPointer(GLESMacros.VDG_ATTRIBUTE_NORMAL, 3, GLES30.GL_FLOAT, false,0,0);
        GLES30.glEnableVertexAttribArray(GLESMacros.VDG_ATTRIBUTE_NORMAL);
        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER,0);

        GLES30.glBindVertexArray(0);

        // todo : check attributes
        // enable depth testing
        GLES30.glEnable(GLES30.GL_DEPTH_TEST);

        // depth test to do
        GLES30.glDepthFunc(GLES30.GL_LEQUAL);

        // We will always cull back faces for better performance
        //GLES30.glEnable(GLES30.GL_CULL_FACE);

        // Set the background color
        GLES30.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        Matrix.setIdentityM(perspectiveProjectionMatrix, 0);
    }

    private void unInitialize() {
        // destroy vboPyramidPosition
        if(vaoPyramid[0] != 0) {
            GLES30.glDeleteVertexArrays(1, vaoPyramid, 0);
            vaoPyramid[0]=0;
        }

        if(vboPyramidPosition[0] != 0) {
            GLES30.glDeleteBuffers(1, vboPyramidPosition, 0);
            vboPyramidPosition[0]=0;
        }

        if(vboPyramidNormal[0] != 0) {
            GLES30.glDeleteBuffers(1, vboPyramidNormal, 0);
            vboPyramidNormal[0]=0;
        }

        if(shaderProgramObject != 0) {
            // delete vertex shader
            if(vertexShaderObject != 0) {
                GLES30.glDetachShader(shaderProgramObject, vertexShaderObject);
                GLES30.glDeleteShader(vertexShaderObject);
                vertexShaderObject = 0;
            }

            // delete fragment shader
            if(fragmentShaderObject != 0) {
                GLES30.glDetachShader(shaderProgramObject, fragmentShaderObject);
                GLES30.glDeleteShader(fragmentShaderObject);
                fragmentShaderObject = 0;
            }

            // delete shader program object
            GLES30.glDeleteProgram(shaderProgramObject);
            shaderProgramObject = 0;
        }
    }

    @Override
    public void onSurfaceChanged(GL10 unsed, int width, int height) {
        resize(width, height);
    }

    private void resize(int width, int height) {
        GLES30.glViewport(0 , 0, width, height);
        Matrix.perspectiveM(perspectiveProjectionMatrix, 0, 45.0f, (float) width/(float) height, 1.0f, 100.0f);
    }

    @Override
    public void onDrawFrame(GL10 gl10) {
        display();
    }

    private void display() {
        /*
	    state function: May be
	    - clear the FrameBuffer for following properties
	    - color Buffer, Depth Buffer, Stencil Buffer, Accumulate Buffer for fragement
	    */
        GLES30.glClear(GLES30.GL_COLOR_BUFFER_BIT | GLES30.GL_DEPTH_BUFFER_BIT);

        // start using OpenGL program object
        GLES30.glUseProgram(shaderProgramObject);

        // OpenGl rendering
        if (isLight) {
            GLES30.glUniform1i(enableLightUniform, 1);

            GLES30.glUniform3fv(laUniform, 1, FloatBuffer.wrap(lightAmbient));
            GLES30.glUniform3fv(ld1Uniform, 1, FloatBuffer.wrap(light1Diffuse));
            GLES30.glUniform3fv(lsUniform, 1, FloatBuffer.wrap(lightSpecular));
            GLES30.glUniform3fv(light1PositionUniform, 1, FloatBuffer.wrap(light1Position));

            GLES30.glUniform3fv(ld2Uniform, 1, FloatBuffer.wrap(light2Diffuse));
            GLES30.glUniform3fv(light2PositionUniform, 1, FloatBuffer.wrap(light2Position));

            GLES30.glUniform3fv(kaUniform, 1, FloatBuffer.wrap(materialAmbient));
            GLES30.glUniform3fv(kdUniform, 1, FloatBuffer.wrap(materialDiffuse));
            GLES30.glUniform3fv(ksUniform, 1, FloatBuffer.wrap(materialSpecular));
            GLES30.glUniform1f(materialShinessUniform, materialShininess);
        } else {
            GLES30.glUniform1i(enableLightUniform, 0);
        }
        // Cube
        // set modelview & modelview projection matrices to identity
        float[] modelMatrix = new float[16];
        float[] viewMatrix = new float[16];

        Matrix.setIdentityM(modelMatrix, 0);
        Matrix.setIdentityM(viewMatrix, 0);

        Matrix.translateM(modelMatrix, 0, 0.0f, 0.0f, -6.0f);
        Matrix.rotateM(modelMatrix, 0, pyramidAngle, 0.0f, 1.0f, 0.0f);

        GLES30.glUniformMatrix4fv(modelMatrixUniform, 1, false, modelMatrix, 0);

        GLES30.glUniformMatrix4fv(viewMatrixUniform, 1, false, viewMatrix, 0);

        GLES30.glUniformMatrix4fv(projectionMatrixUniform, 1, false, perspectiveProjectionMatrix, 0);

        GLES30.glBindVertexArray(vaoPyramid[0]);

        GLES30.glDrawArrays(GLES30.GL_TRIANGLES, 0, 12);

        GLES30.glBindVertexArray(0);

        // stop using OpenGL program object
        GLES30.glUseProgram(0);

        update();

        // render/flush
        requestRender();
    }

    private void update() {
        pyramidAngle  = pyramidAngle > 360 ? 0 : pyramidAngle + 0.5f;
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        // code
        int eventaction = event.getAction();
        if(!gestureDetector.onTouchEvent(event))
            super.onTouchEvent(event);
        return(true);
    }

    @Override
    public boolean onSingleTapConfirmed(MotionEvent motionEvent) {
        return true;
    }

    @Override
    public boolean onDoubleTap(MotionEvent motionEvent) {
        isLight = !isLight;
        return true;
    }

    @Override
    public boolean onDoubleTapEvent(MotionEvent motionEvent) {
        return true;
    }

    @Override
    public boolean onDown(MotionEvent motionEvent) {
        return true;
    }

    @Override
    public void onShowPress(MotionEvent motionEvent) {

    }

    @Override
    public boolean onSingleTapUp(MotionEvent motionEvent) {
        return true;
    }

    @Override
    public boolean onScroll(MotionEvent motionEvent, MotionEvent motionEvent1, float v, float v1) {
        unInitialize();
        System.exit(0);
        return(true);
    }

    @Override
    public void onLongPress(MotionEvent motionEvent) {

    }

    @Override
    public boolean onFling(MotionEvent motionEvent, MotionEvent motionEvent1, float v, float v1) {
        return true;
    }
}
