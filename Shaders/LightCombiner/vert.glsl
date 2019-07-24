#version 330
layout (location = 0) in vec3 Vert; 
layout (location = 1) in vec2 Texc;
out vec2 TexCoord;  

uniform sampler2DArray WaterNormalMap; 
uniform float Time; 
uniform vec3 CameraPosition; 

out float WaterHeightCamera; 


vec4 SampleInterpolatied(sampler2DArray Sampler,vec3 Coord) {

	float BaseTime = mod(Coord.z, 119.); 

	int Coord1 = int(floor(BaseTime)); 
	int Coord2 = int(ceil(BaseTime))%119; 

	return mix(texture(Sampler, vec3(Coord.xy, Coord1)), texture(Sampler,vec3(Coord.xy, Coord2)), fract(BaseTime)); 


}

void main(void) {
TexCoord = Texc; 

WaterHeightCamera = 40.0 + SampleInterpolatied(WaterNormalMap, vec3(CameraPosition.xz * 0.05,mod(Time*10.0, 119.))).w * 2.1; 


gl_Position = vec4(Vert.xy,0.f,1.0); 
}