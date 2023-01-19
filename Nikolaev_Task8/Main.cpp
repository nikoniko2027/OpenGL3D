#include <iostream>
#include <Windows.h>
#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <glm\glm.hpp>
#include <glm\gtx\transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include "Figure.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "Shader.h"

//=====================================================================
//						���������� ����������
//=====================================================================
std::vector<model> models;
glm::mat4 T;
glm::vec3 S, P, u;

float dist;
float fovy, aspect;
float fovy_work, aspect_work;
float near_view, far_view;
float n, f;
float l, r, t, b;

double lastX, lastY;
bool mbState = false;
bool checkPolMode = true;

enum projType { Ortho, Frustum, Perspective } pType;

glm::mat4 lightM;
glm::mat4 myLightM;

glm::vec3 amb = glm::vec3(0.2f, 0.2f, 0.2f);
glm::vec3 dif = glm::vec3(0.5f, 0.5f, 0.5f);
glm::vec3 spec = glm::vec3(1.0f, 1.0f, 1.0f);

//=====================================================================


void initWorkPars() { // ������������� ������� ���������� ������
	n = near_view;
	f = far_view;
	fovy_work = fovy;
	aspect_work = aspect;
	float Vy = 2 * near_view * glm::tan(fovy / 2);
	float Vx = aspect * Vy;
	l = -Vx / 2;
	r = Vx / 2;
	b = -Vy / 2;
	t = Vy / 2;
	dist = glm::length(P - S);
	T = glm::lookAt(S, P, u);
}

void readFromFile(const char* fileName) { // ������ ����� �� �����
	std::ifstream in;
	in.open(fileName);
	if (in.is_open()) {
		models.clear(); // ������� ��������� ������ ��������
		glm::mat4 M = glm::mat4(1.f); // ������� ��� ��������� ��������� �������
		glm::mat4 initM; // ������� ��� ���������� �������������� ������� �������
		std::vector<glm::mat4> transforms; // ���� ������ ��������������
		std::vector<mesh> figure; // ������ ������� ���������� �������
		float thickness = 2;
		glm::vec3 ambient = glm::vec3(0, 0, 0);
		glm::vec3 diffuse = glm::vec3(0, 0, 0);
		glm::vec3 specular = glm::vec3(0, 0, 0);
		float shininess = 1;
		std::string cmd;
		std::string str;
		std::getline(in, str);
		while (in) {
			if ((str.find_first_not_of("\t\r\n") != std::string::npos) && (str[0] != '#')) {
				std::stringstream s(str);
				s >> cmd;
				if (cmd == "camera") { // ��������� ������
					float x, y, z;
					s >> x >> y >> z; // ���������� ����� ����������
					S = glm::vec3(x, y, z);
					s >> x >> y >> z; // �����, � ������� ��������� ������ ����������
					P = glm::vec3(x, y, z);
					s >> x >> y >> z; // ������ ����������� �����
					u = glm::vec3(x, y, z);

				}
				else if (cmd == "screen") { // ��������� ���� ����������
					s >> fovy_work >> aspect >> near_view >> far_view; // ��������� �������
					fovy = glm::radians(fovy_work); // ������� ���� �� �������� � ��������
				}
				else if (cmd == "color") {
					float r, g, b;
					s >> r >> g >> b;
					ambient = glm::vec3(r, g, b) / 255.f;
					diffuse = ambient;
					specular = ambient;
				}
				else if (cmd == "ambient") {
					float r, g, b;
					s >> r >> g >> b;
					ambient = glm::vec3(r, g, b);
				}
				else if (cmd == "diffuse") {
					float r, g, b;
					s >> r >> g >> b;
					diffuse = glm::vec3(r, g, b);
				}
				else if (cmd == "specular") {
					float r, g, b;
					s >> r >> g >> b;
					specular = glm::vec3(r, g, b);
				}
				else if (cmd == "shininess") {
					s >> shininess;
				}
				else if (cmd == "thickness") {
					s >> thickness;
				}
				else if (cmd == "mesh") {
					std::vector<vertex> vertices; // ������ ����� �������
					int N, K;
					s >> N >> K;
					std::string str1;
					while (N > 0) {
						std::getline(in, str1);
						if ((str1.find_first_not_of("\t\r\n") != std::string::npos) && (str1[0] != '#')) {
							float x, y, z; // ���������� ��� ����������
							float nx, ny, nz;
							std::stringstream s1(str1); // ��� ���� ��������� ����� �� ������ str1
							s1 >> x >> y >> z;
							s1 >> nx >> ny >> nz;
							vertices.push_back({ glm::vec3(x, y, z), glm::vec3(nx, ny, nz) }); // ��������� ����� � ������
							N--;
						}
					}

					std::vector<GLuint> indices;
					while (K > 0) {
						std::getline(in, str1);
						if ((str1.find_first_not_of("\t\r\n") != std::string::npos) && (str1[0] != '#')) {
							GLuint x;
							std::stringstream s1(str1);
							for (int i = 0; i < 3; i++) {
								s1 >> x;
								indices.push_back(x);
							}
							K--;
						}

					}
					figure.push_back(mesh(vertices, indices, { ambient, diffuse, specular, shininess }));
				}
				else if (cmd == "model") { // ������ �������� ������ �������
					float mVcx, mVcy, mVcz, mVx, mVy, mVz; // ��������� ������� model
					s >> mVcx >> mVcy >> mVcz >> mVx >> mVy >> mVz; // ��������� �������� ����������
					float S = mVx / mVy < 1 ? 2.f / mVy : 2.f / mVx;
					// ����� ����� �������� �� ������ ��������� � ������ �������
					// ����� �������� �������� ���������������
					initM = glm::scale(glm::vec3(S)) * glm::translate(glm::vec3(-mVcx, -mVcy, -mVcz));
					figure.clear();
				}
				else if (cmd == "figure") { // ������������ ����� ������							
					models.push_back(model(figure, M * initM));
				}
				else if (cmd == "translate") { // �������
					float Tx, Ty, Tz; // ��������� �������������� ��������
					s >> Tx >> Ty >> Tz; // ��������� ���������
					M = glm::translate(glm::vec3(Tx, Ty, Tz)) * M; // ��������� ������� � ������ ��������������	
				}
				else if (cmd == "scale") { // ���������������
					float S; // �������� ���������������
					s >> S; // ��������� ��������
					M = glm::scale(glm::vec3(S)) * M; // ��������� ��������������� � ������ ��������������	
				}
				else if (cmd == "rotate") { // �������
					float theta; // ���� �������� � ��������
					float nx, ny, nz; // ���������� ������������� ������� ��� ��������
					s >> theta >> nx >> ny >> nz; // ��������� ���������
												  // ��������� �������� � ������ ��������������
					M = glm::rotate(glm::radians(theta), glm::vec3(nx, ny, nz)) * M;
				}
				else if (cmd == "pushTransform") { // ���������� ������� � ����
					transforms.push_back(M); // ��������� ������� � ����
				}
				else if (cmd == "popTransform") { // ����� � ������� �� �����
					M = transforms.back(); // �������� ������� ������� �����
					transforms.pop_back(); // ���������� ������� �� �����	
				}
			}
			getline(in, str);
		}
		initWorkPars();
	}
}

//���������� ������� Resize
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}


glm::mat4 rotateP(float theta, glm::vec3 n, glm::vec3 P) {
	return glm::translate(P) * glm::rotate(theta, n) * glm::translate(-P);
}

//���������� ��������� �������
void cursorPos_callback(GLFWwindow* window, double xpos, double ypos) {
	glm::vec3 n = glm::vec3(lastY - ypos, lastX - xpos, 0); // ������, �������� ��� ��������
	glm::mat4 M = glm::rotate(glm::length(n) * 0.002f, n); // ������� ��������
	glm::vec3 P = M * glm::vec4(0, 0, -1, 1); // ������� ����� (0, 0, -1), �� ������� ������� �����������
	T = glm::lookAt(glm::vec3(0), P, u) * T; // ��������� � ����� ������� ��������� �����������

	lastX = xpos;
	lastY = ypos;
}

//���������� ������� ����������� �������
void cursorPosSave_callback(GLFWwindow* window, double xpos, double ypos) {
	if (mbState) {
		glm::vec3 n = glm::vec3(lastY - ypos, lastX - xpos, 0); // ������, �������� ��� ��������

		glm::mat4 M = rotateP(glm::length(n) * 0.002f, n, glm::vec3(0, 0, -dist));
		glm::vec3 u_new = glm::mat3(M) * glm::vec3(0, 1, 0);
		glm::vec3 S_new = glm::vec3(M * glm::vec4(0, 0, 0, 1));

		T = lookAt(S_new, glm::vec3(0, 0, -dist), u_new) * T;
	}

	lastX = xpos;
	lastY = ypos;
}

//O��������� ������� ������
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (action != GLFW_RELEASE) {
		switch (key) {
		case GLFW_KEY_4:
			if (mode == GLFW_MOD_SHIFT)
				myLightM = glm::inverse(T) * glm::translate(glm::vec3(0.0f, 0.0f, -0.3f)) * T * myLightM;
			else
				myLightM = glm::inverse(T) * glm::translate(glm::vec3(0.0f, 0.0f, 0.3f)) * T * myLightM;
			break;
		case GLFW_KEY_5:
			if (mode == GLFW_MOD_SHIFT)
				myLightM = glm::inverse(T) * glm::translate(glm::vec3(-0.3f, 0.0f, 0.0f)) * T * myLightM;
			else
				myLightM = glm::inverse(T) * glm::translate(glm::vec3(0.3f, 0.0f, 0.0f)) * T * myLightM;
			break;
		case GLFW_KEY_6:
			if (mode == GLFW_MOD_SHIFT)
				myLightM = glm::inverse(T) * glm::translate(glm::vec3(0.0f, -0.3f, 0.0f)) * T * myLightM;
			else
				myLightM = glm::inverse(T) * glm::translate(glm::vec3(0.0f, 0.3f, 0.0f)) * T * myLightM;
			break;
		case GLFW_KEY_7:
			if (mode == GLFW_MOD_SHIFT) {
				amb = amb * glm::vec3(1 / 1.1, 1 / 1.1, 1 / 1.1);
				spec = spec * glm::vec3(1 / 1.1, 1 / 1.1, 1 / 1.1);
			}
			else {
				amb = amb * glm::vec3(1.1, 1.1, 1.1);
				spec = spec * glm::vec3(1.1, 1.1, 1.1);
			}
			break;
		case GLFW_KEY_0:
			if (checkPolMode)
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			else
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			checkPolMode = !checkPolMode;
			break;
		case GLFW_KEY_ESCAPE:
			initWorkPars();
			break;
		case GLFW_KEY_3:
			pType = Perspective;
			break;
		case GLFW_KEY_1:
			pType = Ortho;
			break;
		case GLFW_KEY_W:
			if (mode == GLFW_MOD_SHIFT) {
				T = lookAt(glm::vec3(0, 0, -0.1), glm::vec3(0, 0, -0.2), glm::vec3(0, 0.1, 0)) * T;
			}
			else {
				T = lookAt(glm::vec3(0, 0, -1), glm::vec3(0, 0, -2), glm::vec3(0, 1, 0)) * T;
			}
			break;
		case GLFW_KEY_S:
			if (mode == GLFW_MOD_SHIFT)
				T = lookAt(glm::vec3(0, 0, 0.1), glm::vec3(0, 0, 0), glm::vec3(0, 0.1, 0)) * T;
			else
				T = lookAt(glm::vec3(0, 0, 1), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)) * T;
			break;
		case GLFW_KEY_A:
			if (mode == GLFW_MOD_SHIFT)
				T = lookAt(glm::vec3(-0.1, 0, 0), glm::vec3(-0.1, 0, -0.1), glm::vec3(0, 0.1, 0)) * T;
			else
				T = lookAt(glm::vec3(-1, 0, 0), glm::vec3(-1, 0, -1), glm::vec3(0, 1, 0)) * T;
			break;
		case GLFW_KEY_R: {
			glm::vec3 u_new = glm::mat3(rotate(0.1f, glm::vec3(0, 0, 1))) * glm::vec3(0, 1, 0);
			T = lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, -1), u_new) * T;
			break;
		}
		case GLFW_KEY_T: {
			if (mode == GLFW_MOD_SHIFT) {
				glm::mat4 M = rotateP(0.1, glm::vec3(1, 0, 0), glm::vec3(0, 0, -dist)); // ������� �������� ������������ ����� P
				glm::vec3 u_new = glm::mat3(M) * glm::vec3(0, 1, 0); // �������� ����������� �����
				glm::vec3 S_new = glm::vec3(M * glm::vec4(0, 0, 0, 1)); // �������� ������ ���������
																	   // ������� � ��� � ������� ������ ��������� � ����� �����, � �����������
																	   // ���������� - � ����� P
				T = lookAt(S_new, glm::vec3(0, 0, -dist), u_new) * T;
			}
			else {
				glm::mat4 M = rotate(0.1f, glm::vec3(1, 0, 0)); // ������� �������� ������������ Ox			
				glm::vec3 u_new = glm::mat3(M) * glm::vec3(0, 1, 0); // �������� ����������� �����			
				glm::vec3 P_new = glm::vec3(M * glm::vec4(0, 0, -1, 1)); // �������� �����, � ������� ������� �����������					
				T = lookAt(glm::vec3(0, 0, 0), P_new, u_new) * T;
			}
			break;
		}
		case GLFW_KEY_I:
			if (mode == GLFW_MOD_SHIFT) {
				t -= 1;
			}
			else {
				t += 1;
			}
			break;
		case GLFW_KEY_J:
			if (mode == GLFW_MOD_SHIFT) {
				l += 1;
			}
			else {
				l -= 1;
			}
			break;
		case GLFW_KEY_2:
			pType = Frustum;
			break;
		case GLFW_KEY_D:
			if (mode == GLFW_MOD_SHIFT)
				T = lookAt(glm::vec3(0.1, 0, 0), glm::vec3(0.1, 0, -1.f), glm::vec3(0, 0.1, 0)) * T;
			else
				T = lookAt(glm::vec3(1.f, 0, 0), glm::vec3(1.f, 0, -1.f), glm::vec3(0, 1.f, 0)) * T;
			break;

		case GLFW_KEY_Y: {
			glm::vec3 u_new = glm::mat3(rotate(-0.1f, glm::vec3(0, 0, 1))) * glm::vec3(0, 1, 0);
			T = lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, -1), u_new) * T;
			break;
		}

		case GLFW_KEY_G:
			if (mode == GLFW_MOD_SHIFT) {
				glm::mat4 M = rotateP(-0.1, glm::vec3(1, 0, 0), glm::vec3(0, 0, -dist));
				glm::vec3 u_new = glm::mat3(M) * glm::vec3(0, 1, 0);
				glm::vec3 S_new = glm::vec3(M * glm::vec4(0, 0, 0, 1));

				T = lookAt(S_new, glm::vec3(0, 0, -dist), u_new) * T;
			}
			else {
				glm::mat4 M = rotate(-0.1f, glm::vec3(1, 0, 0));
				glm::vec3 u_new = glm::mat3(M) * glm::vec3(0, 1, 0);
				glm::vec3 P_new = glm::vec3(M * glm::vec4(0, 0, -1, 1));
				T = lookAt(glm::vec3(0, 0, 0), P_new, u_new) * T;
			}
			break;
		case GLFW_KEY_F:
			if (mode == GLFW_MOD_SHIFT) {
				glm::mat4 M = rotateP(0.1, glm::vec3(0, 1, 0), glm::vec3(0, 0, -dist));
				glm::vec3 u_new = glm::mat3(M) * glm::vec3(0, 1, 0);
				glm::vec3 S_new = glm::vec3(M * glm::vec4(0, 0, 0, 1));

				T = lookAt(S_new, glm::vec3(0, 0, -dist), u_new) * T;
			}
			else {
				glm::mat4 M = rotate(0.1f, glm::vec3(0, 1, 0));
				glm::vec3 u_new = glm::mat3(M) * glm::vec3(0, 1, 0);
				glm::vec3 P_new = glm::vec3(M * glm::vec4(0, 0, -1, 1));
				T = lookAt(glm::vec3(0, 0, 0), P_new, u_new) * T;
			}
			break;
		case GLFW_KEY_H:
			if (mode == GLFW_MOD_SHIFT) {
				glm::mat4 M = rotateP(-0.1, glm::vec3(0, 1, 0), glm::vec3(0, 0, -dist));
				glm::vec3 u_new = glm::mat3(M) * glm::vec3(0, 1, 0);
				glm::vec3 S_new = glm::vec3(M * glm::vec4(0, 0, 0, 1));

				T = lookAt(S_new, glm::vec3(0, 0, -dist), u_new) * T;
			}
			else {
				glm::mat4 M = rotate(-0.1f, glm::vec3(0, 1, 0));
				glm::vec3 u_new = glm::mat3(M) * glm::vec3(0, 1, 0);
				glm::vec3 P_new = glm::vec3(M * glm::vec4(0, 0, -1, 1));
				T = lookAt(glm::vec3(0, 0, 0), P_new, u_new) * T;
			}
			break;
		case GLFW_KEY_K:
			if (mode == GLFW_MOD_SHIFT)
				b--;
			else
				b++;
			break;
		case GLFW_KEY_L:
			if (mode == GLFW_MOD_SHIFT)
				r--;
			else
				r++;
			break;
		case GLFW_KEY_U:
			if (mode == GLFW_MOD_SHIFT)
				if (n >= 0.3) n -= 0.2;
				else
					if (n <= f - 0.3) n += 0.2;
			break;
		case GLFW_KEY_O:
			if (mode == GLFW_MOD_SHIFT)
				if (f >= n + 0.3) f -= 0.2; else f;
			else
				f += 20;
			break;
		case GLFW_KEY_B:
			if (mode == GLFW_MOD_SHIFT)
				if (dist >= 0.3) dist -= 0.2; else dist;
			else
				dist += 0.2;
			break;
		case GLFW_KEY_Z:
			if (mode == GLFW_MOD_SHIFT)
				if (fovy_work >= 0.08) fovy_work -= 0.05; else fovy_work;
			else
				if (fovy_work <= 2.95) fovy_work += 0.05; else fovy_work;
			break;
		case GLFW_KEY_X:
			if (mode == GLFW_MOD_SHIFT)
				if (aspect_work >= 0.06) aspect_work -= 0.05; else aspect_work;
			else
				aspect_work += 0.05;
			break;
		case GLFW_KEY_F3: {
			OPENFILENAME openFileDialog; // ������ �������� �����
			char fileName[260]; // ����� ��� ����� �����
								// ������������� ��������� �������
			ZeroMemory(&openFileDialog, sizeof(openFileDialog));
			openFileDialog.lStructSize = sizeof(openFileDialog);
			openFileDialog.hwndOwner = NULL;
			openFileDialog.lpstrFile = fileName;
			openFileDialog.lpstrFile[0] = '\0';
			openFileDialog.nMaxFile = sizeof(fileName);
			openFileDialog.lpstrFilter = "Text files (*.txt)\0*.txt\0All files 2007\0*.*\0";
			openFileDialog.nFilterIndex = 1;
			openFileDialog.lpstrFileTitle = NULL;
			openFileDialog.nMaxFileTitle = 0;
			openFileDialog.lpstrInitialDir = NULL;
			openFileDialog.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
			// ������ ��������� ������� � ��������� ����������
			if (GetOpenFileName(&openFileDialog)) { // ���� ���� ������ �������
				readFromFile(fileName);
			}
			break;
		}
		case GLFW_KEY_F5:
			if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				glfwSetCursorPosCallback(window, cursorPosSave_callback);
			}
			else {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				glfwSetCursorPosCallback(window, cursorPos_callback);
			}
		default:
			break;
		}
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	dist += yoffset;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT) mbState = true;
	if (action == GLFW_RELEASE && button == GLFW_MOUSE_BUTTON_LEFT) mbState = false;

}

int main() {
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "Task 8. Khalyavina", NULL, NULL);
	if (window == NULL) {
		std::cout << "����� glfwCreateWindow ���������� ��������." << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursorPosSave_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);


	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "�� ������� ��������� GLAD" << std::endl;
		glfwTerminate();
		return -1;
	}

	glViewport(0, 0, 800, 600);

	shader vertexShader("Vertex.glsl", GL_VERTEX_SHADER);
	shader fragmentShader("Fragment.glsl", GL_FRAGMENT_SHADER);

	program shaderProgram(vertexShader, fragmentShader);

	vertexShader = shader("LightVertex.glsl", GL_VERTEX_SHADER);
	fragmentShader = shader("LightFragment.glsl", GL_FRAGMENT_SHADER);

	program lightShaderProgram(vertexShader, fragmentShader);

	vertexShader = shader("MyLightVertex.glsl", GL_VERTEX_SHADER);
	fragmentShader = shader("MyLightFragment.glsl", GL_FRAGMENT_SHADER);

	program myLightShaderProgram(vertexShader, fragmentShader);

	glDeleteShader(vertexShader.shaderID);
	glDeleteShader(fragmentShader.shaderID);

	shaderProgram.useUniform("material.ambient");
	shaderProgram.useUniform("material.diffuse");
	shaderProgram.useUniform("material.specular");
	shaderProgram.useUniform("material.shininess");
	shaderProgram.useUniform("clipView");
	shaderProgram.useUniform("modelView");
	shaderProgram.useUniform("modelInv");
	shaderProgram.useUniform("lightPos");
	shaderProgram.useUniform("myLightPos");
	shaderProgram.useUniform("viewPos");
	shaderProgram.useUniform("light.ambient");
	shaderProgram.useUniform("light.diffuse");
	shaderProgram.useUniform("light.specular");
	shaderProgram.useUniform("myLight.ambient");
	shaderProgram.useUniform("myLight.diffuse");
	shaderProgram.useUniform("myLight.specular");

	lightShaderProgram.useUniform("pathColor");
	lightShaderProgram.useUniform("clipView");

	myLightShaderProgram.useUniform("pathColor");
	myLightShaderProgram.useUniform("clipView");

	//==========================================================
	// ����� �������� ������ ��� ��������� ��������� �����
	//==========================================================
	GLfloat lightVertices[] = {
		-0.1f, 0.f, 0.f,
		0.1f, 0.f, 0.f,
		0.f, 0.1f, 0.f,
		0.f, -0.1f, 0.f,
		0.f, 0.f, 0.1f,
		0.f, 0.f, -0.1f,
		0.07071f, 0.07071f, 0.f,
		-0.07071f, -0.07071f, 0.f,
		-0.07071f, 0.07071f, 0.f,
		0.07071f, -0.07071f, 0.f,
		0.07071f, 0.f, 0.07071f,
		-0.07071f, 0.f, -0.07071f,
		-0.07071f, 0.f, 0.07071f,
		0.07071f, 0.f, -0.07071f,
		0.f, 0.07071f, 0.07071f,
		0.f, -0.07071f, -0.07071f,
		0.f, -0.07071f, 0.07071f,
		0.f, 0.07071f, -0.07071f,
		0.05774f, 0.05774f, 0.05774f,
		-0.05774f, -0.05774f, -0.05774f,
		-0.05774f, -0.05774f, 0.05774f,
		0.05774f, 0.05774f, -0.05774f,
		-0.05774f, 0.05774f, 0.05774f,
		0.05774f, -0.05774f, -0.05774f,
		0.05774f, -0.05774f, 0.05774f,
		-0.05774f, 0.05774f, -0.05774f
	};

	GLuint lightVertexArray;
	glGenVertexArrays(1, &lightVertexArray);
	glBindVertexArray(lightVertexArray);
	lightM = glm::translate(glm::vec3(0, 0, 5));
	myLightM = glm::translate(glm::vec3(7, 5, -4));

	GLuint lightVertexBuffer;
	glGenBuffers(1, &lightVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, lightVertexBuffer);

	glBufferData(GL_ARRAY_BUFFER, sizeof(lightVertices), lightVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	readFromFile("engine.txt");

	glEnable(GL_DEPTH_TEST);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	while (!glfwWindowShouldClose(window)) {
		glfwSwapBuffers(window);
		glClearColor(0.4, 0.8, 0.3, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 move = glm::rotate((float)glfwGetTime() * glm::radians(10.0f), glm::vec3(0.0f, 1.0f, 1.0f));
		glm::mat4 lightM1 = move * lightM;

		shaderProgram.use();

		glm::mat4 proj; // ������� �������� � ������������ ���������
		switch (pType) {
		case Ortho: // ������������� ��������	
			proj = glm::ortho(l, r, b, t, n, f);
			break;
		case Frustum: // ������������� �������� � Frustum	
			proj = glm::frustum(l, r, b, t, n, f);
			break;
		case Perspective: // ������������� �������� � Perspective			
			proj = glm::perspective(fovy_work, aspect_work, n, f);
			break;
		}


		glm::mat4 C = proj * T; // ������� �������� �� ������� ��������� � ������������ ���������

		shaderProgram.setUniform("light.ambient", amb);
		shaderProgram.setUniform("light.diffuse", dif);
		shaderProgram.setUniform("light.specular", spec);
		shaderProgram.setUniform("myLight.ambient", amb);
		shaderProgram.setUniform("myLight.diffuse", dif);
		shaderProgram.setUniform("myLight.specular", spec);
		shaderProgram.setUniform("lightPos", glm::vec3(lightM1 * glm::vec4(0, 0, 0, 1)));
		shaderProgram.setUniform("myLightPos", glm::vec3(myLightM * glm::vec4(0, 0, 0, 1)));
		shaderProgram.setUniform("viewPos", glm::vec3(glm::inverse(T) * glm::vec4(0, 0, 0, 1)));
		for (int k = 0; k < models.size(); k++) { // ���� �� �������

			std::vector<mesh> figure = models[k].figure; // ������ ������� ��������� ������
			glm::mat4 TM = C * models[k].modelM; // ������� ������ �������������� ������
			shaderProgram.setUniform("clipView", TM);
			shaderProgram.setUniform("modelView", models[k].modelM);
			shaderProgram.setUniform("modelInv", glm::transpose(glm::inverse(models[k].modelM)));

			for (int i = 0; i < figure.size(); i++) {
				shaderProgram.setUniform("material.ambient", figure[i].material.ambient);
				shaderProgram.setUniform("material.diffuse", figure[i].material.diffuse);
				shaderProgram.setUniform("material.specular", figure[i].material.specular);
				shaderProgram.setUniform("material.shininess", figure[i].material.shininess);

				glBindVertexArray(figure[i].vertexArray);
				glDrawElements(GL_TRIANGLES, figure[i].indices.size(), GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);
			}
		}
		lightShaderProgram.use();
		lightShaderProgram.setUniform("clipView", C * lightM1);
		lightShaderProgram.setUniform("pathColor", amb + dif + spec);

		glBindVertexArray(lightVertexArray);
		glDrawArrays(GL_LINES, 0, 26);
		glBindVertexArray(0);

		myLightShaderProgram.use();
		myLightShaderProgram.setUniform("clipView", C * myLightM);
		myLightShaderProgram.setUniform("pathColor", amb + dif + spec);

		glBindVertexArray(lightVertexArray);
		glDrawArrays(GL_LINES, 0, 26);
		glBindVertexArray(0);

		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}