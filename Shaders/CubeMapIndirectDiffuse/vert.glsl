#version 330
layout (location = 0) in vec3 Vert; 

out vec3 Vertice; 

uniform mat4 IdentityMatrix; 

void main(void) {
Vertice = Vert; 
gl_Position = IdentityMatrix * vec4(Vert,1.0); 
}