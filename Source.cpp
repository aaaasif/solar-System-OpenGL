#include <GL/glut.h>
#include <cmath>
#include <iostream>
#include <fstream>
#include <string>

// Rotation angle for the planets
float angle = 0.0f;
GLuint textures[10]; // Array to hold textures for Sun and planets
GLuint backgroundTexture; // Texture for the Milky Way background
GLuint moonTexture; // Texture for all moons (using the same texture for simplicity)
float zoomLevel = -30.0f; // Zoom level (distance from the camera)

// Global variables for camera control
float cameraAngleX = 0.0f; // Horizontal angle
float cameraAngleY = 0.0f; // Vertical angle
float lastMouseX = 0.0f;   // Last mouse X position
float lastMouseY = 0.0f;   // Last mouse Y position
bool isDragging = false;   // Track if mouse is dragging

// Number of planets (excluding the Sun)
const int NUM_PLANETS = 9;

// Moon properties
struct Moon {
    float orbitDistance;  // Distance from the planet
    float orbitSpeed;     // Degrees per update
    float rotationSpeed;  // Degrees per update
};

// Array to hold moon properties for each planet
Moon moons[NUM_PLANETS];

// Current angles for moons' orbits and rotations
float moonOrbitAngles[NUM_PLANETS] = { 0.0f };
float moonRotationAngles[NUM_PLANETS] = { 0.0f };

// Orbital inclinations in degrees for each planet
float orbitalInclinations[NUM_PLANETS] = {
    7.0f,    // Mercury
    3.4f,    // Venus
    0.0f,    // Earth
    1.85f,   // Mars
    1.3f,    // Jupiter
    2.5f,    // Saturn
    0.8f,    // Uranus
    1.77f,   // Neptune
    17.16f   // Pluto
};

// Function to load a BMP texture
GLuint loadBMPTexture(const char* filename) {
    GLuint texture;
    int width, height;
    unsigned char* data;

    FILE* file;
    errno_t err = fopen_s(&file, filename, "rb"); // Use fopen_s for safety
    if (err != 0 || !file) {
        std::cerr << "Failed to open texture file: " << filename << std::endl;
        return 0;
    }

    unsigned char header[54];
    fread(header, sizeof(unsigned char), 54, file);

    // Check the BMP signature
    if (header[0] != 'B' || header[1] != 'M') {
        std::cerr << "Not a valid BMP file: " << filename << std::endl;
        fclose(file);
        return 0;
    }

    width = *(int*)&header[18];
    height = *(int*)&header[22];

    // Assuming 24 bits per pixel (no compression)
    int bitsPerPixel = *(short*)&header[28];
    if (bitsPerPixel != 24) {
        std::cerr << "Only 24-bit BMP files are supported: " << filename << std::endl;
        fclose(file);
        return 0;
    }

    int imageSize = 3 * width * height;
    data = new unsigned char[imageSize];
    if (!data) {
        std::cerr << "Failed to allocate memory for texture: " << filename << std::endl;
        fclose(file);
        return 0;
    }

    fread(data, sizeof(unsigned char), imageSize, file);
    fclose(file);

    for (int i = 0; i < imageSize; i += 3) {
        std::swap(data[i], data[i + 2]); // Convert BGR to RGB
    }

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Upload texture data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

    delete[] data;
    return texture;
}

// Function to draw a textured sphere
void drawTexturedSphere(GLuint texture, float radius, int slices, int stacks) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);

    glColor3f(1.0f, 1.0f, 1.0f); // White color to display texture
    GLUquadric* quad = gluNewQuadric();
    gluQuadricTexture(quad, GL_TRUE);
    gluQuadricNormals(quad, GLU_SMOOTH);
    gluSphere(quad, radius, slices, stacks);
    gluDeleteQuadric(quad);
    glDisable(GL_TEXTURE_2D);
}

// Function to draw the Milky Way background
void drawBackground() {
    glPushMatrix();

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, backgroundTexture);

    glColor3f(1.0f, 1.0f, 1.0f); // White color to display the texture
    GLUquadric* quad = gluNewQuadric();
    gluQuadricTexture(quad, GL_TRUE);
    gluQuadricNormals(quad, GLU_SMOOTH);
    gluSphere(quad, 50.0f, 50, 50); // Large sphere radius
    gluDeleteQuadric(quad);

    glDisable(GL_TEXTURE_2D);

    glPopMatrix();
}

// Function to draw a moon for a given planet
void drawMoon(int planetIndex, float planetRadius) {
    glPushMatrix();

    // Apply moon's orbital inclination (same as planet's for simplicity)
    glRotatef(orbitalInclinations[planetIndex], 0.0f, 0.0f, 1.0f);

    // Rotate around the planet
    glRotatef(moonOrbitAngles[planetIndex], 0.0f, 1.0f, 0.0f);
    glTranslatef(moons[planetIndex].orbitDistance, 0.0f, 0.0f); // Distance from planet

    // Rotate the moon around its own axis
    glRotatef(moonRotationAngles[planetIndex], 0.0f, 1.0f, 0.0f);

    // Draw the moon (smaller than the planet)
    drawTexturedSphere(moonTexture, 0.1f * planetRadius, 10, 10);

    glPopMatrix();
}

// Function to draw the Sun
void drawSun() {
    glPushMatrix();
    glRotatef(angle * 0.5f, 0.0f, 1.0f, 0.0f); // Rotate the Sun slowly
    drawTexturedSphere(textures[0], 1.0f, 50, 50); // Render the textured Sun
    glPopMatrix();
}

// Function to draw Mercury
void drawMercury() {
    glPushMatrix();
    glRotatef(angle * 4.7f, 0.0f, 1.0f, 0.0f); // Orbit speed
    glTranslatef(3.0f, 0.0f, 0.0f);            // Distance from the Sun
    glRotatef(angle * 4.7f, 0.0f, 1.0f, 0.0f); // Mercury's rotation
    drawTexturedSphere(textures[1], 0.2f, 20, 20);

    // Draw Mercury's Moon
    drawMoon(0, 0.2f); // planetIndex = 0 for Mercury, planetRadius = 0.2f

    glPopMatrix();
}

// Function to draw Venus
void drawVenus() {
    glPushMatrix();
    glRotatef(angle * 3.5f, 0.0f, 1.0f, 0.0f);
    glTranslatef(5.0f, 0.0f, 0.0f);
    glRotatef(angle * 3.5f, 0.0f, 1.0f, 0.0f);
    drawTexturedSphere(textures[2], 0.3f, 20, 20);

    // Draw Venus's Moon
    drawMoon(1, 0.3f); // planetIndex = 1 for Venus, planetRadius = 0.3f

    glPopMatrix();
}

// Function to draw Earth
void drawEarth() {
    glPushMatrix();
    glRotatef(angle * 3.0f, 0.0f, 1.0f, 0.0f);
    glTranslatef(7.0f, 0.0f, 0.0f);
    glRotatef(angle * 3.0f, 0.0f, 1.0f, 0.0f);
    drawTexturedSphere(textures[3], 0.3f, 20, 20);

    // Draw Earth's Moon
    drawMoon(2, 0.3f); // planetIndex = 2 for Earth, planetRadius = 0.3f

    glPopMatrix();
}

// Function to draw Mars
void drawMars() {
    glPushMatrix();
    glRotatef(angle * 2.5f, 0.0f, 1.0f, 0.0f);
    glTranslatef(9.0f, 0.0f, 0.0f);
    glRotatef(angle * 2.5f, 0.0f, 1.0f, 0.0f);
    drawTexturedSphere(textures[4], 0.2f, 20, 20);

    // Draw Mars's Moon
    drawMoon(3, 0.2f); // planetIndex = 3 for Mars, planetRadius = 0.2f

    glPopMatrix();
}

// Function to draw Jupiter
void drawJupiter() {
    glPushMatrix();
    glRotatef(angle * 1.3f, 0.0f, 1.0f, 0.0f);
    glTranslatef(12.0f, 0.0f, 0.0f);
    glRotatef(angle * 1.3f, 0.0f, 1.0f, 0.0f);
    drawTexturedSphere(textures[5], 0.6f, 20, 20);

    // Draw Jupiter's Moon
    drawMoon(4, 0.6f); // planetIndex = 4 for Jupiter, planetRadius = 0.6f

    glPopMatrix();
}

// Function to draw Saturn
void drawSaturn() {
    glPushMatrix();
    glRotatef(angle * 1.0f, 0.0f, 1.0f, 0.0f);
    glTranslatef(15.0f, 0.0f, 0.0f);
    glRotatef(angle * 1.0f, 0.0f, 1.0f, 0.0f);
    drawTexturedSphere(textures[6], 0.5f, 20, 20);

    // Draw Saturn's Moon
    drawMoon(5, 0.5f); // planetIndex = 5 for Saturn, planetRadius = 0.5f

    glPopMatrix();
}

// Function to draw Uranus
void drawUranus() {
    glPushMatrix();
    glRotatef(angle * 0.7f, 0.0f, 1.0f, 0.0f);
    glTranslatef(18.0f, 0.0f, 0.0f);
    glRotatef(angle * 0.7f, 0.0f, 1.0f, 0.0f);
    drawTexturedSphere(textures[7], 0.4f, 20, 20);

    // Draw Uranus's Moon
    drawMoon(6, 0.4f); // planetIndex = 6 for Uranus, planetRadius = 0.4f

    glPopMatrix();
}

// Function to draw Neptune
void drawNeptune() {
    glPushMatrix();
    glRotatef(angle * 0.5f, 0.0f, 1.0f, 0.0f);
    glTranslatef(21.0f, 0.0f, 0.0f);
    glRotatef(angle * 0.5f, 0.0f, 1.0f, 0.0f);
    drawTexturedSphere(textures[8], 0.4f, 20, 20);

    // Draw Neptune's Moon
    drawMoon(7, 0.4f); // planetIndex = 7 for Neptune, planetRadius = 0.4f

    glPopMatrix();
}

// Function to draw Pluto
void drawPluto() {
    glPushMatrix();
    glRotatef(angle * 0.2f, 0.0f, 1.0f, 0.0f);
    glTranslatef(24.0f, 0.0f, 0.0f);
    glRotatef(angle * 0.2f, 0.0f, 1.0f, 0.0f);
    drawTexturedSphere(textures[9], 0.1f, 20, 20);

    // Draw Pluto's Moon
    drawMoon(8, 0.1f); // planetIndex = 8 for Pluto, planetRadius = 0.1f

    glPopMatrix();
}

// Display function
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Apply camera transformations
    glTranslatef(0.0f, 0.0f, zoomLevel);        // Apply zoom level
    glRotatef(cameraAngleY, 1.0f, 0.0f, 0.0f);  // Rotate around X-axis
    glRotatef(cameraAngleX, 0.0f, 1.0f, 0.0f);  // Rotate around Y-axis

    // Draw the Milky Way background
    glPushAttrib(GL_ENABLE_BIT | GL_LIGHTING_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_LIGHTING);    // Disable lighting for background
    glDisable(GL_DEPTH_TEST);  // Disable depth testing to ensure background is drawn behind everything

    drawBackground();

    glEnable(GL_LIGHTING);     // Re-enable lighting
    glEnable(GL_DEPTH_TEST);   // Re-enable depth testing
    glPopAttrib();

    // Draw the Sun and planets with moons
    drawSun();
    drawMercury();
    drawVenus();
    drawEarth();
    drawMars();
    drawJupiter();
    drawSaturn();
    drawUranus();
    drawNeptune();
    drawPluto();

    glutSwapBuffers();
}

// Update function for animation
void update(int value) {
    angle += 0.5f;  // Increment rotation angle for planets
    if (angle >= 360.0f) angle -= 360.0f;

    // Update moons' orbital and rotation angles
    for (int i = 0; i < NUM_PLANETS; ++i) {
        moonOrbitAngles[i] += moons[i].orbitSpeed;
        if (moonOrbitAngles[i] >= 360.0f) moonOrbitAngles[i] -= 360.0f;

        moonRotationAngles[i] += moons[i].rotationSpeed;
        if (moonRotationAngles[i] >= 360.0f) moonRotationAngles[i] -= 360.0f;
    }

    glutPostRedisplay(); // Request to redraw the scene
    glutTimerFunc(16, update, 0);  // Redraw every 16ms (~60 FPS)
}

// Reshape function to handle window resizing
void reshape(int w, int h) {
    if (h == 0) h = 1; // Prevent division by zero
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)w / (double)h, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

// Mouse Button and wheel handler with zoom limits
void mouseHandler(int button, int state, int x, int y) {
    if (button == 3 && state == GLUT_DOWN) { // Mouse wheel up
        zoomLevel += 1.0f; // Zoom in
        if (zoomLevel > -5.0f) zoomLevel = -5.0f; // Limit zoom in
    }
    else if (button == 4 && state == GLUT_DOWN) { // Mouse wheel down
        zoomLevel -= 1.0f; // Zoom out
        if (zoomLevel < -100.0f) zoomLevel = -100.0f; // Limit zoom out
    }
    else if (button == GLUT_LEFT_BUTTON) { // Left mouse button
        if (state == GLUT_DOWN) {
            isDragging = true;
            lastMouseX = x; // Record initial mouse position
            lastMouseY = y;
        }
        else if (state == GLUT_UP) {
            isDragging = false; // Stop dragging
        }
    }
    glutPostRedisplay();
}

// Mouse Drag Function
void mouseDrag(int x, int y) {
    if (isDragging) {
        float deltaX = x - lastMouseX; // Calculate horizontal movement
        float deltaY = y - lastMouseY; // Calculate vertical movement

        cameraAngleX += deltaX * 0.2f; // Adjust horizontal angle (sensitivity 0.2)
        cameraAngleY += deltaY * 0.2f; // Adjust vertical angle (sensitivity 0.2)

        lastMouseX = x; // Update last mouse position
        lastMouseY = y;

        glutPostRedisplay(); // Request redraw
    }
}

// Keyboard Handler for additional controls
void keyboardHandler(unsigned char key, int x, int y) {
    switch (key) {
    case 'r': // Reset camera
        cameraAngleX = 0.0f;
        cameraAngleY = 0.0f;
        zoomLevel = -30.0f;
        break;
    case 'w': // Pan up
        cameraAngleY += 5.0f;
        break;
    case 's': // Pan down
        cameraAngleY -= 5.0f;
        break;
    case 'a': // Pan left
        cameraAngleX -= 5.0f;
        break;
    case 'd': // Pan right
        cameraAngleX += 5.0f;
        break;
    default:
        break;
    }
    glutPostRedisplay();
}

// Initialize OpenGL settings
void initOpenGL() {
    glEnable(GL_DEPTH_TEST);                // Enable depth testing
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);   // Set background to black

    // Enable lighting
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0); // Use light 0 for the Sun

    // Define light properties
    GLfloat lightPos[] = { 0.0f, 0.0f, 0.0f, 1.0f }; // Positioned at the Sun
    GLfloat ambientLight[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat diffuseLight[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat specularLight[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);

    // Enable color tracking
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    // Set material properties to follow glColor values
    GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat mat_shininess[] = { 50.0f };
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 800.0 / 600.0, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);

    // Load textures for Sun and planets
    std::string texturePaths[] = {
        "texture/sun.bmp",       // textures[0]
        "texture/mercury.bmp",   // textures[1]
        "texture/venus.bmp",     // textures[2]
        "texture/earth.bmp",     // textures[3]
        "texture/mars.bmp",      // textures[4]
        "texture/jupiter.bmp",   // textures[5]
        "texture/saturn.bmp",    // textures[6]
        "texture/uranus.bmp",    // textures[7]
        "texture/neptune.bmp",   // textures[8]
        "texture/pluto.bmp"      // textures[9]
    };

    for (int i = 0; i < NUM_PLANETS + 1; ++i) { // Including Pluto
        textures[i] = loadBMPTexture(texturePaths[i].c_str());
        if (!textures[i]) {
            std::cerr << "Failed to load texture: " << texturePaths[i] << std::endl;
        }
    }

    // Load moon texture
    moonTexture = loadBMPTexture("texture/moon.bmp");
    if (!moonTexture) {
        std::cerr << "Failed to load moon texture: texture/moon.bmp" << std::endl;
    }

    // Load Milky Way background texture
    backgroundTexture = loadBMPTexture("texture/milkyway.bmp");
    if (!backgroundTexture) {
        std::cerr << "Failed to load background texture: texture/milkyway.bmp" << std::endl;
    }

    // Initialize moon properties for each planet
    for (int i = 0; i < NUM_PLANETS; ++i) {
        moons[i].orbitDistance = 1.5f + i * 0.5f; // Example distances
        moons[i].orbitSpeed = 0.5f + i * 0.1f;    // Example speeds
        moons[i].rotationSpeed = 1.0f + i * 0.2f; // Example rotation speeds
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("3D Solar System with Moons and Milky Way Background");

    initOpenGL();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouseHandler);       // Handle both mouse wheel and button events
    glutMotionFunc(mouseDrag);         // Handle mouse drag
    glutKeyboardFunc(keyboardHandler); // Handle keyboard inputs
    glutTimerFunc(16, update, 0);      // Start the update loop (~60 FPS)
    glutMainLoop();

    return 0;
}











//#include <glad/glad.h>
//#include <GLFW/glfw3.h>
//
//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtc/type_ptr.hpp>
//
//#include <stb_image.h>
//
//#include <ft2build.h>
//#include FT_FREETYPE_H   
//
//#include "Shader.h"
//#include "Sphere.h"
//#include "Camera.h"
//
//#include <cstdlib>
//#include <iostream>
//#include <vector>
//#include <map>
//#include <wtypes.h>
//#include <ctime>
//#include <filesystem>
//#include <string>
//#define _USE_MATH_DEFINES
//#include <math.h>
//
//#define TAU (M_PI * 2.0)
//
//
//void framebuffer_size_callback(GLFWwindow* window, int width, int height);
//void processInput(GLFWwindow* window);
//void RenderText(Shader& s, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);
//unsigned int loadTexture(char const* path);
//unsigned int loadCubemap(std::vector<std::string> faces);
//void ShowInfo(Shader& s);
//void GetDesktopResolution(float& horizontal, float& vertical)
//{
//	RECT desktop;
//	// Get a handle to the desktop window
//	const HWND hDesktop = GetDesktopWindow();
//	// Get the size of screen to the variable desktop
//	GetWindowRect(hDesktop, &desktop);
//	// The top left corner will have coordinates (0,0)
//	// and the bottom right corner will have coordinates
//	// (horizontal, vertical)
//	horizontal = desktop.right;
//	vertical = desktop.bottom;
//
//}
//
//
//GLfloat deltaTime = 0.0f;
//GLfloat lastFrame = 0.0f;
//Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
//bool onRotate = false;
//bool onFreeCam = true;
//bool SkyBoxExtra = false;
//float SCREEN_WIDTH = 800, SCREEN_HEIGHT = 600;
//
//glm::vec3 point = glm::vec3(0.0f, 0.0f, 0.0f);
//glm::vec3 PlanetPos = glm::vec3(0.0f, 0.0f, 0.0f);
//GLfloat lastX = (GLfloat)(SCREEN_WIDTH / 2.0);
//GLfloat lastY = (GLfloat)(SCREEN_HEIGHT / 2.0);
//float PlanetSpeed = .1f;
//int PlanetView = 0;
//
//bool keys[1024];
//GLfloat SceneRotateY = 0.0f;
//GLfloat SceneRotateX = 0.0f;
//bool onPlanet = false;
//void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
//{
//	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
//		glfwSetWindowShouldClose(window, GL_TRUE);
//
//	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
//	{
//		//camera.Position = PlanetPos;
//		onPlanet = true;
//	}
//
//	if (key >= 0 && key < 1024)
//	{
//		if (action == GLFW_PRESS)
//			keys[key] = true;
//		else if (action == GLFW_RELEASE)
//			keys[key] = false;
//	}
//}
//
//bool firstMouse = true;
//GLfloat xoff = 0.0f, yoff = 0.0f;
//
//struct Character {
//	GLuint TextureID;   // ID handle of the glyph texture
//	glm::ivec2 Size;    // Size of glyph
//	glm::ivec2 Bearing;  // Offset from baseline to left/top of glyph
//	GLuint Advance;    // Horizontal offset to advance to next glyph
//};
//std::map<GLchar, Character> Characters;
//GLuint textVAO, textVBO;
//
//struct PlanetInfo {
//	std::string Name;
//	std::string OrbitSpeed;
//	std::string Mass;
//	std::string Gravity;
//};
//PlanetInfo Info;
//
//void mouse_callback(GLFWwindow* window, double xpos, double ypos)
//{
//	if (firstMouse)
//	{
//		lastX = (GLfloat)xpos;
//		lastY = (GLfloat)ypos;
//		firstMouse = false;
//	}
//
//	GLfloat xoffset = (GLfloat)(xpos - lastX);
//	GLfloat yoffset = (GLfloat)(lastY - ypos);
//	xoff = xoffset;
//	yoff = yoff;
//
//	lastX = (GLfloat)xpos;
//	lastY = (GLfloat)ypos;
//	if (onRotate)
//	{
//		SceneRotateY += yoffset * 0.1f;
//		SceneRotateX += xoffset * 0.1f;
//	}
//	camera.ProcessMouseMovement(xoffset, yoffset);
//}
//void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
//{
//	if (yoffset == 1)
//		camera.ProcessKeyboard(SCROLL_FORWARD, deltaTime);
//	else
//	{
//		camera.ProcessKeyboard(SCROLL_BACKWARD, deltaTime);
//	}
//}
//void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
//{
//	if (onFreeCam && !camera.FreeCam)
//	{
//		if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
//			onRotate = true;
//		else onRotate = false;
//	}
//}
//
//int main() {
//	GetDesktopResolution(SCREEN_WIDTH, SCREEN_HEIGHT); // get resolution for create window
//	camera.LookAtPos = point;
//
//	/* GLFW INIT */
//	glfwInit();
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//	glfwWindowHint(GLFW_SAMPLES, 4);
//	/* GLFW INIT */
//
//	/* GLFW WINDOW CREATION */
//	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "LearnOpenGL", glfwGetPrimaryMonitor(), NULL);
//	if (window == NULL)
//	{
//		std::cout << "Failed to create GLFW window" << std::endl;
//		glfwTerminate();
//		return -1;
//	}
//	glfwMakeContextCurrent(window);
//	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
//	glfwSetCursorPosCallback(window, mouse_callback);
//	glfwSetKeyCallback(window, key_callback);
//	glfwSetScrollCallback(window, scroll_callback);
//	glfwSetMouseButtonCallback(window, mouse_button_callback);
//	/* GLFW WINDOW CREATION */
//
//
//	/* LOAD GLAD */
//	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
//	{
//		std::cout << "Failed to initialize GLAD" << std::endl;
//		return -1;
//	}
//	/* LOAD GLAD */
//
//
//	/* CONFIGURATION FOR TEXT RENDER */
//	FT_Library ft;
//	if (FT_Init_FreeType(&ft))
//		std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
//
//	FT_Face face;
//	if (FT_New_Face(ft, "fonts/ff.otf", 0, &face))
//		std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
//
//	// Set size to load glyphs as
//	FT_Set_Pixel_Sizes(face, 0, 48);
//
//	// Disable byte-alignment restriction
//	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//
//	// Load first 128 characters of ASCII set
//	for (GLubyte c = 0; c < 128; c++)
//	{
//		// Load character glyph 
//		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
//		{
//			std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
//			continue;
//		}
//		// Generate texture
//		GLuint texture;
//		glGenTextures(1, &texture);
//		glBindTexture(GL_TEXTURE_2D, texture);
//		glTexImage2D(
//			GL_TEXTURE_2D,
//			0,
//			GL_RED,
//			face->glyph->bitmap.width,
//			face->glyph->bitmap.rows,
//			0,
//			GL_RED,
//			GL_UNSIGNED_BYTE,
//			face->glyph->bitmap.buffer
//		);
//		// Set texture options
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//		// Now store character for later use
//		Character character = {
//			texture,
//			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
//			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
//			face->glyph->advance.x
//		};
//		Characters.insert(std::pair<GLchar, Character>(c, character));
//	}
//	glBindTexture(GL_TEXTURE_2D, 0);
//	// Destroy FreeType once we're finished
//	FT_Done_Face(face);
//	FT_Done_FreeType(ft);
//	/* CONFIGURATION FOR TEXT RENDER */
//
//
//	glEnable(GL_DEPTH_TEST);
//	glEnable(GL_MULTISAMPLE);
//	glEnable(GL_BLEND);
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//
//	/* SHADERS */
//	Shader SimpleShader("simpleVS.vs", "simpleFS.fs");
//	Shader SkyboxShader("skybox.vs", "skybox.fs");
//	Shader texShader("simpleVS.vs", "texFS.fs");
//	Shader TextShader("TextShader.vs", "TextShader.fs");
//	/* SHADERS */
//
//	// PROJECTION FOR TEXT RENDER
//	glm::mat4 Text_projection = glm::ortho(0.0f, SCREEN_WIDTH, 0.0f, SCREEN_HEIGHT);
//	TextShader.Use();
//	glUniformMatrix4fv(glGetUniformLocation(TextShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(Text_projection));
//
//	float cube[] = {
//		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
//		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
//		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
//		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
//		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
//		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
//
//		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
//		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
//		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
//		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
//		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
//		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
//
//		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
//		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
//		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
//		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
//		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
//		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
//
//		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
//		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
//		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
//		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
//		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
//		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
//
//		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
//		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
//		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
//		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
//		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
//		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
//
//		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
//		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
//		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
//		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
//		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
//		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
//	};
//	float skyboxVertices[] = {
//		// positions          
//		-1.0f,  1.0f, -1.0f,
//		-1.0f, -1.0f, -1.0f,
//		 1.0f, -1.0f, -1.0f,
//		 1.0f, -1.0f, -1.0f,
//		 1.0f,  1.0f, -1.0f,
//		-1.0f,  1.0f, -1.0f,
//
//		-1.0f, -1.0f,  1.0f,
//		-1.0f, -1.0f, -1.0f,
//		-1.0f,  1.0f, -1.0f,
//		-1.0f,  1.0f, -1.0f,
//		-1.0f,  1.0f,  1.0f,
//		-1.0f, -1.0f,  1.0f,
//
//		 1.0f, -1.0f, -1.0f,
//		 1.0f, -1.0f,  1.0f,
//		 1.0f,  1.0f,  1.0f,
//		 1.0f,  1.0f,  1.0f,
//		 1.0f,  1.0f, -1.0f,
//		 1.0f, -1.0f, -1.0f,
//
//		-1.0f, -1.0f,  1.0f,
//		-1.0f,  1.0f,  1.0f,
//		 1.0f,  1.0f,  1.0f,
//		 1.0f,  1.0f,  1.0f,
//		 1.0f, -1.0f,  1.0f,
//		-1.0f, -1.0f,  1.0f,
//
//		-1.0f,  1.0f, -1.0f,
//		 1.0f,  1.0f, -1.0f,
//		 1.0f,  1.0f,  1.0f,
//		 1.0f,  1.0f,  1.0f,
//		-1.0f,  1.0f,  1.0f,
//		-1.0f,  1.0f, -1.0f,
//
//		-1.0f, -1.0f, -1.0f,
//		-1.0f, -1.0f,  1.0f,
//		 1.0f, -1.0f, -1.0f,
//		 1.0f, -1.0f, -1.0f,
//		-1.0f, -1.0f,  1.0f,
//		 1.0f, -1.0f,  1.0f
//	};
//
//	/* SKYBOX GENERATION */
//	unsigned int skyboxVAO, skyboxVBO;
//	glGenVertexArrays(1, &skyboxVAO);
//	glGenBuffers(1, &skyboxVBO);
//	glBindVertexArray(skyboxVAO);
//	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
//	glEnableVertexAttribArray(0);
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
//	/* SKYBOX GENERATION */
//
//	/* VERTEX GENERATION FOR ORBITS */
//	std::vector<float> orbVert;
//	GLfloat xx;
//	GLfloat zz;
//	float angl;
//	for (int i = 0; i < 2000; i++)
//	{
//		angl = (float)(M_PI / 2 - i * (M_PI / 1000));
//		xx = sin(angl) * 100.0f;
//		zz = cos(angl) * 100.0f;
//		orbVert.push_back(xx);
//		orbVert.push_back(0.0f);
//		orbVert.push_back(zz);
//
//	}
//	/* VERTEX GENERATION FOR ORBITS */
//
//	/* VAO-VBO for ORBITS*/
//	GLuint VBO_t, VAO_t;
//	glGenVertexArrays(1, &VAO_t);
//	glGenBuffers(1, &VBO_t);
//	glBindVertexArray(VAO_t);
//	glBindBuffer(GL_ARRAY_BUFFER, VBO_t);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * orbVert.size(), orbVert.data(), GL_STATIC_DRAW);
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
//	glEnableVertexAttribArray(0);
//	glBindBuffer(GL_ARRAY_BUFFER, 0);
//	glBindVertexArray(0);
//	/* VAO-VBO for ORBITS*/
//
//	/* TEXT RENDERING VAO-VBO*/
//	glGenVertexArrays(1, &textVAO);
//	glGenBuffers(1, &textVBO);
//	glBindVertexArray(textVAO);
//	glBindBuffer(GL_ARRAY_BUFFER, textVBO);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
//	glEnableVertexAttribArray(0);
//	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
//	glBindBuffer(GL_ARRAY_BUFFER, 0);
//	glBindVertexArray(0);
//	/* TEXT RENDERING VAO-VBO*/
//
//	/* LOAD TEXTURES */
//	unsigned int texture_earth = loadTexture("texture/earth2k.jpg");
//	unsigned int t_sun = loadTexture("texture/2k_sun.jpg");
//	unsigned int texture_moon = loadTexture("texture/2k_moon.jpg");
//	unsigned int texture_mercury = loadTexture("texture/2k_mercury.jpg");
//	unsigned int texture_venus = loadTexture("texture/2k_mercury.jpg");
//	unsigned int texture_mars = loadTexture("texture/2k_mars.jpg");
//	unsigned int texture_jupiter = loadTexture("texture/2k_jupiter.jpg");
//	unsigned int texture_saturn = loadTexture("texture/2k_saturn.jpg");
//	unsigned int texture_uranus = loadTexture("texture/2k_uranus.jpg");
//	unsigned int texture_neptune = loadTexture("texture/2k_neptune.jpg");
//	unsigned int texture_saturn_ring = loadTexture("texture/r.jpg");
//	unsigned int texture_earth_clouds = loadTexture("texture/2k_earth_clouds.jpg");
//	/* LOAD TEXTURES */
//
//	/* SPHERE GENERATION */
//	Sphere Sun(100.0f, 36 * 5, 18 * 5);
//	Sphere Mercury(10.0f, 36, 18);
//	Sphere Venus(12.0f, 36, 18);
//	Sphere Earth(11.8f, 36, 18);
//	Sphere Mars(8.0f, 36, 18);
//	Sphere Jupiter(40.0f, 36, 18);
//	Sphere Saturn(37.0f, 36, 18);
//	Sphere Uranus(30.0f, 36, 18);
//	Sphere Neptune(30.0f, 36, 19);
//	Sphere Moon(5.5f, 36, 18);
//	/* SPHERE GENERATION */
//
//	std::vector<std::string> faces
//	{
//		"resources/skybox/starfield/starfield_rt.tga",
//		"resources/skybox/starfield/starfield_lf.tga",
//		"resources/skybox/starfield/starfield_up.tga",
//		"resources/skybox/starfield/starfield_dn.tga",
//		"resources/skybox/starfield/starfield_ft.tga",
//		"resources/skybox/starfield/starfield_bk.tga",
//	};
//	std::vector<std::string> faces_extra
//	{
//		"resources/skybox/blue/bkg1_right.png",
//		"resources/skybox/blue/bkg1_left.png",
//		"resources/skybox/blue/bkg1_top.png",
//		"resources/skybox/blue/bkg1_bot.png",
//		"resources/skybox/blue/bkg1_front.png",
//		"resources/skybox/blue/bkg1_back.png",
//	};
//
//	unsigned int cubemapTexture = loadCubemap(faces);
//	unsigned int cubemapTextureExtra = loadCubemap(faces_extra);
//	GLfloat camX = 10.0f;
//	GLfloat camZ = 10.0f;
//
//	camera.Position = glm::vec3(0.0f, 250.0f, -450.0f);
//	camera.Yaw = 90.0f;
//	camera.Pitch = -40.0f;
//	camera.ProcessMouseMovement(xoff, yoff);
//	camera.FreeCam = false;
//	onFreeCam = true;
//	glm::mat4 view;
//	glm::vec3 PlanetsPositions[9];
//	while (!glfwWindowShouldClose(window))
//	{
//
//		GLfloat currentFrame = (GLfloat)glfwGetTime();
//		deltaTime = currentFrame - lastFrame;
//		lastFrame = currentFrame;
//
//		/* ZOOM CONTROL */
//		if (!camera.FreeCam)
//		{
//			if (camera.Position.y < 200 && camera.Position.y > 200.0f)
//				camera.MovementSpeed = 300.0f;
//			if (camera.Position.y < 125.f && camera.Position.y > 70.0f)
//				camera.MovementSpeed = 200.0f;
//			if (camera.Position.y < 70.f && camera.Position.y > 50.0f)
//				camera.MovementSpeed = 100.0f;
//
//			if (camera.Position.y > 200 && camera.Position.y < 400.0f)
//				camera.MovementSpeed = 400.0f;
//			if (camera.Position.y > 125.f && camera.Position.y < 200.0f)
//				camera.MovementSpeed = 300.0f;
//			if (camera.Position.y > 70.f && camera.Position.y < 125.0f)
//				camera.MovementSpeed = 200.0f;
//		}
//		/* ZOOM CONTROL */
//
//		processInput(window); // input
//
//		if (!onFreeCam)
//		{
//			SceneRotateY = 0.0f;
//			SceneRotateX = 0.0f;
//		}
//		if (camera.FreeCam || PlanetView > 0)
//			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
//		else glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
//
//		// render
//		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
//		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//		SimpleShader.Use();
//
//		glm::mat4 model = glm::mat4(1.0f);
//
//		double viewX;
//		double viewZ;
//		glm::vec3 viewPos;
//
//		SimpleShader.Use();
//		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 10000.0f);
//		SimpleShader.setMat4("model", model);
//		SimpleShader.setMat4("view", view);
//		SimpleShader.setMat4("projection", projection);
//
//		glActiveTexture(GL_TEXTURE0);
//		glBindTexture(GL_TEXTURE_2D, t_sun);
//
//
//		/* SUN */
//		glm::mat4 model_sun;
//		model_sun = glm::rotate(model_sun, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.f));
//		model_sun = glm::rotate(model_sun, (GLfloat)glfwGetTime() * glm::radians(23.5f) * 0.25f, glm::vec3(0.0f, 0.0f, 1.f));
//		model_sun = glm::translate(model_sun, point);
//		SimpleShader.setMat4("model", model_sun);
//		Sun.Draw();
//		/* SUN */
//
//		/* MERCURY */
//		glm::mat4 model_mercury;
//		double xx = sin(glfwGetTime() * PlanetSpeed) * 100.0f * 2.0f * 1.3f;
//		double zz = cos(glfwGetTime() * PlanetSpeed) * 100.0f * 2.0f * 1.3f;
//		glActiveTexture(GL_TEXTURE0);
//		glBindTexture(GL_TEXTURE_2D, texture_mercury);
//		model_mercury = glm::translate(model_mercury, point);
//		model_mercury = glm::rotate(model_mercury, glm::radians(SceneRotateY), glm::vec3(1.0f, 0.0f, 0.0f));
//		model_mercury = glm::rotate(model_mercury, glm::radians(SceneRotateX), glm::vec3(0.0f, 0.0f, 1.0f));
//		model_mercury = glm::translate(model_mercury, glm::vec3(xx, 0.0f, zz));
//		PlanetsPositions[0] = glm::vec3(xx, 0.0f, zz);
//		model_mercury = glm::rotate(model_mercury, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.f));
//		model_mercury = glm::rotate(model_mercury, (GLfloat)glfwGetTime() * glm::radians(-90.0f) * 0.05f, glm::vec3(0.0f, 0.0f, 1.f));
//		SimpleShader.setMat4("model", model_mercury);
//		Mercury.Draw();
//		/* MERCURY */
//
//		/* VENUS */
//		glm::mat4 model_venus;
//		xx = sin(glfwGetTime() * PlanetSpeed * 0.75f) * 100.0f * 3.0f * 1.3f;
//		zz = cos(glfwGetTime() * PlanetSpeed * 0.75f) * 100.0f * 3.0f * 1.3f;
//		glActiveTexture(GL_TEXTURE0);
//		glBindTexture(GL_TEXTURE_2D, texture_venus);
//		model_venus = glm::translate(model_venus, point);
//		model_venus = glm::rotate(model_venus, glm::radians(SceneRotateY), glm::vec3(1.0f, 0.0f, 0.0f));
//		model_venus = glm::rotate(model_venus, glm::radians(SceneRotateX), glm::vec3(0.0f, 0.0f, 1.0f));
//		model_venus = glm::translate(model_venus, glm::vec3(xx, 0.0f, zz));
//		PlanetsPositions[1] = glm::vec3(xx, 0.0f, zz);
//		model_venus = glm::rotate(model_venus, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.f));
//		model_venus = glm::rotate(model_venus, glm::radians(-132.5f), glm::vec3(0.0f, 1.0f, 0.f));
//		model_venus = glm::rotate(model_venus, (GLfloat)glfwGetTime() * glm::radians(-132.5f) * 0.012f, glm::vec3(0.0f, 0.0f, 1.f));
//		SimpleShader.setMat4("model", model_venus);
//		Venus.Draw();
//		/* VENUS */
//
//		/* EARTH */
//		glm::mat4 model_earth;
//		xx = sin(glfwGetTime() * PlanetSpeed * 0.55f) * 100.0f * 4.0f * 1.3f;
//		zz = cos(glfwGetTime() * PlanetSpeed * 0.55f) * 100.0f * 4.0f * 1.3f;
//		glActiveTexture(GL_TEXTURE0);
//		glBindTexture(GL_TEXTURE_2D, texture_earth);
//		model_earth = glm::translate(model_earth, point);
//		model_earth = glm::rotate(model_earth, glm::radians(SceneRotateY), glm::vec3(1.0f, 0.0f, 0.0f));
//		model_earth = glm::rotate(model_earth, glm::radians(SceneRotateX), glm::vec3(0.0f, 0.0f, 1.0f));
//		model_earth = glm::translate(model_earth, glm::vec3(xx, 0.0f, zz));
//		glm::vec3 EarthPoint = glm::vec3(xx, 0.0f, zz);
//		PlanetsPositions[2] = glm::vec3(xx, 0.0f, zz);
//		model_earth = glm::rotate(model_earth, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.f));
//		model_earth = glm::rotate(model_earth, glm::radians(-33.25f), glm::vec3(0.0f, 1.0f, 0.f));
//		model_earth = glm::rotate(model_earth, (GLfloat)glfwGetTime() * glm::radians(-33.25f) * 2.0f, glm::vec3(0.0f, 0.0f, 1.f));
//		camera.LookAtPos = glm::vec3(model_earth[3][0], model_earth[3][1], model_earth[3][2]);
//		SimpleShader.setMat4("model", model_earth);
//		Earth.Draw();
//
//		/* EARTH */
//
//		/* MOON */
//		glm::mat4 model_moon;
//		xx = sin(glfwGetTime() * PlanetSpeed * 67.55f) * 100.0f * 0.5f * 1.3f;
//		zz = cos(glfwGetTime() * PlanetSpeed * 67.55f) * 100.0f * 0.5f * 1.3f;
//		glActiveTexture(GL_TEXTURE0);
//		glBindTexture(GL_TEXTURE_2D, texture_moon);
//		model_moon = glm::rotate(model_moon, glm::radians(SceneRotateY), glm::vec3(1.0f, 0.0f, 0.0f));
//		model_moon = glm::rotate(model_moon, glm::radians(SceneRotateX), glm::vec3(0.0f, 0.0f, 1.0f));
//		model_moon = glm::translate(model_moon, EarthPoint);
//		model_moon = glm::translate(model_moon, glm::vec3(xx, 0.0f, zz));
//		model_moon = glm::rotate(model_moon, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.f));
//		model_moon = glm::rotate(model_moon, glm::radians(-32.4f), glm::vec3(0.0f, 1.0f, 0.f));
//		model_moon = glm::rotate(model_moon, (GLfloat)glfwGetTime() * glm::radians(-32.4f) * 3.1f, glm::vec3(0.0f, 0.0f, 1.f));
//		SimpleShader.setMat4("model", model_moon);
//		Moon.Draw();
//		/* MOON */
//
//
//		/* MARS */
//		glm::mat4 model_mars;
//		xx = sin(glfwGetTime() * PlanetSpeed * 0.35f) * 100.0f * 5.0f * 1.3f;
//		zz = cos(glfwGetTime() * PlanetSpeed * 0.35f) * 100.0f * 5.0f * 1.3f;
//		glActiveTexture(GL_TEXTURE0);
//		glBindTexture(GL_TEXTURE_2D, texture_mars);
//		model_mars = glm::translate(model_mars, point);
//		model_mars = glm::rotate(model_mars, glm::radians(SceneRotateY), glm::vec3(1.0f, 0.0f, 0.0f));
//		model_mars = glm::rotate(model_mars, glm::radians(SceneRotateX), glm::vec3(0.0f, 0.0f, 1.0f));
//		model_mars = glm::translate(model_mars, glm::vec3(xx, 0.0f, zz));
//		PlanetsPositions[3] = glm::vec3(xx, 0.0f, zz);
//		model_mars = glm::rotate(model_mars, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.f));
//		model_mars = glm::rotate(model_mars, glm::radians(-32.4f), glm::vec3(0.0f, 1.0f, 0.f));
//		model_mars = glm::rotate(model_mars, (GLfloat)glfwGetTime() * glm::radians(-32.4f) * 2.1f, glm::vec3(0.0f, 0.0f, 1.f));
//		SimpleShader.setMat4("model", model_mars);
//		Mars.Draw();
//		/* MARS */
//
//		/* JUPITER */
//		glm::mat4 model_jupiter;
//		xx = sin(glfwGetTime() * PlanetSpeed * 0.2f) * 100.0f * 6.0f * 1.3f;
//		zz = cos(glfwGetTime() * PlanetSpeed * 0.2f) * 100.0f * 6.0f * 1.3f;
//		glActiveTexture(GL_TEXTURE0);
//		glBindTexture(GL_TEXTURE_2D, texture_jupiter);
//		model_jupiter = glm::translate(model_jupiter, point);
//		model_jupiter = glm::rotate(model_jupiter, glm::radians(SceneRotateY), glm::vec3(1.0f, 0.0f, 0.0f));
//		model_jupiter = glm::rotate(model_jupiter, glm::radians(SceneRotateX), glm::vec3(0.0f, 0.0f, 1.0f));
//		model_jupiter = glm::translate(model_jupiter, glm::vec3(xx, 0.0f, zz));
//		PlanetsPositions[4] = glm::vec3(xx, 0.0f, zz);
//		model_jupiter = glm::rotate(model_jupiter, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.f));
//		model_jupiter = glm::rotate(model_jupiter, glm::radians(-23.5f), glm::vec3(0.0f, 1.0f, 0.f));
//		model_jupiter = glm::rotate(model_jupiter, (GLfloat)glfwGetTime() * glm::radians(-23.5f) * 4.5f, glm::vec3(0.0f, 0.0f, 1.f));
//		SimpleShader.setMat4("model", model_jupiter);
//		Jupiter.Draw();
//		/* JUPITER */
//
//		/* SATURN */
//		glm::mat4 model_saturn;
//		xx = sin(glfwGetTime() * PlanetSpeed * 0.15f) * 100.0f * 7.0f * 1.3f;
//		zz = cos(glfwGetTime() * PlanetSpeed * 0.15f) * 100.0f * 7.0f * 1.3f;
//		glActiveTexture(GL_TEXTURE0);
//		glBindTexture(GL_TEXTURE_2D, texture_saturn);
//		model_saturn = glm::translate(model_saturn, point);
//		model_saturn = glm::rotate(model_saturn, glm::radians(SceneRotateY), glm::vec3(1.0f, 0.0f, 0.0f));
//		model_saturn = glm::rotate(model_saturn, glm::radians(SceneRotateX), glm::vec3(0.0f, 0.0f, 1.0f));
//		model_saturn = glm::translate(model_saturn, glm::vec3(xx, 0.0f, zz));
//		glm::vec3 SatrunPoint = glm::vec3(xx, 0.0f, zz);
//		PlanetsPositions[5] = glm::vec3(xx, 0.0f, zz);
//		model_saturn = glm::rotate(model_saturn, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.f));
//		model_saturn = glm::rotate(model_saturn, glm::radians(-34.7f), glm::vec3(0.0f, 1.0f, 0.f));
//		model_saturn = glm::rotate(model_saturn, (GLfloat)glfwGetTime() * glm::radians(-34.7f) * 4.48f, glm::vec3(0.0f, 0.0f, 1.f));
//		SimpleShader.setMat4("model", model_saturn);
//		Saturn.Draw();
//		/* SATURN */
//
//		/* URANUS */
//		glm::mat4 model_uranus;
//		xx = sin(glfwGetTime() * PlanetSpeed * 0.1f) * 100.0f * 8.0f * 1.3f;
//		zz = cos(glfwGetTime() * PlanetSpeed * 0.1f) * 100.0f * 8.0f * 1.3f;
//		glActiveTexture(GL_TEXTURE0);
//		glBindTexture(GL_TEXTURE_2D, texture_uranus);
//		model_uranus = glm::translate(model_uranus, point);
//		model_uranus = glm::rotate(model_uranus, glm::radians(SceneRotateY), glm::vec3(1.0f, 0.0f, 0.0f));
//		model_uranus = glm::rotate(model_uranus, glm::radians(SceneRotateX), glm::vec3(0.0f, 0.0f, 1.0f));
//		model_uranus = glm::translate(model_uranus, glm::vec3(xx, 0.0f, zz));
//		PlanetsPositions[6] = glm::vec3(xx, 0.0f, zz);
//		model_uranus = glm::rotate(model_uranus, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.f));
//		model_uranus = glm::rotate(model_uranus, glm::radians(-99.0f), glm::vec3(0.0f, 1.0f, 0.f));
//		model_uranus = glm::rotate(model_uranus, (GLfloat)glfwGetTime() * glm::radians(-99.0f) * 4.5f, glm::vec3(0.0f, 0.0f, 1.f));
//		SimpleShader.setMat4("model", model_uranus);
//		Uranus.Draw();
//		/* URANUS */
//
//		/* NEPTUNE */
//		glm::mat4 model_neptune;
//		xx = sin(glfwGetTime() * PlanetSpeed * 0.08f) * 100.0f * 9.0f * 1.3f;
//		zz = cos(glfwGetTime() * PlanetSpeed * 0.08f) * 100.0f * 9.0f * 1.3f;
//		glActiveTexture(GL_TEXTURE0);
//		glBindTexture(GL_TEXTURE_2D, texture_neptune);
//
//		model_neptune = glm::translate(model_neptune, point);
//		model_neptune = glm::rotate(model_neptune, glm::radians(SceneRotateY), glm::vec3(1.0f, 0.0f, 0.0f));
//		model_neptune = glm::rotate(model_neptune, glm::radians(SceneRotateX), glm::vec3(0.0f, 0.0f, 1.0f));
//		model_neptune = glm::translate(model_neptune, glm::vec3(xx, 0.0f, zz));
//		PlanetsPositions[7] = glm::vec3(xx, 0.0f, zz);
//		model_neptune = glm::rotate(model_neptune, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.f));
//		model_neptune = glm::rotate(model_neptune, glm::radians(-30.2f), glm::vec3(0.0f, 1.0f, 0.f));
//		model_neptune = glm::rotate(model_neptune, (GLfloat)glfwGetTime() * glm::radians(-30.2f) * 4.0f, glm::vec3(0.0f, 0.0f, 1.f));
//
//		SimpleShader.setMat4("model", model_neptune);
//		Neptune.Draw();
//		/* NEPTUNE */
//
//		glActiveTexture(GL_TEXTURE0);
//		glBindTexture(GL_TEXTURE_2D, texture_venus);
//
//		/* ORBITS */
//		glBindVertexArray(VAO_t);
//		glLineWidth(1.0f);
//		glm::mat4 modelorb;
//		for (float i = 2; i < 10; i++)
//		{
//			modelorb = glm::mat4(1);
//			modelorb = glm::translate(modelorb, point);
//			modelorb = glm::rotate(modelorb, glm::radians(SceneRotateY), glm::vec3(1.0f, 0.0f, 0.0f));
//			modelorb = glm::rotate(modelorb, glm::radians(SceneRotateX), glm::vec3(0.0f, 0.0f, 1.0f));
//			modelorb = glm::scale(modelorb, glm::vec3(i * 1.3f, i * 1.3f, i * 1.3f));
//			SimpleShader.setMat4("model", modelorb);
//			glDrawArrays(GL_LINE_LOOP, 0, (GLsizei)orbVert.size() / 3);
//
//		}
//		modelorb = glm::mat4(1);
//		modelorb = glm::rotate(modelorb, glm::radians(SceneRotateY), glm::vec3(1.0f, 0.0f, 0.0f));
//		modelorb = glm::rotate(modelorb, glm::radians(SceneRotateX), glm::vec3(0.0f, 0.0f, 1.0f));
//		modelorb = glm::translate(modelorb, EarthPoint);
//		modelorb = glm::scale(modelorb, glm::vec3(0.5f * 1.3f, 0.5f * 1.3f, 0.5f * 1.3f));
//		SimpleShader.setMat4("model", modelorb);
//		glDrawArrays(GL_LINE_LOOP, 0, (GLsizei)orbVert.size() / 3);
//		/* ORBITS */
//
//		/* SATURN RINGS */
//		glActiveTexture(GL_TEXTURE0);
//		glBindTexture(GL_TEXTURE_2D, texture_saturn_ring);
//		glLineWidth(2.0f);
//		GLfloat rr = 0.55f;
//		for (int i = 0; i < 25; i++)
//		{
//			modelorb = glm::mat4(1);
//
//			modelorb = glm::rotate(modelorb, glm::radians(SceneRotateY), glm::vec3(1.0f, 0.0f, 0.0f));
//			modelorb = glm::rotate(modelorb, glm::radians(SceneRotateX), glm::vec3(0.0f, 0.0f, 1.0f));
//			modelorb = glm::translate(modelorb, SatrunPoint);
//			modelorb = glm::rotate(modelorb, glm::radians(30.0f), glm::vec3(0.0f, 0.0f, 1.0f));
//			modelorb = glm::scale(modelorb, glm::vec3(rr, rr, rr));
//			SimpleShader.setMat4("model", modelorb);
//			glDrawArrays(GL_LINE_LOOP, 0, (GLsizei)orbVert.size() / 3);
//			if (i == 15)
//				rr += 0.030f;
//			else
//				rr += 0.01f;
//		}
//		glActiveTexture(GL_TEXTURE0);
//		glBindTexture(GL_TEXTURE_2D, texture_venus);
//		glBindVertexArray(0);
//		/* SATURN RINGS */
//
//
//		/* DRAW SKYBOX */
//		glDepthFunc(GL_LEQUAL);
//		SkyboxShader.Use();
//		view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
//		SkyboxShader.setMat4("view", view);
//		SkyboxShader.setMat4("projection", projection);
//		// skybox cube
//		glBindVertexArray(skyboxVAO);
//		glActiveTexture(GL_TEXTURE0);
//		if (SkyBoxExtra)
//			glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextureExtra);
//		else
//			glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
//		glDrawArrays(GL_TRIANGLES, 0, 36);
//		glBindVertexArray(0);
//		glDepthFunc(GL_LESS);
//		/* DRAW SKYBOX */
//
//		/* PLANET TRACKING + SHOW INFO OF PLANET */
//		switch (PlanetView)
//		{
//		case 1:
//			viewX = sin(glfwGetTime() * PlanetSpeed) * 100.0f * 3.5f * 1.3f;
//			viewZ = cos(glfwGetTime() * PlanetSpeed) * 100.0f * 3.5f * 1.3f;
//			viewPos = glm::vec3(viewX, 50.0f, viewZ);
//			view = glm::lookAt(viewPos, PlanetsPositions[0], glm::vec3(0.0f, 1.0f, 0.0f));
//			ShowInfo(TextShader);
//			break;
//
//		case 2:
//			viewX = sin(glfwGetTime() * PlanetSpeed * 0.75f) * 100.0f * 4.5f * 1.2f;
//			viewZ = cos(glfwGetTime() * PlanetSpeed * 0.75f) * 100.0f * 4.5f * 1.2f;
//			viewPos = glm::vec3(viewX, 50.0f, viewZ);
//			view = glm::lookAt(viewPos, PlanetsPositions[1], glm::vec3(0.0f, 1.0f, 0.0f));
//			ShowInfo(TextShader);
//			break;
//
//		case 3:
//			viewX = sin(glfwGetTime() * PlanetSpeed * 0.55f) * 100.0f * 5.5f * 1.2f;
//			viewZ = cos(glfwGetTime() * PlanetSpeed * 0.55f) * 100.0f * 5.5f * 1.2f;
//			viewPos = glm::vec3(viewX, 50.0f, viewZ);
//			view = glm::lookAt(viewPos, PlanetsPositions[2], glm::vec3(0.0f, 1.0f, 0.0f));
//			ShowInfo(TextShader);
//			break;
//
//		case 4:
//			viewX = sin(glfwGetTime() * PlanetSpeed * 0.35f) * 100.0f * 6.0f * 1.2f;
//			viewZ = cos(glfwGetTime() * PlanetSpeed * 0.35f) * 100.0f * 6.0f * 1.2f;
//			viewPos = glm::vec3(viewX, 20.0f, viewZ);
//			view = glm::lookAt(viewPos, PlanetsPositions[3], glm::vec3(0.0f, 1.0f, 0.0f));
//			ShowInfo(TextShader);
//			break;
//
//		case 5:
//			viewX = sin(glfwGetTime() * PlanetSpeed * 0.2f) * 100.0f * 7.5f * 1.3f;
//			viewZ = cos(glfwGetTime() * PlanetSpeed * 0.2f) * 100.0f * 7.5f * 1.3f;
//			viewPos = glm::vec3(viewX, 50.0f, viewZ);
//			view = glm::lookAt(viewPos, PlanetsPositions[4], glm::vec3(0.0f, 1.0f, 0.0f));
//			ShowInfo(TextShader);
//			break;
//
//		case 6:
//			viewX = sin(glfwGetTime() * PlanetSpeed * 0.15f) * 100.0f * 8.5f * 1.3f;
//			viewZ = cos(glfwGetTime() * PlanetSpeed * 0.15f) * 100.0f * 8.5f * 1.3f;
//			viewPos = glm::vec3(viewX, 50.0f, viewZ);
//			view = glm::lookAt(viewPos, PlanetsPositions[5], glm::vec3(0.0f, 1.0f, 0.0f));
//			ShowInfo(TextShader);
//			break;
//
//		case 7:
//			viewX = sin(glfwGetTime() * PlanetSpeed * 0.1f) * 100.0f * 9.5f * 1.3f;
//			viewZ = cos(glfwGetTime() * PlanetSpeed * 0.1f) * 100.0f * 9.5f * 1.3f;
//			viewPos = glm::vec3(viewX, 50.0f, viewZ);
//			view = glm::lookAt(viewPos, PlanetsPositions[6], glm::vec3(0.0f, 1.0f, 0.0f));
//			ShowInfo(TextShader);
//			break;
//
//		case 8:
//			viewX = sin(glfwGetTime() * PlanetSpeed * 0.08f) * 100.0f * 10.5f * 1.3f;
//			viewZ = cos(glfwGetTime() * PlanetSpeed * 0.08f) * 100.0f * 10.5f * 1.3f;
//			viewPos = glm::vec3(viewX, 50.0f, viewZ);
//			view = glm::lookAt(viewPos, PlanetsPositions[7], glm::vec3(0.0f, 1.0f, 0.0f));
//			ShowInfo(TextShader);
//			break;
//
//		case 0:
//			view = camera.GetViewMatrix();
//
//			RenderText(TextShader, "SOLAR SYSTEM ", 25.0f, SCREEN_HEIGHT - 30.0f, 0.50f, glm::vec3(0.7f, 0.7f, 0.11f));
//			RenderText(TextShader, "STARS: 1 (SUN) ", 25.0f, SCREEN_HEIGHT - 55.0f, 0.35f, glm::vec3(0.7f, 0.7f, 0.11f));
//			RenderText(TextShader, "PLANETS: 8 (MAYBE 9) ", 25.0f, SCREEN_HEIGHT - 80.0f, 0.35f, glm::vec3(0.7f, 0.7f, 0.11f));
//			RenderText(TextShader, "SATELLITES: 415 ", 25.0f, SCREEN_HEIGHT - 105.0f, 0.35f, glm::vec3(0.7f, 0.7f, 0.11f));
//			RenderText(TextShader, "COMMETS: 3441 ", 25.0f, SCREEN_HEIGHT - 130.0f, 0.35f, glm::vec3(0.7f, 0.7f, 0.11f));
//
//			if (camera.FreeCam)
//				RenderText(TextShader, "FREE CAM ", SCREEN_WIDTH - 200.0f, SCREEN_HEIGHT - 30.0f, 0.35f, glm::vec3(0.7f, 0.7f, 0.11f));
//			if (onFreeCam)
//				RenderText(TextShader, "STATIC CAM ", SCREEN_WIDTH - 200.0f, SCREEN_HEIGHT - 30.0f, 0.35f, glm::vec3(0.7f, 0.7f, 0.11f));
//			break;
//		}
//		if (PlanetView > 0)
//			RenderText(TextShader, "PLANET CAM ", SCREEN_WIDTH - 200.0f, SCREEN_HEIGHT - 30.0f, 0.35f, glm::vec3(0.7f, 0.7f, 0.11f));
//		/* PLANET TRACKING + SHOW INFO OF PLANET */
//
//
//		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
//		// -------------------------------------------------------------------------------
//		glfwSwapBuffers(window);
//		glfwPollEvents();
//	}
//
//	glDeleteVertexArrays(1, &VAO_t);
//	glDeleteBuffers(1, &VBO_t);
//	glfwTerminate();
//	return 0;
//}
//
//void processInput(GLFWwindow* window)
//{
//	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
//		glfwSetWindowShouldClose(window, true);
//
//	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
//		camera.ProcessKeyboard(FORWARD, deltaTime);
//	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
//		camera.ProcessKeyboard(BACKWARD, deltaTime);
//	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
//		camera.ProcessKeyboard(LEFT, deltaTime);
//	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
//		camera.ProcessKeyboard(RIGHT, deltaTime);
//
//	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
//		SkyBoxExtra = true;
//
//	if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS)
//	{
//		PlanetView = 0;
//		onFreeCam = false;
//		camera.FreeCam = true;
//	}
//
//	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
//	{
//		PlanetView = 0;
//		onFreeCam = true;
//		camera.FreeCam = false;
//		camera.Position = glm::vec3(0.0f, 250.0f, -450.0f);
//		camera.Yaw = 90.0f;
//		camera.Pitch = -40.0f;
//		camera.GetViewMatrix();
//		camera.ProcessMouseMovement(xoff, yoff);
//	}
//
//	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
//	{
//		PlanetView = 1;
//		Info.Name = "MERCURY";
//		Info.OrbitSpeed = "47,87";
//		Info.Mass = "0.32868";
//		Info.Gravity = "0.38";
//		onFreeCam = false;
//		camera.FreeCam = false;
//	}
//	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
//	{
//		PlanetView = 2;
//		Info.Name = "VENUS";
//		Info.OrbitSpeed = "35,02";
//		Info.Mass = "0.32868";
//		Info.Gravity = "0.90";
//		onFreeCam = false;
//		camera.FreeCam = false;
//	}
//	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
//	{
//		PlanetView = 3;
//		Info.Name = "EARTH";
//		Info.OrbitSpeed = "29,76";
//		Info.Mass = "5.97600";
//		Info.Gravity = "1";
//		onFreeCam = false;
//		camera.FreeCam = false;
//	}
//	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
//	{
//		PlanetView = 4;
//		Info.Name = "MARS";
//		Info.OrbitSpeed = "24,13";
//		Info.Mass = "0.63345";
//		Info.Gravity = "0.38";
//		onFreeCam = false;
//		camera.FreeCam = false;
//	}
//	if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
//	{
//		PlanetView = 5;
//		Info.Name = "JUPITER";
//		Info.OrbitSpeed = "13,07";
//		Info.Mass = "1876.64328";
//		Info.Gravity = "2.55";
//		onFreeCam = false;
//	}
//	if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS)
//	{
//		PlanetView = 6;
//		Info.Name = "SATURN";
//		Info.OrbitSpeed = "9,67";
//		Info.Mass = "561.80376";
//		Info.Gravity = "1.12";
//		onFreeCam = false;
//		camera.FreeCam = false;
//	}
//	if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS)
//	{
//		PlanetView = 7;
//		Info.Name = "URANUS";
//		Info.OrbitSpeed = "6,84";
//		Info.Mass = "86.05440";
//		Info.Gravity = "0.97";
//		onFreeCam = false;
//		camera.FreeCam = false;
//	}
//	if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS)
//	{
//		PlanetView = 8;
//		Info.Name = "NEPTUNE";
//		Info.OrbitSpeed = "5,48";
//		Info.Mass = "101.59200";
//		Info.Gravity = "1.17";
//		onFreeCam = false;
//		camera.FreeCam = false;
//	}
//
//}
//
//void framebuffer_size_callback(GLFWwindow* window, int width, int height)
//{
//	glViewport(0, 0, width, height);
//}
//
//unsigned int loadCubemap(std::vector<std::string> faces)
//{
//	unsigned int textureID;
//	glGenTextures(1, &textureID);
//	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
//
//	int width, height, nrChannels;
//
//	for (unsigned int i = 0; i < faces.size(); i++)
//	{
//
//		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
//
//		if (data)
//		{
//			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
//				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
//			);
//			stbi_image_free(data);
//		}
//		else
//		{
//			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
//			stbi_image_free(data);
//		}
//	}
//	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
//
//	return textureID;
//}
//
//unsigned int loadTexture(char const* path)
//{
//	unsigned int textureID;
//	glGenTextures(1, &textureID);
//
//	int width, height, nrComponents;
//	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
//	if (data)
//	{
//		GLenum format;
//		if (nrComponents == 1)
//			format = GL_RED;
//		else if (nrComponents == 3)
//			format = GL_RGB;
//		else if (nrComponents == 4)
//			format = GL_RGBA;
//
//		glBindTexture(GL_TEXTURE_2D, textureID);
//		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
//		glGenerateMipmap(GL_TEXTURE_2D);
//
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//
//		stbi_image_free(data);
//	}
//	else
//	{
//		std::cout << "Texture failed to load at path: " << path << std::endl;
//		stbi_image_free(data);
//	}
//
//	return textureID;
//}
//void RenderText(Shader& s, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
//{
//	// Activate corresponding render state	
//	s.Use();
//	glUniform3f(glGetUniformLocation(s.ID, "textColor"), color.x, color.y, color.z);
//	glActiveTexture(GL_TEXTURE0);
//	glBindVertexArray(textVAO);
//
//	// Iterate through all characters
//	std::string::const_iterator c;
//	for (c = text.begin(); c != text.end(); c++)
//	{
//		Character ch = Characters[*c];
//
//		GLfloat xpos = x + ch.Bearing.x * scale;
//		GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;
//
//		GLfloat w = ch.Size.x * scale;
//		GLfloat h = ch.Size.y * scale;
//		// Update VBO for each character
//		GLfloat vertices[6][4] = {
//			{ xpos,     ypos + h,   0.0, 0.0 },
//			{ xpos,     ypos,       0.0, 1.0 },
//			{ xpos + w, ypos,       1.0, 1.0 },
//
//			{ xpos,     ypos + h,   0.0, 0.0 },
//			{ xpos + w, ypos,       1.0, 1.0 },
//			{ xpos + w, ypos + h,   1.0, 0.0 }
//		};
//		// Render glyph texture over quad
//		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
//		// Update content of VBO memory
//		glBindBuffer(GL_ARRAY_BUFFER, textVBO);
//		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
//		glBindBuffer(GL_ARRAY_BUFFER, 0);
//		// Render quad
//		glDrawArrays(GL_TRIANGLES, 0, 6);
//		// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
//		x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
//	}
//	glBindVertexArray(0);
//	glBindTexture(GL_TEXTURE_2D, 0);
//}
//
//void ShowInfo(Shader& s)
//{
//	RenderText(s, "Planet: " + Info.Name, 25.0f, SCREEN_HEIGHT - 30.0f, 0.35f, glm::vec3(0.7f, 0.7f, 0.11f));
//	RenderText(s, "Avarage Orbital Speed (km/s): " + Info.OrbitSpeed, 25.0f, SCREEN_HEIGHT - 50.0f, 0.35f, glm::vec3(0.7f, 0.7f, 0.11f));
//	RenderText(s, "Mass (kg * 10^24): " + Info.Mass, 25.0f, SCREEN_HEIGHT - 70.0f, 0.35f, glm::vec3(0.7f, 0.7f, 0.11f));
//	RenderText(s, "Gravity (g): " + Info.Gravity, 25.0f, SCREEN_HEIGHT - 90.0f, 0.35f, glm::vec3(0.7f, 0.7f, 0.11f));
//    enum AppState {
//        START_PAGE,
//        MAIN_SIMULATION
//    };
//
//    AppState currentState = START_PAGE;
//    void RenderStartPage(Shader& textShader) {
//        // Clear the screen
//        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
//        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//        // Render your name
//        RenderText(textShader, "Your Name", SCREEN_WIDTH / 2.0f - 50.0f, SCREEN_HEIGHT / 2.0f + 20.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
//
//        // Render the start button
//        RenderText(textShader, "Start", SCREEN_WIDTH / 2.0f - 20.0f, SCREEN_HEIGHT / 2.0f - 20.0f, 0.75f, glm::vec3(1.0f, 1.0f, 1.0f));
//    }
//    int main() {
//        // ... (initialization code)
//
//        while (!glfwWindowShouldClose(window)) {
//            // Calculate delta time
//            GLfloat currentFrame = (GLfloat)glfwGetTime();
//            deltaTime = currentFrame - lastFrame;
//            lastFrame = currentFrame;
//
//            // Process input
//            processInput(window);
//
//            // Render based on the current state
//            if (currentState == START_PAGE) {
//                RenderStartPage(TextShader);
//
//                // Check for mouse click to start the simulation
//                if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
//                    double xpos, ypos;
//                    glfwGetCursorPos(window, &xpos, &ypos);
//
//                    // Check if the click is within the start button area
//                    if (xpos >= SCREEN_WIDTH / 2.0f - 20.0f && xpos <= SCREEN_WIDTH / 2.0f + 20.0f &&
//                        ypos >= SCREEN_HEIGHT / 2.0f - 20.0f && ypos <= SCREEN_HEIGHT / 2.0f + 20.0f) {
//                        currentState = MAIN_SIMULATION;
//                    }
//                }
//            } else if (currentState == MAIN_SIMULATION) {
//                // ... (existing rendering code for the solar system)
//            }
//
//            // Swap buffers and poll IO events
//            glfwSwapBuffers(window);
//            glfwPollEvents();
//        }
//
//        // ... (cleanup code)
//        return 0;
//    }
//    void processInput(GLFWwindow* window) {
//        if (currentState == MAIN_SIMULATION) {
//            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
//                glfwSetWindowShouldClose(window, true);
//
//            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
//                camera.ProcessKeyboard(FORWARD, deltaTime);
//            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
//                camera.ProcessKeyboard(BACKWARD, deltaTime);
//            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
//                camera.ProcessKeyboard(LEFT, deltaTime);
//            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
//                camera.ProcessKeyboard(RIGHT, deltaTime);
//
//            if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
//                SkyBoxExtra = true;
//
//            if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS) {
//                PlanetView = 0;
//                onFreeCam = false;
//                camera.FreeCam = true;
//            }
//
//            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
//                PlanetView = 0;
//                onFreeCam = true;
//                camera.FreeCam = false;
//                camera.Position = glm::vec3(0.0f, 250.0f, -450.0f);
//                camera.Yaw = 90.0f;
//                camera.Pitch = -40.0f;
//                camera.GetViewMatrix();
//                camera.ProcessMouseMovement(xoff, yoff);
//            }
//
//            if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
//                PlanetView = 1;
//                Info.Name = "MERCURY";
//                Info.OrbitSpeed = "47,87";
//                Info.Mass = "0.32868";
//                Info.Gravity = "0.38";
//                onFreeCam = false;
//                camera.FreeCam = false;
//            }
//            if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
//                PlanetView = 2;
//                Info.Name = "VENUS";
//                Info.OrbitSpeed = "35,02";
//                Info.Mass = "0.32868";
//                Info.Gravity = "0.90";
//                onFreeCam = false;
//                camera.FreeCam = false;
//            }
//            if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
//                PlanetView = 3;
//                Info.Name = "EARTH";
//                Info.OrbitSpeed = "29,76";
//                Info.Mass = "5.97600";
//                Info.Gravity = "1";
//                onFreeCam = false;
//                camera.FreeCam = false;
//            }
//            if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
//                PlanetView = 4;
//                Info.Name = "MARS";
//                Info.OrbitSpeed = "24,13";
//                Info.Mass = "0.63345";
//                Info.Gravity = "0.38";
//                onFreeCam = false;
//                camera.FreeCam = false;
//            }
//            if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) {
//                PlanetView = 5;
//                Info.Name = "JUPITER";
//                Info.OrbitSpeed = "13,07";
//                Info.Mass = "1876.64328";
//                Info.Gravity = "2.55";
//                onFreeCam = false;
//            }
//            if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) {
//                PlanetView = 6;
//                Info.Name = "SATURN";
//                Info.OrbitSpeed = "9,67";
//                Info.Mass = "561.80376";
//                Info.Gravity = "1.12";
//                onFreeCam = false;
//                camera.FreeCam = false;
//            }
//            if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) {
//                PlanetView = 7;
//                Info.Name = "URANUS";
//                Info.OrbitSpeed = "6,84";
//                Info.Mass = "86.05440";
//                Info.Gravity = "0.97";
//                onFreeCam = false;
//                camera.FreeCam = false;
//            }
//            if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS) {
//                PlanetView = 8;
//                Info.Name = "NEPTUNE";
//                Info.OrbitSpeed = "5,48";
//                Info.Mass = "101.59200";
//                Info.Gravity = "1.17";
//                onFreeCam = false;
//                camera.FreeCam = false;
//            }
//        }
//    }
//    class Planet {
//    public:
//        Planet(const std::string& name, float semiMajorAxis, float semiMinorAxis, float orbitSpeed, float rotationSpeed, float axialTilt, const std::string& texturePath);
//        void Update(float deltaTime);
//        void Draw(Shader& shader);
//
//        glm::vec3 GetPosition() const;
//
//    private:
//        std::string name;
//        float semiMajorAxis;
//        float semiMinorAxis;
//        float orbitSpeed;
//        float rotationSpeed;
//        float axialTilt;
//        glm::vec3 position;
//        Sphere sphere;
//        unsigned int textureID;
//        glm::mat4 modelMatrix;
//    };
//
//    Planet::Planet(const std::string& name, float semiMajorAxis, float semiMinorAxis, float orbitSpeed, float rotationSpeed, float axialTilt, const std::string& texturePath)
//        : name(name), semiMajorAxis(semiMajorAxis), semiMinorAxis(semiMinorAxis), orbitSpeed(orbitSpeed), rotationSpeed(rotationSpeed), axialTilt(axialTilt), sphere(semiMajorAxis, 36, 18) {
//        // Load texture
//        int width, height, nrComponents;
//        unsigned char* data = stbi_load(texturePath.c_str(), &width, &height, &nrComponents, 0);
//        if (data) {
//            GLenum format;
//            if (nrComponents == 1)
//                format = GL_RED;
//            else if (nrComponents == 3)
//                format = GL_RGB;
//            else if (nrComponents == 4)
//                format = GL_RGBA;
//
//            glGenTextures(1, &textureID);
//            glBindTexture(GL_TEXTURE_2D, textureID);
//            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
//            glGenerateMipmap(GL_TEXTURE_2D);
//
//            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//
//            stbi_image_free(data);
//        } else {
//            std::cout << "Texture failed to load at path: " << texturePath << std::endl;
//            stbi_image_free(data);
//        }
//    }
//
//    void Planet::Update(float deltaTime) {
//        float angle = glfwGetTime() * orbitSpeed;
//        position.x = semiMajorAxis * cos(angle);
//        position.z = semiMinorAxis * sin(angle);
//
//        modelMatrix = glm::mat4(1.0f);
//        modelMatrix = glm::translate(modelMatrix, position);
//        modelMatrix = glm::rotate(modelMatrix, glm::radians(axialTilt), glm::vec3(0.0f, 0.0f, 1.0f));
//        modelMatrix = glm::rotate(modelMatrix, (GLfloat)glfwGetTime() * glm::radians(rotationSpeed), glm::vec3(0.0f, 1.0f, 0.0f));
//    }
//
//    void Planet::Draw(Shader& shader) {
//        shader.Use();
//        shader.setMat4("model", modelMatrix);
//        glActiveTexture(GL_TEXTURE0);
//        glBindTexture(GL_TEXTURE_2D, textureID);
//        sphere.Draw();
//    }
//
//    glm::vec3 Planet::GetPosition() const {
//        return position;
//    }
//    std::vector<Planet> planets;
//    planets.emplace_back("Mercury", 57.9f, 56.7f, 47.87f, 10.83f, 0.034f, "texture/2k_mercury.jpg");
//    planets.emplace_back("Venus", 108.2f, 107.5f, 35.02f, -6.52f, 177.36f, "texture/2k_venus.jpg");
//    planets.emplace_back("Earth", 149.6f, 148.9f, 29.78f, 7.29f, 23.44f, "texture/2k_earth.jpg");
//    planets.emplace_back("Mars", 227.9f, 226.9f, 24.13f, 7.09f, 25.19f, "texture/2k_mars.jpg");
//    planets.emplace_back("Jupiter", 778.3f, 776.9f, 13.07f, 12.6f, 3.13f, "texture/2k_jupiter.jpg");
//    planets.emplace_back("Saturn", 1427.0f, 1423.5f, 9.69f, 9.87f, 26.73f, "texture/2k_saturn.jpg");
//    planets.emplace_back("Uranus", 2871.0f, 2865.0f, 6.81f, -6.49f, 97.77f, "texture/2k_uranus.jpg");
//    planets.emplace_back("Neptune", 4497.1f, 4480.0f, 5.43f, 5.43f, 28.32f, "texture/2k_neptune.jpg");
//    unsigned int galaxyTexture = loadTexture("resources/galaxy/galaxy.jpg");
//    void RenderGalaxyBackground(Shader& shader) {
//        shader.Use();
//        glActiveTexture(GL_TEXTURE0);
//        glBindTexture(GL_TEXTURE_2D, galaxyTexture);
//        // Render a quad or sphere with the galaxy texture
//    }
//    glm::mat4 galacticRotation = glm::rotate(glm::mat4(1.0f), (GLfloat)glfwGetTime() * glm::radians(0.1f), glm::vec3(0.0f, 1.0f, 0.0f));
//    view = galacticRotation * camera.GetViewMatrix();
//}