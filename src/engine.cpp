#include "sistema_solar.hpp"

// Estados gerais da simulacao, alterados pelos controles de teclado.
bool paused = false;
bool showOrbits = true;
bool useLighting = true;
bool useTextures = true;
bool bezierTour = false;
bool screenshotMode = false;

// Controle do tempo de simulacao e da velocidade da animacao.
float timeScale = 1.0f;
float simulationTime = 0.0f;
int lastTimeMs = 0;

// Parametros da camera manual e da camera automatica por Bezier.
float cameraYaw = 35.0f;
float cameraPitch = 22.0f;
float cameraDistance = 38.0f;
float bezierT = 0.0f;

// Objetos compartilhados pelos modulos de renderizacao.
GLUquadric* sphereQuadric = NULL;
std::vector<Planet> planets;
std::vector<Vec3> stars;

// Quatro pontos de controle que definem a curva cubica de Bezier
// percorrida pela camera quando o modo automatico esta ativado.
static Vec3 bezierPoints[4] = {
    {30.0f, 12.0f, 28.0f},
    {10.0f, 28.0f, -32.0f},
    {-32.0f, 10.0f, -18.0f},
    {-18.0f, 18.0f, 34.0f}
};

// Cria os oito planetas e concentra seus principais parametros em um unico
// local. As escalas de tamanho, distancia e velocidade sao didaticas.
void createPlanets() {
    planets.clear();
    // nome, raio, raio orbital, vel. orbital, vel. rotacao, inclinacao,
    // cor base RGB, cor de detalhe RGB, padrao de textura, id da textura.
    planets.push_back({"Mercurio", 0.34f, 4.0f, 38.0f, 55.0f, 0.0f, 0.45f, 0.40f, 0.35f, 0.72f, 0.65f, 0.55f, TEX_ROCKY, 0});
    planets.push_back({"Venus", 0.53f, 5.6f, 28.0f, -18.0f, 177.0f, 0.78f, 0.58f, 0.28f, 0.95f, 0.78f, 0.42f, TEX_ROCKY, 0});
    planets.push_back({"Terra", 0.58f, 7.5f, 23.0f, 70.0f, 23.5f, 0.15f, 0.38f, 0.80f, 0.20f, 0.70f, 0.35f, TEX_EARTH, 0});
    planets.push_back({"Marte", 0.43f, 9.4f, 18.0f, 66.0f, 25.0f, 0.72f, 0.24f, 0.10f, 0.95f, 0.45f, 0.20f, TEX_ROCKY, 0});
    planets.push_back({"Jupiter", 1.26f, 12.6f, 10.5f, 105.0f, 3.0f, 0.78f, 0.58f, 0.40f, 0.45f, 0.25f, 0.15f, TEX_GAS, 0});
    planets.push_back({"Saturno", 1.08f, 16.0f, 7.8f, 95.0f, 26.7f, 0.82f, 0.72f, 0.48f, 0.55f, 0.42f, 0.22f, TEX_GAS, 0});
    planets.push_back({"Urano", 0.82f, 19.4f, 5.7f, -70.0f, 98.0f, 0.42f, 0.78f, 0.82f, 0.72f, 0.95f, 0.95f, TEX_ICE, 0});
    planets.push_back({"Netuno", 0.79f, 22.4f, 4.4f, 72.0f, 28.3f, 0.12f, 0.28f, 0.82f, 0.24f, 0.55f, 1.00f, TEX_ICE, 0});
}

// Limita um valor ao intervalo [0, 1], usado na geracao das cores.
float clamp01(float v) {
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

// Converte um componente de cor em ponto flutuante para um byte RGB.
unsigned char toByte(float v) {
    return static_cast<unsigned char>(clamp01(v) * 255.0f);
}

// Gera um ruido pseudoaleatorio deterministico a partir de x, y e uma semente.
// Isso permite criar texturas repetiveis sem depender de arquivos externos.
float pseudoNoise(int x, int y, int seed) {
    unsigned int n = static_cast<unsigned int>(x * 374761393u + y * 668265263u + seed * 2246822519u);
    n = (n ^ (n >> 13u)) * 1274126177u;
    n ^= (n >> 16u);
    return (n & 0xFFFFu) / 65535.0f;
}

// Gera em memoria uma textura RGB para um planeta, variando o padrao conforme
// o tipo do corpo: rochoso, Terra, gasoso, gelado ou Sol.
GLuint createProceduralTexture(const Planet& p, int seed) {
    std::vector<GLubyte> img(TEXTURE_W * TEXTURE_H * 3);

    for (int y = 0; y < TEXTURE_H; ++y) {
        for (int x = 0; x < TEXTURE_W; ++x) {
            float u = static_cast<float>(x) / static_cast<float>(TEXTURE_W - 1);
            float v = static_cast<float>(y) / static_cast<float>(TEXTURE_H - 1);
            float n = pseudoNoise(x, y, seed);
            float mix = 0.0f;

            // Cada padrao combina funcoes seno/cosseno e ruido para obter
            // variacoes visuais simples, adequadas a demonstracao de texturas.
            switch (p.pattern) {
                case TEX_EARTH: {
                    float continent = std::sin(u * 18.0f + std::sin(v * 11.0f) * 2.0f)
                                    + std::cos(v * 17.0f + u * 5.0f)
                                    + (n - 0.5f) * 1.8f;
                    mix = continent > 0.65f ? 0.92f : 0.08f;
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

            // Interpola entre a cor base e a cor de detalhe do planeta.
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

    // Cria o objeto de textura no OpenGL e gera mipmaps para melhorar a
    // filtragem quando o planeta aparece em diferentes distancias da camera.
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, TEXTURE_W, TEXTURE_H, GL_RGB, GL_UNSIGNED_BYTE, &img[0]);
    return tex;
}

// Gera e associa uma textura procedural a cada planeta.
void createTextures() {
    for (size_t i = 0; i < planets.size(); ++i) {
        planets[i].textureId = createProceduralTexture(planets[i], static_cast<int>(i) + 11);
    }
}

// Distribui estrelas aleatoriamente sobre uma casca esferica ao redor da cena.
// A semente fixa torna o campo de estrelas igual a cada execucao.
void createStars() {
    stars.clear();
    std::srand(42);
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

// Avalia uma curva cubica de Bezier pela base de Bernstein.
// t varia de 0 a 1 e determina a posicao atual da camera automatica.
Vec3 bezierPoint(float t) {
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

// Posiciona a camera com gluLookAt(). No modo automatico, usa a curva de
// Bezier; no modo manual, converte yaw/pitch/distancia para coordenadas 3D.
void applyCamera() {
    if (bezierTour) {
        Vec3 p = bezierPoint(bezierT);
        gluLookAt(p.x, p.y, p.z, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
        return;
    }

    float yaw = cameraYaw * static_cast<float>(M_PI) / 180.0f;
    float pitch = cameraPitch * static_cast<float>(M_PI) / 180.0f;

    float x = cameraDistance * std::cos(pitch) * std::sin(yaw);
    float y = cameraDistance * std::sin(pitch);
    float z = cameraDistance * std::cos(pitch) * std::cos(yaw);

    gluLookAt(x, y, z, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}
