package com.astromedicom.texturetweaksmiley;

import android.content.Context;
import android.opengl.GLES20;
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

    private int[] vaoSquare = new int[1];
    private int[] vboSquarePosition = new int[1];
    private int[] vboSquareTexture = new int[1];

    private float[] perspectiveProjectionMatrix = new float[16];

    private TextureHandler smileyTexture;
    private int tapCount = 0;

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

        smileyTexture = new TextureHandler(context, R.raw.smily);
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

        // square
        final float squareVertices[] = new float[] {
                1.0f, 1.0f, 1.0f, // front
                - 1.0f, 1.0f, 1.0f,
                - 1.0f, -1.0f, 1.0f,
                1.0f, -1.0f, 1.0f,
        };

        GLES30.glGenVertexArrays(1, vaoSquare, 0);
        GLES30.glBindVertexArray(vaoSquare[0]);

        // position
        GLES30.glGenBuffers(1, vboSquarePosition, 0);
        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, vboSquarePosition[0]);
        ByteBuffer byteBuffer = ByteBuffer.allocateDirect(squareVertices.length * 4);
        byteBuffer.order(ByteOrder.nativeOrder());
        FloatBuffer verticesBuffer = byteBuffer.asFloatBuffer();
        verticesBuffer.put(squareVertices);
        verticesBuffer.position(0);
        GLES30.glBufferData(GLES30.GL_ARRAY_BUFFER,
                squareVertices.length * 4,
                null,
                GLES30.GL_STATIC_DRAW);
        GLES30.glVertexAttribPointer(GLESMacros.VDG_ATTRIBUTE_VERTEX,
                3,
                GLES30.GL_FLOAT,
                false, 0, 0);
        GLES30.glEnableVertexAttribArray(GLESMacros.VDG_ATTRIBUTE_VERTEX);
        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, 0);

        // texture
        GLES30.glGenBuffers(1, vboSquareTexture, 0);
        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, vboSquareTexture[0]);
        GLES30.glBufferData(GLES30.GL_ARRAY_BUFFER,
                (2*3) * 4,
                null,
                GLES30.GL_DYNAMIC_DRAW);
        GLES30.glVertexAttribPointer(GLESMacros.VDG_ATTRIBUTE_TEXTURE0,
                2,
                GLES30.GL_FLOAT,
                false, 0, 0);
        GLES30.glEnableVertexAttribArray(GLESMacros.VDG_ATTRIBUTE_TEXTURE0);
        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, 0);

        GLES30.glBindVertexArray(0);

        // load textures
        smileyTexture.loadTexture();

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
        if(vaoSquare[0] != 0) {
            GLES30.glDeleteVertexArrays(1, vaoSquare, 0);
            vaoSquare[0] = 0;
        }

        // destroy VBO's
        if(vboSquarePosition[0] != 0) {
            GLES30.glDeleteBuffers(1, vboSquarePosition, 0);
            vboSquarePosition[0]=0;
        }
        if(vboSquareTexture[0] != 0) {
            GLES30.glDeleteBuffers(1, vboSquareTexture, 0);
            vboSquareTexture[0]=0;
        }
        if(vboSquarePosition[0] != 0) {
            GLES30.glDeleteBuffers(1, vboSquarePosition, 0);
            vboSquarePosition[0]=0;
        }
        if(vboSquareTexture[0] != 0) {
            GLES30.glDeleteBuffers(1, vboSquareTexture, 0);
            vboSquareTexture[0]=0;
        }

        smileyTexture.delete();

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
        Matrix.translateM(modelViewMatrix, 0, 0.0f, 0.0f, -4.0f);
        Matrix.multiplyMM(modelViewProjectionMatrix, 0,
                perspectiveProjectionMatrix, 0,
                modelViewMatrix, 0);


        float textureVertices[] =  getSquareVertices();

        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, vboSquareTexture[0]);
        ByteBuffer byteBuffer = ByteBuffer.allocateDirect(textureVertices.length * 4);
        byteBuffer.order(ByteOrder.nativeOrder());
        FloatBuffer verticesBuffer = byteBuffer.asFloatBuffer();
        verticesBuffer.put(textureVertices);
        verticesBuffer.position(0);

        GLES30.glBufferData(GLES30.GL_ARRAY_BUFFER,
                textureVertices.length * 4,
                verticesBuffer,
                GLES30.GL_DYNAMIC_DRAW);

        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, 0);

        GLES30.glUniformMatrix4fv(mvpUniform, 1, false, modelViewProjectionMatrix, 0);

        GLES30.glBindVertexArray(vaoSquare[0]);

        smileyTexture.bind(texture0SamplerUniform);

        GLES30.glDrawArrays(GLES30.GL_TRIANGLE_FAN, 0, 4);

        GLES30.glBindVertexArray(0);

        // stop using OpenGL program object
        GLES30.glUseProgram(0);

        update();

        // render/flush
        requestRender();
    }

    private float[] getSquareVertices() {

        switch (tapCount) {
            case 1:
                return new float[] {
                    0.0f, 0.0f,
                    0.5f, 0.0f,
                    0.5f, 0.5f,
                    0.0f, 0.5f,
                };

            case 2:
                return new float[] {
                    0.0f, 0.0f,
                    1.0f, 0.0f,
                    1.0f, 1.0f,
                    0.0f, 1.0f,
                };

            case 3:
                return new float[] {
                    0.0f, 0.0f,
                    2.0f, 0.0f,
                    2.0f, 2.0f,
                    0.0f, 2.0f,
                };

            case 4:
                return new float[] {
                    0.5f, 0.5f,
                    0.5f, 0.5f,
                    0.5f, 0.5f,
                    0.5f, 0.5f,
                };

            default:
                return new float[] {
                    0.0f, 0.0f,
                    0.5f, 0.0f,
                    0.5f, 0.5f,
                    0.0f, 0.5f,
                };
        }
    }

    private void update() {
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
        tapCount = (tapCount == 4) ? 0 : tapCount++;
        Log.d("TAP", tapCount + "TapUp");
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
