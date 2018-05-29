package com.astromedicom.texturesquaretriangle3drotation;

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

    private int mvpUniform;
    private int texture0SamplerUniform;

    private int[] vaoPyramid = new int[1];
    private int[] vaoCube = new int[1];
    private int[] vboPyramidPosition = new int[1];
    private int[] vboPramidTexture = new int[1];
    private int[] vboSquarePosition = new int[1];

    private int[] vboCubeTexture = new int[1];

    private float[] perspectiveProjectionMatrix = new float[16];

    private float angleTriangle = 0;
    private float angleSquare = 0;

    private TextureHandler stoneTexture;
    private TextureHandler kundaliTexture;

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

        stoneTexture = new TextureHandler(context, R.raw.stone);
        kundaliTexture = new TextureHandler(context, R.raw.vijay_kundali_horz_inverted);
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
                "\n"+
                "in vec4 vPosition;"+
                "in vec2 vTexture0_coord;" +
                "out vec2 out_texture0_coord;" +
                "uniform mat4 u_mvp_matrix;"+
                "void main(void)"+
                "{"+
                "gl_Position = u_mvp_matrix * vPosition;"+
                "out_texture0_coord = vTexture0_coord;"+
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
                "\n"+
                "precision highp float;"+
                "in vec2 out_texture0_coord;"+
                "uniform highp sampler2D u_texture0_sampler;" +
                "out vec4 FragColor;"+
                "void main(void)"+
                "{"+
                "FragColor = texture(u_texture0_sampler, out_texture0_coord);"+
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
        GLES30.glBindAttribLocation(shaderProgramObject, GLESMacros.VDG_ATTRIBUTE_TEXTURE0, "vTexture0_coord");

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
        mvpUniform = GLES30.glGetUniformLocation(shaderProgramObject,"u_mvp_matrix");
        texture0SamplerUniform = GLES30.glGetUniformLocation(shaderProgramObject, "u_texture0_sampler");

        // triangle
        final float pyramidVertices[] = new float[] {
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

        final float pyramidTextureCoords[] = new float[] {
                0.5f, 1.0f, // front-top
                0.0f, 0.0f, // front-left
                1.0f, 0.0f, // front-right

                0.5f, 1.0f, // right-top
                1.0f, 0.0f, // right-left
                0.0f, 0.0f, // right-right

                0.5f, 1.0f, // back-top
                1.0f, 0.0f, // back-left
                0.0f, 0.0f, // back-right

                0.5f, 1.0f, // left-top
                0.0f, 0.0f, // left-left
                1.0f, 0.0f, // left-right
        };

        // Triangle
        GLES30.glGenVertexArrays(1, vaoPyramid, 0);
        GLES30.glBindVertexArray(vaoPyramid[0]);

        // position
        GLES30.glGenBuffers(1, vboPyramidPosition, 0);
        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, vboPyramidPosition[0]);
        ByteBuffer byteBuffer = ByteBuffer.allocateDirect(pyramidVertices.length * 4);
        //Log.d("BUFFER", "Buffer length " + byteBuffer.array().length);
        byteBuffer.order(ByteOrder.nativeOrder());
        FloatBuffer verticesBuffer = byteBuffer.asFloatBuffer();
        //Log.d("BUFFER", "Vertice Buffer length " + verticesBuffer.capacity());
        verticesBuffer.put(pyramidVertices);
        verticesBuffer.position(0);
        GLES30.glBufferData(GLES30.GL_ARRAY_BUFFER,
                pyramidVertices.length * 4,
                verticesBuffer,
                GLES30.GL_STATIC_DRAW);
        GLES30.glVertexAttribPointer(GLESMacros.VDG_ATTRIBUTE_VERTEX,
                3,
                GLES30.GL_FLOAT,
                false, 0, 0);
        GLES30.glEnableVertexAttribArray(GLESMacros.VDG_ATTRIBUTE_VERTEX);
        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, 0);

        // for texture
        GLES30.glGenBuffers(1, vboPramidTexture, 0);
        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, vboPramidTexture[0]);

        byteBuffer = ByteBuffer.allocateDirect(pyramidTextureCoords.length * 4);
        //Log.d("BUFFER", "Buffer length " + byteBuffer.array().length);
        byteBuffer.order(ByteOrder.nativeOrder());
        verticesBuffer = byteBuffer.asFloatBuffer();
        //Log.d("BUFFER", "Vertice Buffer length " + verticesBuffer.capacity());
        verticesBuffer.put(pyramidTextureCoords);
        verticesBuffer.position(0);
        GLES30.glBufferData(GLES30.GL_ARRAY_BUFFER,
                pyramidTextureCoords.length * 4,
                verticesBuffer,
                GLES30.GL_STATIC_DRAW);
        GLES30.glVertexAttribPointer(GLESMacros.VDG_ATTRIBUTE_TEXTURE0,
                2,
                GLES30.GL_FLOAT,
                false, 0, 0);
        GLES30.glEnableVertexAttribArray(GLESMacros.VDG_ATTRIBUTE_TEXTURE0);

        GLES30.glBindVertexArray(0);

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

        final float cubeTextureCoords[] = new float[] {
                0.0f,0.0f,
                1.0f,0.0f,
                1.0f,1.0f,
                0.0f,1.0f,

                0.0f,0.0f,
                1.0f,0.0f,
                1.0f,1.0f,
                0.0f,1.0f,

                0.0f,0.0f,
                1.0f,0.0f,
                1.0f,1.0f,
                0.0f,1.0f,

                0.0f,0.0f,
                1.0f,0.0f,
                1.0f,1.0f,
                0.0f,1.0f,

                0.0f,0.0f,
                1.0f,0.0f,
                1.0f,1.0f,
                0.0f,1.0f,

                0.0f,0.0f,
                1.0f,0.0f,
                1.0f,1.0f,
                0.0f,1.0f,
        };

        GLES30.glGenVertexArrays(1, vaoCube, 0);
        GLES30.glBindVertexArray(vaoCube[0]);

        // position
        GLES30.glGenBuffers(1, vboSquarePosition, 0);
        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, vboSquarePosition[0]);
        byteBuffer = ByteBuffer.allocateDirect(cubeVertices.length * 4);
        //Log.d("BUFFER", "Buffer length " + byteBuffer.array().length);
        byteBuffer.order(ByteOrder.nativeOrder());
        verticesBuffer = byteBuffer.asFloatBuffer();
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

        // texture
        GLES30.glGenBuffers(1, vboCubeTexture, 0);
        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, vboCubeTexture[0]);
        byteBuffer = ByteBuffer.allocateDirect(cubeTextureCoords.length * 4);
        //Log.d("BUFFER", "Buffer length " + byteBuffer.array().length);
        byteBuffer.order(ByteOrder.nativeOrder());
        verticesBuffer = byteBuffer.asFloatBuffer();
        //Log.d("BUFFER", "Vertice Buffer length " + verticesBuffer.capacity());
        verticesBuffer.put(cubeTextureCoords);
        verticesBuffer.position(0);
        GLES30.glBufferData(GLES30.GL_ARRAY_BUFFER,
                cubeTextureCoords.length * 4,
                verticesBuffer,
                GLES30.GL_STATIC_DRAW);
        GLES30.glVertexAttribPointer(GLESMacros.VDG_ATTRIBUTE_TEXTURE0,
                2,
                GLES30.GL_FLOAT,
                false, 0, 0);
        GLES30.glEnableVertexAttribArray(GLESMacros.VDG_ATTRIBUTE_TEXTURE0);
        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, 0);

        GLES30.glBindVertexArray(0);

        // load textures
        stoneTexture.loadTexture();
        kundaliTexture.loadTexture();

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
            vaoPyramid[0] = 0;
        }
        if (vaoCube[0] != 0) {
            GLES30.glDeleteVertexArrays(1, vaoCube, 0);
            vaoCube[0] = 0;
        }

        // destroy VBO's
        if(vboPyramidPosition[0] != 0) {
            GLES30.glDeleteBuffers(1, vboPyramidPosition, 0);
            vboPyramidPosition[0]=0;
        }
        if(vboPramidTexture[0] != 0) {
            GLES30.glDeleteBuffers(1, vboPramidTexture, 0);
            vboPramidTexture[0]=0;
        }
        if(vboSquarePosition[0] != 0) {
            GLES30.glDeleteBuffers(1, vboSquarePosition, 0);
            vboSquarePosition[0]=0;
        }
        if(vboCubeTexture[0] != 0) {
            GLES30.glDeleteBuffers(1, vboCubeTexture, 0);
            vboCubeTexture[0]=0;
        }

        stoneTexture.delete();
        kundaliTexture.delete();

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

        // Pyramid
        // set modelview & modelview projection matrices to identity
        float[] modelViewMatrix = new float[16];
        Matrix.setIdentityM(modelViewMatrix, 0);
        float[] modelViewProjectionMatrix = new float[16];
        Matrix.setIdentityM(modelViewProjectionMatrix, 0);
        Matrix.translateM(modelViewMatrix, 0, -1.5f, 0.0f, -4.0f);
        Matrix.scaleM(modelViewMatrix, 0, 0.5f, 0.5f, 0.5f);
        Matrix.rotateM(modelViewMatrix, 0, angleTriangle, 0.0f, 1.0f, 0.0f);
        Matrix.multiplyMM(modelViewProjectionMatrix, 0,
                perspectiveProjectionMatrix, 0,
                modelViewMatrix, 0);

        GLES30.glUniformMatrix4fv(mvpUniform, 1, false, modelViewProjectionMatrix, 0);

        GLES30.glBindVertexArray(vaoPyramid[0]);

        stoneTexture.bind(texture0SamplerUniform);

        GLES30.glDrawArrays(GLES30.GL_TRIANGLES, 0, 12);

        GLES30.glBindVertexArray(0);

        // Cube
        // set modelview & modelview projection matrices to identity
        modelViewMatrix = new float[16];
        Matrix.setIdentityM(modelViewMatrix, 0);
        modelViewProjectionMatrix = new float[16];
        Matrix.setIdentityM(modelViewProjectionMatrix, 0);
        Matrix.translateM(modelViewMatrix, 0, 1.5f, 0.0f, -4.0f);
        Matrix.scaleM(modelViewMatrix, 0, 0.4f, 0.4f, 0.4f);
        Matrix.rotateM(modelViewMatrix, 0, angleSquare, 1.0f, 1.0f, 1.0f);
        Matrix.multiplyMM(modelViewProjectionMatrix, 0,
                perspectiveProjectionMatrix, 0,
                modelViewMatrix, 0);

        GLES30.glUniformMatrix4fv(mvpUniform, 1, false, modelViewProjectionMatrix, 0);

        GLES30.glBindVertexArray(vaoCube[0]);

        kundaliTexture.bind(texture0SamplerUniform);

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
        angleTriangle = angleTriangle > 360 ? 0 : angleTriangle + 1.0f;
        angleSquare = angleSquare> 360 ? 0 : angleSquare+ 1.0f;
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
