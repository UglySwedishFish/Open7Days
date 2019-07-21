#version 330
in vec2 TexCoord; 

layout(location = 0) out vec4 Albedo; //w component stores metalness
layout(location = 1) out vec4 Normal; //w component stores roughness 
layout(location = 2) out vec3 ScreenSpaceWorldPosition; //might seem a bit weird to store
//but its actually much faster to store it than calculating it for the screen-space based methods

uniform mat4 InverseProjection; 
uniform mat4 InverseView; 


uniform sampler2D DeferredNormal; 
uniform sampler2D DeferredDepth; 
uniform sampler2DArray Textures; 





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
vec2 GenerateTextureCoordinate( float Scale, vec3 Normal, vec3 Position) {
	vec3 blend_weights = normalize(abs(Normal ));   // Tighten up the blending zone:
	blend_weights = (blend_weights - 0.2);
	blend_weights = pow(max(blend_weights, 0),vec3(1.0));      // Force weights to sum to 1.0 (very important!)
	blend_weights /= (blend_weights.x + blend_weights.y + blend_weights.z );

	vec2 SampleXMajor = fract(Position.yz * Scale); 
	vec2 SampleYMajor = fract(Position.zx * Scale); 
	vec2 SampleZMajor = fract(Position.xy * Scale); 
	
	return SampleXMajor * blend_weights.x + SampleYMajor * blend_weights.y + SampleZMajor * blend_weights.z; 
} 


vec4 TextureNew(sampler2DArray Sampler, uint Texture, float Scale, vec3 Normal, vec3 Position) {

	return texture(Sampler, vec3(GenerateTextureCoordinate(Scale,Normal,Position), Texture)); 

}



vec4 CalculateViewSpacePosition() {


    vec4 ClipSpace = vec4(TexCoord * 2.0 - 1.0, texture(DeferredDepth, TexCoord).x * 2. - 1., 1.0);
    vec4 ViewSpace = InverseProjection * ClipSpace;

	return ViewSpace / ViewSpace.w; 

}

void UnPackMaterialData(float Data, out uvec3 Materials, out vec2 Interpolation) {
	
	uint Packed = floatBitsToUint(Data); 

	Materials.x = Packed / 16777216u; 
	Materials.y = (Packed / 65536u) % 256u; 
	Materials.z = (Packed / 256u) % 256u; 
	Interpolation.x = float((Packed / 16u) % 16u) / 15.f; 
	Interpolation.y = float(Packed % 16u) / 15.f; 

}


void main() {
	
	vec4 ViewSpace = CalculateViewSpacePosition(); 
	vec3 WorldSpace = (InverseView * ViewSpace).xyz; 
	vec4 SampleNormal = texelFetch(DeferredNormal, ivec2(gl_FragCoord.xy),0); 

	uvec3 Materials; 
	vec2 Interpolation; 

	UnPackMaterialData(SampleNormal.w, Materials, Interpolation); 


	vec4 Texture1 = TextureOld(Textures,Materials.x, .5, SampleNormal.xyz, WorldSpace);  
	vec4 Texture2 = TextureOld(Textures,Materials.y, .5, SampleNormal.xyz, WorldSpace); 
	vec4 Texture3 = TextureOld(Textures,Materials.z, .5, SampleNormal.xyz, WorldSpace);  

	vec4 Texture = Texture1 * Interpolation.x + Texture2 * Interpolation.y + Texture3 * (1.0 - (Interpolation.x + Interpolation.y)); 



	Texture.xyz = pow(Texture.xyz, vec3(2.2)); 

	Albedo = vec4(Texture.xyz, 0.0); 
	Normal = vec4(SampleNormal.xyz,Texture.a); 
	ScreenSpaceWorldPosition = vec3(ViewSpace.xyz); 

}