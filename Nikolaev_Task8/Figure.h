#pragma once
#include <glm\glm.hpp>
#include <glad\glad.h>
#include <vector>

struct vertex {
	glm::vec3 position;
	glm::vec3 normal;
};

struct Material {
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	float shininess;
};

class path {
public:
	std::vector<glm::vec3> vertices;
	glm::vec3 color;
	float thickness;
	GLuint vertexArray;

	path(std::vector<glm::vec3> verts, glm::vec3 clr, float thck) {
		vertices = verts;
		color = clr;
		thickness = thck;
		setupPath();
	}

private:
	GLuint vertexBuffer;

	void setupPath() {
		glGenVertexArrays(1, &vertexArray);
		glGenBuffers(1, &vertexBuffer);

		glBindVertexArray(vertexArray);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
};



class mesh {
public:
	std::vector<vertex> vertices;
	Material material;
	GLuint vertexArray;
	std::vector<GLuint> indices;

	mesh(std::vector<vertex> verts, std::vector<GLuint> inds, Material matl) {
		vertices = verts;
		material = matl;
		indices = inds;
		setupMesh();
	}

private:
	GLuint vertexBuffer;
	GLuint elementBuffer;

	void setupMesh() {
		glGenVertexArrays(1, &vertexArray);
		glGenBuffers(1, &vertexBuffer);
		glGenBuffers(1, &elementBuffer);

		glBindVertexArray(vertexArray);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertex), &vertices[0], GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (GLvoid*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (GLvoid*)offsetof(vertex, normal));
		glEnableVertexAttribArray(1);


		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
};

class model {
public:
	std::vector<mesh> figure;
	glm::mat4 modelM;

	model(std::vector<mesh> fig, glm::mat4 mat) {
		figure = fig;
		modelM = mat;
	}


};