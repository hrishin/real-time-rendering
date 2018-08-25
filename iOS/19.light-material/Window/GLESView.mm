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
#import "sphere.h"

enum
{
    VDG_ATTRIBUTE_VERTEX = 0,
    VDG_ATTRIBUTE_COLOR,
    VDG_ATTRIBUTE_NORMAL,
    VDG_ATTRIBUTE_TEXTURE0,
};

@interface GLESView(PrivateMethods)
    -(void) exitIfError : (GLuint) glObject : (NSString*) message;
    -(void) copyArray : (GLfloat*) inputArray : (GLfloat*) values : (int) length;
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
    
    GLfloat gLightAmbience[4];
    GLfloat gLightDiffuse[4];
    GLfloat gLightSpecular[4];
    GLfloat light1_x_position[4];
    GLfloat light1_y_position[4];
    GLfloat light1_z_position[4];
    
    GLfloat material_ambience[24][4];
    GLfloat material_diffuse[24][4];
    GLfloat material_specular[24][4];
    GLfloat material_shiness[24][2];
    
    bool gbLight;
    int gLightRotationAxis;
    
    GLfloat angleXLight;
    GLfloat angleYLight;
    GLfloat angleZLight;
    
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
        "precision highp float;" \
        "in vec4 vPosition;" \
        "in vec3 vNormal;" \
        "uniform mat4 u_model_matrix;" \
        "uniform mat4 u_view_matrix;" \
        "uniform mat4 u_projection_matrix;" \
        "uniform float u_lighting_enabled;" \
        "uniform vec4 u_light_position;" \
        "out vec3 trasnformed_normals;" \
        "out vec3 light_direction ;" \
        "out vec3 viewer_vector;" \
        "void main(void)" \
        "{" \
        "if (u_lighting_enabled >= 1.0)" \
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
        "in vec3 trasnformed_normals;" \
        "in vec3 light_direction;" \
        "in vec3 viewer_vector;" \
        "out vec4 FragColor;" \
        "uniform float u_lighting_enabled;" \
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
        "if (u_lighting_enabled >= 1.0)" \
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
        
        // Sphere VAO
        glGenVertexArrays(1, &gVaoSphere);
        glBindVertexArray(gVaoSphere);
        
        // position
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
        
        [self copyArray: gLightAmbience : (GLfloat[]){0.0f, 0.0f, 0.0f, 1.0f} : 4];
        [self copyArray: gLightDiffuse : (GLfloat[]){ 1.0f, 1.0f, 1.0f, 1.0f } : 4];
        [self copyArray: gLightSpecular : (GLfloat[]){ 1.0f, 0.0f, 0.0f, 1.0f } : 4];
        [self copyArray: light1_x_position : (GLfloat[]){ 0.0f, 0.0f, 0.0f, 0.0f } : 4];
        [self copyArray: light1_y_position : (GLfloat[]){ 0.0f, 0.0f, 0.0f, 0.0f } : 4];
        [self copyArray: light1_z_position : (GLfloat[]){ 0.0f, 0.0f, 0.0f, 0.0f } : 4];
        
        gbLight = false;
        gLightRotationAxis = 0;
        
        angleXLight = 0.0f;
        angleYLight = 0.0f;
        angleZLight = 0.0f;
        
        [self parseMaterialData];
        
        // gesture handling
        // tap handling
        UITapGestureRecognizer *singleTap = [[UITapGestureRecognizer alloc] initWithTarget: self action: @selector(onSingleTap:)];
        [singleTap setNumberOfTapsRequired: 1];
        [singleTap setNumberOfTouchesRequired: 1]; // 1 finger
        [singleTap setDelegate: self];
        [self addGestureRecognizer: singleTap];
        
        UITapGestureRecognizer *doubleTap = [[UITapGestureRecognizer alloc] initWithTarget: self action: @selector(onDoubleTap:)];
        [doubleTap setNumberOfTapsRequired: 2];
        [doubleTap setNumberOfTouchesRequired: 1]; // 1 finger
        [doubleTap setDelegate: self];
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

- (int) parseMaterialData
{
    FILE *file = NULL;
    int objects, i;
    NSString *textureFileWithPath = [[NSBundle mainBundle] pathForResource: @"material" ofType: @"dat"];
    
    const char *pfileNameWithPath =[textureFileWithPath cStringUsingEncoding: NSASCIIStringEncoding];
    
    objects = 24;
    file = fopen(pfileNameWithPath, "r");
    if (file == NULL)
    {
        fprintf(stderr, "\nError opening file\n");
        exit(EXIT_FAILURE);
    }
    
    for (i = 0; i < objects; i++)
    {
        if (!feof(file))
        {
            fscanf(file, "%*[^\n]\n");
            fscanf(file, "%*[^\n]\n");
            fscanf(file, "%f\n", &material_ambience[i][0]);
            fscanf(file, "%*[^\n]\n");
            fscanf(file, "%f\n", &material_ambience[i][1]);
            fscanf(file, "%*[^\n]\n");
            fscanf(file, "%f\n", &material_ambience[i][2]);
            fscanf(file, "%*[^\n]\n");
            fscanf(file, "%f\n", &material_ambience[i][3]);
            
            fscanf(file, "%*[^\n]\n");
            fscanf(file, "%*[^\n]\n");
            fscanf(file, "%f\n", &material_diffuse[i][0]);
            fscanf(file, "%*[^\n]\n");
            fscanf(file, "%f\n", &material_diffuse[i][1]);
            fscanf(file, "%*[^\n]\n");
            fscanf(file, "%f\n", &material_diffuse[i][2]);
            fscanf(file, "%*[^\n]\n");
            fscanf(file, "%f\n", &material_diffuse[i][3]);
            
            fscanf(file, "%*[^\n]\n");
            fscanf(file, "%*[^\n]\n");
            fscanf(file, "%f\n", &material_specular[i][0]);
            fscanf(file, "%*[^\n]\n");
            fscanf(file, "%f\n", &material_specular[i][1]);
            fscanf(file, "%*[^\n]\n");
            fscanf(file, "%f\n", &material_specular[i][2]);
            fscanf(file, "%*[^\n]\n");
            fscanf(file, "%f\n", &material_specular[i][3]);
            
            fscanf(file, "%*[^\n]\n");
            fscanf(file, "%*[^\n]\n");
            fscanf(file, "%f * %f\n", &material_shiness[i][0], &material_shiness[i][1]);
            
            fscanf(file, "%*[^\n]\n");
        }
    }
    
    fclose(file);
    
    return 0;
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
    int index;
    int row, column;
    float xPos, yPos;
    
    [EAGLContext setCurrentContext: eaglContext];
    
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    
    glUseProgram(shaderProgramObject);
    
    if (gbLight)
    {
        // setting light enabled uniform
        glUniform1f(gLKeyPressedUniform, 2.0);
        
        // setting light properties uniform
        glUniform3fv(gLaUniform, 1, gLightAmbience);
        glUniform3fv(gLdUniform, 1, gLightDiffuse);
        glUniform3fv(gLsUniform, 1, gLightSpecular);
        
        if (gLightRotationAxis == 1)
        {
            light1_x_position[0] = sin(angleXLight) * 20.0f;
            light1_x_position[1] = 0.0f;
            light1_x_position[2] = cos(angleXLight) * 20.0f;
            glUniform4fv(gLightPositionUniform, 1, light1_x_position);
        }
        else if (gLightRotationAxis == 2)
        {
            light1_y_position[0] = 0.0f;
            light1_y_position[1] = sin(angleYLight) * 20.0f;
            light1_y_position[2] = cos(angleYLight) * 20.0f;
            glUniform4fv(gLightPositionUniform, 1, light1_y_position);
        }
        else if (gLightRotationAxis == 3)
        {
            light1_z_position[0] = sin(angleZLight) * 20.0f;
            light1_z_position[1] = cos(angleZLight) * 20.0f;
            light1_z_position[2] = 0.0f;
            glUniform4fv(gLightPositionUniform, 1, light1_z_position);
        }
    }
    else
    {
        glUniform1f(gLKeyPressedUniform, 0.0);
    }
    
    // set all matrices to identity
    vmath::mat4 modelMatrix = vmath::mat4::identity();
    vmath::mat4 viewMatrix = vmath::mat4::identity();
    
    // bind VAO
    glBindVertexArray(gVaoSphere);
    
    // pass the viewMatrix to vertex shader variable
    glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
    
    // pass the projectionlViewMatrix to vertex shader variable
    glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
    
    index = 0;
    yPos = 3.5f;
    for (row = 0; row < 6; row++)
    {
        xPos = -2.5f;
        for (column = 0; column < 4; column++)
        {
            // translate z axis
            modelMatrix = vmath::translate(xPos, yPos, -12.0f);
            
            // pass the modelMatrix to vertex shader variable
            glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
            
            if (gbLight)
            {
                glUniform3fv(gKaUniform, 1, material_ambience[index]);
                glUniform3fv(gKdUniform, 1, material_diffuse[index]);
                glUniform3fv(gKsUniform, 1, material_specular[index]);
                glUniform1f(gMaterialShinessUniform, ((GLfloat)material_shiness[index][0] * (GLfloat)material_shiness[index][1]));
            }
            
            // draw sphere
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVboSphereElements);
            glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            xPos = xPos + 1.5f;
            index ++;
        }
        
        yPos = yPos - 1.5f;
    }
    
    // unbind VAO
    glBindVertexArray(0);
    
    glUseProgram(0);
    
    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
    
    [eaglContext presentRenderbuffer: GL_RENDERBUFFER];
    
    angleXLight = angleXLight + 0.1f;
    angleYLight = angleYLight + 0.1f;
    angleZLight = angleZLight + 0.1f;
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
    gLightRotationAxis = gLightRotationAxis >= 4 ? 1 : gLightRotationAxis + 1;
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

- (void) copyArray : (GLfloat*) inputArray : (GLfloat*) values : (int) length
{
    int i;
    
    for(i=0; i < length; i++)
    {
        inputArray[i] = values [i];
    }
}
@end
