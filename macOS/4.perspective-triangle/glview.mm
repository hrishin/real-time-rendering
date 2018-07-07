#import "glview.h"

@implementation GLView
{
@private
    CVDisplayLinkRef displayLink;

	GLuint vertexShaderObject;
	GLuint fragmentShaderObject;
	GLuint shaderProgramObject;
	
	GLuint vao;
	GLuint vbo;
	GLuint mvpUniform;
	vmath::mat4 perspectiveProjectionMatrix;
}

- (id) initWithFrame: (NSRect) frame
{
	self = [super initWithFrame: frame];

	if (self) 
	{
        [[self window] setContentView: self];
        
        NSOpenGLPixelFormatAttribute attrs[] =
        {
             // Must specify the 4.1 Core profile to use OpenGL 4.1
            NSOpenGLPFAOpenGLProfile,
            NSOpenGLProfileVersion4_1Core,
            // Specify the diplay ID to associate the GL context with (main display for now)
            NSOpenGLPFAScreenMask, CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay),
            NSOpenGLPFANoRecovery,
            NSOpenGLPFAAccelerated,
            NSOpenGLPFAColorSize, 24,
            NSOpenGLPFADepthSize, 24,
            NSOpenGLPFAAlphaSize, 8,
            NSOpenGLPFADoubleBuffer,
            0
        };
       
	    // auto release is like defer in golang once function scope exit it automatically get released
        NSOpenGLPixelFormat *pixelFormat  = [[[NSOpenGLPixelFormat alloc] initWithAttributes: attrs] autorelease];
        
        if(pixelFormat == nil)
        {
            fprintf(gpFile, "No valid OpenGL pixel format is available. Exiting\n");
            [self release];
            [NSApp terminate:self];
        }
        
        NSOpenGLContext *glContext = [[[NSOpenGLContext alloc] initWithFormat: pixelFormat shareContext: nil] autorelease];
        
        [self setPixelFormat: pixelFormat];
        
        [self setOpenGLContext: glContext];
	}

	return(self);
}

// over riding GLView provided message
- (void) prepareOpenGL
{
    fprintf(gpFile, "OpenGL Version : %s\n", glGetString(GL_VERSION));
    fprintf(gpFile, "GLSL Version : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    [[self openGLContext] makeCurrentContext];
   
   	// adjust the buffering speed according to display refresh rate
	// default value is 0
    GLint swapInt = 1;
    [[self openGLContext] setValues: &swapInt forParameter:NSOpenGLCPSwapInterval];
															// context paramter

	// VERTEX shader
	//create shader object
	vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

	// program
	const char *vertexShaderSourceCode = 
		"#version 410" \
		"\n" \
		"in vec4 vPosition;" \
		"uniform mat4 uMvpMatrix;" \
		"void main(void)" \
		"{" \
		"gl_Position = uMvpMatrix * vPosition;" \
		"}";
	glShaderSource(vertexShaderObject, 1, (const char **) &vertexShaderSourceCode, NULL);

	// compile shader
	glCompileShader(vertexShaderObject);
	// check compilation errors
	GLint iShaderCompileStatus = 0;
	glGetShaderiv(vertexShaderObject, GL_COMPILE_STATUS, &iShaderCompileStatus);
	if(iShaderCompileStatus == GL_FALSE)
	{
		GLint iInfoLength = 0;
		glGetShaderiv(vertexShaderObject, GL_INFO_LOG_LENGTH, &iInfoLength);
		if (iInfoLength > 0)
		{
			char *szInfoLog = NULL;
			GLsizei written;
			
			szInfoLog = (char*) malloc(iInfoLength);
			if(szInfoLog == NULL)
			{
				fprintf(gpFile, "Vertex shader compilation log : malloc failed");
				[self release];
         		[NSApp terminate: self];
			}

			glGetShaderInfoLog(vertexShaderObject, iInfoLength, &written, szInfoLog);
			fprintf(gpFile, "Vertex shader compilation log : %s \n", szInfoLog);
			free(szInfoLog);
			[self release];
			[NSApp terminate: self];
		}
	}
	
	// FRAGMENT shader
	//create shader object
	fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

	// program
	const char *fragmentShaderSourceCode = 
		"#version 410" \
		"\n" \
		"out vec4 FragColor;" \
		"void main(void)" \
		"{" \
		"FragColor = vec4(1.0, 1.0, 1.0, 1.0);" \
		"}";
	glShaderSource(fragmentShaderObject, 1, (const char **) &fragmentShaderSourceCode, NULL);

	// compile shader
	glCompileShader(fragmentShaderObject);
	// check compilation errors
	iShaderCompileStatus = 0;
	glGetShaderiv(fragmentShaderObject, GL_COMPILE_STATUS, &iShaderCompileStatus);
	if(iShaderCompileStatus == GL_FALSE)
	{
		GLint iInfoLength = 0;
		glGetShaderiv(fragmentShaderObject, GL_INFO_LOG_LENGTH, &iInfoLength);
		if (iInfoLength > 0)
		{
			char *szInfoLog = NULL;
			szInfoLog = (char*) malloc(iInfoLength);
			if(szInfoLog == NULL)
			{
				fprintf(gpFile, "Fragment shader compilation log : malloc failed");
				[self release];
         		[NSApp terminate: self];
			}

			GLsizei written;
			glGetShaderInfoLog(fragmentShaderObject, iInfoLength, &written, szInfoLog);
			fprintf(gpFile, "Fragment shader compilation log : %s \n", szInfoLog);
			free(szInfoLog);
			[self release];
			[NSApp terminate: self];
		}
	}

	// SHADER PROGRAM
	shaderProgramObject = glCreateProgram();

	// attach vertex shader to shader program
	glAttachShader(shaderProgramObject, vertexShaderObject);

	// attach fragment shader to shader program
	glAttachShader(shaderProgramObject, fragmentShaderObject);

	// pre-link shader program attributes
	glBindAttribLocation(shaderProgramObject, VDG_ATTRIBUTE_VERTEX, "vPosition");

	// link shader
	glLinkProgram(shaderProgramObject);

	iShaderCompileStatus = 0;
	glGetShaderiv(shaderProgramObject, GL_COMPILE_STATUS, &iShaderCompileStatus);
	if(iShaderCompileStatus == GL_FALSE)
	{
		GLint iInfoLength = 0;
		glGetShaderiv(shaderProgramObject, GL_INFO_LOG_LENGTH, &iInfoLength);
		if (iInfoLength > 0)
		{
			char *szInfoLog = NULL;
			szInfoLog = (char*) malloc(iInfoLength);
			if(szInfoLog == NULL)
			{
				fprintf(gpFile, "Progam shader compilation log : malloc failed");
				[self release];
         		[NSApp terminate: self];
			}

			GLsizei written;
			glGetShaderInfoLog(shaderProgramObject, iInfoLength, &written, szInfoLog);
			fprintf(gpFile, "Prpgram shader compilation log : %s \n", szInfoLog);
			free(szInfoLog);
			[self release];
			[NSApp terminate: self];
		}
	}
	
	mvpUniform = glGetUniformLocation(shaderProgramObject, "uMvpMatrix");

	// pass vertices, colors, normals, textures to VBO, VAO initilization
	const GLfloat traingleVertices[] = 
	{
		0.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f
	};

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glBufferData(GL_ARRAY_BUFFER, sizeof(traingleVertices), traingleVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(VDG_ATTRIBUTE_VERTEX, 3, GL_FLOAT,GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(VDG_ATTRIBUTE_VERTEX);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glBindVertexArray(0);

	glClearDepth(1.0f);
	// enable depth testing
	glEnable(GL_DEPTH_TEST);
	// depth test to do
	glDepthFunc(GL_LEQUAL);
	// always cullback faces for better performance
	glEnable(GL_CULL_FACE);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// set projection matrix to identity matrix
	perspectiveProjectionMatrix = vmath::mat4::identity();

    CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
    CVDisplayLinkSetOutputCallback(displayLink, &MyDisplayLinkCallback, self);
    CGLContextObj cglContext = (CGLContextObj) [[self openGLContext] CGLContextObj];
    CGLPixelFormatObj cglPixelFormat = (CGLPixelFormatObj) [[self pixelFormat] CGLPixelFormatObj];
    CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, cglContext, cglPixelFormat);
	CVDisplayLinkStart(displayLink);
}

- (CVReturn) getFrameForTime: (const CVTimeStamp *)pOutputTime
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
    [self drawView];

    [pool release];

    return kCVReturnSuccess;
}

- (void) drawRect: (NSRect) dirtyRect
{
	[self drawView];
}

- (void) drawView
{
    [[self openGLContext] makeCurrentContext];

    CGLLockContext((CGLContextObj) [[self openGLContext] CGLContextObj]);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// start using program object
	glUseProgram(shaderProgramObject);

	vmath::mat4 modelViewMatrix = vmath::mat4::identity();
	vmath::mat4 modelViewProjectionMatrix = vmath::mat4::identity();

	// translate z axis
	modelViewMatrix = vmath::translate(0.0f, 0.0f, -6.0f);

	// multiply the modelview and perspective matrix to get modelViewProjection
	// order is important
	modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;

	// pass above modelViewMatrixProjection matrix to the vertex shader in uMvpMatrix
	// whose position value we already calculated in initilization by using glGetUniformLocation
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);
	
	// bind vao
	glBindVertexArray(vao);

	// draw either by glDrawTraingles() or dlDrawArrays() or glDrawElements()
	glDrawArrays(GL_TRIANGLES, 0, 3);

	// unbind vao
	glBindVertexArray(0);

    CGLFlushDrawable((CGLContextObj) [[self openGLContext] CGLContextObj]);
    CGLUnlockContext((CGLContextObj) [[self openGLContext] CGLContextObj]);
}

-(void) reshape
{
    CGLLockContext((CGLContextObj) [[self openGLContext] CGLContextObj]);
    
    NSRect rect = [self bounds];
    GLfloat width = rect.size.width;
    GLfloat height = rect.size.height;

    if (height == 0) 
    {
        height = 1;
    }

    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
   	perspectiveProjectionMatrix = vmath::perspective(45.0f, (GLfloat) width/ (GLfloat) height, 0.1f, 100.0f);

    CGLUnlockContext((CGLContextObj) [[self openGLContext] CGLContextObj]);
}

- (BOOL) acceptsFirstResponder
{
	[[self window] makeFirstResponder:self];
	return (YES);
}

-(void) keyDown:(NSEvent *) theEvent
{
    int key=(int)[[theEvent characters]characterAtIndex: 0];
    switch(key)
    {
        case 27: // Esc key
            [self release];
            [NSApp terminate: self];
            break;
        
        case 'F':
        case 'f':
            [[self window]toggleFullScreen: self]; // repaint automatically
            break;
        
        default:
            break;
    }
}

-(void)mouseDown:(NSEvent *) theEvent
{
    [self setNeedsDisplay: YES]; // repaint
}

-(void)mouseDragged:(NSEvent *) theEvent
{
 
}

-(void)rightMouseDown:(NSEvent *) theEvent
{
     [self setNeedsDisplay: YES]; // repaint
}

- (void) dealloc
{
	if(vao)
	{
		glDeleteVertexArrays(1, &vao);
		vao = 0;
	}
	
	if(vbo)
	{
		glDeleteVertexArrays(1, &vbo);
		vbo = 0;
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

	CVDisplayLinkStop(displayLink);
	CVDisplayLinkRelease(displayLink);
	
	[super dealloc];
}

@end


CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp *pNow, const CVTimeStamp *pOutputTime, 
                    CVOptionFlags flagsIn, CVOptionFlags *pFlagsOut, void *pDisplayLinkContext)
{
    CVReturn result = [(GLView *) pDisplayLinkContext getFrameForTime: pOutputTime]; 
    return result;
}

