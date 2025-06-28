/*
 *Prova do Grau B - Processamento Gráfico
 *Aluno: José Márcio Krüger Costantini
 */

#include <iostream>
#include <string>
#include <assert.h>
#include <cmath>

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

// STB_IMAGE
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

//GLM
#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;


struct Sprite
{
	GLuint VAO;
	GLuint texID;
	vec3 position;
	vec3 dimensions; // Tamanho do frame
	float ds, dt;
	int iAnimation, iFrame;
	int nAnimations, nFrames;
};
	
struct Tile
{
	GLuint VAO;
	GLuint texID; // de qual tileset
	int iTile; // indice dele no tileset
	vec3 position;
	vec3 dimensions; //tamanho do losango 2:1
	float ds, dt;
	bool caminhavel;
	//bool shells;

};	

struct Shell // Contém a posição e um bool para verificar se já foi coletada a concha.
{
	vec2 pos;
	bool coletado;
};

// Protótipo da função de callback de teclado
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);

// Protótipos das funções
int setupShader(); // Configurar os Shaders
int setupSprite(int nAnimations, int nFrames, float &ds, float &dt); // Configurar os Sprites.
int setupTile(int nTiles, float &ds, float &dt); // COnfigurar os Tiles.
int loadTexture(string filePath, int &width, int &height); // Carregar imagens para textura.
void desenharMapa(GLuint shaderID); // Desenhar o mapa isométrico de tiles.
void desenharPersonagem(GLuint shaderID); // Desenhar o quadrado rosa de onde o personagem está.
void gerarConchas(); // Gerar posiçôes aleatórias para as conchas.

// Dimensões da janela (pode ser alterado em tempo de execução)
void desenharConchas(GLuint shaderID, Sprite shells); // Desenhar as conchas com as posições e o Sprite  shells(objeto com a textura da concha).
const GLuint WIDTH = 800, HEIGHT = 600;

// Código fonte do Vertex Shader (em GLSL): ainda hardcoded
const GLchar *vertexShaderSource = R"(
 #version 400
 layout (location = 0) in vec3 position;
 layout (location = 1) in vec2 texc;
 out vec2 tex_coord;
 uniform mat4 model;
 uniform mat4 projection;
 void main()
 {
	tex_coord = vec2(texc.s, 1.0 - texc.t);
	gl_Position = projection * model * vec4(position, 1.0);
 }
 )";

// Código fonte do Fragment Shader (em GLSL): ainda hardcoded
const GLchar *fragmentShaderSource = R"(
 #version 400
 in vec2 tex_coord;
 out vec4 color;
 uniform sampler2D tex_buff;
 uniform vec2 offsetTex;

 void main()
 {
	 color = texture(tex_buff,tex_coord + offsetTex);
 }
 )";

 #define TILEMAP_WIDTH 15
 #define TILEMAP_HEIGHT 15

// Mapa com a maré baixa.
int map[15][15] = {
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,  
1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4, 4,
1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4, 4,   
1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4, 4,   
1, 1, 0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,   
1, 1, 1, 1, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,  
1, 1, 1, 1, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
1, 1, 1, 0, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,   
1, 1, 0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,   
1, 1, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,   
1, 0, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4
};

//Mapa com a maré cheia
int mapMareCheia[15][15] = {
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,  
1, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4, 4, 4,
1, 0, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
1, 1, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,   
1, 1, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,   
1, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,   
1, 1, 1, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,  
1, 1, 1, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
1, 1, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,   
1, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,   
1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,   
1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4
};

Tile mapa[15][15]; // Matriz com tiles e suas propriedades para desenhar o mapa e garantir a jogablidade
vector <Tile> tileset; // Vetor de tiles
vec2 pos; // Armazena o indice i e j de onde o personagem está na cena, ou seja, o local do personagem no mapa.
vector <Shell> conchas; // Vetor do Struct de conchas 
vec2 camera = vec2(0.0f, 0.0f); // Câmera para acompanhar a movimentação do personagem.


bool mareCheia = false; // Indicador da altura da maré.
// Configuraçôes para gerar o timer da maré.
double tempoMare = 30;
double inicioMare = 0;
double agora = 0;
GLuint texShell, texSailor;
int points = 0;
int maxPoints = 10;
// Função MAIN
int main()
{
	// Inicialização da GLFW
	glfwInit();
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4.6);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Ativa a suavização de serrilhado (MSAA) com 8 amostras por pixel
	glfwWindowHint(GLFW_SAMPLES, 8);

	// Essencial para computadores da Apple
	// #ifdef __APPLE__
	//	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	// #endif

	// Criação da janela GLFW
	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Shell Hunter", nullptr, nullptr);
	if (!window)
	{
		std::cerr << "Falha ao criar a janela GLFW" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	// Fazendo o registro da função de callback para a janela GLFW
	glfwSetKeyCallback(window, key_callback);

	// GLAD: carrega todos os ponteiros d funções da OpenGL
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Falha ao inicializar GLAD" << std::endl;
		return -1;
	}

	// Obtendo as informações de versão
	const GLubyte *renderer = glGetString(GL_RENDERER); /* get renderer string */
	const GLubyte *version = glGetString(GL_VERSION);	/* version as a string */
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;

	// Definindo as dimensões da viewport com as mesmas dimensões da janela da aplicação
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	// Compilando e buildando o programa de shader
	GLuint shaderID = setupShader();

	//Carregando textura dos tilesets. 
	int imgWidth, imgHeight;
	
	
	texSailor = loadTexture("../assets/sprites/marinheiro.png",imgWidth,imgHeight); // Textura do personagem (um pequeno marinheiro).
	// Configuração do sprite do marinheiro.
	Sprite sailor;
	sailor.nAnimations = 1;
	sailor.nFrames = 1;
	sailor.VAO = setupSprite(sailor.nAnimations,sailor.nFrames,sailor.ds,sailor.dt);
	sailor.position = vec3(pos.x, pos.y, 0.0);
	sailor.dimensions = vec3(114, 57, 1.0);
	sailor.texID = texSailor;
	sailor.iAnimation = 1;
	sailor.iFrame = 0; 

	// Textura das conchas a serem coletadas.
	texShell = loadTexture("../assets/backgrounds/shell.png",imgWidth,imgHeight);
	// Configuração do sprite das conchas.
	Sprite shells;
	shells.nAnimations = 1;
	shells.nFrames = 1;
	shells.VAO = setupSprite(shells.nAnimations,shells.nFrames,shells.ds,shells.dt);
	shells.position = vec3(pos.x, pos.y, 0.0);
	shells.dimensions = vec3(114, 57, 1.0);
	shells.texID = texShell;
	shells.iAnimation = 1;
	shells.iFrame = 0; 

	GLuint texID = loadTexture("../assets/tilesets/tilesetIso.png",imgWidth,imgHeight); // Carregamento das texturas do tileset.
	// Configura o tileset - conjunto de tiles do mapa
	for (int i = 0; i < 7; i++)
	{
		Tile tile;
		tile.dimensions = vec3(114,57,1.0);
		tile.iTile = i;
		tile.texID = texID;
		tile.VAO = setupTile(7,tile.ds,tile.dt);
		tile.caminhavel = true;
		tileset.push_back(tile);
	}
	tileset[4].caminhavel = false; // Configura a água como não caminhável.
	// Configurar o mapa como conjunto de tiles, cada um com suas características.
	for (int i = 0; i < TILEMAP_HEIGHT; i++) 
	{
		for (int j = 0; j < TILEMAP_WIDTH; j++) 
		{
			Tile tile = tileset[map[i][j]]; // Textura, VAO, dimensões...
			tile.iTile =  map[i][j];
			tile.position = vec3(j, i, 0);
			tile.caminhavel = ( map[i][j]!= 4); // Água não caminhável.
			mapa[i][j] = tile; // preenche o novo mapa com tiles reais
		}
}

	// Inicializar a posição do marinheiro e do tile rosa, que realça o tile onde ele está.
	pos.x = 0;
	pos.y = 0;

	gerarConchas(); // gerar posições randômicas para colocar as conchas.
	glUseProgram(shaderID); // Reseta o estado do shader para evitar problemas futuros.

	 // desenharConchas(shaderID, shells);
	double prev_s = glfwGetTime();	// Define o "tempo anterior" inicial.
	double title_countdown_s = 0.1; // Intervalo para atualizar o título da janela com o FPS.
	
	float colorValue = 0.0;

	// Ativando o primeiro buffer de textura do OpenGL.
	glActiveTexture(GL_TEXTURE0);

	// Criando a variável uniform pra mandar a textura pro shader.
	glUniform1i(glGetUniformLocation(shaderID, "tex_buff"), 0);

	// Matriz de projeção paralela ortográfica.
	mat4 projection = ortho(0.0, 800.0, 600.0, 0.0, -1.0, 1.0);
	glUniformMatrix4fv(glGetUniformLocation(shaderID, "projection"), 1, GL_FALSE, value_ptr(projection));

	glEnable(GL_DEPTH_TEST); // Habilita o teste de profundidade.
	glDepthFunc(GL_ALWAYS); // Testa a cada ciclo.

	glEnable(GL_BLEND); // Habilita a transparência -- canal alpha.
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Seta função de transparência.


	double lastTime = 0.0;
	double deltaT = 0.0;
	double currTime = glfwGetTime();
	double FPS = 12.0;
	inicioMare = glfwGetTime();
	
	// Loop da aplicação - "game loop".
	while (!glfwWindowShouldClose(window))
	{
		// Este trecho de código é totalmente opcional: calcula e mostra a contagem do FPS na barra de título.
		{
			double curr_s = glfwGetTime();		// Obtém o tempo atual.
			double elapsed_s = curr_s - prev_s; // Calcula o tempo decorrido desde o último frame.
			prev_s = curr_s;					// Atualiza o "tempo anterior" para o próximo frame.

			// Exibe o FPS, mas não a cada frame, para evitar oscilações excessivas.
			title_countdown_s -= elapsed_s;
			if (title_countdown_s <= 0.0 && elapsed_s > 0.0)
			{
				double fps = 1.0 / elapsed_s; // Calcula o FPS com base no tempo decorrido.

				// Cria uma string e define o FPS como título da janela.
				char tmp[256];
				sprintf(tmp, "Shell Hunter: THE GAME \tFPS %.2lf", fps);
				glfwSetWindowTitle(window, tmp);

				title_countdown_s = 0.1; // Reinicia o temporizador para atualizar o título periodicamente.
			}
		}

		// Checa se houveram eventos de input (key pressed, mouse moved etc.) e chama as funções de callback correspondentes.
		glfwPollEvents();
		double agora = glfwGetTime();
		// Timer que controla a maré.
		if(agora - inicioMare >= tempoMare)
		{
			mareCheia = !mareCheia; // Alternarnância entre as marés.
			inicioMare = agora; // Timer é reiniciado.
			if(mareCheia)
			{
				cout << "Mare cheia! Tu estas preso!" << endl; 
				for(int i = 0; i < TILEMAP_HEIGHT; i++)   // Muda para o mapa de maré cheia.
				{
					for(int j = 0; j < TILEMAP_WIDTH; j++)
					{
						int aux = mapMareCheia[i][j];
						Tile auxTile = tileset[aux];
						auxTile.position = vec3(j, i, 0);
						if(aux == 4)
						{
							auxTile.caminhavel = false;
						}	
						else
						{
						auxTile.caminhavel = true;
						}
						mapa[i][j] = auxTile;
					}
				}
				int i = (int)pos.y;
				int j = (int)pos.x;
				int tileAtual = mapMareCheia[i][j];
			}

			else // Maré baixa
			{
			cout<<"Maré baixa! Estas livre!"<<endl;
			for(int i = 0; i < TILEMAP_HEIGHT; i++)   // Muda para o mapa de maré baixa.
				{
					for(int j = 0; j < TILEMAP_WIDTH; j++)
					{
						int aux = map[i][j];
						Tile auxTile = tileset[aux];
						auxTile.position = vec3(j, i, 0);
						if(aux == 4)
						{
							auxTile.caminhavel = false;
						}	
						else
						{
						auxTile.caminhavel = true;
						}
						mapa[i][j] = auxTile;
					}
				}
			}

		}
		camera = pos; // A "câmera" recebe a posição do local do personagem.

		// Limpa o buffer de cor
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // cor de fundo.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Limpa o buffer de profundidade.

		glLineWidth(10);
		glPointSize(20);

		// Desenhar o mapa
		desenharMapa(shaderID);
		// Desenhar o tile rosa.
		desenharPersonagem(shaderID);
		// Desenhar o marinheiro no tile rosa, poderia ser um uma função também.
		float tileWidth = tileset[0].dimensions.x;
		float tileHeight = tileset[0].dimensions.y;

		float x0 = WIDTH / 2.0f;
		float y0 = HEIGHT / 2.0f;

		float offsetX = x0 - (camera.x - camera.y) * tileWidth / 2.0f;
		float offsetY = y0 - (camera.x + camera.y) * tileHeight / 2.0f;

		float x = offsetX + (pos.x - pos.y) * tileWidth / 2.0f;
		float y = offsetY + (pos.x + pos.y) * tileHeight / 2.0f;

		mat4 model = mat4(1.0f);
		model = translate(model, vec3(x + 40.0f, y, 0.0f)); // "+40.0f" para centralizar o marinheiro na base do tile.
		model = rotate(model, radians(180.0f), vec3(0.0, 0.0, 1.0));
		model = scale(model, sailor.dimensions);
		glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(model));

		vec2 offsetTex;
		offsetTex.s = sailor.iFrame * sailor.ds;
		offsetTex.t = sailor.iAnimation * sailor.dt;
		glUniform2f(glGetUniformLocation(shaderID, "offsetTex"), offsetTex.s, offsetTex.t);

		glBindVertexArray(sailor.VAO);
		glBindTexture(GL_TEXTURE_2D, sailor.texID);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		desenharConchas(shaderID, shells); // Desenho das conchas.
		glfwSwapBuffers(window);
	}
		
	// Finaliza a execução da GLFW, limpando os recursos alocados por ela
	glfwTerminate();
	return 0;
}

// Função de callback de teclado - só pode ter uma instância (deve ser estática se
// estiver dentro de uma classe) - É chamada sempre que uma tecla for pressionada
// ou solta via GLFW
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	vec2 aux = pos;

	if (key == GLFW_KEY_W && action == GLFW_PRESS) // NORTE
	{
		if (pos.x > 0)
		{
			pos.x--;
		}

		if (pos.y > 0)
		{
			pos.y--;
		}
	}

	if (key == GLFW_KEY_A && action == GLFW_PRESS) // OESTE
	{
		if (pos.x > 0)
		{
			pos.x--;
		}
		if (pos.y <= TILEMAP_HEIGHT - 2)
		{
			pos.y++;
		}
	}

	if (key == GLFW_KEY_S && action == GLFW_PRESS) // SUL
	{
		if (pos.x <= TILEMAP_WIDTH -2)
		{
			pos.x++;
		}

		if (pos.y <= TILEMAP_HEIGHT - 2)
		{
			pos.y++;
		}
	}

	if (key == GLFW_KEY_D && action == GLFW_PRESS) // LESTE
	{
		if (pos.x <= TILEMAP_WIDTH -2)
		{
			pos.x++;
		}

		if (pos.y > 0)
		{
			pos.y--;
		}

	}

	if (key == GLFW_KEY_Q && action == GLFW_PRESS) // NOROESTE
	{
		if (pos.x > 0)
		{
			pos.x--;
		}

	}

	if (key == GLFW_KEY_E && action == GLFW_PRESS) // NORDESTE
	{
		if (pos.y > 0)
		{
			pos.y--;
		}

	}

	if (key == GLFW_KEY_Z && action == GLFW_PRESS) // SUDOESTE
	{
		if (pos.y <= TILEMAP_HEIGHT - 2)
		{
			pos.y++;
		}

	}

	if (key == GLFW_KEY_X && action == GLFW_PRESS) //SUDESTE
	{
		if (pos.x <= TILEMAP_WIDTH -2)
		{
			pos.x++;
		}

	}

	if (!mapa[(int)pos.y][(int)pos.x].caminhavel)
	{
		pos = aux; //Caso a posição seja não caminável, recebe a pos não mudada.
	}

	cout << "(" << pos.x <<"," << pos.y << ")" << endl;

	for (auto &item : conchas) {
    if (!item.coletado && (int)item.pos.x == (int)pos.x && (int)item.pos.y == (int)pos.y) {
        item.coletado = true;
        cout << "Concha coletada em (" << item.pos.x << "," << item.pos.y << ")" << endl;
		points ++;
		cout << "Pontos: " << points << endl;
		if(points == maxPoints)
			cout << "Parabens!!!! Todas as conchasforam coletadas!!!" << points << endl;
    }
}

}

// Esta função está bastante hardcoded - objetivo é compilar e "buildar" um programa de
//  shader simples e único neste exemplo de código.
//  O código fonte do vertex e fragment shader está nos arrays vertexShaderSource e
//  fragmentShader source no iniçio deste arquivo.
//  A função retorna o identificador do programa de shader.
int setupShader()
{
	// Vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	// Checando erros de compilação (exibição via log no terminal)
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
				  << infoLog << std::endl;
	}
	// Fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	// Checando erros de compilação (exibição via log no terminal)
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
				  << infoLog << std::endl;
	}
	// Linkando os shaders e criando o identificador do programa de shader
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	// Checando por erros de linkagem
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
				  << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

// Gerar os Sprites, com dois triângulos para formar um quadrilátero e uma textura correspondente.
int setupSprite(int nAnimations, int nFrames, float &ds, float &dt)
{
	ds = 1.0 / (float) nFrames;
	dt = 1.0 / (float) nAnimations;
	// Aqui setamos as coordenadas x, y e z do triângulo e as armazenamos de forma
	// sequencial, já visando mandar para o VBO (Vertex Buffer Objects).
	// Cada atributo do vértice (coordenada, cores, coordenadas de textura, normal, etc)
	// Pode ser arazenado em um VBO único ou em VBOs separados
	GLfloat vertices[] = {
		// x   y    z    s     t
		-0.5,  0.5, 0.0, 0.0, dt, //V0
		-0.5, -0.5, 0.0, 0.0, 0.0, //V1
		 0.5,  0.5, 0.0, ds, dt, //V2
		 0.5, -0.5, 0.0, ds, 0.0  //V3
		};

	GLuint VBO, VAO;
	// Geração do identificador do VBO.
	glGenBuffers(1, &VBO);
	// Faz a conexão (vincula) do buffer como um buffer de array.
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// Envia os dados do array de floats para o buffer da OpenGl.
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Geração do identificador do VAO (Vertex Array Object).
	glGenVertexArrays(1, &VAO);
	// Vincula (bind) o VAO primeiro, e em seguida  conecta e seta o(s) buffer(s) de vértices
	// e os ponteiros para os atributos.
	glBindVertexArray(VAO);
	// Para cada atributo do vertice, criamos um "AttribPointer" (ponteiro para o atributo), indicando:
	//  Localização no shader * (a localização dos atributos devem ser correspondentes no layout especificado no vertex shader).
	//  Numero de valores que o atributo tem (por ex, 3 coordenadas xyz).
	//  Tipo do dado.
	//  Se está normalizado (entre zero e um).
	//  Tamanho em bytes.
	//  Deslocamento a partir do byte zero.

	// Ponteiro pro atributo 0 - Posição - coordenadas x, y, z.
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid *)0);
	glEnableVertexAttribArray(0);

	// Ponteiro pro atributo 1 - Coordenada de textura s, t.
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// Observe que isso é permitido, a chamada para glVertexAttribPointer registrou o VBO como o objeto de buffer de vértice.
	// atualmente vinculado - para que depois possamos desvincular com segurança.
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Desvincula o VAO (é uma boa prática desvincular qualquer buffer ou array para evitar bugs medonhos).
	glBindVertexArray(0);

	return VAO;
}
// Função para configurar os tiles em losangos.
int setupTile(int nTiles, float &ds, float &dt)
{
	ds = 1.0 / (float) nTiles;
	dt = 1.0;
	
	//th e tw serão 1.0 pois serão escalados posteriormente.
	float th = 1.0, tw = 1.0;

	GLfloat vertices[] = {
		// x     y        z    s       t
		0.0,  th/2.0f,   0.0, 0.0,    dt/2.0f, //A
		tw/2.0f, th,     0.0, ds/2.0f, dt,     //B
		tw/2.0f, 0.0,    0.0, ds/2.0f, 0.0,    //D
		tw,     th/2.0f, 0.0, ds,     dt/2.0f  //C
		};

	GLuint VBO, VAO;
	// Geração do identificador do VBO
	glGenBuffers(1, &VBO);
	// Faz a conexão (vincula) do buffer como um buffer de array
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// Envia os dados do array de floats para o buffer da OpenGl
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Geração do identificador do VAO (Vertex Array Object)
	glGenVertexArrays(1, &VAO);
	// Vincula (bind) o VAO primeiro, e em seguida  conecta e seta o(s) buffer(s) de vértices
	// e os ponteiros para os atributos
	glBindVertexArray(VAO);
	// Para cada atributo do vertice, criamos um "AttribPointer" (ponteiro para o atributo), indicando:
	//  Localização no shader * (a localização dos atributos devem ser correspondentes no layout especificado no vertex shader).
	//  Numero de valores que o atributo tem (por ex, 3 coordenadas xyz).
	//  Tipo do dado.
	//  Se está normalizado (entre zero e um).
	//  Tamanho em bytes.
	//  Deslocamento a partir do byte zero.

	// Ponteiro pro atributo 0 - Posição - coordenadas x, y, z.
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid *)0);
	glEnableVertexAttribArray(0);

	// Ponteiro pro atributo 1 - Coordenada de textura s, t.
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// Observe que isso é permitido, a chamada para glVertexAttribPointer registrou o VBO como o objeto de buffer de vértice.
	// atualmente vinculado - para que depois possamos desvincular com segurança.
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Desvincula o VAO (é uma boa prática desvincular qualquer buffer ou array para evitar bugs medonhos).
	glBindVertexArray(0);

	return VAO;
}
// Load de textura. Passa width e height por referência.
int loadTexture(string filePath, int &width, int &height)
{
	GLuint texID;

	// Gera o identificador da textura na memória
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	int nrChannels;

	unsigned char *data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);

	if (data)
	{
		if (nrChannels == 3) // jpg, bmp. apenas 3 canais: R, G, B.
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		}
		else // png. 4 Canais: R, G, B, Alpha(transparência).
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}

	stbi_image_free(data);

	glBindTexture(GL_TEXTURE_2D, 0);

	return texID;
}
// Desenhar o mapa com as características de cada tile.

void desenharMapa(GLuint shaderID)
{
	float tileWidth = tileset[0].dimensions.x;
	float tileHeight = tileset[0].dimensions.y;

	float x0 = WIDTH / 2.0f;
	float y0 = HEIGHT / 2.0f;
	// Acompanhar a câmera.
	float offsetX = x0 - (camera.x - camera.y) * tileWidth / 2.0f;
	float offsetY = y0 - (camera.x + camera.y) * tileHeight / 2.0f;

	
	for(int i = 0; i < TILEMAP_HEIGHT; i++)
	{
		for (int j = 0; j < TILEMAP_WIDTH; j++)
		{
			// Matriz de transformaçao do objeto - Matriz de modelo
			mat4 model = mat4(1); //matriz identidade
			// Recebe o "mapa[i][j]" de acordo com a maré do momento.
			Tile curr_tile = mapa[i][j];

			float x = offsetX + (j - i) * tileWidth / 2.0;
			float y = offsetY + (j + i) * tileHeight / 2.0;

			model = translate(model, vec3(x,y,0.0));
			model = scale(model, curr_tile.dimensions);
			glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(model));

		vec2 offsetTex;

		offsetTex.s = curr_tile.iTile * curr_tile.ds;
		offsetTex.t = 0.0;
		glUniform2f(glGetUniformLocation(shaderID, "offsetTex"),offsetTex.s, offsetTex.t);

		glBindVertexArray(curr_tile.VAO); // Conectando ao buffer de geometria
		glBindTexture(GL_TEXTURE_2D, curr_tile.texID); // Conectando ao buffer de textura

		// Chamada de desenho - drawcall
		// Poligono Preenchido - GL_TRIANGLES
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); 
		}
	}
}

void desenharPersonagem(GLuint shaderID)
{
	Tile curr_tile = tileset[6]; //tile rosa
	
	float tileWidth = curr_tile.dimensions.x;
	float tileHeight = curr_tile.dimensions.y;

	float x0 = WIDTH / 2.0f;
	float y0 = HEIGHT / 2.0f;

	float offsetX = x0 - (camera.x - camera.y) * tileWidth / 2.0f;
	float offsetY = y0 - (camera.x + camera.y) * tileHeight / 2.0f;

	float x = offsetX + (pos.x-pos.y) * curr_tile.dimensions.x/2.0;
	float y = offsetY + (pos.x+pos.y) * curr_tile.dimensions.y/2.0;

	mat4 model = mat4(1);
	model = translate(model, vec3(x,y,0.0));
	model = scale(model,curr_tile.dimensions);
	glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(model));

	vec2 offsetTex;

	offsetTex.s = curr_tile.iTile * curr_tile.ds;
	offsetTex.t = 0.0;
	glUniform2f(glGetUniformLocation(shaderID, "offsetTex"),offsetTex.s, offsetTex.t);

	glBindVertexArray(curr_tile.VAO); // Conectando ao buffer de geometria
	glBindTexture(GL_TEXTURE_2D, curr_tile.texID); // Conectando ao buffer de textura

	// Chamada de desenho - drawcall
	// Poligono Preenchido - GL_TRIANGLES
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void gerarConchas()
{
	srand(time(0));
	conchas.clear();

	 while (conchas.size() < 10) {
        int i = rand() % TILEMAP_HEIGHT;
        int j = rand() % TILEMAP_WIDTH;

        if (mapa[i][j].iTile == 0) { // Apenas na areia
            bool repetido = false;
            for (auto &c : conchas)
                if ((int)c.pos.x == j && (int)c.pos.y == i)
                    repetido = true;

            if (!repetido){
                conchas.push_back({vec2(j, i), false});
				
			}
        }
    }
}

void desenharConchas(GLuint shaderID, Sprite shells)
{
    for (auto &item : conchas) {
       
		if(!item.coletado) // Verificar se a concha ja foi coletada ou não. 
		{
			float x0 = WIDTH / 2.0f;
			float y0 = HEIGHT / 2.0f;
			float offsetX = x0 - (camera.x - camera.y) * shells.dimensions.x / 2.0f;
			float offsetY = y0 - (camera.x + camera.y) * shells.dimensions.y / 2.0f;
			float x = offsetX + (item.pos.x - item.pos.y) * shells.dimensions.x / 2.0f;
			float y = offsetY + (item.pos.x + item.pos.y) * shells.dimensions.y / 2.0f;

			mat4 model = mat4(1.0f);
			model = translate(model, vec3(x + 30.0f, y, 0.0)); // "+30.0f" para centralizar a concha na base do tile.
			model = scale(model, shells.dimensions);
			glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(model));

			glUniform2f(glGetUniformLocation(shaderID, "offsetTex"), 0.0, 0.0);

			glBindVertexArray(shells.VAO);
			glBindTexture(GL_TEXTURE_2D, shells.texID);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		} 
		else continue;
    }
}
