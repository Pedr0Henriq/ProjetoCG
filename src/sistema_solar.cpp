/*
 * Projeto Final - Introducao a Computacao Grafica
 * Tema: Sistema Solar 3D em OpenGL 2.1
 *
 * Integrantes:
 * Gabriel Rafá Martins Freire       - 20230145310
 * Tobias Freire Numeriano           - 20230012378
 * Pedro Henrique de Araujo Lima     - 20220005950
 *
 * O codigo foi mantido em um unico arquivo para facilitar estudo, alteracoes
 * e apresentacao. Os parametros principais dos planetas estao concentrados
 * na funcao createPlanets().
 */

#include <GL/glut.h>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

struct Vec3 {
    float x, y, z;
};

enum TexturePattern {
    TEX_ROCKY,
    TEX_EARTH,
    TEX_GAS,
    TEX_ICE,
    TEX_SUN
};

struct Planet {
    const char* name;
    float radius;
    float orbitRadius;
    float orbitSpeed;      // graus por segundo de simulacao
    float rotationSpeed;   // graus por segundo de simulacao
    float axialTilt;
    float baseR, baseG, baseB;
    float accentR, accentG, accentB;
    TexturePattern pattern;
    GLuint textureId;
};

// -----------------------------
// PARAMETROS FACILMENTE ALTERAVEIS
// -----------------------------
static const int WINDOW_WIDTH = 1100;
static const int WINDOW_HEIGHT = 720;
static const int TEXTURE_W = 96;
static const int TEXTURE_H = 48;
static const int STAR_COUNT = 700;
static const float SUN_RADIUS = 2.4f;

static bool paused = false;
static bool showOrbits = true;
static bool useLighting = true;
static bool useTextures = true;
static bool bezierTour = false;
static bool screenshotMode = false;

static float timeScale = 1.0f;
static float simulationTime = 0.0f;
static int lastTimeMs = 0;

static float cameraYaw = 35.0f;
static float cameraPitch = 22.0f;
static float cameraDistance = 38.0f;
static float bezierT = 0.0f;

static GLUquadric* sphereQuadric = NULL;
static std::vector<Planet> planets;
static std::vector<Vec3> stars;
static void saveScreenshot(const char* filename, GLenum readBuffer = GL_FRONT);

// P0, P1, P2, P3 da curva de Bezier usada pela camera.
static Vec3 bezierPoints[4] = {
    { 30.0f, 12.0f,  28.0f },
    { 10.0f, 28.0f, -32.0f },
    {-32.0f, 10.0f, -18.0f },
    {-18.0f, 18.0f,  34.0f }
};

// ------------------------------------------------------------
// Tabela principal do projeto.
// Para alterar tamanho, distancia, velocidade e aparencia,
// basta editar os valores abaixo.
// As escalas sao didaticas e nao astronomicamente proporcionais.
// ------------------------------------------------------------
static void createPlanets() {
    planets.clear();
    planets.push_back({"Mercurio", 0.34f,  4.0f, 38.0f,  55.0f,  0.0f, 0.45f,0.40f,0.35f, 0.72f,0.65f,0.55f, TEX_ROCKY, 0});
    planets.push_back({"Venus",    0.53f,  5.6f, 28.0f, -18.0f,177.0f, 0.78f,0.58f,0.28f, 0.95f,0.78f,0.42f, TEX_ROCKY, 0});
    planets.push_back({"Terra",    0.58f,  7.5f, 23.0f,  70.0f, 23.5f, 0.15f,0.38f,0.80f, 0.20f,0.70f,0.35f, TEX_EARTH, 0});
    planets.push_back({"Marte",    0.43f,  9.4f, 18.0f,  66.0f, 25.0f, 0.72f,0.24f,0.10f, 0.95f,0.45f,0.20f, TEX_ROCKY, 0});
    planets.push_back({"Jupiter",  1.26f, 12.6f, 10.5f, 105.0f,  3.0f, 0.78f,0.58f,0.40f, 0.45f,0.25f,0.15f, TEX_GAS,   0});
    planets.push_back({"Saturno",  1.08f, 16.0f,  7.8f,  95.0f, 26.7f, 0.82f,0.72f,0.48f, 0.55f,0.42f,0.22f, TEX_GAS,   0});
    planets.push_back({"Urano",    0.82f, 19.4f,  5.7f, -70.0f, 98.0f, 0.42f,0.78f,0.82f, 0.72f,0.95f,0.95f, TEX_ICE,   0});
    planets.push_back({"Netuno",   0.79f, 22.4f,  4.4f,  72.0f, 28.3f, 0.12f,0.28f,0.82f, 0.24f,0.55f,1.00f, TEX_ICE,   0});
}

static float clamp01(float v) {
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

static unsigned char toByte(float v) {
    return static_cast<unsigned char>(clamp01(v) * 255.0f);
}

static float pseudoNoise(int x, int y, int seed) {
    unsigned int n = static_cast<unsigned int>(x * 374761393u + y * 668265263u + seed * 2246822519u);
    n = (n ^ (n >> 13u)) * 1274126177u;
    n ^= (n >> 16u);
    return (n & 0xFFFFu) / 65535.0f;
}

static GLuint createProceduralTexture(const Planet& p, int seed) {
    std::vector<GLubyte> img(TEXTURE_W * TEXTURE_H * 3);

    for (int y = 0; y < TEXTURE_H; ++y) {
        for (int x = 0; x < TEXTURE_W; ++x) {
            float u = static_cast<float>(x) / static_cast<float>(TEXTURE_W - 1);
            float v = static_cast<float>(y) / static_cast<float>(TEXTURE_H - 1);
            float n = pseudoNoise(x, y, seed);
            float mix = 0.0f;

            switch (p.pattern) {
                case TEX_EARTH: {
                    float continent = std::sin(u * 18.0f + std::sin(v * 11.0f) * 2.0f)
                                    + std::cos(v * 17.0f + u * 5.0f)
                                    + (n - 0.5f) * 1.8f;
                    mix = continent > 0.65f ? 0.92f : 0.08f;
                    // Calotas polares simples.
                    if (v < 0.10f || v > 0.90f) mix = 0.55f;
                    break;
                }
                case TEX_GAS:
                    mix = 0.5f + 0.45f * std::sin(v * 55.0f + std::sin(u * 7.0f) * 1.3f);
                    mix += (n - 0.5f) * 0.12f;
                    break;
                case TEX_ICE:
                    mix = 0.35f + 0.25f * std::sin(v * 26.0f) + (n - 0.5f) * 0.18f;
                    break;
                case TEX_SUN:
                    mix = 0.45f + 0.35f * std::sin(u * 25.0f + v * 19.0f)
                                  + 0.20f * (n - 0.5f);
                    break;
                case TEX_ROCKY:
                default:
                    mix = 0.20f + 0.60f * n;
                    mix += 0.10f * std::sin(u * 30.0f + v * 13.0f);
                    break;
            }

            mix = clamp01(mix);
            float r = p.baseR * (1.0f - mix) + p.accentR * mix;
            float g = p.baseG * (1.0f - mix) + p.accentG * mix;
            float b = p.baseB * (1.0f - mix) + p.accentB * mix;

            int idx = (y * TEXTURE_W + x) * 3;
            img[idx + 0] = toByte(r);
            img[idx + 1] = toByte(g);
            img[idx + 2] = toByte(b);
        }
    }

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, TEXTURE_W, TEXTURE_H,
                      GL_RGB, GL_UNSIGNED_BYTE, &img[0]);
    return tex;
}

static void createTextures() {
    for (size_t i = 0; i < planets.size(); ++i) {
        planets[i].textureId = createProceduralTexture(planets[i], static_cast<int>(i) + 11);
    }
}

static void createStars() {
    stars.clear();
    std::srand(42); // resultado deterministico
    for (int i = 0; i < STAR_COUNT; ++i) {
        float theta = static_cast<float>(std::rand()) / RAND_MAX * 2.0f * static_cast<float>(M_PI);
        float phi = static_cast<float>(std::rand()) / RAND_MAX * static_cast<float>(M_PI);
        float r = 55.0f + static_cast<float>(std::rand()) / RAND_MAX * 25.0f;
        stars.push_back({
            r * std::sin(phi) * std::cos(theta),
            r * std::cos(phi),
            r * std::sin(phi) * std::sin(theta)
        });
    }
}

static Vec3 bezierPoint(float t) {
    float u = 1.0f - t;
    float b0 = u * u * u;
    float b1 = 3.0f * u * u * t;
    float b2 = 3.0f * u * t * t;
    float b3 = t * t * t;

    return {
        b0 * bezierPoints[0].x + b1 * bezierPoints[1].x + b2 * bezierPoints[2].x + b3 * bezierPoints[3].x,
        b0 * bezierPoints[0].y + b1 * bezierPoints[1].y + b2 * bezierPoints[2].y + b3 * bezierPoints[3].y,
        b0 * bezierPoints[0].z + b1 * bezierPoints[1].z + b2 * bezierPoints[2].z + b3 * bezierPoints[3].z
    };
}

static void applyCamera() {
    if (bezierTour) {
        Vec3 p = bezierPoint(bezierT);
        gluLookAt(p.x, p.y, p.z,
                  0.0, 0.0, 0.0,
                  0.0, 1.0, 0.0);
        return;
    }

    float yaw = cameraYaw * static_cast<float>(M_PI) / 180.0f;
    float pitch = cameraPitch * static_cast<float>(M_PI) / 180.0f;

    float x = cameraDistance * std::cos(pitch) * std::sin(yaw);
    float y = cameraDistance * std::sin(pitch);
    float z = cameraDistance * std::cos(pitch) * std::cos(yaw);

    gluLookAt(x, y, z,
              0.0, 0.0, 0.0,
              0.0, 1.0, 0.0);
}

static void drawStars() {
    GLboolean lightingWasOn = glIsEnabled(GL_LIGHTING);
    GLboolean textureWasOn = glIsEnabled(GL_TEXTURE_2D);
    if (lightingWasOn) glDisable(GL_LIGHTING);
    if (textureWasOn) glDisable(GL_TEXTURE_2D);

    glPointSize(1.6f);
    glColor3f(0.92f, 0.94f, 1.0f);
    glBegin(GL_POINTS);
    for (size_t i = 0; i < stars.size(); ++i)
        glVertex3f(stars[i].x, stars[i].y, stars[i].z);
    glEnd();

    if (textureWasOn) glEnable(GL_TEXTURE_2D);
    if (lightingWasOn) glEnable(GL_LIGHTING);
}

static void drawOrbit(float radius) {
    GLboolean lightingWasOn = glIsEnabled(GL_LIGHTING);
    GLboolean textureWasOn = glIsEnabled(GL_TEXTURE_2D);
    if (lightingWasOn) glDisable(GL_LIGHTING);
    if (textureWasOn) glDisable(GL_TEXTURE_2D);

    glColor3f(0.22f, 0.25f, 0.32f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 180; ++i) {
        float a = 2.0f * static_cast<float>(M_PI) * i / 180.0f;
        glVertex3f(std::cos(a) * radius, 0.0f, std::sin(a) * radius);
    }
    glEnd();

    if (textureWasOn) glEnable(GL_TEXTURE_2D);
    if (lightingWasOn) glEnable(GL_LIGHTING);
}

static void drawTexturedSphere(float radius, GLuint textureId, float r, float g, float b) {
    glColor3f(r, g, b);

    if (useTextures) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        gluQuadricTexture(sphereQuadric, GL_TRUE);
    } else {
        glDisable(GL_TEXTURE_2D);
        gluQuadricTexture(sphereQuadric, GL_FALSE);
    }

    gluSphere(sphereQuadric, radius, 32, 20);
}

static void drawSaturnRing() {
    GLboolean lightingWasOn = glIsEnabled(GL_LIGHTING);
    GLboolean textureWasOn = glIsEnabled(GL_TEXTURE_2D);
    if (lightingWasOn) glDisable(GL_LIGHTING);
    if (textureWasOn) glDisable(GL_TEXTURE_2D);

    glColor4f(0.72f, 0.63f, 0.42f, 0.85f);
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= 100; ++i) {
        float a = 2.0f * static_cast<float>(M_PI) * i / 100.0f;
        float c = std::cos(a), s = std::sin(a);
        glVertex3f(c * 1.48f, 0.0f, s * 1.48f);
        glVertex3f(c * 2.05f, 0.0f, s * 2.05f);
    }
    glEnd();

    if (textureWasOn) glEnable(GL_TEXTURE_2D);
    if (lightingWasOn) glEnable(GL_LIGHTING);
}

static void drawMoon(float earthOrbitAngle) {
    // Transformacao hierarquica: a Lua herda a transformacao da Terra.
    glPushMatrix();
    float moonAngle = earthOrbitAngle * 4.0f + simulationTime * 30.0f;
    glRotatef(moonAngle, 0.0f, 1.0f, 0.0f);
    glTranslatef(1.15f, 0.08f, 0.0f);
    glColor3f(0.72f, 0.72f, 0.72f);
    glDisable(GL_TEXTURE_2D);
    gluQuadricTexture(sphereQuadric, GL_FALSE);
    gluSphere(sphereQuadric, 0.16f, 18, 12);
    glPopMatrix();
}

static void drawSun() {
    GLboolean lightingWasOn = glIsEnabled(GL_LIGHTING);
    if (lightingWasOn) glDisable(GL_LIGHTING);

    glDisable(GL_TEXTURE_2D);
    glColor3f(1.0f, 0.78f, 0.18f);
    gluQuadricTexture(sphereQuadric, GL_FALSE);
    gluSphere(sphereQuadric, SUN_RADIUS, 42, 28);

    if (lightingWasOn) glEnable(GL_LIGHTING);
}

static void drawPlanet(const Planet& p, size_t index) {
    float orbitAngle = std::fmod(simulationTime * p.orbitSpeed + static_cast<float>(index) * 34.0f, 360.0f);
    float spinAngle = std::fmod(simulationTime * p.rotationSpeed, 360.0f);

    glPushMatrix();
        glRotatef(orbitAngle, 0.0f, 1.0f, 0.0f);
        glTranslatef(p.orbitRadius, 0.0f, 0.0f);

        // Corrige a orientacao para a rotacao propria do planeta nao depender da orbita.
        glRotatef(-orbitAngle, 0.0f, 1.0f, 0.0f);
        glRotatef(p.axialTilt, 0.0f, 0.0f, 1.0f);
        glRotatef(spinAngle, 0.0f, 1.0f, 0.0f);

        drawTexturedSphere(p.radius, p.textureId, p.baseR, p.baseG, p.baseB);

        if (std::strcmp(p.name, "Terra") == 0)
            drawMoon(orbitAngle);

        if (std::strcmp(p.name, "Saturno") == 0) {
            glDisable(GL_TEXTURE_2D);
            drawSaturnRing();
        }
    glPopMatrix();
}

static void configureLighting() {
    if (!useLighting) {
        glDisable(GL_LIGHTING);
        return;
    }

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // O Sol esta na origem. A posicao e definida apos a camera ser aplicada.
    GLfloat lightPosition[] = {0.0f, 0.0f, 0.0f, 1.0f};
    GLfloat ambient[]  = {0.08f, 0.08f, 0.10f, 1.0f};
    GLfloat diffuse[]  = {1.00f, 0.94f, 0.78f, 1.0f};
    GLfloat specular[] = {0.75f, 0.75f, 0.75f, 1.0f};

    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
}

static void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    applyCamera();
    configureLighting();

    drawStars();

    if (showOrbits) {
        for (size_t i = 0; i < planets.size(); ++i)
            drawOrbit(planets[i].orbitRadius);
    }

    drawSun();

    if (useLighting) glEnable(GL_LIGHTING);
    for (size_t i = 0; i < planets.size(); ++i)
        drawPlanet(planets[i], i);

    glDisable(GL_TEXTURE_2D);

    // Modo automatico usado apenas para gerar a imagem do README/testes.
    static int renderedFrames = 0;
    if (screenshotMode && ++renderedFrames >= 3) {
        saveScreenshot("screenshot.ppm", GL_BACK);
        std::exit(0);
    }

    glutSwapBuffers();
}

static void reshape(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, static_cast<GLfloat>(w) / static_cast<GLfloat>(h), 0.5, 140.0);

    glMatrixMode(GL_MODELVIEW);
}

static void printHelp() {
    std::cout << "\n=== SISTEMA SOLAR 3D - CONTROLES ===\n"
              << "ESPACO : pausa/continua\n"
              << "+ / -  : aumenta/diminui velocidade\n"
              << "1      : camera padrao\n"
              << "2      : camera de cima\n"
              << "3      : camera aproximada\n"
              << "C      : ativa/desativa passeio Bezier\n"
              << "L      : ativa/desativa iluminacao\n"
              << "T      : ativa/desativa texturas\n"
              << "O      : mostra/esconde orbitas\n"
              << "R      : restaura camera e velocidade\n"
              << "P      : salva screenshot.ppm\n"
              << "Setas  : gira camera\n"
              << "PageUp/PageDown : zoom\n"
              << "ESC    : sair\n\n";
}

static void saveScreenshot(const char* filename, GLenum readBuffer) {
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);
    std::vector<unsigned char> pixels(w * h * 3);

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer(readBuffer);
    glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, &pixels[0]);

    FILE* f = std::fopen(filename, "wb");
    if (!f) {
        std::cerr << "Nao foi possivel salvar " << filename << "\n";
        return;
    }

    std::fprintf(f, "P6\n%d %d\n255\n", w, h);
    // OpenGL le de baixo para cima; o PPM e salvo de cima para baixo.
    for (int y = h - 1; y >= 0; --y)
        std::fwrite(&pixels[y * w * 3], 1, w * 3, f);
    std::fclose(f);
    std::cout << "Imagem salva em: " << filename << "\n";
}

static void keyboard(unsigned char key, int, int) {
    switch (key) {
        case 27: std::exit(0); break;
        case ' ': paused = !paused; break;
        case '+': case '=': timeScale *= 1.35f; break;
        case '-': case '_': timeScale /= 1.35f; if (timeScale < 0.05f) timeScale = 0.05f; break;
        case '1':
            bezierTour = false;
            cameraYaw = 35.0f; cameraPitch = 22.0f; cameraDistance = 38.0f;
            break;
        case '2':
            bezierTour = false;
            cameraYaw = 0.0f; cameraPitch = 82.0f; cameraDistance = 40.0f;
            break;
        case '3':
            bezierTour = false;
            cameraYaw = 50.0f; cameraPitch = 10.0f; cameraDistance = 22.0f;
            break;
        case 'c': case 'C': bezierTour = !bezierTour; break;
        case 'l': case 'L': useLighting = !useLighting; break;
        case 't': case 'T': useTextures = !useTextures; break;
        case 'o': case 'O': showOrbits = !showOrbits; break;
        case 'r': case 'R':
            bezierTour = false;
            cameraYaw = 35.0f; cameraPitch = 22.0f; cameraDistance = 38.0f;
            timeScale = 1.0f;
            break;
        case 'p': case 'P': saveScreenshot("screenshot.ppm"); break;
        case 'h': case 'H': printHelp(); break;
        default: break;
    }
    glutPostRedisplay();
}

static void specialKeys(int key, int, int) {
    bezierTour = false;
    switch (key) {
        case GLUT_KEY_LEFT:      cameraYaw -= 4.0f; break;
        case GLUT_KEY_RIGHT:     cameraYaw += 4.0f; break;
        case GLUT_KEY_UP:        cameraPitch += 3.0f; break;
        case GLUT_KEY_DOWN:      cameraPitch -= 3.0f; break;
        case GLUT_KEY_PAGE_UP:   cameraDistance -= 1.5f; break;
        case GLUT_KEY_PAGE_DOWN: cameraDistance += 1.5f; break;
        default: break;
    }

    if (cameraPitch > 88.0f) cameraPitch = 88.0f;
    if (cameraPitch < -30.0f) cameraPitch = -30.0f;
    if (cameraDistance < 10.0f) cameraDistance = 10.0f;
    if (cameraDistance > 75.0f) cameraDistance = 75.0f;
    glutPostRedisplay();
}

static void idle() {
    int now = glutGet(GLUT_ELAPSED_TIME);
    float dt = (now - lastTimeMs) / 1000.0f;
    lastTimeMs = now;

    if (dt > 0.1f) dt = 0.1f;

    if (!paused) {
        simulationTime += dt * timeScale;
        if (bezierTour) {
            bezierT += dt * 0.055f;
            if (bezierT > 1.0f) bezierT -= 1.0f;
        }
    }

    glutPostRedisplay();
}

static void initOpenGL() {
    glClearColor(0.005f, 0.005f, 0.018f, 1.0f);
    glShadeModel(GL_SMOOTH);

    // Pratica 03: z-buffer para oclusao correta entre objetos 3D.
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    GLfloat specular[] = {0.35f, 0.35f, 0.35f, 1.0f};
    GLfloat shininess[] = {24.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shininess);

    sphereQuadric = gluNewQuadric();
    gluQuadricNormals(sphereQuadric, GLU_SMOOTH);

    createPlanets();
    createTextures();
    createStars();
    printHelp();
}

int main(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--screenshot") == 0)
            screenshotMode = true;
    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(80, 50);
    glutCreateWindow("Sistema Solar 3D - OpenGL 2.1");

    initOpenGL();
    lastTimeMs = glutGet(GLUT_ELAPSED_TIME);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutIdleFunc(idle);

    glutMainLoop();
    return 0;
}
