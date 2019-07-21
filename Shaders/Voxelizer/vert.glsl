#version 330 

layout(location=0) in vec4 Vertices; 
layout(location=1) in vec4 Normals; 

uniform mat4 IdentitiyMatrix; 
uniform mat4 ModelMatrix; 
uniform vec3 CameraPosition; 

out GeometryData {
	out vec3 Normal; 
	out vec3 WorldPosition; 
}VS_OUT; 

void main() {


	VS_OUT.WorldPosition = (vec3(ModelMatrix * vec4(Vertices.xyz,1.0)) - CameraPosition); 
	VS_OUT.Normal = Normals.xyz;
	gl_Position =  IdentitiyMatrix * vec4(VS_OUT.WorldPosition,1.0); 
}