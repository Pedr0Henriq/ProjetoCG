# Sistema Solar 3D — Projeto Final de Computação Gráfica

Projeto desenvolvido em C++ com OpenGL 2.1, GLU e GLUT para a disciplina de Computação Gráfica.

## Integrantes

- Gabriel Rafá Martins Freire — 20230145310
- Tobias Freire Numeriano — 20230012378
- Pedro Henrique de Araujo Lima — 20220005950

## O que o programa faz

O programa apresenta uma simulação 3D didática do Sistema Solar com:

- Sol e oito planetas;
- rotação dos planetas e translação em torno do Sol;
- Lua orbitando a Terra por transformação hierárquica;
- anéis de Saturno;
- câmera 3D controlável pelo teclado;
- passeio automático de câmera usando uma curva cúbica de Bézier;
- iluminação posicional com o Sol como fonte de luz;
- sombreamento suave;
- texturas procedurais aplicadas aos planetas;
- z-buffer para resolver corretamente a oclusão;
- órbitas e campo de estrelas;
- controles para pausar, acelerar e ligar/desligar luz, textura e órbitas;
- captura de tela da aplicação.

As escalas de tamanho, distância e velocidade foram adaptadas para fins didáticos e de visualização. Elas não representam proporções astronômicas reais.

## Imagem do programa

![Sistema Solar 3D](docs/screenshot.png)

## Estrutura do projeto

```text
.
├── .gitignore
├── Makefile
├── README.md
├── docs/
│   └── screenshot.png
└── src/
    ├── engine.cpp
    ├── input.cpp
    ├── main.cpp
    ├── render.cpp
    └── sistema_solar.hpp
```

O executável `sistema_solar` é gerado por `make` e, por isso, está listado no `.gitignore`.

## Dependências

### Fedora

```bash
sudo dnf install gcc-c++ freeglut-devel mesa-libGLU-devel
```

### Ubuntu / Debian

```bash
sudo apt update
sudo apt install build-essential freeglut3-dev libglu1-mesa-dev
```

## Como compilar

Na pasta raiz do projeto:

```bash
make
```

Ou diretamente:

```bash
g++ -std=c++11 -O2 -Wall -Wextra -pedantic \
    src/main.cpp src/engine.cpp src/render.cpp src/input.cpp \
    -o sistema_solar -lGL -lGLU -lglut
```

## Como executar

```bash
./sistema_solar
```

Também é possível usar:

```bash
make run
```

## Controles

| Tecla | Ação |
|---|---|
| Espaço | Pausa ou continua a animação |
| `+` / `-` | Aumenta ou diminui a velocidade da simulação |
| `1` | Câmera padrão |
| `2` | Câmera vista de cima |
| `3` | Câmera aproximada |
| `C` | Ativa/desativa passeio automático por curva de Bézier |
| `L` | Ativa/desativa iluminação |
| `T` | Ativa/desativa texturas |
| `O` | Mostra/esconde as órbitas |
| `R` | Restaura câmera e velocidade |
| Setas | Gira a câmera manualmente |
| PageUp / PageDown | Aproxima/afasta a câmera |
| `P` | Salva uma captura em `screenshot.ppm` |
| `H` | Mostra os controles no terminal |
| Esc | Encerra o programa |

## Onde alterar os parâmetros

Os parâmetros principais ficam concentrados na função `createPlanets()` em `src/engine.cpp`.

Cada planeta possui, nesta ordem:

```text
nome,
raio,
distancia_da_orbita,
velocidade_orbital,
velocidade_de_rotacao,
inclinacao_do_eixo,
cor_base,
cor_de_detalhe,
padrao_de_textura
```

Exemplo simplificado da Terra:

```cpp
planets.push_back({
    "Terra",
    0.58f,
    7.5f,
    23.0f,
    70.0f,
    23.5f,
    ...
});
```

Assim, é possível modificar o comportamento sem alterar a lógica do restante do programa.

## Organização do código

- `main.cpp`: criação da janela e registro dos callbacks GLUT.
- `engine.cpp`: dados da simulação, planetas, texturas, estrelas, Bézier e câmera.
- `render.cpp`: desenho da cena, iluminação, transformações e projeção.
- `input.cpp`: teclado, animação, captura e inicialização do OpenGL.
- `sistema_solar.hpp`: estruturas, constantes, estados globais e protótipos.

## Técnicas de computação gráfica usadas

- fluxo básico GLUT com callbacks e loop de eventos;
- projeção perspectiva com `gluPerspective()` e câmera com `gluLookAt()`;
- transformações hierárquicas com `glPushMatrix()` e `glPopMatrix()`;
- z-buffer com `GL_DEPTH_TEST` para oclusão correta;
- iluminação com `GL_LIGHTING`, `GL_LIGHT0` e `GL_SMOOTH`;
- texturas procedurais com `GL_TEXTURE_2D` e mipmaps;
- curva cúbica de Bézier para o passeio automático da câmera.

## Melhorias futuras

- usar imagens reais de maior resolução para os planetas;
- adicionar mais luas, cinturão de asteroides e cometas;
- usar órbitas elípticas e inclinações orbitais específicas;
- criar foco de câmera por planeta;
- adicionar nomes dos planetas na interface;
- permitir editar parâmetros por arquivo de configuração;
- migrar futuramente para OpenGL moderno com shaders.
