#version 330



layout(location=0) in vec3 Vertices; 
layout(location=1) in vec3 TexCoords; 
layout(location=2) in vec3 Normals; 

uniform mat4 IdentityMatrix; 


out vec3 Position; 
out vec3 Normal; 
out vec2 TexCoord; 

uniform vec3 CameraPosition; 
uniform sampler2DArray NormalMap; 
uniform float Time; 
#define ITERATIONS_NORMAL 24


vec4 SampleInterpolatied(sampler2DArray Sampler,vec3 Coord) {

	float BaseTime = mod(Coord.z, 119.); 

	int Coord1 = int(floor(BaseTime)); 
	int Coord2 = int(ceil(BaseTime))%119; 

	return mix(texture(Sampler, vec3(Coord.xy, Coord1)), texture(Sampler,vec3(Coord.xy, Coord2)), fract(BaseTime)); 


}

void main(void) {	
	
	Position = Vertices.xyz; 
	TexCoord = ((Position.xz)+CameraPosition.xz) * 0.05; 

	vec4 Height  = SampleInterpolatied(NormalMap, vec3(TexCoord,mod(Time*10.0, 119.))); 

	Normal = Height.xyz; 


	gl_Position = IdentityMatrix * vec4(Position.xyz + vec3(0.0,40.0 + Height.w * 2.1,0.0),1.0); 
}