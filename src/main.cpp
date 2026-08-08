#include "sistema_solar.hpp"

// Ponto de entrada da aplicacao. A funcao configura a janela GLUT,
// inicializa os recursos do OpenGL e registra os callbacks de desenho,
// redimensionamento, teclado e animacao.
int main(int argc, char** argv) {
    // Opcao usada para gerar uma captura automaticamente durante testes.
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--screenshot") == 0)
            screenshotMode = true;
    }

    // Inicializacao do GLUT com double buffering, RGB e z-buffer.
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(80, 50);
    glutCreateWindow("Sistema Solar 3D - OpenGL 2.1");

    // Configura estado global do OpenGL e cria planetas, texturas e estrelas.
    initOpenGL();
    lastTimeMs = glutGet(GLUT_ELAPSED_TIME);

    // Registro dos callbacks que controlam o ciclo de vida da aplicacao.
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutIdleFunc(idle);

    // Loop principal de eventos do GLUT.
    glutMainLoop();
    return 0;
}
