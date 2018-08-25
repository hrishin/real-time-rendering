//
//  CustomView.m
//  Window
//
//  Created by Hrishikesh Shinde on 22/08/18.
//

#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>

#import "GLESView.h"

#import "vmath.h"

enum
{
    VDG_ATTRIBUTE_VERTEX = 0,
    VDG_ATTRIBUTE_COLOR,
    VDG_ATTRIBUTE_NORMAL,
    VDG_ATTRIBUTE_TEXTURE0,
};

@interface GLESView(PrivateMethods)
    -(void) exitIfError : (GLuint) glObject : (NSString*) message;
@end

@implementation GLESView
{
    EAGLContext *eaglContext;
    
    GLuint defaultFramebuffer;
    GLuint colorRenderbuffer;
    GLuint depthRenderbuffer;
    id displayLink;

    GLuint vertexShaderObject;
    GLuint fragmentShaderObject;
    GLuint shaderProgramObject;
    
    GLuint gVaoSquare;
    GLuint gVboSquarePosition;
    GLuint gVboSquareNormal;
    
    GLuint gModelViewMatrixUniform, gProjectionMatrixUniform;
    GLuint gLdUniform, gKdUniform, gLightPositionUniform;
    GLuint gLKeyPressedUniform;
    
    vmath::mat4 perspectiveProjectionMatrix;
    
    GLfloat cubeRotation;
    
    bool gbAnimate;
    bool gbLight;
    
    NSInteger animationFrameInterval;
    BOOL isAnimating;
}

- (id) initWithFrame: (CGRect)frameRect
{
    self = [super initWithFrame: frameRect];
    if(self)
    {
        CAEAGLLayer *eaglLayer = (CAEAGLLayer *) super.layer;
        eaglLayer.opaque = YES;
        eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys: [NSNumber numberWithBool: FALSE], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
        
        eaglContext = [[EAGLContext alloc] initWithAPI: kEAGLRenderingAPIOpenGLES3];
        if(eaglContext==nil)
        {
            [self release];
            return(nil);
        }
        [EAGLContext setCurrentContext: eaglContext];
        
        glGenBuffers(1, &defaultFramebuffer);
        glGenBuffers(1, &colorRenderbuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
        
        [eaglContext renderbufferStorage: GL_RENDERBUFFER fromDrawable: eaglLayer];
        
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRenderbuffer);
        GLint backingWidth;
        GLint backingHeight;
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &backingWidth);
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &backingHeight);
        
        glGenRenderbuffers(1, &depthRenderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, backingWidth, backingHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,depthRenderbuffer);
        
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            NSLog(@"Failed To Create Framebuffer Object %x\n", glCheckFramebufferStatus(GL_FRAMEBUFFER));
            glDeleteFramebuffers(1, &defaultFramebuffer);
            glDeleteRenderbuffers(1, &colorRenderbuffer);
            glDeleteRenderbuffers(1, &depthRenderbuffer);
            
            return nil;
        }
        
        NSLog(@"Renderer : %s | GL Version : %s | GLSL Version : %s\n", glGetString(GL_RENDERER), glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));
        
        // VERTEX shader
        //create shader object
        vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
        
        // program
        const char *vertexShaderSourceCode =
        "#version 300 es" \
        "\n" \
        "in vec4 vPosition;" \
        "in vec3 vNormal;" \
        "uniform mat4 u_model_view_matrix;" \
        "uniform mat4 u_projection_matrix;" \
        "uniform int u_LKeyPressed;" \
        "uniform vec3 u_Ld;" \
        "uniform vec3 u_Kd;" \
        "uniform vec4 u_light_position;" \
        "out vec3 diffuse_light;" \
        "void main(void)" \
        "{" \
            "if (u_LKeyPressed == 1)" \
            "{" \
                "vec4 eyeCoordinates = u_model_view_matrix * vPosition;"\
                "vec3 tnorm = normalize(mat3(u_model_view_matrix) * vNormal);" \
                "vec3 s = normalize(vec3(u_light_position - eyeCoordinates));" \
                "diffuse_light = u_Ld * u_Kd * max(dot(s, tnorm), 0.0);" \
            "}" \
            "gl_Position = u_projection_matrix * u_model_view_matrix * vPosition;" \
        "}";
        glShaderSource(vertexShaderObject, 1, (const char **) &vertexShaderSourceCode, NULL);
        
        // compile shader
        glCompileShader(vertexShaderObject);
        
        // check compilation error
        [self exitIfError: vertexShaderObject : @"Vertex shader compilation"];
        
        // FRAGMENT shader
        //create shader object
        fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
        
        // program
        const char *fragmentShaderSourceCode =
        "#version 300 es" \
        "\n" \
        "precision highp float;" \
        "in vec3 diffuse_light;" \
        "out vec4 FragColor;" \
        "uniform int u_LKeyPressed;" \
        "void main(void)" \
        "{" \
            "vec4 color;" \
            "if (u_LKeyPressed == 1)" \
            "{" \
                "color = vec4(diffuse_light, 1.0);"
            "}" \
            "else" \
            "{" \
                "color = vec4(1.0, 1.0, 1.0, 1.0);" \
            "}" \
            "FragColor = color;" \
        "}";
        glShaderSource(fragmentShaderObject, 1, (const char **) &fragmentShaderSourceCode, NULL);
        
        // compile shader
        glCompileShader(fragmentShaderObject);
       
        // check compilation errors
        [self exitIfError: fragmentShaderObject : @"Fragment shader compilation"];
        
        // SHADER PROGRAM
        shaderProgramObject = glCreateProgram();
        
        // attach vertex shader to shader program
        glAttachShader(shaderProgramObject, vertexShaderObject);
        
        // attach fragment shader to shader program
        glAttachShader(shaderProgramObject, fragmentShaderObject);
        
        // pre-link shader program attributes
        glBindAttribLocation(shaderProgramObject, VDG_ATTRIBUTE_VERTEX, "vPosition");
        glBindAttribLocation(shaderProgramObject, VDG_ATTRIBUTE_NORMAL, "vNormal");
        
        // link shader
        glLinkProgram(shaderProgramObject);
        
        // check linking errors
        [self exitIfError: shaderProgramObject : @"Shader program object compilation"];
        
        // get MVP uniform location
        gModelViewMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_model_view_matrix");
        gProjectionMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_projection_matrix");
        
        gLKeyPressedUniform = glGetUniformLocation(shaderProgramObject, "u_LKeyPressed");
        
        gLdUniform = glGetUniformLocation(shaderProgramObject, "u_Ld");
        gKdUniform = glGetUniformLocation(shaderProgramObject, "u_Kd");
        
        gLightPositionUniform = glGetUniformLocation(shaderProgramObject, "u_light_position");
        
        // Cube VAO
        glGenVertexArrays(1, &gVaoSquare);
        glBindVertexArray(gVaoSquare);
        
        // Position
        const GLfloat cubeVertices[] =
        {
            1.0f, 1.0f, -1.0f, // TOP
            -1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            
            1.0f, -1.0f, -1.0f, // bootom
            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, 1.0f,
            
            1.0f, 1.0f, 1.0f, // front
            -1.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, 1.0f,
            
            1.0f, 1.0f, -1.0f, // back
            -1.0f, 1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            
            1.0f, 1.0f, -1.0f, // right
            1.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, -1.0f,
            
            -1.0f, 1.0f, -1.0f, // left
            -1.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f,
            -1.0f, -1.0f, -1.0f
        };
        
        glGenBuffers(1, &gVboSquarePosition);
        glBindBuffer(GL_ARRAY_BUFFER, gVboSquarePosition);
        // move data from main memory to graphics memory
        // GL_STATIC_DRAW or GL_DYNAMIC_DRAW : How you want load data run or preloading. example game shows loadingbar and "loading" messages
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(VDG_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(VDG_ATTRIBUTE_VERTEX);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        // Normals
        const GLfloat cubeNormals[] =
        {
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
        
        glGenBuffers(1, &gVboSquareNormal);
        glBindBuffer(GL_ARRAY_BUFFER, gVboSquareNormal);
        // move data from main memory to graphics memory
        // GL_STATIC_DRAW or GL_DYNAMIC_DRAW : How you want load data run or preloading. example game shows loadingbar and "loading" messages
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeNormals), cubeNormals, GL_STATIC_DRAW);
        glVertexAttribPointer(VDG_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(VDG_ATTRIBUTE_NORMAL);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        glBindVertexArray(0);
        
        //glClearDepthf(1.0f);
        
        // enable depth testing
        glEnable(GL_DEPTH_TEST);
        // depth test to do
        glDepthFunc(GL_LEQUAL);
        // always cullback faces for better performance
        //glEnable(GL_CULL_FACE);
        
        // clear color
        glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
        
        // set projection matrix to identity matrix
        perspectiveProjectionMatrix = vmath::mat4::identity();
        
        // other property intialization
        animationFrameInterval = 60;
        isAnimating = NO;
        
        cubeRotation   = 0.0f;
        
        gbAnimate = false;
        gbLight = false;
        
        // gesture handling
        // tap handling
        UITapGestureRecognizer *singleTap = [[UITapGestureRecognizer alloc] initWithTarget: self action: @selector(onSingleTap:)];
        [singleTap setNumberOfTapsRequired: 1];
        [singleTap setNumberOfTouchesRequired: 1]; // 1 finger
        //[singleTap setDelegate: self];
        [self addGestureRecognizer: singleTap];
        
        UITapGestureRecognizer *doubleTap = [[UITapGestureRecognizer alloc] initWithTarget: self action: @selector(onDoubleTap:)];
        [doubleTap setNumberOfTapsRequired: 2];
        [doubleTap setNumberOfTouchesRequired: 1]; // 1 finger
        //[singleTap setDelegate: self];
        [self addGestureRecognizer: doubleTap];
        
        //  let single ignore it if its double tapped
        [singleTap requireGestureRecognizerToFail: doubleTap];
        
        // swipe gesture
        UISwipeGestureRecognizer *swipe = [[UISwipeGestureRecognizer alloc] initWithTarget: self action: @selector(onSwipe:)];
        [self addGestureRecognizer: swipe];
        
        // long press
        UILongPressGestureRecognizer *longPressGestureRecognizer = [[UILongPressGestureRecognizer alloc]initWithTarget: self action: @selector(onLongPress:)];
        [self addGestureRecognizer: longPressGestureRecognizer];
    }
    
    return self;
}

// override if want to do custom drawing
// empty implemntation may impact rendering performance
//- (void) drawRect: (CGRect)rect
//{
//
//}

+ (Class) layerClass
{
    return [CAEAGLLayer class];
}

- (void) drawView: (id)sender
{
    [EAGLContext setCurrentContext: eaglContext];
    
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    
    // start using program object
    glUseProgram(shaderProgramObject);
    
    if (gbLight)
    {
        glUniform1i(gLKeyPressedUniform, 1);
        
        glUniform3f(gLdUniform, 1.0f, 1.0f, 1.0f);
        glUniform3f(gKdUniform, 0.5f, 0.5f, 0.5f);
        
        float lightPosition[] = {0.0f, 0.0f, 2.0f, 1.0f};
        glUniform4fv(gLightPositionUniform, 1, (GLfloat*) lightPosition);
    }
    else
    {
        glUniform1i(gLKeyPressedUniform, 0);
    }
    
    // set all matrices to identity
    vmath::mat4 modelMatrix = vmath::mat4::identity();
    vmath::mat4 modelViewMatrix = vmath::mat4::identity();
    vmath::mat4 rotationMatrix = vmath::mat4::identity();
    
    // translate z axis
    modelMatrix = vmath::translate(0.0f, 0.0f, -4.0f);
    
    // all axes rotation by gAngle angle
    rotationMatrix = vmath::rotate(cubeRotation, cubeRotation, cubeRotation);
    
    modelViewMatrix = modelMatrix * rotationMatrix; // order is important
    
    // pass the modelViewMatrix to vertex shader variable
    glUniformMatrix4fv(gModelViewMatrixUniform, 1, GL_FALSE, modelViewMatrix);
    
    // pass the projectionlViewMatrix to vertex shader variable
    glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
    
    // bind VAO
    glBindVertexArray(gVaoSquare);
    
    // draw 6 faces for cube
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 20, 4);
    
    // unbind VAO
    glBindVertexArray(0);
    
    glUseProgram(0);
    
    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
    
    [eaglContext presentRenderbuffer: GL_RENDERBUFFER];
    
    if(gbAnimate)
    {
        cubeRotation = cubeRotation >= 360.0f ? 0 : cubeRotation + 1.0f;
    }
}

- (void) layoutSubviews
{
    // code
    GLint width;
    GLint height;
    
    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
    [eaglContext renderbufferStorage: GL_RENDERBUFFER fromDrawable: (CAEAGLLayer*)self.layer];
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
    
    glGenRenderbuffers(1,&depthRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,
                              depthRenderbuffer);
    
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
    
    perspectiveProjectionMatrix = vmath::perspective(45.0f, (GLfloat) width/ (GLfloat) height, 0.1f, 100.0f);
    
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        NSLog(@"Failed To Create Complete Framebuffer Object %x",  glCheckFramebufferStatus(GL_FRAMEBUFFER));
    }
    
    [self drawView:nil];
}

- (void) startAnimation
{
    if(!isAnimating)
    {
        displayLink = [NSClassFromString(@"CADisplayLink") displayLinkWithTarget: self selector: @selector(drawView:)];
        [displayLink setPreferredFramesPerSecond: animationFrameInterval];
        [displayLink addToRunLoop: [NSRunLoop currentRunLoop] forMode: NSDefaultRunLoopMode];
        
        isAnimating = YES;
    }
}

- (void) stopAnimation
{
    if(isAnimating)
    {
        [displayLink invalidate];
        displayLink = nil;
        
        isAnimating = NO;
    }
}

-(BOOL) acceptsFirstResponder
{
    return YES;
}

- (void) touchesBegan: (NSSet *)touches withEvent: (UIEvent *)event
{
    
 
}

- (void) onSingleTap: (UITapGestureRecognizer *)gr
{
    NSLog(@"Single tap");
    gbLight = !gbLight;
}

- (void) onDoubleTap: (UITapGestureRecognizer*)gr
{
    NSLog(@"Double tap");
    gbAnimate = !gbAnimate;
}

- (void) onSwipe: (UITapGestureRecognizer*)gr
{
    [self release];
    exit(0);
}

- (void) onLongPress: (UILongPressGestureRecognizer*)gr
{
    
}

- (void) dealloc
{
    if (gVaoSquare)
    {
        glDeleteVertexArrays(1, &gVaoSquare);
        gVaoSquare = 0;
    }
    
    if (gVboSquarePosition)
    {
        glDeleteBuffers(1, &gVboSquarePosition);
        gVboSquarePosition = 0;
    }
    
    if (gVboSquareNormal)
    {
        glDeleteBuffers(1, &gVboSquareNormal);
        gVboSquareNormal = 0;
    }
    
    // detach shader objects
    glDetachShader(shaderProgramObject, vertexShaderObject);
    glDetachShader(shaderProgramObject, fragmentShaderObject);
    
    // delete
    glDeleteShader(vertexShaderObject);
    vertexShaderObject = 0;
    glDeleteShader(fragmentShaderObject);
    fragmentShaderObject = 0;
    
    // delete sahder program objects
    glDeleteProgram(shaderProgramObject);
    shaderProgramObject = 0;
    
    if(depthRenderbuffer)
    {
        glDeleteRenderbuffers(1, &depthRenderbuffer);
        depthRenderbuffer = 0;
    }
    
    if(colorRenderbuffer)
    {
        glDeleteRenderbuffers(1, &colorRenderbuffer);
        colorRenderbuffer = 0;
    }
    
    if(defaultFramebuffer)
    {
        glDeleteFramebuffers(1,&defaultFramebuffer);
        defaultFramebuffer = 0;
    }
    
    if([EAGLContext currentContext]==eaglContext)
    {
        [EAGLContext setCurrentContext: nil];
    }
    [eaglContext release];
    eaglContext=nil;
    
    [super dealloc];
}

- (void) exitIfError : (GLuint) glObject : (NSString*) message
{
    GLint iShaderCompileStatus = 0;
    const char *msg = [message cStringUsingEncoding: NSASCIIStringEncoding];
    glGetShaderiv(glObject, GL_COMPILE_STATUS, &iShaderCompileStatus);
    if(iShaderCompileStatus == GL_FALSE)
    {
        GLint iInfoLength = 0;
        glGetShaderiv(glObject, GL_INFO_LOG_LENGTH, &iInfoLength);
        if (iInfoLength > 0)
        {
            char *szInfoLog = NULL;
            GLsizei written;
            
            szInfoLog = (char*) malloc(iInfoLength);
            if(szInfoLog == NULL)
            {
                NSLog(@"%s log : malloc failed", msg);
                [self release];
                exit(0);
            }
            
            glGetShaderInfoLog(glObject, iInfoLength, &written, szInfoLog);
            NSLog(@"%s log : %s \n", msg, szInfoLog);
            free(szInfoLog);
            [self release];
            exit(0);
        }
    }
}
@end
