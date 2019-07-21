#version 330
#extension GL_ARB_bindless_texture : enable

in vec3 Vertice; 
out vec4 FinalColor; 

uniform samplerCube Normal;
uniform samplerCube WorldPosition; 
uniform sampler2DArray Textures; 
uniform sampler2D ShadowMaps[3]; 

uniform mat4 ShadowCombined[3]; 
uniform vec3 ShadowDirection; 
uniform vec3 SunColor; 

void UnPackMaterialData(float Data, out uvec3 Materials, out vec2 Interpolation) {
	
	uint Packed = floatBitsToUint(Data); 

	Materials.x = Packed / 16777216u; 
	Materials.y = (Packed / 65536u) % 256u; 
	Materials.z = (Packed / 256u) % 256u; 
	Interpolation.x = float((Packed / 16u) % 16u) / 15.f; 
	Interpolation.y = float(Packed % 16u) / 15.f; 

}

vec4 TextureOld(sampler2DArray Sampler, uint Texture, float Scale, vec3 Normal, vec3 Position) {

	vec3 blend_weights = normalize(abs(Normal ));   // Tighten up the blending zone:
	blend_weights = (blend_weights - 0.2);
	blend_weights = pow(max(blend_weights, 0),vec3(1.0));      // Force weights to sum to 1.0 (very important!)
	blend_weights /= (blend_weights.x + blend_weights.y + blend_weights.z );

	vec4 SampleXMajor = texture(Sampler, vec3(fract(Position.yz * Scale),float(Texture))); 
	vec4 SampleYMajor = texture(Sampler, vec3(fract(Position.zx * Scale),float(Texture))); 
	vec4 SampleZMajor = texture(Sampler, vec3(fract(Position.xy * Scale),float(Texture))); 
	
	return SampleXMajor * blend_weights.x + SampleYMajor * blend_weights.y + SampleZMajor * blend_weights.z; 

}


float ShadowsESM(sampler2D ShadowMap, vec3 Coord) {

	//return (Coord.z > (texture(ShadowMap, Coord.xy).x+0.0003) ? 0. : 1.0); 

	return clamp(exp(-10000.0*(Coord.z-(texture(ShadowMap, Coord.xy).x)-.0015)), 0.0,1.0); 
}


float GetDirectionalShadows(vec3 WorldPosition, vec3 Normal) {
	vec3 ShadowNDC = vec3(0.); 

	int Index = -1; 

	for(int try = 0; try < 3; try++) {

	vec4 ShadowCoord = ShadowCombined[try] * vec4(WorldPosition, 1.0); 

	ShadowNDC = ShadowCoord.xyz / ShadowCoord.w; 




	if(abs(ShadowNDC.x) < 1.0 && abs(ShadowNDC.y) < 1.0) {
		Index = try; 
		break; 
	}

	}

	


	if(Index == -1) 
		return 1.0; 

	float ShadowMapSample = ShadowsESM((Index == 0 ? ShadowMaps[0] : Index == 1 ? ShadowMaps[1] : ShadowMaps[2]), ShadowNDC.xyz * 0.5 + 0.5);

	return ShadowMapSample * max(pow(dot(Normal,normalize(ShadowDirection)),3.),0.) * 100.;  

}

void main() {
	
	vec3 Coord = normalize(Vertice); 

	vec4 Position = texture(WorldPosition, Coord); 
	vec3 Normal = texture(Normal, Coord).xyz; 

	uvec3 Materials; 
	vec2 Interpolation; 

	UnPackMaterialData(Position.w, Materials, Interpolation); 

	vec4 Texture1 = TextureOld(Textures,Materials.x, .5, Normal.xyz, Position.xyz);  
	vec4 Texture2 = TextureOld(Textures,Materials.y, .5, Normal.xyz, Position.xyz); 
	vec4 Texture3 = TextureOld(Textures,Materials.z, .5, Normal.xyz, Position.xyz);  

	vec4 Texture = Texture1 * Interpolation.x + Texture2 * Interpolation.y + Texture3 * (1.0 - (Interpolation.x + Interpolation.y)); 

	Texture.xyz = pow(Texture.xyz, vec3(2.2)); 
	

	vec3 Shadows = GetDirectionalShadows(Position.xyz, Normal) * SunColor; 


	FinalColor.xyz = Texture.xyz * Shadows; 
	FinalColor.xyz = length(Normal) < 0.5 ? vec3(0.0) : FinalColor.xyz; 

	FinalColor.a = length(Normal) < 0.5 ? 1.0 : 0.0; 


}