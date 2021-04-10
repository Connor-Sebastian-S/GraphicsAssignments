#define POSITION_ATTRIBUTE 0
#define NORMAL_ATTRIBUTE 2
#define DIFFUSE_ATTRIBUTE 3
#define SPECULAR_ATTRIBUTE 4
#define TEXCOORD0_ATTRIBUTE 8
#define TEXCOORD1_ATTRIBUTE 9
#define TEXCOORD2_ATTRIBUTE 10
/**
 * Purpose: To simulate (roughly) the movement of the Earth, Moon and Sun.
 * Done: Earth rotation
 *				Sun rotation
 *				Moon rotation
 *				Earth displays day and night sides as defined by sun position
 *				Earth displays lights at night 
 * ToDo:	Skybox (ignore, substituted with skysphere. Original skybox code still present, just disabled)	
 * ToDo:	Planet Normal mapping	
*/
#define GLM_ENABLE_EXPERIMENTAL
#define GLEW_STATIC
#ifdef _WIN32
#endif
#define GLM_FORCE_RADIANS

#define BUFFER_OFFSET(offset) ((void*)(offset))
#define MEMBER_OFFSET(s,m) ((char*)NULL + (offsetof(s,m)))

#include "../inc/CameraSetup.h"

// Window variables
int width = 1920;
int height = 1080;
int handle = 0;

// Log key entry variables
int wKeyLogger;
int aKeyLogger;
int sKeyLogger;
int dKeyLogger;
int qKeyLogger;
int eKeyLogger;

// Object rotation speed variables
float sunRotationSpeed = 0.0f;
float earthRotationSpeed = 0.0f;
float moonRotationSpeed = 0.0f;

// Camera object variables
Camera spaceCamera;
glm::vec3 initialCameraPosition;
glm::quat initialCameraRotation;

// Mesh variables
GLuint universalSphere = 0;
GLuint cloudSphere = 0;
GLuint skySphere = 0;

// Various texture ID reference variables
GLuint earthTexture = 0;
GLuint earthTexture_night = 0;
GLuint earthTexture_normal = 0;
GLuint earthTexture_clouds = 0;
GLuint moonTexture = 0;
GLuint moonTexture_n = 0;
GLuint starrySky = 0;

// Shader object variables
GLuint texturedShader = 0;
GLuint nonTexturedShader = 0;
GLuint skyShader = 0;

// Model, View, Projection matrix uniform variable in shader program.
GLint uniformMVP = -1;
GLint uniformModelMatrix = -1;
GLint uniformEyePosW = -1;
GLint uniformColor = -1;
// Light uniform variables.
GLint uniformLightPosW = -1;
GLint uniformLightColor = -1;
GLint uniformAmbient = -1;
// Material properties (as used by the Moon and Earth
GLint uniformMaterialEmissive = -1;
GLint uniformMaterialDiffuse = -1;
GLint uniformMaterialSpecular = -1;
GLint uniformMaterialShininess = -1;

// System clock tick variables
std::clock_t previousTick;
std::clock_t currentTick;

// Cube points
float points[] = {
	-10.0f, 10.0f, -10.0f, -10.0f, -10.0f, -10.0f, 10.0f, -10.0f, -10.0f,
	10.0f, -10.0f, -10.0f, 10.0f, 10.0f, -10.0f, -10.0f, 10.0f, -10.0f,

	-10.0f, -10.0f, 10.0f, -10.0f, -10.0f, -10.0f, -10.0f, 10.0f, -10.0f,
	-10.0f, 10.0f, -10.0f, -10.0f, 10.0f, 10.0f, -10.0f, -10.0f, 10.0f,

	10.0f, -10.0f, -10.0f, 10.0f, -10.0f, 10.0f, 10.0f, 10.0f, 10.0f,
	10.0f, 10.0f, 10.0f, 10.0f, 10.0f, -10.0f, 10.0f, -10.0f, -10.0f,

	-10.0f, -10.0f, 10.0f, -10.0f, 10.0f, 10.0f, 10.0f, 10.0f, 10.0f,
	10.0f, 10.0f, 10.0f, 10.0f, -10.0f, 10.0f, -10.0f, -10.0f, 10.0f,

	-10.0f, 10.0f, -10.0f, 10.0f, 10.0f, -10.0f, 10.0f, 10.0f, 10.0f,
	10.0f, 10.0f, 10.0f, -10.0f, 10.0f, 10.0f, -10.0f, 10.0f, -10.0f,

	-10.0f, -10.0f, -10.0f, -10.0f, -10.0f, 10.0f, 10.0f, -10.0f, -10.0f,
	10.0f, -10.0f, -10.0f, -10.0f, -10.0f, 10.0f, 10.0f, -10.0f, 10.0f
};

// Function definitions -should have put these in a header file, ideally
void IdleOpenGLFunctions();
void DisplayGL();
void KeyboardLogger(unsigned char c, int x, int y);
void KeyboardUpLogger(unsigned char c, int x, int y);
void ReshapeGL(int w, int h);

/**
 * Initialize OpenGL, register keyboard callbacks, and create a render window.
 */
void InitGL(int argc, char* argv[])
{
	// https://www.opengl.org/resources/libraries/glut/spec3/node9.html
	glutInit(&argc, argv);

	// Set display window size
	const int iScreenWidth = glutGet(GLUT_SCREEN_WIDTH);
	const int iScreenHeight = glutGet(GLUT_SCREEN_HEIGHT);

	glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DOUBLE | GLUT_DEPTH);

	// Is this necessary...?
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);

	// Set window position on screen go centre
	glutInitWindowPosition((iScreenWidth - width) / 2, (iScreenHeight - height) / 2);
	glutInitWindowSize(width, height);

	// Title
	handle = glutCreateWindow("Earth rendering");

	// GLUT callbacks.
	glutIdleFunc(IdleOpenGLFunctions);
	glutDisplayFunc(DisplayGL);
	glutKeyboardFunc(KeyboardLogger);
	glutKeyboardUpFunc(KeyboardUpLogger);
	glutReshapeFunc(ReshapeGL);

	glEnable(GL_DEPTH_TEST);
}

// https://www.khronos.org/opengl/wiki/OpenGL_Loading_Library
void InitialiseGLEWFunctionality()
{
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
		exit(-1);
	if (!GLEW_VERSION_3_3)
		exit(-1);

#ifdef _WIN32
	if (WGLEW_EXT_swap_control)
		wglSwapIntervalEXT(0);
#endif
}

/**
 * Loads a shader and returns the shader object.
 * https://www.khronos.org/opengl/wiki/Shader_Compilation
 */
GLuint LoadShader(GLenum shaderType, const std::string& shaderFile)
{
	std::ifstream ifs;

	// Load the shader.
	ifs.open(shaderFile);

	std::string source(std::istreambuf_iterator<char>(ifs), (std::istreambuf_iterator<char>()));
	ifs.close();

	// Create a shader object.
	GLuint shader = glCreateShader(shaderType);

	// Load the shader source for each shader object.
	const GLchar* sources[] = {source.c_str()};
	glShaderSource(shader, 1, sources, nullptr);

	// Compile the shader.
	glCompileShader(shader);

	// Check for errors
	GLint compileStatus;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);

	return shader;
}

/**
 * Create a shader program from a set of compiled shader objects.
*/
GLuint CreateShaderProgram(std::vector<GLuint> shaders)
{
	// Create a shader program.
	const GLuint program = glCreateProgram();

	// Attach the appropriate shader objects.
	for (GLuint shader : shaders)
		glAttachShader(program, shader);

	// Link the program
	glLinkProgram(program);

	// Check the link status.
	GLint linkStatus;
	glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

	return program;
}

/**
 * Load in a texture and save its textureID for later reference
*/
GLuint LoadTexture(const std::string& file)
{
	const GLuint textureID = SOIL_load_OGL_texture(file.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);

	return textureID;
}

/**
 * Create a sphere of size radius with horizontal slices 'slices' and vertical slice 'stacks'
 * https://stackoverflow.com/questions/8959338/draw-a-sphere-with-opengl
*/
GLuint SolidSphere(float radius, int slices, int stacks)
{
	const float pi = 3.1415926535897932384626433832795f; // is there a pre-made pi variable I can use?
	const float _2pi = 2.0f * pi;

	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> textureCoords;

	for (int i = 0; i <= stacks; ++i)
	{
		float V = i / static_cast<float>(stacks);
		const float phi = V * pi;

		for (int j = 0; j <= slices; ++j)
		{
			// U texture coordinate.
			const float U = j / static_cast<float>(slices);
			const float theta = U * _2pi;

			const float X = cos(theta) * sin(phi);
			const float Y = cos(phi);
			const float Z = sin(theta) * sin(phi);

			positions.push_back(glm::vec3(X, Y, Z) * radius);
			normals.push_back(glm::vec3(X, Y, Z));
			textureCoords.push_back(glm::vec2(U, V));
		}
	}

	// Now generate the index buffer
	std::vector<GLuint> indicies;

	for (int i = 0; i < slices * stacks + slices; ++i)
	{
		indicies.push_back(i);
		indicies.push_back(i + slices + 1);
		indicies.push_back(i + slices);

		indicies.push_back(i + slices + 1);
		indicies.push_back(i);
		indicies.push_back(i + 1);
	}

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint vbos[4];
	glGenBuffers(4, vbos);

	glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
	glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(POSITION_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(nullptr));
	glEnableVertexAttribArray(POSITION_ATTRIBUTE);

	glBindBuffer(GL_ARRAY_BUFFER, vbos[1]);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(NORMAL_ATTRIBUTE, 3, GL_FLOAT, GL_TRUE, 0, BUFFER_OFFSET(nullptr));
	glEnableVertexAttribArray(NORMAL_ATTRIBUTE);

	glBindBuffer(GL_ARRAY_BUFFER, vbos[2]);
	glBufferData(GL_ARRAY_BUFFER, textureCoords.size() * sizeof(glm::vec2), textureCoords.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(TEXCOORD0_ATTRIBUTE, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(nullptr));
	glEnableVertexAttribArray(TEXCOORD0_ATTRIBUTE);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos[3]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicies.size() * sizeof(GLuint), indicies.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return vao;
}

/**
 * Build skybox mesh from vertice array defined above
*/
void CreateSkybox()
{
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 3 * 36 * sizeof(GLfloat), &points,
	             GL_STATIC_DRAW);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	skySphere = vao;
}

/**
 * Load cubemap images and compile them into a single texture to be used in the skybox
 * http://antongerdelan.net/opengl/cubemaps.html
*/
void LoadSkybox()
{
	char* cubeImages[6] = {
		"./data/Textures/a.dds",
		"./data/Textures/a.dds",
		"./data/Textures/a.dds",
		"./data/Textures/a.dds",
		"./data/Textures/a.dds",
		"./data/Textures/a.dds"
	};

	// Set up texture and set parameters (using texture unit 1)
	glActiveTexture(GL_TEXTURE1);
	glGenTextures(1, &starrySky);
	glBindTexture(GL_TEXTURE_CUBE_MAP, starrySky);

	int width, height, nrChannels;
	// iterate through sky textures and read them into a specific coordinate block of a cubemap
	for (GLuint i = 0; i < 6; i++)
	{
		unsigned char* data = SOIL_load_image(cubeImages[i], &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			SOIL_free_image_data(data);
		}
		else
		{
			SOIL_free_image_data(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

/**
 *Loads in all of the required textures, moved into a seperate function to make everything more legible
 */
void LoadAllTextures()
{
	// Load required textures
	earthTexture = LoadTexture("./data/Textures/earth.dds");
	earthTexture_night = LoadTexture("./data/Textures/earth_night.dds");
	earthTexture_normal = LoadTexture("./data/Textures/earth_normal.dds");
	moonTexture = LoadTexture("./data/Textures/moon.dds");
	starrySky = LoadTexture("./data/Textures/sky.dds");
}

/**
 *Loads in all of the required shaders, moved into a seperate function to make everything more legible
 */
void LoadAllShaders()
{
	// Sun shader -a simple, untextured, white sphere represents the sun. As it is so far 
	// away, and so small, there is little point in it utilising a textured shader
	GLuint vertexShader = LoadShader(GL_VERTEX_SHADER, "./data/shaders/simpleShader.vert");
	GLuint fragmentShader = LoadShader(GL_FRAGMENT_SHADER, "./data/shaders/simpleShader.frag");

	std::vector<GLuint> allShaders;
	// Push the sun shader onto the shader stack
	allShaders.push_back(vertexShader);
	allShaders.push_back(fragmentShader);

	// Create shader program and define as global variable
	nonTexturedShader = CreateShaderProgram(allShaders);
	assert( nonTexturedShader );

	// Set the colour variable in the simple shader program to white.
	uniformColor = glGetUniformLocation(nonTexturedShader, "colour");

	// Moon and Earth shader -as these meshes are textured they use a specific textured shader, as opposed to the sun
	vertexShader = LoadShader(GL_VERTEX_SHADER, "./data/shaders/texturedDiffuse.vert");
	fragmentShader = LoadShader(GL_FRAGMENT_SHADER, "./data/shaders/texturedDiffuse.frag");

	allShaders.clear();

	// Push the textured Earth and Moon shader onto the shader stack
	allShaders.push_back(vertexShader);
	allShaders.push_back(fragmentShader);
	texturedShader = CreateShaderProgram(allShaders);
	assert( texturedShader );

	uniformMVP = glGetUniformLocation(texturedShader, "ModelViewProjectionMatrix");
	uniformModelMatrix = glGetUniformLocation(texturedShader, "ModelMatrix");
	uniformEyePosW = glGetUniformLocation(texturedShader, "EyePosW");
	uniformLightPosW = glGetUniformLocation(texturedShader, "LightPosW");
	uniformLightColor = glGetUniformLocation(texturedShader, "LightColour");
	uniformAmbient = glGetUniformLocation(texturedShader, "Ambient");
	uniformMaterialEmissive = glGetUniformLocation(texturedShader, "MaterialEmissive");
	uniformMaterialDiffuse = glGetUniformLocation(texturedShader, "MaterialDiffuse");
	uniformMaterialSpecular = glGetUniformLocation(texturedShader, "MaterialSpecular");
	uniformMaterialShininess = glGetUniformLocation(texturedShader, "MaterialShininess");

	// Pass night texture to shader
	if (earthTexture_night != 0)
	{
		earthTexture_night = glGetUniformLocation(texturedShader, "night");
		glUniform1i(earthTexture_night, earthTexture_night);
	}

	// Pass normal texture to shader
	if (earthTexture_normal != 0)
	{
		earthTexture_normal = glGetUniformLocation(texturedShader, "earthNormalMap");
		glUniform1i(earthTexture_normal, earthTexture_normal);
	}
}

/**
 * Application entry point
*/
int main(int argc, char* argv[])
{
	// Start time tick on application start
	previousTick = std::clock();

	// Set a key press loggers to 0 by default = not pressed/being pressed
	aKeyLogger = wKeyLogger = sKeyLogger = dKeyLogger = qKeyLogger = eKeyLogger = 0;

	// Set starting camera location/position to 50 units in front of the Earth object (at 0,0,0)
	initialCameraPosition = glm::vec3(0, 0, 50);
	spaceCamera.SetPosition(initialCameraPosition);
	spaceCamera.SetRotation(initialCameraRotation);

	// Initialise windows
	InitGL(argc, argv);
	InitialiseGLEWFunctionality();

	// Load all required textures, kept in its own function to keep things tidy-ish
	LoadAllTextures();

	// Load all required shaders, kept in its own function to keep things tidy-ish
	LoadAllShaders();

	glutMainLoop();
}

/**
 * Handle resizing of window
*/
void ReshapeGL(int w, int h)
{
	// If height of the window is equal to 0, set height to 1, so as there is always a place to render to
	// Wasn't sure if this was necessary, I read it somewhere so decided to add it :)
	if (h == 0) h = 1;
	width = w;
	height = h;
	spaceCamera.SetViewport(0, 0, w, h);
	spaceCamera.SetProjectionRH(30.0f, w / static_cast<float>(h), 0.1f, 2000.0f);

	glutPostRedisplay();
}

/**
 * Draw the sky object
*/
void DrawSky(int numIndicies, const glm::vec4 white, const glm::vec4 black, glm::mat4& modelMatrix, glm::mat4& mvp)
{
	glDepthMask(GL_FALSE);
	glBindTexture(GL_TEXTURE_2D, starrySky);

	modelMatrix = rotate(glm::radians(earthRotationSpeed / 10), glm::vec3(1, 1, 1)) *
		translate(spaceCamera.GetPosition()) * scale(glm::vec3(200));
	mvp = spaceCamera.GetProjectionMatrix() * spaceCamera.GetViewMatrix() * modelMatrix;

	glUniformMatrix4fv(uniformMVP, 1, GL_FALSE, value_ptr(mvp));
	glUniformMatrix4fv(uniformModelMatrix, 1, GL_FALSE, value_ptr(modelMatrix));

	glUniform4fv(uniformMaterialEmissive, 1, value_ptr(black));
	glUniform4fv(uniformMaterialDiffuse, 1, value_ptr(white));
	glUniform4fv(uniformMaterialSpecular, 1, value_ptr(white));
	glUniform1f(uniformMaterialShininess, 9.0f);

	glDrawElements(GL_TRIANGLES, numIndicies, GL_UNSIGNED_INT, BUFFER_OFFSET(nullptr));

	glBindVertexArray(0);
	glUseProgram(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

/**
 * Draw the moon object
*/
void DrawMoon(int numIndicies, const glm::vec4 white, const glm::vec4 black, glm::mat4& modelMatrix, glm::mat4& mvp)
{
	glBindTexture(GL_TEXTURE_2D, moonTexture);

	modelMatrix = rotate(glm::radians(moonRotationSpeed), glm::vec3(0, 1, 0)) * translate(glm::vec3(60, 0, 0)) * scale(
		glm::vec3(3.476f));
	mvp = spaceCamera.GetProjectionMatrix() * spaceCamera.GetViewMatrix() * modelMatrix;

	glUniformMatrix4fv(uniformMVP, 1, GL_FALSE, value_ptr(mvp));
	glUniformMatrix4fv(uniformModelMatrix, 1, GL_FALSE, value_ptr(modelMatrix));

	glUniform4fv(uniformMaterialEmissive, 1, value_ptr(black));
	glUniform4fv(uniformMaterialDiffuse, 1, value_ptr(white));
	glUniform4fv(uniformMaterialSpecular, 1, value_ptr(white));
	glUniform1f(uniformMaterialShininess, 5.0f);

	glDrawElements(GL_TRIANGLES, numIndicies, GL_UNSIGNED_INT, BUFFER_OFFSET(nullptr));
}

/**
 * Draw the Earth object
*/
void DrawEarth(int numIndicies, const glm::vec4 white, const glm::vec4 black, const glm::vec4 ambient,
               glm::mat4& modelMatrix, glm::mat4& mvp)
{
	glUseProgram(texturedShader);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture(GL_TEXTURE_2D, earthTexture);
	glDisable(GL_BLEND);

	glUniform4fv(uniformLightPosW, 1, value_ptr(modelMatrix[3]));
	glUniform4fv(uniformLightColor, 1, value_ptr(white));
	glUniform4fv(uniformAmbient, 1, value_ptr(ambient));

	modelMatrix = rotate(glm::radians(earthRotationSpeed), glm::vec3(0, 1, 0)) * scale(glm::vec3(12.756f));
	glm::vec4 eyePosW = glm::vec4(spaceCamera.GetPosition(), 1);
	mvp = spaceCamera.GetProjectionMatrix() * spaceCamera.GetViewMatrix() * modelMatrix;

	glUniformMatrix4fv(uniformMVP, 1, GL_FALSE, value_ptr(mvp));
	glUniformMatrix4fv(uniformModelMatrix, 1, GL_FALSE, value_ptr(modelMatrix));
	glUniform4fv(uniformEyePosW, 1, value_ptr(eyePosW));

	// Material properties.
	glUniform4fv(uniformMaterialEmissive, 1, value_ptr(black));
	glUniform4fv(uniformMaterialDiffuse, 1, value_ptr(white));
	glUniform4fv(uniformMaterialSpecular, 1, value_ptr(white));
	glUniform1f(uniformMaterialShininess, 50.0f);

	glDrawElements(GL_TRIANGLES, numIndicies, GL_UNSIGNED_INT, BUFFER_OFFSET(nullptr));
}

/**
 * Display function. Sets up models, render meshes with shaders and binds textures. Also handles transforms.
*/
void DisplayGL()
{
	// draw sphere of slices*stacks
	const int x = 128; // x
	const int y = 128; // y
	const int ind = (x * y + x) * 6;
	if (universalSphere == 0 && cloudSphere == 0)
	{
		universalSphere = SolidSphere(1, x, y); // Moon, Sun, and Earth sphere
		cloudSphere = SolidSphere(1.1, x, y); // Cloud sphere
		skySphere = SolidSphere(100, x, y); // Sky sphere
	}

	//if (skySphere == 0) CreateSkybox(); // Create skybox mesh

	const glm::vec4 white(1);
	const glm::vec4 black(0);
	const glm::vec4 ambient(0.1f, 0.1f, 0.1f, 1.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Draw the sun using a untextured shader.
	glBindVertexArray(universalSphere);

	glUseProgram(nonTexturedShader);
	glm::mat4 modelMatrix = rotate(
		glm::radians(sunRotationSpeed), // Rotate based on sun rotation speed
		glm::vec3(0, -1, 0)) * translate(glm::vec3(90, 0, 0)) * scale(
		glm::vec3(0.476f) // Scale to 0.476 the size of the default sphere (1)
	);
	glm::mat4 mvp = spaceCamera.GetProjectionMatrix() * spaceCamera.GetViewMatrix() * modelMatrix;
	const GLuint uniformMVP = glGetUniformLocation(nonTexturedShader, "MVP");
	glUniformMatrix4fv(uniformMVP, 1, GL_FALSE, value_ptr(mvp));
	glUniform4fv(uniformColor, 1, value_ptr(white));

	glDrawElements(GL_TRIANGLES, ind, GL_UNSIGNED_INT, BUFFER_OFFSET(nullptr)); // Draw sun

	// Draw the earth. Stored in its own function for legibility
	DrawEarth(ind, white, black, ambient, modelMatrix, mvp);

	// Draw the moon. Stored in its own function for legibility
	DrawMoon(ind, white, black, modelMatrix, mvp);

	//	Draw sky.  Stored in its own function for legibility
	DrawSky(ind, white, black, modelMatrix, mvp);

	glDepthMask(GL_TRUE);

	// Update display buffers
	glutSwapBuffers();
}

/**
 * Moslty timer functionality here
*/
void IdleOpenGLFunctions()
{
	// http://www.cplusplus.com/reference/ctime/clock/
	currentTick = std::clock();
	const float deltaTicks = static_cast<float>(currentTick - previousTick);
	previousTick = currentTick;

	// https://stackoverflow.com/questions/6129029/type-of-clocks-per-sec
	const float fDeltaTime =
		deltaTicks / static_cast<float>(CLOCKS_PER_SEC);

	// Camera movement speed
	const float cameraSpeed = 1.0f;

	// Translate camera based on keyboard inputs
	spaceCamera.Translate(glm::vec3(
			dKeyLogger - aKeyLogger,
			qKeyLogger - eKeyLogger,
			sKeyLogger - wKeyLogger)
		* cameraSpeed * fDeltaTime
	);

	// Rate of rotation in degrees per second
	const float fRotationRate1 = 1.0f;
	const float fRotationRate2 = 2.0;
	const float fRotationRate3 = 3.5f;
#
	// Set speeds and rotations amount -Earth
	earthRotationSpeed += fRotationRate1 * fDeltaTime;
	earthRotationSpeed = fmod(earthRotationSpeed, 360.0f);


	// Set speeds and rotations amount -Moon
	moonRotationSpeed += fRotationRate2 * fDeltaTime;
	moonRotationSpeed = fmod(moonRotationSpeed, 360.0f);


	// Set speeds and rotations amount -Sun
	sunRotationSpeed += fRotationRate3 * fDeltaTime;
	sunRotationSpeed = fmod(sunRotationSpeed, 360.0f);

	glutPostRedisplay();
}

/**
 * Check when a key is pressed -i.e. down
 */
void KeyboardLogger(unsigned char characterPressed, int x, int y)
{
	switch (characterPressed)
	{
	case 'w':
	case 'W':
		wKeyLogger = 1;
		break;
	case 'a':
	case 'A':
		aKeyLogger = 1;
		break;
	case 's':
	case 'S':
		sKeyLogger = 1;
		break;
	case 'd':
	case 'D':
		dKeyLogger = 1;
		break;
	case 'q':
	case 'Q':
		qKeyLogger = 1;
		break;
	case 'e':
	case 'E':
		eKeyLogger = 1;
		break;
	case 'r':
	case 'R':
		spaceCamera.SetPosition(initialCameraPosition);
		spaceCamera.SetRotation(initialCameraRotation);
		earthRotationSpeed = 0.0f;
		sunRotationSpeed = 0.0f;
		moonRotationSpeed = 0.0f;
		break;
	case 27:
		glutLeaveMainLoop();
		break;
	default: ;
	}
}

/**
 * Check when a key is released -i.e. up
 */
void KeyboardUpLogger(unsigned char characterPressed, int x, int y)
{
	switch (characterPressed)
	{
	case 'w':
	case 'W':
		wKeyLogger = 0;
		break;
	case 'a':
	case 'A':
		aKeyLogger = 0;
		break;
	case 's':
	case 'S':
		sKeyLogger = 0;
		break;
	case 'd':
	case 'D':
		dKeyLogger = 0;
		break;
	case 'q':
	case 'Q':
		qKeyLogger = 0;
		break;
	case 'e':
	case 'E':
		eKeyLogger = 0;
		break;

	default:
		break;
	}
}
