#version 330
layout(location=0) in vec4 Vertices; 
layout(location=1) in vec4 Normals; 

uniform mat4 IdentityMatrix; 

void main(void) {	
	gl_Position = IdentityMatrix * vec4(Vertices.xyz,1.0); 
}