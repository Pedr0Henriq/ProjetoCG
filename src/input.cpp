#include "sistema_solar.hpp"

// Exibe no terminal um resumo dos comandos disponiveis ao usuario.
void printHelp() {
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

// Le os pixels da janela OpenGL e salva uma imagem PPM (P6). O percurso das
// linhas e invertido porque a origem do framebuffer fica no canto inferior.
void saveScreenshot(const char* filename, GLenum readBuffer) {
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
    for (int y = h - 1; y >= 0; --y)
        std::fwrite(&pixels[y * w * 3], 1, w * 3, f);
    std::fclose(f);
    std::cout << "Imagem salva em: " << filename << "\n";
}

// Callback para teclas ASCII. Cada comando altera um estado da simulacao
// e solicita um novo desenho com glutPostRedisplay().
void keyboard(unsigned char key, int, int) {
    switch (key) {
        case 27: std::exit(0); break;
        case ' ': paused = !paused; break;
        case '+': case '=': timeScale *= 1.35f; break;
        case '-': case '_': timeScale /= 1.35f; if (timeScale < 0.05f) timeScale = 0.05f; break;

        // Presets de camera: padrao, vista superior e aproximada.
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

        // Alternancia dos principais recursos demonstrados no projeto.
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

// Callback para teclas especiais do GLUT. As setas alteram yaw/pitch e
// PageUp/PageDown controlam a distancia da camera.
void specialKeys(int key, int, int) {
    // Ao mover manualmente a camera, o passeio automatico e encerrado.
    bezierTour = false;
    switch (key) {
        case GLUT_KEY_LEFT: cameraYaw -= 4.0f; break;
        case GLUT_KEY_RIGHT: cameraYaw += 4.0f; break;
        case GLUT_KEY_UP: cameraPitch += 3.0f; break;
        case GLUT_KEY_DOWN: cameraPitch -= 3.0f; break;
        case GLUT_KEY_PAGE_UP: cameraDistance -= 1.5f; break;
        case GLUT_KEY_PAGE_DOWN: cameraDistance += 1.5f; break;
        default: break;
    }

    // Limites evitam inversoes excessivas ou zoom para dentro dos objetos.
    if (cameraPitch > 88.0f) cameraPitch = 88.0f;
    if (cameraPitch < -30.0f) cameraPitch = -30.0f;
    if (cameraDistance < 10.0f) cameraDistance = 10.0f;
    if (cameraDistance > 75.0f) cameraDistance = 75.0f;
    glutPostRedisplay();
}

// Callback de animacao. O tempo real entre quadros e usado para manter o
// movimento independente da taxa de quadros do computador.
void idle() {
    int now = glutGet(GLUT_ELAPSED_TIME);
    float dt = (now - lastTimeMs) / 1000.0f;
    lastTimeMs = now;

    // Evita saltos muito grandes quando a janela fica temporariamente parada.
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

// Configuracao inicial do pipeline fixo do OpenGL 2.1 e criacao dos dados
// necessarios para a cena. O z-buffer e habilitado aqui para tratar oclusao.
void initOpenGL() {
    glClearColor(0.005f, 0.005f, 0.018f, 1.0f);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    // Propriedades especulares compartilhadas pelos materiais dos planetas.
    GLfloat specular[] = {0.35f, 0.35f, 0.35f, 1.0f};
    GLfloat shininess[] = {24.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shininess);

    // GLUquadric facilita o desenho de esferas com normais e coordenadas UV.
    sphereQuadric = gluNewQuadric();
    gluQuadricNormals(sphereQuadric, GLU_SMOOTH);

    createPlanets();
    createTextures();
    createStars();
    printHelp();
}
