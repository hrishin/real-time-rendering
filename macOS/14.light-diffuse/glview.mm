#import "glview.h"

@interface GLView(PrivateMethods)
-(void) exitIfError : (GLuint) object : (NSString*) message;
@end

@implementation GLView
{
@private
    CVDisplayLinkRef displayLink;

	GLuint vertexShaderObject;
	GLuint fragmentShaderObject;
	GLuint shaderProgramObject;
	
	GLuint gVaoSquare;
	GLuint gVboSquarePosition;
	GLuint gVboSquareNormal;

	GLuint gModelViewMatrixUniform, gProjectionMatrixUniform;
	GLuint gLdUniform, gKdUniform, gLightPositionUniform;
	GLuint gLKeyPressedUniform;

	GLfloat cubeRotation;

	bool gbAnimate;
	bool gbLight;

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
	// check compilation errors
	[self exitIfError: vertexShaderObject : @"Vertex shader compilation"];
	
	// FRAGMENT shader
	//create shader object
	fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

	// program
	const char *fragmentShaderSourceCode = 
		"#version 410" \
		"\n" \
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

	glClearDepth(1.0f);
	// enable depth testing
	glEnable(GL_DEPTH_TEST);
	// depth test to do
	glDepthFunc(GL_LEQUAL);
	// always cullback faces for better performance
	//glEnable(GL_CULL_FACE);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	cubeRotation   = 0.0f;

	gbAnimate = false;
	gbLight = false;

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
	modelMatrix = vmath::translate(0.0f, 0.0f, -5.0f);

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
	
	if(gbAnimate)
		cubeRotation = cubeRotation >= 360.0f ? 0 : cubeRotation + 1.0f;

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
        
		case 'A':
		case 'a':
			gbAnimate = !gbAnimate;
			break;

		case 'L':
		case 'l':
			gbLight = !gbLight;
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

	CVDisplayLinkStop(displayLink);
	CVDisplayLinkRelease(displayLink);
	
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
				fprintf(gpFile, "%s log : malloc failed", msg);
				[self release];
         		[NSApp terminate: self];
			}

			glGetShaderInfoLog(glObject, iInfoLength, &written, szInfoLog);
			fprintf(gpFile, "%s log : %s \n", msg, szInfoLog);
			free(szInfoLog);
			[self release];
			[NSApp terminate: self];
		}
	}
}

@end


CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp *pNow, const CVTimeStamp *pOutputTime, 
                    CVOptionFlags flagsIn, CVOptionFlags *pFlagsOut, void *pDisplayLinkContext)
{
    CVReturn result = [(GLView *) pDisplayLinkContext getFrameForTime: pOutputTime]; 
    return result;
}

