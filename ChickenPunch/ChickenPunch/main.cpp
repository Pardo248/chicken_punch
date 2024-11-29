#include <iostream>
#include <vector>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Shader.h"
#include "Cubo.h"
#include "Piso.h"
#include "Camera.h"
#include "Model.h"
#include "stb_image.h"



using namespace std;

constexpr unsigned width = 500;
constexpr unsigned height = 900;

unsigned int VBO, VAO, EBO;
unsigned int VBO_P, VAO_P, EBO_P;
unsigned int VAO_L;

mat4 projection, view;

vec3 cameraPos = vec3(0.0f, 0.0f, 3.0f);
vec3 cameraFront = vec3(0.0f, 0.0f, -1.0f);
vec3 cameraUp = vec3(0.0f, 1.0f, 0.0f);
float deltaTime = 0.0f;
float lastTime = 0.0f;

Camera camera(vec3(0.0f, 2.0f, 7.0f));
float lastX = width / 2.0f;
float lastY = height / 2.0f;
bool firstMouse = true;
bool gravedadActive = false;

struct AABB {
	vec3 min;
	vec3 max;
};

void initGLFWVersion();
bool gladLoad();
void updateWindow(GLFWwindow* window, Shader ourShader, Shader ourLight, Shader ourShaderPiso, std::vector<Model> models);

void framebuffer_size_callback(GLFWwindow* window, int w, int h);
void processInput(GLFWwindow* window);
void CameraInput(GLFWwindow* window);
void PlayerInput(GLFWwindow* window);
void Mouse_callback(GLFWwindow* window, double xpos, double ypos);
void Scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

void GeneracionBuffer(
	GLuint& VAO,
	GLuint& VBO,
	GLuint& EBO,
	const GLfloat* vertices,
	size_t verticesSize,
	const GLuint* indices,
	size_t indicesSize,
	GLuint& VAO_L);
void VertexAttribute(int layout, int data, int total, int start);
void DeleteVertexArrays(GLuint& VA);
void DeleteBuffer(GLuint& VBO, GLuint& EBO);
void TransformCubo(Shader ourShader, std::vector<Model> models);
void TransformPiso(Shader ourShader);
void TransformCuboLight(Shader ourLight);
void TransformCamera(Shader ourShader);
void CameraUniform(Shader shaderName);
void updatePhysics(float deltaTime);

bool AABBIntersect(AABB box1, AABB box2);
AABB GenerateBoindingBox(vec3 position, float w, float h, float d);
bool DetecCollision();
bool CheckCollision(const vec3& cube1Pos, const vec3& cube2Pos, float size1, float size2);
bool colisionGuante();

// L�mites para la proyecci�n ortogonal
float leftCam = -6.0f;
float rightCam = 6.0f;
float bottom = -2.5f;
float top = 10.0f;
float near = 0.1f;
float far = 100.0f;


// Vertices de un cuadrado
vector<Vertex> verticesA = {
	// Posiciones         // Normales          // TexCoords
	{{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
	{{0.5f, -0.5f, 0.0f},  {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
	{{0.5f,  0.5f, 0.0f},  {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
	{{-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}
};

// Indices para formar un cuadrado (dos tri�ngulos)
vector<unsigned int> indicesA = {
	0, 1, 2, // Primer tri�ngulo
	2, 3, 0  // Segundo tri�ngulo
};

string directory = "Modelos/backpack";
std::vector<unsigned int> textures;
//unsigned int textureID1;// = TextureFromFile("malla.jpg", directory);
//unsigned int textureID2;// = TextureFromFile("egg_obj.jpg", directory);

int health_i = 5;
int points = 0;

int main()
{
	initGLFWVersion();

	//Creacion de ventana
	GLFWwindow* window = glfwCreateWindow(width, height, "OpenGl", NULL, NULL);
	if (window == NULL)
	{
		cout << "Falla al cargar ventana" << endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoad())
	{
		return -1;
	}

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//glfwSetCursorPosCallback(window, Mouse_callback);
	//glfwSetScrollCallback(window, Scroll_callback);
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	stbi_set_flip_vertically_on_load(true);
	srand(time(0));  // Semilla para la aleatoriedad

	glEnable(GL_DEPTH_TEST);

	Shader ourShader("vertexShader.vs", "fragmentShader.fs");
	Shader ourShaderPiso("vertexShader.vs", "fragmentShader.fs");
	Shader ourLight("vertexLight.vl", "fragmentLight.fl");

	GeneracionBuffer(VAO, VBO, EBO, vertices, sizeof(vertices), indices, sizeof(indices), VAO_L);
	GeneracionBuffer(VAO_P, VBO_P, EBO_P, pisoVertices, sizeof(pisoVertices), pisoIndices, sizeof(pisoIndices), VAO_L);


	std::vector<Model> models;
	
	models.push_back(Model("Modelos/backpack/personaje.obj"));
	models.push_back(Model("Modelos/backpack/egg_obj.obj"));
	models.push_back(Model("Modelos/backpack/gallo.obj"));
	models.push_back(Model("Modelos/backpack/guante.obj"));

	

	
	//updateWindow(window, ourShader, ourModel);
	float x;
	float y;
	int type;
	float speed;

	// Generar cubos al inicio
	for (int i = 0; i < 6; ++i) {

		

		if (i != 0)
		{
			x = ((rand() % 200) / 100.0f - 1.0f) * width / 80; // posici�n x aleatoria
			y = 12.5f;
			speed = ((rand() % 100) / 100.0f) * 1.0f + 0.5f; // velocidad aleatoria
			type = 2 + (rand() % 2);
			//type = 2;
		}
		else
		{
			x = 0;
			y = 1.5f;
			type = 1;
			speed = 0.0f;
		}
		// parte superior de la pantalla

		posCube.push_back({ vec3(x, y, 0.0f), speed ,type });
	}
	posCube.push_back({ vec3(0, 1.5, 0.0f), 0.0f ,4 });
	
	textures.push_back(TextureFromFile("egg_obj.jpg", directory));
	textures.push_back(TextureFromFile("malla.jpg", directory));
	textures.push_back(TextureFromFile("oro.jpg", directory));

	updateWindow(window, ourShader, ourLight, ourShaderPiso, models);

	DeleteVertexArrays(VAO);
	DeleteVertexArrays(VAO_L);
	DeleteBuffer(VBO, EBO);

	glfwTerminate();

	while (getchar() != 'e')
	{

	}
	return 0;
}
void initGLFWVersion()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif // __APPLE__
}
bool gladLoad()
{
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cout << "Falla al cargar glad" << endl;
		return false;
	}
	return true;
}
void framebuffer_size_callback(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, w, h);
}
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS || health_i < 0)
	{
		std::cout << "Puntos Totales: " << points << std::endl;
		glfwSetWindowShouldClose(window, true);
		
	}
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	CameraInput(window);
	PlayerInput(window);
}
void CameraInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(FORWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(LEFT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(RIGHT, deltaTime);
	}
}
float moveStartTime = -1.0f;  // Almacena el momento en que se presiona F
bool isMoving = false;        // Controla el estado del movimiento (avanzando o retrocediendo)

void PlayerInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		posCube[0].position.y += 0.1f;
		posCube[6].position.y += 0.1f;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		posCube[0].position.y -= 0.1f;
		posCube[6].position.y -= 0.1f;
	}	//
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		posCube[0].position.x -= 0.1f;
		//posCube[6].position.x -= 0.1f;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		posCube[0].position.x += 0.1f;
		//posCube[6].position.x += 0.1f;
	}

	// Controlar el movimiento con la tecla F
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && !isMoving)
	{
		//moveStartTime = glfwGetTime();  // Guardar el tiempo en que se presiona F
		posCube[6].speed = 1;
		
	}
	// Si el cubo est� en movimiento
	if (posCube[6].speed != 0) {
		colisionGuante();
	}
		
		

	if (DetecCollision())
	{
		// L�gica de colisi�n aqu�
	}


}

void Mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	/*float xpos = xposIn;
	float ypos = yposIn;

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);*/
}
void Scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}
void updateWindow(GLFWwindow* window, Shader ourShader, Shader ourLight, Shader ourShaderPiso, std::vector<Model> models)
{
	while (!glfwWindowShouldClose(window))
	{
		float currenTime = glfwGetTime();
		deltaTime = currenTime - lastTime;
		lastTime = currenTime;

		processInput(window);

		
		
		updatePhysics(deltaTime);
		

		glClearColor(0.1f, 0.2f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ourShader.use();

		ourShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
		ourShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
		ourShader.setVec3("dirLight.diffuse", 0.3f, 0.3f, 0.3f);
		ourShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);

		ourShader.setVec3("viewPos", camera.Position);

		ourShader.setVec3("material.diffuse", 5.0f, 5.0f, 5.0f);
		ourShader.setVec3("material.specular", 0.5f, 0.5f, 0.5f);
		ourShader.setFloat("material.shininess", 32.0f);

		for (auto& cube : posCube) {
			cube.position.y -= cube.speed * deltaTime;

			//Tipo 1 = Personaje
			//tipo 2 = Huevos
			//tipo 3 = gallinas
			//tipo 4 = pu�o / ataque 
			if (cube.type == 2 || cube.type == 3)
			{
				// Reposicionar cubo en la parte superior si sale de la pantalla
				if (cube.position.y < 0.0f) {
					cube.position.y = 12.5f;
					cube.position.x = ((rand() % 200) / 100.0f - 1.0f) * width / 80; // posici�n x aleatoria
					//cube.speed = ((rand() % 100) / 100.0f) * 2.0f + 1.5f;
					cube.type = 2 + (rand() % 2);
				}

				
			}

			//Limitar el movimiento del personaje
			if (cube.type == 1)
			{
				cube.position.y = std::max(cube.position.y, -1.5f);
				cube.position.x = std::max(cube.position.x, -5.0f);
				cube.position.x = std::min(cube.position.x, 5.0f);
				cube.position.y = std::min(cube.position.y, 4.5f);

				/*if (cube.position.y < 0.0f)
				{
					cube.position.y = 0.0f;
				}*/
			}
			if (cube.type == 4)
			{
				
				cube.position.x = posCube[0].position.x;
				cube.position.y += cube.speed;

				if (cube.position.y > posCube[0].position.y + 3)  // Si han pasado menos de 1 segundo
				{
					 posCube[6].position = posCube[0].position;
					 posCube[6].speed = 0;
				}
				//cube.position.x = posCube[0].position.x;
				/*if (cube.position.y < 0.0f)
				{
					cube.position.y = 0.0f;
				}*/
			}
		}
		/*// Verificar colisi�n con el cubo jugador
		if (CheckCollision(pos, cube.position, 0.5f, 0.2f)) {
			isColliding = true;
		}
		shader.setVec3("overrideColor", vec3(1.0f, 1.0f, 1.0f));
		TransformCubo2(shader, cube.position);
	}

	// Cambiar color del cubo principal seg�n colisi�n
	if (isColliding) {
		shader.setVec3("overrideColor", vec3(1.0f, 0.0f, 0.0f));  // Rojo
	}
	else {
		shader.setVec3("overrideColor", vec3(1.0f, 1.0f, 1.0f));  // Blanco o el color original
	}*/

	//seeeeeeeeeeeeeeeeeeeeeeeeeeeeeee

		TransformCamera(ourShader);
		TransformCubo(ourShader, models);



		ourShaderPiso.use();

		ourShaderPiso.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
		ourShaderPiso.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
		ourShaderPiso.setVec3("dirLight.diffuse", 0.3f, 0.3f, 0.3f);
		ourShaderPiso.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);

		ourShaderPiso.setVec3("viewPos", camera.Position);

		ourShaderPiso.setVec3("material.diffuse", 0.2f, 1.0f, 0.2f);
		ourShaderPiso.setVec3("material.specular", 0.5f, 0.5f, 1.5f);
		ourShaderPiso.setFloat("material.shininess", 32.0f);

		CameraUniform(ourShaderPiso);
		TransformPiso(ourShaderPiso);

		//ourModel.Draw(ourShader);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}
void GeneracionBuffer(GLuint& VAO,
	GLuint& VBO,
	GLuint& EBO,
	const GLfloat* vertices,
	size_t verticesSize,
	const GLuint* indices,
	size_t indicesSize,
	GLuint& VAO_L)
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, verticesSize, vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize, indices, GL_STATIC_DRAW);

	VertexAttribute(0, 3, 6, 0);
	VertexAttribute(1, 3, 6, 3);

	//light
	glGenVertexArrays(1, &VAO_L);
	glBindVertexArray(VAO_L);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

	VertexAttribute(0, 3, 6, 0);
	VertexAttribute(1, 3, 6, 3);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
void VertexAttribute(int layout, int data, int total, int start)
{
	glVertexAttribPointer(layout, data, GL_FLOAT, GL_FALSE, total * sizeof(float), (void*)(start * sizeof(float)));
	glEnableVertexAttribArray(layout);
}
void DeleteVertexArrays(GLuint& VA)
{
	glDeleteVertexArrays(1, &VA);
}
void DeleteBuffer(GLuint& VBO, GLuint& EBO)
{
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}
void TransformCubo(Shader ourShader, std::vector<Model> models)
{
	int tam = posCube.size();
	glBindVertexArray(VAO);
	for (int i = 0; i < tam; i++)
	{
		mat4 modelo = mat4(1.0f);
		modelo = translate(modelo, posCube[i].position);
		glActiveTexture(GL_TEXTURE0); // Unidad de textura 0
		if (posCube[i].type == 1 )
		{
			modelo = glm::rotate(modelo, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			modelo = glm::rotate(modelo, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));

			
			glBindTexture(GL_TEXTURE_2D, textures[0]); // Vincular textura 1
		}else
		{
			if (posCube[i].type == 2)
				glBindTexture(GL_TEXTURE_2D, textures[2]);
			if (posCube[i].type == 3)
				glBindTexture(GL_TEXTURE_2D, textures[1]);
			if (posCube[i].type == 4)
				glBindTexture(GL_TEXTURE_2D, textures[0]);
			
		}

		ourShader.setInt("texture1", 0); // "texture1" corresponde a la unidad de textura 0
		ourShader.setMat4("model", modelo);

		
		models[posCube[i].type - 1].Draw(ourShader);


		glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(indices[0]), GL_UNSIGNED_INT, 0);
	}

}

void TransformPiso(Shader ourShader)
{
	glBindVertexArray(VAO_P);
	mat4 modelo = mat4(1.0f);
	modelo = translate(modelo, pisoPosCube[0]);

	ourShader.setMat4("model", modelo);
	glDrawElements(GL_TRIANGLES, sizeof(pisoIndices) / sizeof(pisoIndices[0]), GL_UNSIGNED_INT, 0);
}
void TransformCuboLight(Shader ourLight)
{
	int tam = sizeof(posCubeLight) / sizeof(posCubeLight[0]);
	glBindVertexArray(VAO_L);
	for (int i = 0; i < tam; i++)
	{
		mat4 modelo = mat4(1.0f);
		modelo = translate(modelo, posCubeLight[i]);
		modelo = scale(modelo, vec3(0.2f));
		ourLight.setMat4("model", modelo);
		glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(indices[0]), GL_UNSIGNED_INT, 0);
	}
}


void TransformCamera(Shader ourShader)
{
	projection = ortho(leftCam, rightCam, bottom, top, near, far);
	view = camera.GetViewMatrix();

	CameraUniform(ourShader);
}
void CameraUniform(Shader shaderName)
{
	shaderName.setMat4("projection", projection);
	shaderName.setMat4("view", view);
}

void updatePhysics(float deltaTime)
{
	int tam = posCube.size();
	

	for (int i = 1; i < tam - 1; i++)
	{
		posCube[i].speed += posCube[i].speed * deltaTime * 0.1;
		posCube[i].position.y -= posCube[i].speed * deltaTime;//cuboVel.y *deltaTime;
	}


	/*if (posCube[0].y < 0.0f)
	{
		posCube[0].y = 0.0f;
		cuboVel.y = 0;
	}*/
}

bool AABBIntersect(AABB box1, AABB box2)
{
	bool col;
	bool xOverlap = box1.min.x <= box2.max.x && box1.max.x >= box2.min.x;
	bool yOverlap = box1.min.y <= box2.max.y && box1.max.y >= box2.min.y;
	bool zOverlap = box1.min.z <= box2.max.z && box1.max.z >= box2.min.z;

	col = xOverlap && yOverlap && zOverlap;
	return col;
}
AABB GenerateBoindingBox(vec3 position, float w, float h, float d)
{
	AABB box;
	box.min = vec3((position.x - w) / 2.0f, position.y, (position.z - d) / 2.0f);
	box.max = vec3((position.x + w) / 2.0f, position.y + h, (position.z + d) / 2.0f);
	return box;
}
bool DetecCollision()
{
	bool col = false;

	// Generamos la caja de colisi�n para el jugador
	AABB cajaPersonaje = GenerateBoindingBox(posCube[0].position, 1.0f, 1.0f, 1.0f);

	// Iteramos sobre los enemigos (posCube[1] hasta posCube[6])
	for (int i = 1; i < 6; ++i)
	{
		// Generamos la caja de colisi�n para cada enemigo
		AABB enemigo = GenerateBoindingBox(posCube[i].position, 1.0f, 1.0f, 1.0f);

		// Si hay colisi�n, actualizamos el valor de col
		if (AABBIntersect(cajaPersonaje, enemigo))
		{
			//std::cout << "Colisi�n con el enemigo #" << i << std::endl;
			posCube[i].position.y = 12.5f;
			posCube[i].position.x = ((rand() % 200) / 100.0f - 1.0f) * width / 80; // posici�n x aleatoria
			//posCube[i].speed = ((rand() % 100) / 100.0f) * 2.0f + 1.5f;
			posCube[i].type = 2 + (rand() % 2);
			col = true;
			if (posCube[i].type == 3)
			{
				health_i --;
			}
			else
			{
				points ++;
			}
		}
	}

	return col;
}

bool colisionGuante()
{
	bool col = false;
	int tam = 6;

	// Generamos la caja de colisi�n para el guante
	AABB golpe = GenerateBoindingBox(posCube[6].position, 1.0f, 1.0f, 1.0f);

	// Iteramos sobre los enemigos (posCube[1] hasta posCube[6])
	for (int i = 1; i < tam; ++i)
	{
		// Generamos la caja de colisi�n para cada enemigo
		AABB enemigo = GenerateBoindingBox(posCube[i].position, 1.0f, 1.0f, 1.0f);

		// Si hay colisi�n, actualizamos el valor de col
		if (AABBIntersect(golpe, enemigo))
		{
			//std::cout << "Golope al enemigo #" << i << std::endl;
			if (posCube[i].type == 3)
			{
				posCube[i].position.y = 12.5f;
				posCube[i].position.x = ((rand() % 200) / 100.0f - 1.0f) * width / 80; // posici�n x aleatoria
				posCube[i].speed = ((rand() % 100) / 100.0f) * 2.0f + 1.5f;
				posCube[i].type = 2 + (rand() % 2);
			}

			posCube[6].position = posCube[0].position;
			posCube[6].speed = 0;
			col = true;
		}
	}

	return col;
}




bool CheckCollision(const vec3& cube1Pos, const vec3& cube2Pos, float size1, float size2) {
	return abs(cube1Pos.x - cube2Pos.x) < (size1 + size2) / 2 &&
		abs(cube1Pos.y - cube2Pos.y) < (size1 + size2) / 2;
}