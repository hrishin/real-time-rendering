#import "glview.h"

@interface GLView(PrivateMethods)
-(void) exitIfError : (GLuint) object : (NSString*) message;
-(void) makeCheckImage: (GLubyte*) checkImage : (int)checkImageWidth : (int) checkImageHeight;
@end

@implementation GLView
{
@private
    CVDisplayLinkRef displayLink;

	GLuint vertexShaderObject;
	GLuint fragmentShaderObject;
	GLuint shaderProgramObject;
	
	GLuint vaoSquare;
	GLuint vboPosition;
	GLuint vboTexcoords;
	GLuint mvpUniform;
	GLuint texSamplerUniform;

	GLuint checkerBoardTexture;

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
	// check compilation errors
	[self exitIfError: vertexShaderObject : @"Vertex shader compilation"];
	
	// FRAGMENT shader
	//create shader object
	fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

	// program
	const char *fragmentShaderSourceCode = 
		"#version 410" \
		"\n" \
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
	glBindAttribLocation(shaderProgramObject, VDG_ATTRIBUTE_TEXTURE, "vTexCoord");

	// link shader
	glLinkProgram(shaderProgramObject);

	[self exitIfError: shaderProgramObject : @"Shader program object compilation"];
	
	mvpUniform        = glGetUniformLocation(shaderProgramObject, "uMvpMatrix");
	texSamplerUniform = glGetUniformLocation(shaderProgramObject, "uTextureSampler");

	// pass vertices, colors, normals, textures to VBO, VAO initilization
	const GLfloat squareTextureCords[] = 
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
	glBufferData(GL_ARRAY_BUFFER, 4 * 3 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(VDG_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(VDG_ATTRIBUTE_VERTEX);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	// texture
	glGenBuffers(1, &vboTexcoords);
	glBindBuffer(GL_ARRAY_BUFFER, vboTexcoords);
	glBufferData(GL_ARRAY_BUFFER, sizeof(squareTextureCords), NULL, GL_STATIC_DRAW);
	glVertexAttribPointer(VDG_ATTRIBUTE_TEXTURE, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(VDG_ATTRIBUTE_TEXTURE);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	glClearDepth(1.0f);
	// enable depth testing
	glEnable(GL_DEPTH_TEST);
	// depth test to do
	glDepthFunc(GL_LEQUAL);
	// always cullback faces for better performance
	glEnable(GL_CULL_FACE);

    glClearColor(0.0f, 0.0f, 1.0f, 0.0f);

	// set projection matrix to identity matrix
	perspectiveProjectionMatrix = vmath::mat4::identity();

	// load textures
	checkerBoardTexture  = [self loadTextureFromProcedure];
	fprintf(gpFile, "texture in value %d \n", checkerBoardTexture);
    
	CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
    CVDisplayLinkSetOutputCallback(displayLink, &MyDisplayLinkCallback, self);
    CGLContextObj cglContext = (CGLContextObj) [[self openGLContext] CGLContextObj];
    CGLPixelFormatObj cglPixelFormat = (CGLPixelFormatObj) [[self pixelFormat] CGLPixelFormatObj];
    CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, cglContext, cglPixelFormat);
	CVDisplayLinkStart(displayLink);
}

- (GLuint) loadTextureFromBMP : (const char *) fileName
{
	NSBundle *mainBundle = [NSBundle mainBundle];
	NSString *appDirName = [mainBundle bundlePath];
	NSString *parentDirPath = [appDirName stringByDeletingLastPathComponent];
	NSString *textureFileNameWithPath = [NSString stringWithFormat: @"%@/%s", parentDirPath, fileName];

	NSImage *bmpImage = [[NSImage alloc] initWithContentsOfFile: textureFileNameWithPath];
	if (!bmpImage)
	{
		NSLog(@"can't find %@", textureFileNameWithPath);
		return(0);
	}

	CGImageRef cgImage = [bmpImage CGImageForProposedRect:nil context:nil hints:nil];

	int w = CGImageGetWidth(cgImage);
	int h = CGImageGetHeight(cgImage);
	CFDataRef imageData = CGDataProviderCopyData(CGImageGetDataProvider(cgImage));
	void* pixels = (void*) CFDataGetBytePtr(imageData);

	GLuint bmpTexture;
	glGenTextures(1, &bmpTexture);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glBindTexture(GL_TEXTURE_2D, bmpTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	glGenerateMipmap(GL_TEXTURE_2D);

	CFRelease(imageData);

	return (bmpTexture);
}

- (GLuint) loadTextureFromProcedure
{
	GLuint texture;
    GLubyte *checkImage;
    int checkImageHeight, checkImageWidth;

	checkImageHeight = 64;
	checkImageWidth  = 64;
	checkImage = (GLubyte*) malloc(checkImageHeight * checkImageWidth * 4);
    
	[self makeCheckImage: checkImage : checkImageWidth : checkImageHeight];
	for(int i=0; i < 16384; i++)
	{
		fprintf(gpFile,"%d %d \n", i, checkImage[i]);
	}

    glGenTextures(1, &texture);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, checkImageWidth, checkImageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, checkImage);

	free(checkImage);
	
	fprintf(gpFile, "teture in function %d \n", texture);
	return (texture);
}

-(void) makeCheckImage: (GLubyte*) checkImage : (int)checkImageWidth : (int) checkImageHeight
{
	int i, j, c, heightIndex, widthIndex, position;

	for (i = 0; i < checkImageHeight; i++)
    {
        heightIndex = i * checkImageWidth * 4;

        for (j = 0; j < checkImageWidth; j++)
        {
            widthIndex = j * 4;
            position = heightIndex + widthIndex;
			
			c = (((i & 0X8) == 0) ^ ((j & 0X8) == 0)) * 255;
           	fprintf(gpFile, " position : %d, value : %d\n", position, c);
			checkImage[position + 0] = (GLubyte)c;
            checkImage[position + 1] = (GLubyte)c;
            checkImage[position + 2] = (GLubyte)c;
            checkImage[position + 3] = (GLubyte)255;
        }
    }
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
	GLfloat squareTextureCords[12];

    [[self openGLContext] makeCurrentContext];

    CGLLockContext((CGLContextObj) [[self openGLContext] CGLContextObj]);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	[self drawLeft];

	// start using program object
	glUseProgram(shaderProgramObject);

    CGLFlushDrawable((CGLContextObj) [[self openGLContext] CGLContextObj]);
    CGLUnlockContext((CGLContextObj) [[self openGLContext] CGLContextObj]);
}

- (void) drawLeft
{
	vmath::mat4 modelViewMatrix = vmath::mat4::identity();
	vmath::mat4 modelViewProjectionMatrix = vmath::mat4::identity();

	// translate z axis
	modelViewMatrix = vmath::translate(0.0f, 0.0f, -4.0f);

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

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, checkerBoardTexture);
	glUniform1i(texSamplerUniform, 0);

	// bind vaoSquare
	glBindVertexArray(vaoSquare);

	// draw either by glDrawTraingles() or dlDrawArrays() or glDrawElements()
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	// unbind vaoSquare
	glBindVertexArray(0);
}

- (void) drawRight
{
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

	if(checkerBoardTexture)
	{
		glDeleteTextures(1, &checkerBoardTexture);
		checkerBoardTexture = 0;
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

