#import "glview.h"

@interface GLView(PrivateMethods)
-(void) exitIfError : (GLuint) object : (NSString*) message;
-(void) copyArray : (GLfloat*) inputArray : (GLfloat*) values : (int) length;
@end

@implementation GLView
{
@private
    CVDisplayLinkRef displayLink;

	GLuint vertexShaderObject;
	GLuint fragmentShaderObject;
	GLuint shaderProgramObject;

	GLuint gNumElements;
	GLuint gNumVertices;
	float sphere_vertices[S_VERTICES];
	float sphere_normals[S_NORMALS];
	float sphere_textures[S_TEXTURES];
	unsigned short sphere_elements[S_ELEMENTS];

	GLuint gVaoSphere;
	GLuint gVboSpherePosition;
	GLuint gVboShereNormal;
	GLuint gVboSphereElements;

	GLuint gModelMatrixUniform, gViewMatrixUniform, gProjectionMatrixUniform;
	GLuint gLaUniform, gLdUniform, gLsUniform, gLightPositionUniform;
	GLuint gKaUniform, gKdUniform, gKsUniform, gMaterialShinessUniform;
	GLuint gLKeyPressedUniform;

	vmath::mat4 perspectiveProjectionMatrix;

	GLfloat gLightAmbient[4];
	GLfloat gLightDiffuse[4];
	GLfloat gLightSpecular[4];
	GLfloat gLightPosition[4];

	GLfloat gMaterialAmbient[4];
	GLfloat gMaterialDiffuse[4];
	GLfloat gMaterialSpecular[4];
	GLfloat gMaterialShininess;

	bool gbLight;
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
														
	[self copyArray: gLightAmbient : (GLfloat[]){0.0f, 0.0f, 0.0f, 1.0f} : 4];
	[self copyArray: gLightDiffuse : (GLfloat[]){ 1.0f, 1.0f, 1.0f, 1.0f } : 4];
	[self copyArray: gLightSpecular : (GLfloat[]){ 1.0f, 1.0f, 1.0f, 1.0f } : 4];
	[self copyArray: gLightPosition : (GLfloat[]){ 100.0f, 100.0f, 100.0f, 1.0f } : 4];

	[self copyArray: gMaterialAmbient : (GLfloat[]){ 0.0f, 0.0f, 0.0f, 1.0f } : 4];
	[self copyArray: gMaterialDiffuse : (GLfloat[]){ 1.0f, 1.0f, 1.0f, 1.0f } : 4];
	[self copyArray: gMaterialSpecular : (GLfloat[]){ 1.0f, 1.0f, 1.0f, 1.0f } : 4];
	[self copyArray: gLightPosition : (GLfloat[]){ 100.0f, 100.0f, 100.0f, 1.0f } : 4];

	gMaterialShininess = 50.0f;
	
	// VERTEX shader
	//create shader object
	vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

	// program
	const char *vertexShaderSourceCode = 
		"#version 410" \
		"\n" \
		"in vec4 vPosition;" \
		"in vec3 vNormal;" \
		"uniform mat4 u_model_matrix;" \
		"uniform mat4 u_view_matrix;" \
		"uniform mat4 u_projection_matrix;" \
		"uniform int u_lighting_enabled;" \
		"uniform vec4 u_light_position;" \
		"out vec3 trasnformed_normals;" \
		"out vec3 light_direction ;" \
		"out vec3 viewer_vector;" \
		"void main(void)" \
		"{" \
		    "if (u_lighting_enabled == 1)" \
		    "{" \
		        "vec4 eye_coordinates = u_view_matrix * u_model_matrix * vPosition;"\
		        "trasnformed_normals = mat3(u_view_matrix * u_model_matrix) * vNormal;" \
		        "light_direction = vec3(u_light_position) - eye_coordinates.xyz;" \
		        "viewer_vector = -eye_coordinates.xyz;" \
		    "}" \
		    "gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vPosition;" \
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
		"in vec3 trasnformed_normals;" \
		"in vec3 light_direction;" \
		"vec3 viewer_vector;" \
		"out vec4 FragColor;" \
		"uniform int u_lighting_enabled;" \
		"uniform vec3 u_La;" \
		"uniform vec3 u_Ld;" \
		"uniform vec3 u_Ls;" \
		"uniform vec3 u_Ka;" \
		"uniform vec3 u_Kd;" \
		"uniform vec3 u_Ks;" \
		"uniform float u_material_shininess;" \
		"void main(void)" \
		"{" \
		    "vec3 phong_ads_color;" \
		    "if (u_lighting_enabled == 1)" \
		    "{" \
		        "vec3 normalized_trasnformed_normals = normalize(trasnformed_normals);" \
		        "vec3 normalized_light_direction = normalize(light_direction);" \
		        "float tn_dot_ld = max(dot(normalized_trasnformed_normals, normalized_light_direction), 0.0);" \
		        "vec3 reflection_vector = reflect(-normalized_light_direction, normalized_trasnformed_normals);" \
		        "vec3 normalized_viewer_vector = normalize(viewer_vector);" \
		        "vec3 specular = u_Ls * u_Ks * pow(max(dot(reflection_vector, normalized_viewer_vector) , 0.0) , u_material_shininess);" \
		        "vec3 ambient = u_La * u_Ka;" \
		        "vec3 diffuse = u_Ld * u_Kd * tn_dot_ld;" \
		        "phong_ads_color = ambient + diffuse + specular;" \
		    "}" \
		    "else" \
		    "{" \
		        "phong_ads_color = vec3(1.0, 1.0, 1.0);" \
		    "}" \
		    "FragColor = vec4(phong_ads_color, 1.0);" \
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
	// check comoilation error
	[self exitIfError: shaderProgramObject : @"Shader program object compilation"];
	
	// get MVP uniform location
	gModelMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_model_matrix");
	gViewMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_view_matrix");
	gProjectionMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_projection_matrix");

	gLKeyPressedUniform = glGetUniformLocation(shaderProgramObject, "u_lighting_enabled");

	// ambient color of light
	gLaUniform = glGetUniformLocation(shaderProgramObject, "u_La");
	// diffuse color of light
	gLdUniform = glGetUniformLocation(shaderProgramObject, "u_Ld");
	// specular color of light
	gLsUniform = glGetUniformLocation(shaderProgramObject, "u_Ls");
	// light position
	gLightPositionUniform = glGetUniformLocation(shaderProgramObject, "u_light_position");

	// ambient color of material
	gKaUniform = glGetUniformLocation(shaderProgramObject, "u_Ka");
	// diffuse color of material
	gKdUniform = glGetUniformLocation(shaderProgramObject, "u_Kd");
	// specular color of light
	gKsUniform = glGetUniformLocation(shaderProgramObject, "u_Ks");
	// shininess of material
	gMaterialShinessUniform = glGetUniformLocation(shaderProgramObject, "u_material_shininess");

    // get sphere vertices, normals, textures, elements
	getSphereVertexData(sphere_vertices, sphere_normals, sphere_textures, sphere_elements);
	gNumVertices = getNumberOfSphereVertices();
	gNumElements = getNumberOfSphereElements();

	// sphere VAO
	glGenVertexArrays(1, &gVaoSphere);
	glBindVertexArray(gVaoSphere);

    // Position
	glGenBuffers(1, &gVboSpherePosition);
	glBindBuffer(GL_ARRAY_BUFFER, gVboSpherePosition);
	// move data from main memory to graphics memory
	// GL_STATIC_DRAW or GL_DYNAMIC_DRAW : How you want load data run or preloading. example game shows loadingbar and "loading" messages
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_vertices), sphere_vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(VDG_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(VDG_ATTRIBUTE_VERTEX);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Normals
	glGenBuffers(1, &gVboShereNormal);
	glBindBuffer(GL_ARRAY_BUFFER, gVboShereNormal);
	// move data from main memory to graphics memory
	// GL_STATIC_DRAW or GL_DYNAMIC_DRAW : How you want load data run or preloading. example game shows loadingbar and "loading" messages
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_normals), sphere_normals, GL_STATIC_DRAW);
	glVertexAttribPointer(VDG_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(VDG_ATTRIBUTE_NORMAL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Elements
	glGenBuffers(1, &gVboSphereElements);
	glBindBuffer(GL_ARRAY_BUFFER, gVboSphereElements);
	// move data from main memory to graphics memory
	// GL_STATIC_DRAW or GL_DYNAMIC_DRAW : How you want load data run or preloading. example game shows loadingbar and "loading" messages
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_elements), sphere_elements, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	gbLight = false;

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

    glUseProgram(shaderProgramObject);

    if (gbLight)
	{
		// setting light enabled uniform
		glUniform1i(gLKeyPressedUniform, 1);

		// setting light properties uniform
		glUniform3fv(gLaUniform, 1, gLightAmbient);
		glUniform3fv(gLdUniform, 1, gLightDiffuse);
		glUniform3fv(gLsUniform, 1, gLightSpecular);
		glUniform4fv(gLightPositionUniform, 1, gLightPosition);

		glUniform3fv(gKaUniform, 1, gMaterialAmbient);
		glUniform3fv(gKdUniform, 1, gMaterialDiffuse);
		glUniform3fv(gKsUniform, 1, gMaterialSpecular);
		glUniform1f(gMaterialShinessUniform, gMaterialShininess);
	}
	else 
	{
		glUniform1i(gLKeyPressedUniform, 0);
	}

	// set all matrices to identity
	vmath::mat4 modelMatrix = vmath::mat4::identity();
	vmath::mat4 viewMatrix = vmath::mat4::identity();

	// translate z axis
	modelMatrix = vmath::translate(0.0f, 0.0f, -2.0f);

	// pass the modelMatrix to vertex shader variable
	glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);

	// pass the viewMatrix to vertex shader variable
	glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);

	// pass the projectionlViewMatrix to vertex shader variable
	glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

	// bind VAO
	glBindVertexArray(gVaoSphere);

	// draw sphere
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVboSphereElements);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

	// unbind VAO
	glBindVertexArray(0);

    glUseProgram(0);

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

		case 'L':
        case 'l':
           gbLight = !gbLight;
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
	// destroy vao
	if (gVaoSphere)
	{
		glDeleteVertexArrays(1, &gVaoSphere);
		gVaoSphere = 0;
	}

	// destroy vbo
	if (gVboSpherePosition)
	{
		glDeleteBuffers(1, &gVboSpherePosition);
		gVboSpherePosition = 0;
	}
	if (gVboShereNormal)
	{
		glDeleteBuffers(1, &gVboShereNormal);
		gVboShereNormal = 0;
	}
	if (gVboSphereElements)
	{
		glDeleteBuffers(1, &gVboSphereElements);
		gVboSphereElements = 0;
	}

	// detach shader objects
	glDetachShader(shaderProgramObject, vertexShaderObject);
	glDetachShader(shaderProgramObject, fragmentShaderObject);

	// delete sahder objects
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

- (void) copyArray : (GLfloat*) inputArray : (GLfloat*) values : (int) length
{
	int i;

	for(i=0; i < length; i++) 
	{
		inputArray[i] = values [i];
	}
}

@end


CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp *pNow, const CVTimeStamp *pOutputTime, 
                    CVOptionFlags flagsIn, CVOptionFlags *pFlagsOut, void *pDisplayLinkContext)
{
    CVReturn result = [(GLView *) pDisplayLinkContext getFrameForTime: pOutputTime]; 
    return result;
}

