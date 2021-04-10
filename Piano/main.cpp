// Standard libraries
#include <cstdlib>

// Header files
#include "helper.h"

// GLM
#include "glm/glm.hpp"

// Camera rotation angle
float cameraRotationAngle = 0.0f;

// Camera rotation vector
float camVecX = 0.0f, camVecZ = -1.0f;

// Camera XZ position
float x = 0.0f, z = 5.0f;

// Key states
float cameraDeltaAngle	= 0.0f;
float cameraDeltaMove	= 0;
int cameraXOrigin		= -1;

float yPos = 0;

// App state
bool playAudio = false;

// Really convoluted solution ahead
bool keys[88]; // 88 refers to the number of keys on the piano; 52 white and 36 black
// An array filled with our 88 keys, as named in the .obj file
std::string meshNames[] =		{	"KEY_A0",	"KEY_B0",	"KEY_C1",	"KEY_D1",	"KEY_E1",	"KEY_F1",	"KEY_G1",	"KEY_A1",	"KEY_B1",	"KEY_C2",	"KEY_D2",	"KEY_E2",	"KEY_F2",	"KEY_G2",	"KEY_A2",	"KEY_B2",	"KEY_C3",	"KEY_D3",	"KEY_E3",	"KEY_F3",	"KEY_G3",	"KEY_A3",	"KEY_B3",	"KEY_C4",	"KEY_D4",	"KEY_E4",	"KEY_F4",	"KEY_G4",	"KEY_A4",	"KEY_B4",	"KEY_C5",	"KEY_D5",	"KEY_E5",	"KEY_F5",	"KEY_G5",	"KEY_A5",	"KEY_B5",	"KEY_C6",	"KEY_D6",	"KEY_E6",	"KEY_F6",	"KEY_G6",	"KEY_A6",	"KEY_B6",	"KEY_C7",	"KEY_D7",	"KEY_E7",	"KEY_F7",	"KEY_G7",	"KEY_A7",	"KEY_B7",	"KEY_C8",	"KEY_A_0",	"KEY_C_1",	"KEY_D_1",	"KEY_F_1",	"KEY_G_1",	"KEY_A_1", "KEY_D_2", "KEY_C_2", "KEY_F_2", "KEY_G_2", "KEY_A_2", "KEY_F_3", "KEY_G_3", "KEY_A_3", "KEY_C_3", "KEY_D_3", "KEY_F_4", "KEY_G_4", "KEY_A_4", "KEY_C_4", "KEY_D_4", "KEY_F_5", "KEY_G_5", "KEY_A_5", "KEY_C_5", "KEY_D_5", "KEY_F_6", "KEY_A_6", "KEY_C_6", "KEY_D_6", "KEY_F_7", "KEY_G_7", "KEY_A_7", "KEY_C_7", "KEY_D_7", "KEY_G_6" };

// An filled with our 88 keys, as named in the .mat file. Each index corresponds to the mesh name above.
std::string materialNames[] = {		"white1 ",	"white2 ",	"white3 ",	"white4 ",	"white5 ",	"white6 ",	"white7 ",	"white8 ",	"white9 ",	"white10",	"white11",	"white12",	"white13",	"white14",	"white15",	"white16",	"white17",	"white18",	"white19",	"white20",	"white21",	"white22",	"white23",	"white24",	"white25",	"white26",	"white27",	"white28",	"white29",	"white30",	"white31",	"white32",	"white33",	"white34",	"white35",	"white36",	"white37",	"white38",	"white39",	"white40",	"white41",	"white42",	"white43",	"white44",	"white45",	"white46",	"white47",	"white48",	"white49",	"white50",	"white51",	"white52",	"black1 ",	"black2 ",	"black3 ",	"black4 ",	"black5 ",	"black6 ", "black7 ", "black8 ", "black9 ", "black10", "black11", "black12", "black13", "black14", "black15", "black16", "black17", "black18", "black19", "black20", "black21", "black22", "black23", "black24", "black25", "black26", "black27", "black28", "black29", "black30", "black31", "black32", "black33", "black34", "black35", "black36" };

// Key boolean state
bool isDown = false;
bool runCheckOnce = false;
std::vector<std::string> input;

// Map the name of a material to its corresponding mesh name and audio file
void MapKeys(std::string keyMatName)
{
	// capture name of mesh and material
	int c = 0;
	for (int i = 0; i < 88; i++)
	{
		if (materialNames[i] == keyMatName) c = i;
	}

	// identify key colour and capture name of audio file
	std::string audioName;
	if (keyMatName[0] == 'b') // black key
		audioName = meshNames[c].substr(meshNames[c].size() - 3);
	else
		audioName = meshNames[c].substr(meshNames[c].size() - 2);

	// read matching audio file (wav format)
	const std::ifstream fin("./audio/" + audioName + ".wav");

	// cannot find the audio file :(
	if (!fin)
		std::cerr << "Unable to open " << "./audio/" + audioName + ".wav" << std::endl;

	std::string path = "./audio/" + audioName + ".wav";

	std::cout << "\tCurrent mapping: " << keyMatName << " --> " << meshNames[c] << " --> "  << path << std::endl;

	// And finally, lay the note!
	PlaySound(path.c_str(), nullptr, SND_ASYNC);
}

// Handle readjustment of the window size
void SizeAdjustment(int w, int h)
{
	// Prevent a screen division by zero
	if (h == 0) h = 1;

	const float aspect = w * 1.0 / h;

	glMatrixMode(GL_PROJECTION);

	// Reset Matrix
	glLoadIdentity();

	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);

	// Set the correct perspective.
	gluPerspective(45.0f, aspect, 0.1f, 100.0f);

	// Return to the Modelview
	glMatrixMode(GL_MODELVIEW);
}

// Compute the new position of the camera
void SortPosition(float deltaMove)
{
	x += deltaMove * camVecX * 0.1f;
	z += deltaMove * camVecZ * 0.1f;
}

float ang = 0;
bool keyBeingPressed = false;
char keyPressed = ' ';
bool is_q_pressed = false;

// Scene render handler
void renderScene(void)
{
	// Check for and handle camera movement
	if (cameraDeltaMove) SortPosition(cameraDeltaMove);

	// Clear Color and Depth Buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.2, 0.2, 0.5, 1);

	// Reset transformations
	glLoadIdentity();

	// Set the camera look at position
	gluLookAt(x, 1.0f, z, x + camVecX, 0.7f, z + camVecZ, 0.0f, 1.0f, 0.0f);

	// Scale scene
	glScalef(3, 3, 3);

	// Rotate scene
	glRotatef(ang, 0, 1, 0);

	// Draw the frame
	DrawFrame(name_key_pressed);

	//if (nameKeyPressed.length() > 0) 
	//	MapKeys(nameKeyPressed);

	glutSwapBuffers();

}

// Current iteration of the piano roll
int iter = 0;

void KeyHandler(int extra);

// Handle the 'release' of a key
void KeyExitHandler(int extra)
{
	// Toggle pressed state boolean
	is_q_pressed = false;
	//std::cout << is_q_pressed << std::endl;
	has_key_pressed = false;
	runCheckOnce = false;
	isDown = true;
	// Reset current playing key to empty
	name_key_pressed = "";
	//glutSwapBuffers();
	// Play new key after 400ms
	glutTimerFunc(400, KeyHandler, 0);
}

bool CompareEnding(std::string const &completeString, std::string const &comparisionEnding) {
	// Check if our complete string is longer than the comparison ending string.
	if (completeString.length() >= comparisionEnding.length())
		// Return true if the end of the string contains the comparison string, or false if not
		return (0 == completeString.compare(completeString.length() - comparisionEnding.length(), comparisionEnding.length(), comparisionEnding));
	return false;
}

// Handle the 'pressing' of a key
void KeyHandler(int extra)
{
	is_q_pressed = !is_q_pressed;
	// If currently being pressed
	if (is_q_pressed) {
		//hasKeyPressed = true;
		isDown = false;

		// Find matching material, by comparing the current note in the input sequence to the final characters of the mesh
		std::string s;
		for (int i = 0; i < 88; i++) {
			if (CompareEnding(meshNames[i], input[iter]))
			{
				s = materialNames[i];
			}
		}

		// Rename current key name to the material name (material name is the reference for moving themesh)
		name_key_pressed = s;
	}

	glutPostRedisplay();

	// Run exit timer after 1000ms (a new key runs every 1400ms)
	glutTimerFunc(1000, KeyExitHandler, 0);
	std::cout << "Current note: " << input[iter] << std::endl;

	if (!has_key_pressed) {
		MapKeys(name_key_pressed);
		has_key_pressed = true;
	}

	// Increment iteration counter
	iter += 1;
}

void KeyboardHandler(unsigned char key, int x, int y)
{
	/*
	switch (key)
	{
		case 't':
		playAudio = !playAudio;
			break;
	}
	*/
}

void KeyboardUpHandler(unsigned char key, int x, int y)
{
	/*
	switch (key)
	{
	case 'a':
		is_q_pressed = false;
		//std::cout << is_q_pressed << std::endl;
		hasKeyPressed = false;
		runCheckOnce = false;
		isDown = true;
		nameKeyPressed = "";
		//glutSwapBuffers();
		break;
	case 'b':
		is_q_pressed = false;
		//std::cout << is_q_pressed << std::endl;
		hasKeyPressed = false;
		isDown = true;
		nameKeyPressed = "";
		//glutSwapBuffers();
		break;
	}
	*/
}

// Camera movement control handler
void PressKey(int key, int xx, int yy)
{
	switch (key)
	{
	case GLUT_KEY_UP: cameraDeltaMove = 0.1f;
		break;
	case GLUT_KEY_DOWN: cameraDeltaMove = -0.1f;
		break;
	case GLUT_KEY_LEFT:
		ang -= 0.1;
		break;
	case GLUT_KEY_RIGHT:
		ang += 0.1;
		break;
	default: ;
	}
	
}

// Camera movement control handler
void ReleaseKey(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP:
	case GLUT_KEY_DOWN: cameraDeltaMove = 0;
		break;
	case GLUT_KEY_LEFT:
		ang += 0.0;
		break;
	case GLUT_KEY_RIGHT:
		ang += 0.0;
		break;
	default: ;
	}
}

void MouseController(int x, int y)
{
	// Only be true on left button press
	if (cameraXOrigin >= 0)
	{
		// Update cameraDeltaAngle
		cameraDeltaAngle = (x - cameraXOrigin) * 0.001f;

		// Update camera direction
		camVecX = sin(cameraRotationAngle + cameraDeltaAngle);
		camVecZ = -cos(cameraRotationAngle + cameraDeltaAngle);
	}
}

void MouseButtonHandler(int button, int state, int x, int y)
{
	// Only start movement when the left mouse button is pressed
	if (button == GLUT_LEFT_BUTTON)
	{
		// When the left mouse button is released
		if (state == GLUT_UP)
		{
			cameraRotationAngle += cameraDeltaAngle;
			cameraXOrigin = -1;
		}
		else
		{
			cameraXOrigin = x;
		}
	}
}

// Timer testing -doesn't do anything
void timer(int value)
{
	//glutPostRedisplay();
	//glutTimerFunc(16, timer, 0);
}

// Application entry point
int main(int argc, char** argv)
{
	std::cout << "Beginning initialisation..." << std::endl;
	// Initialise GLUT
	glutInit(&argc, argv);
	// Create window of size 1280x800
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(1200, 800);
	// Window name!
	glutCreateWindow("Ghostly Piano");

	// Audio sequence input
	std::ifstream file;
	file.open("./audio/input.txt");
	std::string word;
	// Read audio file word for word and add each word to the vector input
	while (file >> word) input.push_back(word);

	// Object name
	InitApp("piano.obj");

	std::cout << "...initialisation complete!" << std::endl;

	// Keyboard handler
	glutKeyboardFunc(KeyboardHandler);
	// Keyboard up handler
	//glutKeyboardUpFunc(KeyboardUpHandler);

	

	// Scene callbacks handlers
	glutDisplayFunc(renderScene);
	glutReshapeFunc(SizeAdjustment);
	glutIdleFunc(renderScene);

	// Keyboard callback handler
	glutSpecialFunc(PressKey);
	glutSpecialUpFunc(ReleaseKey);

	// mouse callback handler
	glutMouseFunc(MouseButtonHandler);
	glutMotionFunc(MouseController);

	do
	{
		std::cout << '\n' << "Now that we've initialised everything, a quick introduction and the instructions. \n";
		std::cout << '\n' << "INTRODUCTION \n";
		std::cout << '\n' << "-Originally I'd intended to allow the user to control the keys on the piano using keyboard keys.";
		std::cout << '\n' << "-However there are far too many (piano) keys to map to (keyboard) keys!";
		std::cout << '\n' << "-So instead the program reads in a text file containing a roll of notes available to the piano and plays the corresponding note every 1400ms.";
		std::cout << '\n' << "-Think of it as a ghost piano!";
		std::cout << '\n' << "-I was unable to add in functionality to allow multiple keys to be played at once, also this piano has no pedals (una corda, sostenuto, and damper). So it's a really simple implementation of a piano.";
		std::cout << '\n' << "-The original piece that the input file is derived from is part of Rachmaninoff's Piano Concerto No.2 op.18. Albeit with some tweaks to account for the above omissions/issues and changes to the timing. \n";
		std::cout << '\n' << "INSTRUCTIONS \n";
		std::cout << '\n' << "-As mentioned previously the audio plays itself because I didn't quite have time to add in the ability for the user to control the keys.";
		std::cout << '\n' << "-The piano can be rotated using the left and right arrow keys.";
		std::cout << '\n' << "-The camera can be re-orientated on the x axis using the left mouse button.";
		std::cout << '\n' << "\n";
		std::cout << '\n' << "That's all. Press any key to continue, the graphics windows should show immediately.\n";
	} while (std::cin.get() != '\n');

	glutTimerFunc(0, KeyHandler, 0);

	// Initialise GLUT event cycle
	glutMainLoop();

	

	return 1;
}
