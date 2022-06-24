#include <glad/glad.h>// 必须放在所有头文件之前，不管在哪里，不管在.h还是.cpp;
//如果包含了  glew.h头文件，就不要在包含glad.h头文件
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <windows.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <irrklang/irrKlang.h>
#include "shader.h"
#include "skyboxcube.h"
#include "camera.h"
#include "model.h"
#include "texture.h"
#include "file/shprogram.h"
#include "file/Object3D.h"
#include "file/Ocean.h"
using namespace irrklang;

//HJ_女神像碰撞检测
#define outsideD  150 //包围盒外圈半径
#define insideD  149  //包围盒内圈半径

class House
{
public:
	float x = 0;
	float y = 0;
	float z = -40;//记录中心坐标
};

float remember_distance[3] = { 0,0,0 };

//HJ_女神像碰撞检测

/***** 一些定义在main函数外面的公共参数 *****/
// ocean海洋基本参数
#define WIND_DIRECTION	{ -0.4f, -0.9f } //海洋风向
float oceancolors[][3] = {
	{ 0.0056f, 0.0194f, 0.0331f },	// deep blue
	{ 0.1812f, 0.4678f, 0.5520f },	// carribbean
	{ 0.0000f, 0.2307f, 0.3613f },	// light blue
	{ 0.2122f, 0.6105f, 1.0000f },
	{ 0.0123f, 0.3613f, 0.6867f },
	{ 0.0000f, 0.0999f, 0.4508f },
	{ 0.0000f, 0.0331f, 0.1329f },
	{ 0.0000f, 0.0103f, 0.0331f }
}; //海洋颜色的方案，有好几套
int currentcolor = 2; //海洋颜色选择2号

ISoundEngine* SoundEngine = createIrrKlangDevice(); //声明一个声音的引擎

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
bool light = false; //控制手电筒开关的变量
bool first = true;//这是一个打印鲁迅的话的bool

// camera
Camera camera(glm::vec3(-1.0f, 3.0f, -1.0f)); //相机的位置
//Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

//控制鼠标移动的几个变量
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// show
int show = 0; //切换场景1 2

//LXM
#define minDistance 3 //判断最小距离
#define modelnum 13 //贝壳数量
bool isclose[modelnum]; //标记是否靠近某个贝壳
float Mdistance[modelnum]; //实时更新每个贝壳到摄像机的距离
int nearmodel = -1;
glm::vec3 shellPosition[modelnum];//记录每个贝壳的pos
glm::mat4 modelMatrices[modelnum];//记录每个贝壳的modelmatrix

//测试靠近哪个贝壳的函数
void testclose()
{
	//初始化
	nearmodel = -1;
	float neardistance = 10000;
	for (int i = 0; i < modelnum; i++)
	{
		isclose[i] = false;
		Mdistance[i] = 0;
	}

	for (int i = 0; i < modelnum; i++)
	{
		//更新每个贝壳到摄像机的距离
		Mdistance[i] = pow(abs(camera.Position.x - shellPosition[i].x), 2) + pow(abs(camera.Position.y - shellPosition[i].y), 2) + pow(abs(camera.Position.z - shellPosition[i].z), 2);

		if (Mdistance[i] < minDistance)//如果小于最小距离，设置为靠近
		{
			isclose[i] = true;
			//printf("I am closing %d", i);
		}
		else isclose[i] = false;
	}

	for (int i = 0; i < modelnum; i++)
	{
		//在所有靠近的贝壳中，找到一个最近的
		if (isclose[i] == 1 && Mdistance[i] < neardistance)
		{
			nearmodel = i;
			neardistance = Mdistance[i];
		}
	}
}

//判断鼠标是否按下
int press = 0;
void mouseset(GLFWwindow* window, int button, int move, int mode)
{
	if (button == 0 && move == 1)
	{
		press = 1;
	}
	if (button == 0 && move == GLFW_RELEASE)
		press = 0;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		light = true;
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

//HJ_女神像碰撞检测
void collideTest()
{
	House h;
	float current_diatance = pow(abs(camera.Position.x - h.x), 2) + pow(abs(camera.Position.y - h.y), 2) + pow(abs(camera.Position.z - h.z), 2);

	if (current_diatance < outsideD && current_diatance > insideD)//当闯入包围盒内外圈之间时，记录相机此时坐标
	{
		remember_distance[0] = camera.Position.x;
		remember_distance[1] = camera.Position.y;
		remember_distance[2] = camera.Position.z;
	}
	if (current_diatance <= insideD)//当闯入包围盒内圈时，将它弹回去
	{
		camera.Position.x = remember_distance[0];
		camera.Position.y = remember_distance[1];
		camera.Position.z = remember_distance[2];
		//Camera c;
		//c.Position = glm::vec3(remember_distance[0], remember_distance[1], remember_distance[2]);
	}
}

float inter(float a, float b, float time, float interval);

int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "FreedomBeach", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouseset);//LXM
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);// 这里是设置指针的显示

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	// build and compile shaders
	// -------------------------
	Shader skyboxShader("resources/shader/skybox.vs", "resources/shader/skybox.fs"); //第一幕天空盒的shader
	Skybox skybox(0);
	Shader skyboxShader1("resources/shader/skybox1.vs", "resources/shader/skybox1.fs");//第二幕天空盒的shader
	Skybox skybox1(1);
	Shader lightingShader("resources/shader/light_casters.vs", "resources/shader/light_casters.fs"); //第一幕手电筒灯光的shader
	Shader lightingShader1("resources/shader/light_casters1.vs", "resources/shader/light_casters1.fs");//第二幕太阳平行光的shader
	Shader introShellShader("resources/shader/knowL.vs", "resources/shader/knowL.fs");//LXM

	ShaderProgram oceanProgram("file/ocean.vert", "file/ocean.frag");
	Shader beachshader("resources/shader/beach.vs", "resources/shader/beach.fs");

	// shader configuration
	// --------------------
	lightingShader.use();
	lightingShader.setInt("material.diffuse", 0);//CQY懂了
	lightingShader.setInt("material.specular", 1);
	lightingShader1.use();
	lightingShader1.setInt("material.diffuse", 0);
	lightingShader1.setInt("material.specular", 1);
	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);
	//float fogColor[] = { 1,1,1,1 };

	//声明导入的模型
	Model rock("resources/objects/rock/rock.obj");
	Model bomb("resources/objects/Bomb/Bomb.obj");
	Model goddness("resources/objects/goddness/goddness.obj");
	Model shell0("resources/objects/shell/shell0.obj");
	Model shell1("resources/objects/shell/shell1.obj");
	Model shell2("resources/objects/shell/shell2.obj");
	Model shell3("resources/objects/shell/shell3.obj");
	Model shell4("resources/objects/shell/shell4.obj");
	Model shell5("resources/objects/shell/shell5.obj");
	Model shell6("resources/objects/shell/shell6.obj");
	Model shell7("resources/objects/shell/shell7.obj");
	Model shell8("resources/objects/shell/shell8.obj");
	Model shell9("resources/objects/shell/shell9.obj");
	Model shell10("resources/objects/shell/shell10.obj");
	Model shell11("resources/objects/shell/shell11.obj");
	Model shell12("resources/objects/shell/shell12.obj");
	Model house("resources/objects/house/house.obj");

	// lighting info
	// -------------
	glm::vec3 lightPos(0.5f, 0.5f, 1.3f);

	// lights
	// ------
	glm::vec3 lightPositions[] = {
		glm::vec3(5.0f, 5.0f, 20.0f),
	};
	glm::vec3 lightColors[] = {
		glm::vec3(150.0f, 150.0f, 150.0f),
	};
	int nrRows = 7;
	int nrColumns = 7;
	float spacing = 2.5;

	// initialize static shader uniforms before rendering
	// --------------------------------------------------
	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

	//LXM 以下代码都是在处理点击贝壳出现的图片
	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float vertices[] = {
		// positions          // colors           // texture coords
		 0.8f,  0.8f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
		 0.8f, -0.8f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
		-0.8f, -0.8f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
		-0.8f,  0.8f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left
	};
	unsigned int indices[] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};
	unsigned int VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// texture coord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	// load and create a texture
	// -------------------------
	unsigned int texture[modelnum];
	int width[modelnum], height[modelnum], nrChannels[modelnum];
	unsigned char* data[modelnum];

	for (int i = 0; i < modelnum; i++)
	{
		glGenTextures(1, &texture[i]);
		glBindTexture(GL_TEXTURE_2D, texture[i]); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
		stbi_set_flip_vertically_on_load(true);
		switch (i)
		{
		case 0:
			data[i] = stbi_load("resources/textures/0.jpg", &width[i], &height[i], &nrChannels[i], 0);
			break;
		case 1:
			data[i] = stbi_load("resources/textures/1.jpg", &width[i], &height[i], &nrChannels[i], 0);
			break;
		case 2:
			data[i] = stbi_load("resources/textures/2.jpg", &width[i], &height[i], &nrChannels[i], 0);
			break;
		case 3:
			data[i] = stbi_load("resources/textures/3.jpg", &width[i], &height[i], &nrChannels[i], 0);
			break;
		case 4:
			data[i] = stbi_load("resources/textures/0.jpg", &width[i], &height[i], &nrChannels[i], 0);
			break;
		case 5:
			data[i] = stbi_load("resources/textures/5.jpg", &width[i], &height[i], &nrChannels[i], 0);
			break;
		case 6:
			data[i] = stbi_load("resources/textures/6.jpg", &width[i], &height[i], &nrChannels[i], 0);
			break;
		case 7:
			data[i] = stbi_load("resources/textures/7.jpg", &width[i], &height[i], &nrChannels[i], 0);
			break;
		case 8:
			data[i] = stbi_load("resources/textures/8.jpg", &width[i], &height[i], &nrChannels[i], 0);
			break;
		case 9:
			data[i] = stbi_load("resources/textures/9.jpg", &width[i], &height[i], &nrChannels[i], 0);
			break;
		case 10:
			data[i] = stbi_load("resources/textures/10.jpg", &width[i], &height[i], &nrChannels[i], 0);
			break;
		case 11:
			data[i] = stbi_load("resources/textures/5.jpg", &width[i], &height[i], &nrChannels[i], 0);
			break;
		case 12:
			data[i] = stbi_load("resources/textures/12.jpg", &width[i], &height[i], &nrChannels[i], 0);
			break;
		}
		if (data[i])
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width[i], height[i], 0, GL_RGB, GL_UNSIGNED_BYTE, data[i]);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			std::cout << "Failed to load texture " << i << "  " << std::endl;
		}
		stbi_image_free(data[i]);
	}

	//随机生成贝壳的位置和matrix
	srand(glfwGetTime()); // initialize random seed
	for (unsigned int i = 0; i < modelnum; i++)
	{
		glm::mat4 model = glm::mat4(1.0f);
		shellPosition[i].x = (rand() % 12 + (double)(rand() / (double)RAND_MAX)) - 5;
		shellPosition[i].z = -(rand() % 30 + (double)(rand() / (double)RAND_MAX));
		shellPosition[i].y = 0.65;
		model = glm::translate(model, glm::vec3(shellPosition[i].x, shellPosition[i].y, shellPosition[i].z)); //相当于translate量就是模型的坐标
		model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));
		float rotAngle = (rand() % 360); //随机旋转角度
		model = glm::rotate(model, rotAngle, glm::normalize(glm::vec3(0.0, 1.0, 0.0)));
		modelMatrices[i] = model;
	}

	//生成海洋
	Ocean ocean;
	float			flipYZ[16] = { 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 };

	glm::vec2		perlinoffset = { 0, 0 };
	float			w[2] = WIND_DIRECTION;
	int				pattern[4];
	GLuint			subset = 0;
	glm::vec3 colors = { oceancolors[currentcolor][0],
						 oceancolors[currentcolor][1],
						 oceancolors[currentcolor][2] };

	// 生成沙滩
	//row_num表示网格行数，col_num表示网格列数
	int row_num = 1000, col_num = 800;
	std::vector<float> p;
	float xx, zz = -50;
	for (int i = 0; i < row_num; i++)
	{
		xx = -5;
		for (int j = 0; j < col_num; j++)
		{
			p.push_back(xx);
			p.push_back(0);
			p.push_back(zz);
			p.push_back(1.0f / col_num * j);
			p.push_back(1 - i * 1.0f / row_num);
			xx += 0.1;//-5~75
		}
		zz += 0.1;
	}
	std::vector<unsigned int> indicess;
	for (int i = 1; i < row_num; i++) {
		for (int j = 1; j < col_num; j++) {
			indicess.push_back((i - 1) * col_num + j - 1);
			indicess.push_back((i - 1) * col_num + j);
			indicess.push_back(i * col_num + j - 1);

			indicess.push_back(i * col_num + j - 1);
			indicess.push_back((i - 1) * col_num + j);
			indicess.push_back(i * col_num + j);
		}
	}
	unsigned int VBOsand, VAOsand, EBOsand;
	glGenVertexArrays(1, &VAOsand);
	glGenBuffers(1, &VBOsand);
	glGenBuffers(1, &EBOsand);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAOsand);

	glBindBuffer(GL_ARRAY_BUFFER, VBOsand);
	glBufferData(GL_ARRAY_BUFFER, p.size() * sizeof(float), &p[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOsand);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicess.size() * sizeof(unsigned int), &indicess[0], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// color attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	//加载纹理，创建shader并连接到程序上
	unsigned int tex1 = loadTexture("file/res/SAND3.bmp");
	unsigned int tex2 = loadTexture("file/wavenew9.png");//这个有透明通道
	unsigned int dep = loadTexture("file/Terrain5.bmp");
	beachshader.use();
	beachshader.setInt("dep", 0);
	beachshader.setInt("tex1", 1);
	beachshader.setInt("tex2", 2);
	//生成沙滩end

	SoundEngine->play2D("resources/audio/wave1.wav", true); //播放音乐

	float startTime = glfwGetTime();
	float time;
	float alpha = 0;
	float time1 = 0;
	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		time = currentFrame - startTime;
		//time=15
		if (time < 15) show = 0;
		else show = 1;

		processInput(window);

		// render
		// ------
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//清屏

		glm::mat4 view = camera.GetViewMatrix();
		//model初始化为单位矩阵
		glm::mat4 model = glm::mat4(1.0f);

		//第一幕
		if (show == 0)
		{
			//1到2幕的摄像机转场
			if (time > 12 && time < 15)
			{
				camera.ProcessKeyboard(FORWARD, deltaTime);
				//camera.ProcessKeyboard(UPWARD, deltaTime);
				//camera.ProcessKeyboard(RIGHT, deltaTime);
			}

			//设置lightingshader里的一些参数
			lightingShader.use();
			lightingShader.setVec3("light.position", camera.Position);
			lightingShader.setVec3("light.direction", camera.Front);
			lightingShader.setFloat("light.cutOff", glm::cos(glm::radians(12.5f)));
			lightingShader.setFloat("light.outerCutOff", glm::cos(glm::radians(17.5f)));
			lightingShader.setVec3("viewPos", camera.Position);
			if (light == 0)//没开灯
			{
				lightingShader.setVec3("light.ambient", 0.1f, 0.1f, 0.1f);
				lightingShader.setVec3("light.diffuse", 0.1f, 0.1f, 0.1f);
				lightingShader.setVec3("light.specular", 0.1f, 0.1f, 0.1f);
				lightingShader.setFloat("light.constant", 1.0f);
				lightingShader.setFloat("light.linear", 0.09f);
				lightingShader.setFloat("light.quadratic", 0.032f);
			}
			else//开灯了
			{
				lightingShader.setVec3("light.ambient", 0.1f + time * 0.07, 0.1f + time * 0.07, 0.1f + time * 0.07);
				lightingShader.setVec3("light.diffuse", 0.8f, 0.8f, 0.8f);
				lightingShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
				lightingShader.setFloat("light.constant", 1.0f);
				lightingShader.setFloat("light.linear", 0.09f);
				lightingShader.setFloat("light.quadratic", 0.032f);
			}
			lightingShader.setFloat("material.shininess", 2.0f);

			//configure view/projection matrices
			projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
			glm::mat4 view = camera.GetViewMatrix();

			//女神像
			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(-1.2f, -0.5f, -9.0f)); // translate it down so it's at the center of the scene
			model = glm::rotate(model, glm::radians(-50.0f), glm::normalize(glm::vec3(0.0, 1.0, 0.0)));
			model = glm::rotate(model, glm::radians((float)glfwGetTime() * -40.0f), glm::normalize(glm::vec3(0.0, 1.0, 0.0)));
			model = glm::scale(model, glm::vec3(0.002f, 0.002f, 0.002f));	// it's a bit too big for our scene, so scale it down
			lightingShader.setMat4("model", model);
			lightingShader.setMat4("projection", projection);
			lightingShader.setMat4("view", view);
			goddness.Draw(lightingShader);

			//天空盒
			skyboxShader.use();
			view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
			projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_WIDTH, 0.1f, 100.0f);
			skyboxShader.setMat4("view", view);
			skyboxShader.setMat4("projection", projection);
			if (light == 1)
			{
				skyboxShader.setVec3("light.ambient", 0.4f + time * 0.1, 0.4f + time * 0.1, 0.4f + time * 0.1);
				skyboxShader.setVec4("fogcolor", 1, 1, 1, 0.5);
				if (time < 10)
					skyboxShader.setFloat("u_time", 0);
				else
					skyboxShader.setFloat("u_time", time - 10);
			}
			else
			{
				skyboxShader.setVec3("light.ambient", 0.4f, 0.4f, 0.4f);
				skyboxShader.setVec4("fogcolor", 0, 0, 0, 0.8);
				if (time < 10)
					skyboxShader.setFloat("u_time", 0);
				else
					skyboxShader.setFloat("u_time", time - 10);
			}
			skybox.Draw();
		}

		//第二幕
		if (show == 1)
		{
			if (light == 0)
			{
				cout << "有一分热，发一分光，就令萤火一般，也可以在黑暗里发一点光，不必等候炬火。" << endl << "此后如竟没有炬火，我便是唯一的光。" << endl;
				SoundEngine->drop();
				glfwTerminate();
				return 0;
			}
			else
			{
				if (first)
				{
					cout << "倘若有了炬火，出了太阳，我们自然心悦诚服的消失。" << endl << "不但毫无不平，而且还要随喜赞美这炬火或太阳；因为他照了人类，连我都在内。" << endl << "我又愿中国青年都只是向上走，不必理会这冷笑和暗箭。" << endl;
					first = false;
				}
			}

			if (time > 15 && time < 18)
				camera.ProcessKeyboard(BACKWARD, deltaTime);

			glm::vec3 lightpos(0.0f, 5.0f, 20.0f);
			glm::vec3 lightdir(0.0f, -5.0f, -10.0f);
			lightingShader1.use();
			lightingShader1.setVec3("light.position", camera.Position);
			//lightingShader1.setVec3("light.position", lightpos);
			//lightingShader1.setVec3("light.direction", lightdir);
			lightingShader1.setVec3("light.direction", camera.Front);
			lightingShader1.setVec3("viewPos", camera.Position);
			lightingShader1.setVec3("light.ambient", 0.4f, 0.4f, 0.4f);
			lightingShader1.setVec3("light.diffuse", 0.66f, 0.55f, 0.60f);
			lightingShader1.setVec3("light.specular", 0.5f, 0.5f, 0.5f);
			lightingShader1.setFloat("material.shininess", 2.0f);
			projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
			view = camera.GetViewMatrix();
			lightingShader1.setMat4("projection", projection);
			lightingShader1.setMat4("view", view);
			model = glm::mat4(1.0f);
			lightingShader1.setMat4("model", model);

			//HJ_女神像碰撞检测
			collideTest();
			//printf("当前坐标:X=%3.1f  Y=%3.1f Z =%3.1f  \n", camera.Position.x, camera.Position.y, camera.Position.z); /**< 字符串赋值 */

			//女神像
			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(0.0f, 0.0f, -40.0f)); // translate it down so it's at the center of the scene
			model = glm::rotate(model, glm::radians(-50.0f), glm::normalize(glm::vec3(0.0, 1.0, 0.0)));
			model = glm::scale(model, glm::vec3(0.005f, 0.005f, 0.005f));	// it's a bit too big for our scene, so scale it down
			lightingShader1.setMat4("model", model);
			goddness.Draw(lightingShader1);

			//阿那亚
			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(20.0f, 0.0f, -15.0f)); // translate it down so it's at the center of the scene
			model = glm::rotate(model, glm::radians(-90.0f), glm::normalize(glm::vec3(0.0, 1.0, 0.0)));
			model = glm::scale(model, glm::vec3(0.001f, 0.001f, 0.001f));	// it's a bit too big for our scene, so scale it down
			lightingShader1.setMat4("model", model);
			house.Draw(lightingShader1);

			testclose();

			//只放大
			for (int i = 0; i < modelnum; i++)
			{
				if (i != nearmodel)
				{
					lightingShader1.setMat4("model", modelMatrices[i]);
				}
				else
				{
					model = modelMatrices[i];
					model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));
					lightingShader1.setMat4("model", model);
					printf("closing\n");
					printf("%d\n", i);
				}
				switch (i)
				{
				case 0:shell0.Draw(lightingShader1); break;
				case 1:shell1.Draw(lightingShader1); break;
				case 2:shell2.Draw(lightingShader1); break;
				case 3:shell3.Draw(lightingShader1); break;
				case 4:shell4.Draw(lightingShader1); break;
				case 5:shell5.Draw(lightingShader1); break;
				case 6:shell6.Draw(lightingShader1); break;
				case 7:shell7.Draw(lightingShader1); break;
				case 8:shell8.Draw(lightingShader1); break;
				case 9:shell9.Draw(lightingShader1); break;
				case 10:shell10.Draw(lightingShader1); break;
				case 11:shell11.Draw(lightingShader1); break;
				case 12:shell12.Draw(lightingShader1); break;
				}
				//出现科普信息
				if (press && nearmodel == i)
				{
					printf("当前最近的是%d号贝壳\n", i);
					//使用

					// bind Texture
					glBindTexture(GL_TEXTURE_2D, texture[i]);

					// render container
					introShellShader.use();
					glBindVertexArray(VAO);
					glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				}
			}

			// CQY 添加海洋
			perlinoffset[0] = -w[0] * currentFrame * 0.06f;
			perlinoffset[1] = -w[1] * currentFrame * 0.06f;

			//glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			oceanProgram.Use();
			oceanProgram.setMat4Uniform("matViewProj", projection * view);
			oceanProgram.setMat4Uniform("matWorld", glm::mat4(1.0));
			oceanProgram.setVec2Uniform("perlinOffset", perlinoffset);
			oceanProgram.setVec3Uniform("eyePos", camera.Position);
			oceanProgram.setVec3Uniform("oceanColor", colors);
			ocean.Draw(oceanProgram);

			// CQY 添加沙滩
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			//glDisable(GL_DEPTH_TEST);  // 此处需要禁止深度测试
			beachshader.use();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, dep);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, tex1);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, tex2);
			projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
			view = camera.GetViewMatrix();
			beachshader.setMat4("projection", projection);
			beachshader.setMat4("view", view);

			glBindVertexArray(VAOsand);
			glm::mat4 Model = glm::mat4(1.0f);
			Model = glm::translate(Model, glm::vec3(-10.0f, -1.3f, 0.0f));//-2.3
			beachshader.setMat4("model", Model);

			glm::mat4 localtraf_b = glm::scale(glm::mat4(1), glm::vec3(1.8f, 1.0f, 2.0f));//(1.5f, 0, 2.5f)
			beachshader.setMat4("matLocal", localtraf_b);
			glDrawElements(GL_TRIANGLES, (row_num - 1) * (col_num - 1) * 6, GL_UNSIGNED_INT, 0);
			glDisable(GL_BLEND);
			//glEnable(GL_DEPTH_TEST);

			//skybox
			skyboxShader1.use();
			projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
			//view = camera.GetViewMatrix();
			view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
			skyboxShader1.setMat4("view", view);
			skyboxShader1.setMat4("projection", projection);
			skyboxShader1.setVec3("light.ambient", 0.85f, 0.85f, 0.85f);
			skybox1.Draw();
		}

		glDepthFunc(GL_LESS); // set depth function back to default

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	SoundEngine->drop();
	glfwTerminate();
	return 0;
}

float inter(float a, float b, float time, float interval)
{
	float tmp = ((b - a) / interval) * time + a;
	if (a < b)
		return tmp > b ? b : tmp;
	else
		return tmp > a ? a : tmp;
}