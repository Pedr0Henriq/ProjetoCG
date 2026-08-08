#include "sistema_solar.hpp"

// Desenha o campo de estrelas como pontos. Iluminacao e textura sao
// temporariamente desativadas para manter as estrelas uniformemente visiveis.
void drawStars() {
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

// Desenha uma orbita circular no plano XZ usando um GL_LINE_LOOP.
void drawOrbit(float radius) {
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

// Renderiza uma esfera GLU. Quando texturas estao ativadas, GL_MODULATE
// combina a textura com a cor/material e permite resposta a iluminacao.
void drawTexturedSphere(float radius, GLuint textureId, float r, float g, float b) {
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

// Desenha os aneis de Saturno como uma faixa circular formada por um
// GL_QUAD_STRIP. A faixa e desenhada sem iluminacao nem textura.
void drawSaturnRing() {
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

// Desenha a Lua no sistema de coordenadas da Terra. Como esta funcao e
// chamada dentro da matriz da Terra, a Lua herda sua translacao orbital,
// exemplificando transformacao hierarquica Sol -> Terra -> Lua.
void drawMoon(float earthOrbitAngle) {
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

// Desenha o Sol na origem. A iluminacao e desativada durante seu desenho
// para que ele apareca emissivo e nao seja sombreado pela propria luz.
void drawSun() {
    GLboolean lightingWasOn = glIsEnabled(GL_LIGHTING);
    if (lightingWasOn) glDisable(GL_LIGHTING);

    glDisable(GL_TEXTURE_2D);
    glColor3f(1.0f, 0.78f, 0.18f);
    gluQuadricTexture(sphereQuadric, GL_FALSE);
    gluSphere(sphereQuadric, SUN_RADIUS, 42, 28);

    if (lightingWasOn) glEnable(GL_LIGHTING);
}

// Aplica a cadeia de transformacoes de cada planeta: translacao orbital,
// inclinacao do eixo e rotacao em torno do proprio eixo. glPushMatrix() e
// glPopMatrix() isolam as transformacoes para que um planeta nao afete outro.
void drawPlanet(const Planet& p, size_t index) {
    float orbitAngle = std::fmod(simulationTime * p.orbitSpeed + static_cast<float>(index) * 34.0f, 360.0f);
    float spinAngle = std::fmod(simulationTime * p.rotationSpeed, 360.0f);

    glPushMatrix();
        glRotatef(orbitAngle, 0.0f, 1.0f, 0.0f);
        glTranslatef(p.orbitRadius, 0.0f, 0.0f);
        glRotatef(-orbitAngle, 0.0f, 1.0f, 0.0f);
        glRotatef(p.axialTilt, 0.0f, 0.0f, 1.0f);
        glRotatef(spinAngle, 0.0f, 1.0f, 0.0f);

        drawTexturedSphere(p.radius, p.textureId, p.baseR, p.baseG, p.baseB);

        // Terra possui uma Lua e Saturno possui aneis como detalhes extras.
        if (std::strcmp(p.name, "Terra") == 0)
            drawMoon(orbitAngle);

        if (std::strcmp(p.name, "Saturno") == 0) {
            glDisable(GL_TEXTURE_2D);
            drawSaturnRing();
        }
    glPopMatrix();
}

// Configura uma luz posicional na origem, representando o Sol, com
// componentes ambiente, difusa e especular.
void configureLighting() {
    if (!useLighting) {
        glDisable(GL_LIGHTING);
        return;
    }

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    GLfloat lightPosition[] = {0.0f, 0.0f, 0.0f, 1.0f};
    GLfloat ambient[]  = {0.08f, 0.08f, 0.10f, 1.0f};
    GLfloat diffuse[]  = {1.00f, 0.94f, 0.78f, 1.0f};
    GLfloat specular[] = {0.75f, 0.75f, 0.75f, 1.0f};

    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
}

// Callback principal de renderizacao. O color buffer e o depth buffer sao
// limpos a cada quadro antes da configuracao de camera, luz e objetos 3D.
void display() {
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

    // O modo --screenshot espera alguns quadros para garantir que a janela
    // esteja pronta, salva o back buffer em PPM e encerra a aplicacao.
    static int renderedFrames = 0;
    if (screenshotMode && ++renderedFrames >= 3) {
        saveScreenshot("screenshot.ppm", GL_BACK);
        std::exit(0);
    }

    // Double buffering evita cintilacao ao trocar o quadro ja renderizado.
    glutSwapBuffers();
}

// Atualiza viewport e matriz de projecao quando a janela muda de tamanho.
// A projecao perspectiva reforca a profundidade da cena 3D.
void reshape(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, static_cast<GLfloat>(w) / static_cast<GLfloat>(h), 0.5, 140.0);

    glMatrixMode(GL_MODELVIEW);
}
