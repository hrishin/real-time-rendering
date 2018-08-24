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
    
    GLuint vaoSquare;
    GLuint vboPosition;
    GLuint vboTexcoords;
    GLuint mvpUniform;
    GLuint texSamplerUniform;
    vmath::mat4 perspectiveProjectionMatrix;
    
    GLuint checkerBoardtexture;
    
    
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
        "in vec2 vTexCoord;" \
        "uniform mat4 uMvpMatrix;" \
        "out vec2 outTextCoord;" \
        "void main(void)" \
        "{" \
            "gl_Position = uMvpMatrix * vPosition;" \
            "outTextCoord = vTexCoord;" \
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
        "in vec2 outTextCoord;" \
        "uniform sampler2D uTextureSampler;" \
        "out vec4 FragColor;" \
        "void main(void)" \
        "{" \
            "vec3 tex  = vec3(texture(uTextureSampler, outTextCoord));" \
            "FragColor = vec4(tex, 1.0f);" \
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
        glBindAttribLocation(shaderProgramObject, VDG_ATTRIBUTE_TEXTURE0, "vTexCoord");
        
        // link shader
        glLinkProgram(shaderProgramObject);
        
        // check linking errors
        [self exitIfError: shaderProgramObject : @"Shader program object compilation"];
        
        mvpUniform = glGetUniformLocation(shaderProgramObject, "uMvpMatrix");
        texSamplerUniform = glGetUniformLocation(shaderProgramObject, "uTextureSampler");
        
        // pass vertices, colors, normals, textures to VBO, VAO initilization
        const GLfloat squareTexCords[] =
        {
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,
        };
        
        glGenVertexArrays(1, &vaoSquare);
        glBindVertexArray(vaoSquare);
        
        // poisition
        glGenBuffers(1, &vboPosition);
        glBindBuffer(GL_ARRAY_BUFFER, vboPosition);
        glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(VDG_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(VDG_ATTRIBUTE_VERTEX);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // texture
        glGenBuffers(1, &vboTexcoords);
        glBindBuffer(GL_ARRAY_BUFFER, vboTexcoords);
        glBufferData(GL_ARRAY_BUFFER, sizeof(squareTexCords), squareTexCords, GL_STATIC_DRAW);
        glVertexAttribPointer(VDG_ATTRIBUTE_TEXTURE0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(VDG_ATTRIBUTE_TEXTURE0);
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
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        
        // set projection matrix to identity matrix
        perspectiveProjectionMatrix = vmath::mat4::identity();
        
        // other property intialization
        animationFrameInterval = 60;
        isAnimating = NO;
        
        // load textures
        checkerBoardtexture  = [self loadTextureFromProcedure];
        
        
        // gesture handling
        // tap handling
        UITapGestureRecognizer *singleTap = [[UITapGestureRecognizer alloc] initWithTarget: self action: @selector(onSingleTap:)];
        [singleTap setNumberOfTapsRequired: 1];
        [singleTap setNumberOfTouchesRequired: 1]; // 1 finger
        [singleTap setDelegate: self];
        [self addGestureRecognizer: singleTap];
        
        UITapGestureRecognizer *doubleTap = [[UITapGestureRecognizer alloc] initWithTarget: self action: @selector(onDoubleTap:)];
        [singleTap setNumberOfTapsRequired: 1];
        [singleTap setNumberOfTouchesRequired: 1]; // 1 finger
        [singleTap setDelegate: self];
        [self addGestureRecognizer: doubleTap];
        
        //  let single ignore it if its double tapped
        [singleTap requireGestureRecognizerToFail:doubleTap];
        
        // swipe gesture
        UISwipeGestureRecognizer *swipe = [[UISwipeGestureRecognizer alloc] initWithTarget: self action: @selector(onSwipe:)];
        [self addGestureRecognizer: swipe];
        
        // long press
        UILongPressGestureRecognizer *longPressGestureRecognizer = [[UILongPressGestureRecognizer alloc]initWithTarget: self action: @selector(onLongPress:)];
        [self addGestureRecognizer: longPressGestureRecognizer];
    }
    
    return self;
}

- (GLuint) loadTextureFromProcedure
{
    GLuint texture;
    GLubyte *checkImage;
    int checkImageHeight, checkImageWidth;
    
    checkImageHeight = 64;
    checkImageWidth  = 64;
    checkImage= [self makeCheckImage: checkImageWidth : checkImageHeight];
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    glGenTextures(1, &texture);
    
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, checkImageWidth, checkImageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, checkImage);
    
    free(checkImage);
    
    return (texture);
}

-(GLubyte*) makeCheckImage: (int)checkImageWidth :(int) checkImageHeight
{
    int i, j, c, heightIndex, widthIndex, postion;
    GLubyte *checkImage = (GLubyte*) malloc(checkImageHeight * checkImageWidth * 4);
    
    for (i = 0; i < checkImageHeight; i++)
    {
        heightIndex = i * checkImageWidth * 4;
        
        for (j = 0; j < checkImageWidth; j++)
        {
            widthIndex = j * 4;
            postion = heightIndex + widthIndex;
            
            c = (((i & 0X8) == 0) ^ ((j & 0X8) == 0)) * 255;
            
            checkImage[postion + 0] = (GLubyte)c;
            checkImage[postion + 1] = (GLubyte)c;
            checkImage[postion + 2] = (GLubyte)c;
            checkImage[postion + 3] = (GLubyte)255;
        }
    }
    
    return (checkImage);
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
    
    [self drawLeft];
    
    [self drawRight];
    
    glUseProgram(0);
    
    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
    
    [eaglContext presentRenderbuffer: GL_RENDERBUFFER];
}

- (void) drawLeft
{
    vmath::mat4 modelViewMatrix = vmath::mat4::identity();
    vmath::mat4 modelViewProjectionMatrix = vmath::mat4::identity();
    
    // translate z axis
    modelViewMatrix = vmath::translate(-1.5f, 0.0f, -6.0f);
    
    // multiply the modelview and perspective matrix to get modelViewProjection
    // order is important
    modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;
    
    // pass above modelViewMatrixProjection matrix to the vertex shader in uMvpMatrix
    // whose position value we already calculated in initilization by using glGetUniformLocation
    glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);
    
    // Square 1 position
    GLfloat square1Vertices[] =
    {
        1.0f, 1.0f, 1.0f, // front
        -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
    };
    
    // load texture co-ords dynamically
    glBindBuffer(GL_ARRAY_BUFFER, vboPosition);
    glBufferData(GL_ARRAY_BUFFER, sizeof(square1Vertices), square1Vertices, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glBindTexture(GL_TEXTURE_2D, checkerBoardtexture);
    
    // bind vaoSquare
    glBindVertexArray(vaoSquare);
    
    // draw either by glDrawTraingles() or dlDrawArrays() or glDrawElements()
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    
    // unbind vaoSquare
    glBindVertexArray(0);
}

- (void) drawRight
{
    vmath::mat4 modelViewMatrix = vmath::mat4::identity();
    vmath::mat4 modelViewProjectionMatrix = vmath::mat4::identity();
    
    // translate z axis
    modelViewMatrix = vmath::translate(1.5f, 0.0f, -6.0f);
    
    // multiply the modelview and perspective matrix to get modelViewProjection
    // order is important
    modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;
    
    // pass above modelViewMatrixProjection matrix to the vertex shader in uMvpMatrix
    // whose position value we already calculated in initilization by using glGetUniformLocation
    glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);
    
    // Square 2 position
    GLfloat square2Vertices[] =
    {
        2.41421f, 1.0f, -1.41421f, // front
        1.0f, 1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        2.41421f, -1.0f, -1.41421f,
    };
    
    // load texture co-ords dynamically
    glBindBuffer(GL_ARRAY_BUFFER, vboPosition);
    glBufferData(GL_ARRAY_BUFFER, sizeof(square2Vertices), square2Vertices, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glBindTexture(GL_TEXTURE_2D, checkerBoardtexture);
    
    // bind vaoSquare
    glBindVertexArray(vaoSquare);
    
    // draw either by glDrawTraingles() or dlDrawArrays() or glDrawElements()
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    
    // unbind vaoSquare
    glBindVertexArray(0);
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
   
}

- (void) onDoubleTap: (UITapGestureRecognizer*)gr
{
    
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
    if(vaoSquare)
    {
        glDeleteVertexArrays(1, &vaoSquare);
        vaoSquare = 0;
    }
    
    if(vboPosition)
    {
        glDeleteVertexArrays(1, &vboPosition);
        vboPosition = 0;
    }
    
    if(vboTexcoords)
    {
        glDeleteVertexArrays(1, &vboTexcoords);
        vboTexcoords = 0;
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
