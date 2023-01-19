#version 330 core

struct Light {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct Light2 {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct Material {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};


in vec3 fragPos;
in vec3 fragNorm; 

out vec4 color;

uniform Material material;
uniform Light light;
uniform Light light2;
uniform vec3 lightPos;
uniform vec3 lightPos2;
uniform vec3 viewPos;

void main() {
	vec3 norm = normalize(fragNorm);
	vec3 lightDir = normalize(lightPos - fragPos);
	float cosTheta = max(dot(norm, lightDir), 0.0);
	float powOfCos;
	if (cosTheta > 0.0) {
		vec3 viewDir = normalize(viewPos - fragPos);
		vec3 lightDirR = reflect(-lightDir, norm);
		powOfCos = pow(max(dot(viewDir, lightDirR), 0.0),  material.shininess);
	}
	else
		powOfCos = 0.0;
	
	vec3 ambient = light.ambient * material.ambient;
	vec3 diffuse = light.diffuse * cosTheta * material.diffuse;
	vec3 specular = light.specular * powOfCos * material.specular;
	
	vec3 result = ambient + diffuse + specular;
	
	vec3 lightDir2 = normalize(lightPos2 - fragPos);
	float cosTheta2 = max(dot(norm, lightDir2), 0.0);
	float powOfCos2;
	if (cosTheta2 > 0.0) {
		vec3 viewDir2 = normalize(viewPos - fragPos);
		vec3 lightDirR2 = reflect(-lightDir2, norm);
		powOfCos2 = pow(max(dot(viewDir2, lightDirR2), 0.0),  material.shininess);
	} else
		powOfCos2 = 0.0;
		
	vec3 ambient2 = light2.ambient * material.ambient;
	vec3 diffuse2 = light2.diffuse * cosTheta2 * material.diffuse;
	vec3 specular2 = light2.specular * powOfCos2 * material.specular;
	
	vec3 result2 = ambient2 + diffuse2 + specular2;
	
	color = vec4(result + result2, 1.0f);
}