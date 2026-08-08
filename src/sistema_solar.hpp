#ifndef SISTEMA_SOLAR_HPP
#define SISTEMA_SOLAR_HPP

// Cabecalho compartilhado pelos modulos do Sistema Solar 3D.
// O projeto usa o pipeline fixo compativel com OpenGL 2.1 e GLUT/GLU.
#include <GL/glut.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Vetor tridimensional simples, usado para estrelas e pontos de Bezier.
struct Vec3 {
    float x, y, z;
};

// Categorias de textura procedural utilizadas para variar a aparencia
// dos diferentes tipos de corpos celestes.
enum TexturePattern {
    TEX_ROCKY,
    TEX_EARTH,
    TEX_GAS,
    TEX_ICE,
    TEX_SUN
};

// Dados necessarios para animar e renderizar um planeta.
struct Planet {
    const char* name;
    float radius;
    float orbitRadius;
    float orbitSpeed;
    float rotationSpeed;
    float axialTilt;
    float baseR, baseG, baseB;
    float accentR, accentG, accentB;
    TexturePattern pattern;
    GLuint textureId;
};

// Constantes de configuracao da janela, texturas e cena.
static const int WINDOW_WIDTH = 1100;
static const int WINDOW_HEIGHT = 720;
static const int TEXTURE_W = 96;
static const int TEXTURE_H = 48;
static const int STAR_COUNT = 700;
static const float SUN_RADIUS = 2.4f;

// Estados globais da simulacao.
extern bool paused;
extern bool showOrbits;
extern bool useLighting;
extern bool useTextures;
extern bool bezierTour;
extern bool screenshotMode;

extern float timeScale;
extern float simulationTime;
extern int lastTimeMs;

// Parametros da camera.
extern float cameraYaw;
extern float cameraPitch;
extern float cameraDistance;
extern float bezierT;

// Recursos e colecoes compartilhados entre os modulos.
extern GLUquadric* sphereQuadric;
extern std::vector<Planet> planets;
extern std::vector<Vec3> stars;

// engine.cpp: dados da cena, texturas, estrelas e camera.
void createPlanets();
float clamp01(float v);
unsigned char toByte(float v);
float pseudoNoise(int x, int y, int seed);
GLuint createProceduralTexture(const Planet& p, int seed);
void createTextures();
void createStars();
Vec3 bezierPoint(float t);
void applyCamera();

// render.cpp: desenho da cena e projecao.
void drawStars();
void drawOrbit(float radius);
void drawTexturedSphere(float radius, GLuint textureId, float r, float g, float b);
void drawSaturnRing();
void drawMoon(float earthOrbitAngle);
void drawSun();
void drawPlanet(const Planet& p, size_t index);
void configureLighting();
void display();
void reshape(int w, int h);

// input.cpp: comandos, captura, animacao e inicializacao do OpenGL.
void printHelp();
void saveScreenshot(const char* filename, GLenum readBuffer = GL_FRONT);
void keyboard(unsigned char key, int, int);
void specialKeys(int key, int, int);
void idle();
void initOpenGL();

#endif
