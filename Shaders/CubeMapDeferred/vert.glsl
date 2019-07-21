#version 330



layout(location=0) in vec4 Vertices; 
layout(location=1) in vec4 Normals; 

uniform mat4 ModelMatrix; 
uniform mat4 IdentityMatrix; 

out vec3 Position; 
out vec3 Normal; 
out vec2 TexCoord; 

out vec3 Flag;

flat out uvec3 Materials;

const vec3 Flags[3] = vec3[](vec3(1.0,0.0,0.0), vec3(0.0,1.0,0.0), vec3(0.0,0.0,1.0)); 


void ConvertMaterials(out uvec3 Materials, out uint Flag) {
	uint Raw = floatBitsToUint(Vertices.w); 

	Flag = Raw % 4u; 
	Materials.x = Raw / 262144u; 
	Materials.y = (Raw / 1024u) % 256u; 
	Materials.z = (Raw / 4u) % 256u; 

}



void main(void) {	
	
	Position = vec3(ModelMatrix * vec4(Vertices.xyz,1.0)); 
	TexCoord = vec2(Vertices.w, Normals.w); 
	Normal = Normals.xyz; 

	uint FlagIndex; 


	ConvertMaterials(Materials, FlagIndex); 

	Flag = Flags[FlagIndex]; 

	gl_Position = IdentityMatrix * vec4(Vertices.xyz,1.0); 
}