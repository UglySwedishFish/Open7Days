#version 330



layout(location=0) in vec3 Vertices; 
layout(location=1) in vec2 TexCoords; 
layout(location=2) in vec3 Normals; 

uniform mat4 IdentityMatrix; 


out vec3 Position; 
out vec3 Normal; 
out vec2 TexCoord; 


void main(void) {	
	
	Position = Vertices.xyz; 
	TexCoord = TexCoords; 
	Normal = Normals.xyz; 

	gl_Position = IdentityMatrix * vec4(Vertices.xyz,1.0); 
}