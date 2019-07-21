#version 420
#extension GL_ARB_bindless_texture : enable

in vec2 TexCoord; 

layout(location = 0) out vec4 Packed;

//samplers: (ie textures)  
uniform sampler2D ViewPosition; 
uniform sampler2D Normal; 
uniform sampler2D BlueNoise; 
uniform sampler2D Depth;	
uniform sampler2D Previous; 
uniform sampler2D ShadowMaps[3]; 
uniform sampler3D Voxels[3]; 
uniform samplerCube Sky; 

uniform int Frame; 


//matrices		

uniform mat4 ShadowCombined[3]; 
uniform mat4 InverseView;
uniform mat4 PreviousCombined; 

//vectors / scalars / other

uniform vec3 ShadowDirection; 
uniform vec3 CameraPosition; 
uniform vec3 SunColor; 
uniform vec3 CameraPositionLight[3]; 
uniform vec2 Resolution; 
uniform float Size[3]; 
uniform bool First; 


sampler3D GetVoxelVolume(int volume) {
	return volume == 0 ? Voxels[0] : volume == 1 ? Voxels[1] : Voxels[2]; 
}




vec4 PackData(vec3 Normal, vec3 Diffuse, float Depth, float Temporal) {
	
	vec4 Packed; 

	Packed.x = uintBitsToFloat(packHalf2x16(Diffuse.xy)); 
	Packed.y = uintBitsToFloat(packHalf2x16(vec2(Diffuse.z, Normal.x))); 
	Packed.z = uintBitsToFloat(packHalf2x16(Normal.yz)); 
	Packed.w = uintBitsToFloat(packHalf2x16(vec2(Depth, Temporal))); 

	return Packed; 

}

void UnPackData(vec4 Packed, out vec3 Diffuse, out vec3 Normal, out float Depth, out float Temporal) {
	
	vec2 UnPacked1 = unpackHalf2x16(floatBitsToUint(Packed.x)); 
	vec2 UnPacked2 = unpackHalf2x16(floatBitsToUint(Packed.y)); 
	vec2 UnPacked3 = unpackHalf2x16(floatBitsToUint(Packed.z)); 
	vec2 UnPacked4 = unpackHalf2x16(floatBitsToUint(Packed.w)); 

	Diffuse = vec3(UnPacked1, UnPacked2.x); 
	Normal = vec3(UnPacked2.y, UnPacked3.xy); 
	Depth = UnPacked4.x; 
	Temporal = UnPacked4.y; 

}


vec3 TransformToVoxelSpace(int Volume, vec3 WorldPosition) {
	
	return ((WorldPosition - CameraPositionLight[Volume]) / (Size[Volume]/2.)); 

}

vec4 ConeTrace(vec3 Origin, vec3 Direction, vec3 Normal) {

	vec4 Result = vec4(0.0,0.0,0.0,1.0); 

	int Volume = 0; 

	float StepSize = (1.0 / 64.) * Size[0]; 

	vec3 Position = Origin + Normal * StepSize * 2.0; 

	while(Volume != 3 && Result.a > 0.05) {
		
		Position += Direction * StepSize; 

		vec3 Coordinate = TransformToVoxelSpace(Volume, Position); 

		if(abs(Coordinate.x) > .96 || abs(Coordinate.y) > .96 || abs(Coordinate.z) > .96) {
			if(Volume == 2) 
				break; 

			Volume++; 

			StepSize *= 2.0; 


		}



		vec4 Sample = textureLod((Volume == 0 ? Voxels[0] : Volume == 1 ? Voxels[1] : Voxels[2]), Coordinate * 0.5 + 0.5, 0.);

		Sample.xyz = pow(Sample.xyz * 8.333,vec3(2.2)); 

		Sample.a = 1.0-pow(1.0-Sample.a, 4.0); 
			
		Result.xyz += Sample.xyz * Result.a; 
				
		Result.a *= pow(clamp(1.0 - Sample.a, 0.0,1.0), 1.0); 

	}

	float UpFactor = 1.0; 
	float SunFactor = clamp(dot(Direction, ShadowDirection), 0.0, 1.0); 

	Result.a *= clamp(UpFactor * 10.0, 0.0, 1.0); 

	vec3 SkyColor = vec3(0.3,0.6,1.0) * sqrt(UpFactor); 

	

	Result.xyz += texture(Sky,Direction).xyz * pow(Result.a,128.0) * 3.0; 

	return vec4(Result.xyz, 1.0); 


}




vec3 TransformToWorld(vec3 Direction, vec3 Normal) {
    // Find an axis that is not parallel to normal
    vec3 majorAxis;
    if (abs(Normal.x) < 0.57735026919f /* 1 / sqrt(3) */) {
        majorAxis = vec3(1, 0, 0);
    } else if (abs(Normal.y) < 0.57735026919f /* 1 / sqrt(3) */) {
        majorAxis = vec3(0, 1, 0);
    } else {
        majorAxis = vec3(0, 0, 1);
    }

    // Use majorAxis to create a coordinate system relative to world space
    vec3 u = normalize(cross(Normal, majorAxis));
    vec3 v = cross(Normal, u);
    vec3 w = Normal;


    // Transform from local coordinates to world coordinates
    return u * Direction.x +
           v * Direction.y +
           w * Direction.z;
}

vec2 hash2(float seed) {
	return fract(vec2(sin(vec2(seed, seed + 0.1f))*vec2(43758.5453123f, 22578.1459123f)));
}

vec3 LambertBRDF(vec3 Normal, vec2 Hash) {



    float r = sqrt(Hash.x); 

    float theta = Hash.y * 6.2831;

    float x = r * cos(theta);
    float y = r * sin(theta);

    // Project z up to the unit hemisphere
    float z = sqrt(1.0f - x * x - y * y);

    return normalize(TransformToWorld(vec3(x,y,z), Normal));

}

float zNear = 0.1; 
float zFar = 1000.; 

float LinearlizeDepth(float z) {
	
	return 2.0 * zNear * zFar / (zFar + zNear - (z*2.-1.) * (zFar - zNear)); 

}

void main() {

	float Seed = (TexCoord.x + TexCoord.y) * 10000. + Frame; 

	//STEP 1: grab important data 

	vec4 NormalRoughness = texture(Normal, TexCoord); 
	vec3 ViewPosition = texture(ViewPosition, TexCoord).xyz; 
	vec3 WorldPosition = (InverseView * vec4(ViewPosition, 1.0)).xyz;
	vec3 Incident = normalize(WorldPosition - CameraPosition); 

	vec2 TexCoordNoise = (TexCoord / 128.) * textureSize(Normal, 0);  


	//vec3 VoxelSpace = (((WorldPosition+NormalRoughness.xyz * 1.)-CameraPositionLight[0])/(Size[0]/2.))*0.5+0.5; 
	
	vec3 Diffuse = vec3(0.0); 
	
	vec3 Direction = LambertBRDF(NormalRoughness.xyz,hash2(Seed)); 

	if(distance(WorldPosition, CameraPosition) < (Size[2]/2.))
		Diffuse = ConeTrace(WorldPosition,Direction, NormalRoughness.xyz).xyz;  
	else
		Diffuse = texture(Sky, Direction).xyz * 3.0; 


	float Depth = LinearlizeDepth(texture(Depth, TexCoord).x); 

	//reconstruct motion vectors



	float Frame = 0.0; 

	if(!First) {
		
		vec4 NDC = PreviousCombined * vec4(WorldPosition, 1.0); 

		NDC.xyz /= NDC.w; 

		

		if (abs(NDC.x) < 1.0 && abs(NDC.y) < 1.0) {
			NDC.xy = NDC.xy * 0.5 + 0.5; 



			ivec2 PreviousCoordinate = ivec2(NDC.xy * vec2(480,270)); 

			//go through coordinate + coordinates neighboor. Find the best one 

			float Closest = -1.0; 
			vec4 Value = vec4(0.0); 

			for(int x = 0; x <= 0; x++) {
				for(int y = 0; y <= 0; y++) {
					
					vec4 Sample = texelFetch(Previous, PreviousCoordinate + ivec2(x,y),0); 

					vec3 SampleNormal, Lighting; 
					float SampleDepth, SampleTemporal; 

					UnPackData(Sample, Lighting, SampleNormal, SampleDepth, SampleTemporal); 

					float Weight = (1.0 - dot(NormalRoughness.xyz, SampleNormal)) + abs(Depth-SampleDepth); 

					if(Weight < Closest || Closest < 0) {
						Closest = Weight; 
						Value.xyz = Lighting; 
						Value.w = SampleTemporal; 
					}

				}
			}

			if(Closest < 1.0) {
				Frame = min(Value.w + 1.0,21.0); 

				Diffuse = mix(Diffuse, Value.xyz, float(Frame) / float(Frame + 1.0)); 

			}

		}

	}



	

	Packed = PackData(NormalRoughness.xyz, Diffuse, Depth, Frame); 
	


	//now, utilize temporal filtering. yeah yeah yeah yeah 


	
}