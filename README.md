# Sistema Solar 3D - Projeto Final de Computacao Grafica

Projeto desenvolvido em **OpenGL 2.1 / GLUT** para a disciplina de Introducao a Computacao Grafica.

## Integrantes

- Gabriel Rafá Martins Freire - 20230145310
- Tobias Freire Numeriano - 20230012378
- Pedro Henrique de Araujo Lima - 20220005950

## O que o programa faz

O programa apresenta uma simulacao 3D didatica do Sistema Solar com:

- Sol e oito planetas;
- rotacao dos planetas e translacao em torno do Sol;
- Lua orbitando a Terra por transformacao hierarquica;
- aneis de Saturno;
- camera 3D controlavel pelo teclado;
- passeio automatico de camera usando uma curva cubica de Bezier;
- iluminacao posicional com o Sol como fonte de luz;
- sombreamento suave;
- texturas procedurais aplicadas aos planetas;
- z-buffer para resolver corretamente a oclusao;
- orbitas e campo de estrelas;
- controles para pausar, acelerar, ligar/desligar luz, textura e orbitas.

> As escalas de tamanho, distancia e velocidade foram adaptadas para fins didaticos e de visualizacao. Nao representam proporcoes astronomicas reais.

## Imagem do programa

![Sistema Solar 3D](docs/screenshot.png)

## Estrutura do projeto

```text
.
├── Makefile
├── README.md
├── src/
│   └── sistema_solar.cpp
├── docs/
│   ├── screenshot.png
│   ├── RELATORIO_PROJETO.pdf
│   ├── ROTEIRO_APRESENTACAO.pdf
│   ├── GUIA_ESTUDO_E_PERGUNTAS.pdf
│   └── APRESENTACAO_SISTEMA_SOLAR.pptx
└── scripts/
    └── ppm_para_png.sh
```

## Dependencias

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
g++ -std=c++11 -O2 -Wall -Wextra src/sistema_solar.cpp -o sistema_solar -lGL -lGLU -lglut
```

## Como executar

```bash
./sistema_solar
```

Tambem e possivel usar:

```bash
make run
```

## Controles

| Tecla | Acao |
|---|---|
| Espaco | Pausa ou continua a animacao |
| `+` / `-` | Aumenta ou diminui a velocidade da simulacao |
| `1` | Camera padrao |
| `2` | Camera vista de cima |
| `3` | Camera aproximada |
| `C` | Ativa/desativa passeio automatico por curva de Bezier |
| `L` | Ativa/desativa iluminacao |
| `T` | Ativa/desativa texturas |
| `O` | Mostra/esconde as orbitas |
| `R` | Restaura camera e velocidade |
| Setas | Gira a camera manual |
| PageUp / PageDown | Aproxima/afasta a camera |
| `P` | Salva uma captura em `screenshot.ppm` |
| `H` | Mostra os controles no terminal |
| Esc | Encerra o programa |

Para converter a captura PPM em PNG, com ImageMagick instalado:

```bash
./scripts/ppm_para_png.sh
```

## Onde alterar os parametros

Os parametros principais ficam concentrados na funcao `createPlanets()` em `src/sistema_solar.cpp`.

Cada planeta possui, nesta ordem:

```cpp
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

Exemplo:

```cpp
planets.push_back({
    "Terra",
    0.58f,  // tamanho
    7.5f,   // distancia ao Sol
    23.0f,  // velocidade orbital
    70.0f,  // velocidade de rotacao
    23.5f,  // inclinacao
    ...
});
```

Assim, e possivel modificar o comportamento sem alterar a logica do restante do programa.

## Elementos das atividades praticas usados no projeto

### Pratica 01 - Introducao ao OpenGL

- fluxo basico GLUT: `main`, `display`, `reshape`, callbacks de teclado e loop de eventos;
- primitivas OpenGL para orbitas, estrelas e aneis;
- double buffering com `GLUT_DOUBLE` e `glutSwapBuffers()`;
- interacao por teclado.

### Pratica 02 - Visualizacao 3D

- projecao perspectiva com `gluPerspective()`;
- camera com `gluLookAt()`;
- translacoes e rotacoes 3D;
- uso de `glPushMatrix()` e `glPopMatrix()`;
- transformacao hierarquica na relacao Sol -> Terra -> Lua.

### Pratica 03 - Transformacoes e Visibilidade

- z-buffer habilitado com `GL_DEPTH_TEST`;
- limpeza simultanea de `GL_COLOR_BUFFER_BIT` e `GL_DEPTH_BUFFER_BIT`;
- oclusao correta entre Sol, planetas, Lua e aneis.

### Pratica 04 - Iluminacao e Sombreamento

- `GL_LIGHTING` e `GL_LIGHT0`;
- fonte de luz posicional na origem, representando o Sol;
- componentes ambiente, difusa e especular;
- `GL_SMOOTH` para sombreamento suave;
- tecla `L` para comparar a cena com e sem iluminacao.

### Pratica 05 - Mapeamento de Texturas

- criacao de objetos de textura;
- `glBindTexture()` e `GL_TEXTURE_2D`;
- filtragem linear e mipmaps;
- coordenadas de textura na superficie esferica;
- texturas procedurais geradas pelo proprio codigo, evitando arquivos externos;
- tecla `T` para comparar objetos com e sem textura.

### Pratica 06 - Curvas Parametricas

- curva cubica de Bezier definida por quatro pontos de controle;
- avaliacao da curva pela formula de Bernstein;
- camera movimentada automaticamente ao longo da curva;
- tecla `C` para ativar/desativar o passeio.

## Principais problemas encontrados e solucoes

### 1. Escalas astronomicas reais deixam a cena impraticavel

As diferencas reais de tamanho e distancia fariam os planetas praticamente desaparecerem. A solucao foi usar uma escala didatica, preservando apenas a ideia de ordem dos planetas e diferencas relativas visuais.

### 2. Organizar varias transformacoes sem afetar os outros objetos

Rotacoes e translacoes se acumulam na matriz ModelView. O uso sistematico de `glPushMatrix()` e `glPopMatrix()` isola as transformacoes de cada planeta e permite criar a hierarquia da Lua em relacao a Terra.

### 3. Objetos aparecendo na frente mesmo estando mais distantes

Sem teste de profundidade, a ordem de desenho poderia determinar incorretamente qual objeto ficaria visivel. Foi habilitado o z-buffer com `GL_DEPTH_TEST`.

### 4. Combinar iluminacao com texturas

Para que a textura continue visivel e responda a luz, o projeto usa `GL_MODULATE`, normais suaves e `GL_COLOR_MATERIAL`.

### 5. Criar uma camera automatica suave

O passeio foi implementado com uma curva cubica de Bezier. A posicao da camera e calculada continuamente pelo parametro `t`, enquanto `gluLookAt()` mantem o foco no centro do sistema.

## O que pode ser melhorado

- usar imagens reais de alta resolucao dos planetas;
- adicionar mais luas, cinturao de asteroides e cometas;
- usar orbitas elipticas e inclinacoes orbitais especificas;
- criar selecao de planetas e camera focada em cada corpo;
- adicionar nomes dos planetas na interface;
- permitir editar parametros por arquivo de configuracao;
- usar escalas astronomicas alternaveis com modos de visualizacao diferentes;
- implementar sombras mais avancadas e efeitos de atmosfera.

## Divisao de responsabilidades do grupo

### Gabriel Rafá Martins Freire

- integracao geral do projeto;
- estrutura da simulacao e parametros dos planetas;
- transformacoes hierarquicas, animacao orbital e controles de camera;
- organizacao do codigo, documentacao e testes finais.

### Tobias Freire Numeriano

- iluminacao e sombreamento;
- texturas procedurais e aparencia dos corpos;
- campo de estrelas e elementos visuais;
- testes dos controles de luz e textura.

### Pedro Henrique de Araujo Lima

- camera automatica por curva de Bezier;
- z-buffer, verificacao de visibilidade e perspectivas de camera;
- testes de interacao e demonstracao;
- apoio na preparacao da apresentacao.

## Observacao para a apresentacao

A demonstracao recomendada e:

1. iniciar na camera padrao;
2. explicar rotacao e translacao;
3. pressionar `T` para mostrar a contribuicao das texturas;
4. pressionar `L` para mostrar a iluminacao;
5. mudar para camera superior com `2`;
6. usar as setas e zoom;
7. ativar a camera Bezier com `C`;
8. pausar com Espaco e mostrar a estrutura da cena;
9. encerrar retomando o mapa das seis praticas.

O roteiro completo de fala e as perguntas provaveis estao na pasta `docs/`.
