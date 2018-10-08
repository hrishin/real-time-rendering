package com.astromedicom.lightdefuse;

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

    private int modelViewMatrixUniform, projectionMatrixUniform;
    private int ldUniform, kdUniform, lightPositionUniform;
    private int keyPressedUniform;

    private int[] vaoCube = new int[1];
    private int[] vboSquarePosition = new int[1];
    private int[] vboSquareNormal = new int[1];

    private float[] perspectiveProjectionMatrix = new float[16];

    private float angleSquare = 0.0f;

    private boolean isAnimate, isLight;

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
                "uniform mat4 u_model_view_matrix;" +
                "uniform mat4 u_projection_matrix;" +
                "uniform int u_LKeyPressed;" +
                "uniform vec3 u_Ld;" +
                "uniform vec3 u_Kd;" +
                "uniform vec4 u_light_position;" +
                "out vec3 diffuse_light;" +
                "void main(void)" +
                "{" +
                "if (u_LKeyPressed == 1)" +
                "{" +
                "vec4 eyeCoordinates = u_model_view_matrix * vPosition;" +
                "vec3 tnorm = normalize(mat3(u_model_view_matrix) * vNormal);" +
                "vec3 s = normalize(vec3(u_light_position - eyeCoordinates));" +
                "diffuse_light = u_Ld * u_Kd * max(dot(s, tnorm), 0.0);" +
                "}" +
                "gl_Position = u_projection_matrix * u_model_view_matrix * vPosition;" +
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
                "in vec3 diffuse_light;" +
                "out vec4 FragColor;" +
                "uniform int u_LKeyPressed;" +
                "void main(void)" +
                "{" +
                "vec4 color;" +
                "if (u_LKeyPressed == 1)" +
                "{" +
                "color = vec4(diffuse_light, 1.0);" +
                "}" +
                "else" +
                "{" +
                "color = vec4(1.0, 1.0, 1.0, 1.0);" +
                "}" +
                "FragColor = color;" +
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
        modelViewMatrixUniform = GLES30.glGetUniformLocation(shaderProgramObject,"u_model_view_matrix");
        projectionMatrixUniform = GLES30.glGetUniformLocation(shaderProgramObject,"u_projection_matrix");

        keyPressedUniform = GLES30.glGetUniformLocation(shaderProgramObject,"u_LKeyPressed");

        ldUniform = GLES30.glGetUniformLocation(shaderProgramObject,"u_Ld");
        kdUniform = GLES30.glGetUniformLocation(shaderProgramObject,"u_Kd");
        lightPositionUniform = GLES30.glGetUniformLocation(shaderProgramObject,"u_light_position");

        // square
        final float cubeVertices[] = new float[] {
            1.0f, 1.0f, -1.0f, // TOP
            - 1.0f, 1.0f, -1.0f,
            - 1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,

            1.0f, -1.0f, -1.0f, // bootom
            - 1.0f, -1.0f, -1.0f,
            - 1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, 1.0f,

            1.0f, 1.0f, 1.0f, // front
            - 1.0f, 1.0f, 1.0f,
            - 1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, 1.0f,

            1.0f, 1.0f, -1.0f, // back
            - 1.0f, 1.0f, -1.0f,
            - 1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            1.0f, 1.0f, -1.0f, // right
            1.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, 1.0f, -1.0f, // left
            - 1.0f, 1.0f, 1.0f,
            - 1.0f, -1.0f, 1.0f,
            - 1.0f, -1.0f, -1.0f
        };

        final float cubeNormals[] = new float[] {
            0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,

            0.0f, -1.0f, 0.0f,
            0.0f, -1.0f, 0.0f,
            0.0f, -1.0f, 0.0f,
            0.0f, -1.0f, 0.0f,

            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,

            0.0f, 0.0f, -1.0f,
            0.0f, 0.0f, -1.0f,
            0.0f, 0.0f, -1.0f,
            0.0f, 0.0f, -1.0f,

            -1.0f, 0.0f, 0.0f,
            -1.0f, 0.0f, 0.0f,
            -1.0f, 0.0f, 0.0f,
            -1.0f, 0.0f, 0.0f,

            1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f
        };

        GLES30.glGenVertexArrays(1, vaoCube, 0);
        GLES30.glBindVertexArray(vaoCube[0]);

        // position
        GLES30.glGenBuffers(1, vboSquarePosition, 0);
        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, vboSquarePosition[0]);
        ByteBuffer byteBuffer = ByteBuffer.allocateDirect(cubeVertices.length * 4);
        //Log.d("BUFFER", "Buffer length " + byteBuffer.array().length);
        byteBuffer.order(ByteOrder.nativeOrder());
        FloatBuffer verticesBuffer = byteBuffer.asFloatBuffer();
        //Log.d("BUFFER", "Vertice Buffer length " + verticesBuffer.capacity());
        verticesBuffer.put(cubeVertices);
        verticesBuffer.position(0);
        GLES30.glBufferData(GLES30.GL_ARRAY_BUFFER,
                cubeVertices.length * 4,
                verticesBuffer,
                GLES30.GL_STATIC_DRAW);
        GLES30.glVertexAttribPointer(GLESMacros.VDG_ATTRIBUTE_VERTEX,
                3,
                GLES30.GL_FLOAT,
                false, 0, 0);
        GLES30.glEnableVertexAttribArray(GLESMacros.VDG_ATTRIBUTE_VERTEX);
        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, 0);

        // normals
        GLES30.glGenBuffers(1, vboSquareNormal, 0);
        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, vboSquareNormal[0]);
        byteBuffer = ByteBuffer.allocateDirect(cubeVertices.length * 4);
        //Log.d("BUFFER", "Buffer length " + byteBuffer.array().length);
        byteBuffer.order(ByteOrder.nativeOrder());
        verticesBuffer = byteBuffer.asFloatBuffer();
        //Log.d("BUFFER", "Vertice Buffer length " + verticesBuffer.capacity());
        verticesBuffer.put(cubeNormals);
        verticesBuffer.position(0);
        GLES30.glBufferData(GLES30.GL_ARRAY_BUFFER,
                cubeVertices.length * 4,
                verticesBuffer,
                GLES30.GL_STATIC_DRAW);
        GLES30.glVertexAttribPointer(GLESMacros.VDG_ATTRIBUTE_NORMAL,
                3,
                GLES30.GL_FLOAT,
                false, 0, 0);
        GLES30.glEnableVertexAttribArray(GLESMacros.VDG_ATTRIBUTE_NORMAL);
        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, 0);

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
        if(vboSquarePosition[0] != 0) {
            GLES30.glDeleteBuffers(1, vboSquarePosition, 0);
            vboSquarePosition[0]=0;
        }

        if(vboSquareNormal[0] != 0) {
            GLES30.glDeleteBuffers(1, vboSquareNormal, 0);
            vboSquareNormal[0]=0;
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
            GLES30.glUniform1i(keyPressedUniform, 1);
            GLES30.glUniform3f(ldUniform, 1.0f, 1.0f, 1.0f);
            GLES30.glUniform3f(kdUniform, 0.5f, 0.5f, 0.5f);
            GLES30.glUniform4fv(lightPositionUniform, 1, FloatBuffer.wrap(new float[]{0.0f, 0.0f, 2.0f, 1.0f}));
        } else {
            GLES30.glUniform1i(keyPressedUniform, 0);
        }
        // Cube
        // set modelview & modelview projection matrices to identity
        float[] modelMatrix = new float[16];
        float[] modelViewMatrix = new float[16];
        float[] rotationMatrix = new float[16];

        Matrix.setIdentityM(modelMatrix, 0);
        Matrix.setIdentityM(modelViewMatrix, 0);
        Matrix.setIdentityM(rotationMatrix, 0);

        Matrix.translateM(modelMatrix, 0, 0.0f, 0.0f, -4.0f);
        Matrix.rotateM(rotationMatrix, 0, angleSquare, 1.0f, 1.0f, 1.0f);

        Matrix.multiplyMM(modelViewMatrix, 0,
                modelMatrix, 0,
                rotationMatrix, 0);

        GLES30.glUniformMatrix4fv(modelViewMatrixUniform, 1, false, modelViewMatrix, 0);

        GLES30.glUniformMatrix4fv(projectionMatrixUniform, 1, false, perspectiveProjectionMatrix, 0);

        GLES30.glBindVertexArray(vaoCube[0]);

        GLES30.glDrawArrays(GLES30.GL_TRIANGLE_FAN, 0, 4);
        GLES30.glDrawArrays(GLES30.GL_TRIANGLE_FAN, 4, 4);
        GLES30.glDrawArrays(GLES30.GL_TRIANGLE_FAN, 8, 4);
        GLES30.glDrawArrays(GLES30.GL_TRIANGLE_FAN, 12, 4);
        GLES30.glDrawArrays(GLES30.GL_TRIANGLE_FAN, 16, 4);
        GLES30.glDrawArrays(GLES30.GL_TRIANGLE_FAN, 20, 4);

        GLES30.glBindVertexArray(0);

        // stop using OpenGL program object
        GLES30.glUseProgram(0);

        update();

        // render/flush
        requestRender();
    }

    private void update() {
        if(isAnimate) {
            angleSquare = angleSquare > 360 ? 0 : angleSquare + 1.0f;
        }
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
        isAnimate =!isAnimate;
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
        Log.d("OPENGL", "tapped");
        isLight = !isLight;
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
