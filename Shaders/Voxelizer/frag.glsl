#version 420 
#extension GL_ARB_shader_image_size : enable


in FragmentData {
	vec3 WorldPosition; 
	vec3 Normal; 
}FS_IN; 



uniform mat4 ShadowCombined[3]; 
uniform sampler2D ShadowMaps[3]; 
uniform vec3 ShadowDirection; 
uniform vec3 SunColor; 
uniform vec3 CameraPosition; 
uniform vec3 CameraPositionPrevious; 
uniform sampler3D PreviousVolume; 
uniform int Resolution; 

layout(rgba8) uniform image3D Voxels;
uniform float Size; 

bool InsideCube(vec3 p, float e) { return abs(p.x) < e && abs(p.y) < e && abs(p.z) < e; }


float ShadowsESM(sampler2D ShadowMap, vec3 Coord) {

	//return (Coord.z > (texture(ShadowMap, Coord.xy).x+0.0003) ? 1. : 0.0); 

	return clamp(exp(-30000.0*(Coord.z-(texture(ShadowMap, Coord.xy).x)-.00015)), 0.0,1.0); 
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
		return 0.0;  

	float ShadowMapSample = ShadowsESM((Index == 0 ? ShadowMaps[0] : Index == 1 ? ShadowMaps[1] : ShadowMaps[2]), ShadowNDC.xyz * 0.5 + 0.5) * 100.;

	return ShadowMapSample * max(dot(Normal, ShadowDirection),0.); 

}



void main() {

	


	
	if(InsideCube(FS_IN.WorldPosition, Size/2)) {
		vec3 Voxel = (FS_IN.WorldPosition / (Size/2)) * 0.5 + 0.5; 

		vec3 DiffuseLighting = GetDirectionalShadows(FS_IN.WorldPosition + CameraPosition, FS_IN.Normal) * SunColor; 

		vec4 CurrentResult = vec4(pow(DiffuseLighting* SunColor* 0.12,vec3(0.4545454545)),1.0); 

		//reproject to previous volume. This is to avoid massive aliasing with voxels when the camera moves. (although this doesnt entirely solve it still)
		//does result in some temporal lag. But this is quickly overshadowed by the temporal lag already caused 
		//by the temporal filtering 


		//do gamma correction here for performance sakes 
		imageStore(Voxels, ivec3(Resolution * Voxel), CurrentResult); 
	}

}