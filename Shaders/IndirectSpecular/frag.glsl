#version 420

in vec2 TexCoord; 
layout(location = 0) out vec4 IndirectSpecular;

//samplers: (ie textures)  
uniform sampler2D ViewPosition; 
uniform sampler2D Normal; 
uniform sampler2D FirstPassDiffuse; 
uniform samplerCube Test; 
uniform samplerCube CubeMapWorldPosition; 
uniform sampler2D Albedo; 
uniform sampler2D Previous; 
uniform sampler2DArray Sky; 
uniform sampler2D Depth; 
uniform sampler2D WaterDepth; 
uniform sampler2D WaterNormal; 

uniform mat4 InverseView; 
uniform mat4 View; 
uniform mat4 Project; 
uniform mat4 InverseProject; 
uniform mat4 PreviousCombined; 
uniform float Time; 

uniform vec3 CameraPosition; 
uniform vec3 LightDirection; 

uniform int Frame; 
uniform bool First;

vec3 ImportanceGGX(vec2 xi, float roughness)
{
    float r_square = roughness * roughness;
    float phi = 6.2831 * xi.x;
    float cos_theta = sqrt((1 - xi.y) / (1 + (r_square * r_square - 1) * xi.y));
    float sin_theta = sqrt(1 - cos_theta * cos_theta);

    return vec3(sin_theta * cos(phi), sin_theta * sin(phi), cos_theta);
}

float Seed; 

vec2 hash2() {
	return fract(vec2(sin(vec2(Seed++, Seed++))*vec2(43758.5453123f, 22578.1459123f)));
}



vec3 TraceCubeMap(samplerCube WorldPositionCubeMap, vec3 Direction, vec3 WorldPosition, vec3 CameraPosition, float Dither,
					float StepSize, float Estimation, int BinarySearchSteps, int Steps, out bool success, out float traversaldistance, float IncreaseFactor) { 
	success = false; 
	
	vec3 Hit = WorldPosition + Direction * Dither * StepSize; 

	traversaldistance = Dither * StepSize; 

	for(int Step = 1; Step < Steps; Step++) {
	
	Hit += Direction*StepSize; 

	StepSize *= IncreaseFactor; 

	traversaldistance += StepSize; 

	vec3 PositionOnCubeMap = texture(WorldPositionCubeMap,normalize(Hit - CameraPosition), 0.f).xyz; 
	
	//are we close enough to estimate a hit? 
	
	if(distance(PositionOnCubeMap,Hit) < (StepSize * Estimation)) {
	
	float StepSizeBinarySearch = StepSize * 0.5;
	
	for(int StepBinarySearch=0;StepBinarySearch<BinarySearchSteps;StepBinarySearch++) {
	
	PositionOnCubeMap = textureLod(WorldPositionCubeMap,normalize(Hit - CameraPosition), 0.f).xyz; 

	if(distance(PositionOnCubeMap,Hit) < (StepSize * Estimation)) {
	traversaldistance -= StepSizeBinarySearch; 
	Hit -= Direction*StepSizeBinarySearch;
	}
	else {
	traversaldistance += StepSizeBinarySearch;  
	Hit += Direction*StepSizeBinarySearch;
	}

	StepSizeBinarySearch*=0.5; 
	}


	success = true; 

	return normalize(Hit - CameraPosition); 

	}

	if(Step == Steps-1) {
	return vec3(Hit - CameraPosition); 
	}
	

	}
}
vec3 GetSpecularRayDirection(vec3 RawDirection, vec3 Normal, vec3 Incident, float Roughness) {

	vec3 v0 = abs(Normal.z) < 0.999f ? vec3(0.f, 0.f, 1.f) : vec3(0.f, 1.f, 0.f);

	vec3 Tangent = normalize(cross(v0, Normal));
	vec3 Bitangent = normalize(cross(Tangent, Normal));


	for(int Try = 0; Try < 3; Try++) {
		
		vec2 Xi = hash2() * vec2(1.f, 0.2f); 

		vec3 rho = ImportanceGGX(Xi, clamp(sqrt(Roughness), 0.001f, 1.0f)); 

		vec3 TryDirection = normalize(0.001f + rho.x * Tangent + rho.y * Bitangent + rho.z * RawDirection); 

		if(dot(TryDirection, Normal) > 0.0005f) {
			return TryDirection; 
		}

	}
	return RawDirection; 
}

vec2 ScreenSpaceTracing(vec3 ViewPos, vec3 Normal, float Roughness) {

	vec3 Incident = normalize(ViewPos); 
	vec3 Direction = reflect(Incident, Normal); 
	Direction = GetSpecularRayDirection(Direction, Normal, Incident, Roughness); 

	float StepSize = mix(0.75,1.5,Roughness); 

	float Traversal = 0.0; 

	vec3 RayPos = ViewPos; 

	for(int Step = 0; Step < int(mix(12,6,Roughness)); Step++) {
		Traversal += StepSize; 
		StepSize *= 1.4; 
		RayPos += Direction * StepSize;  
		
		vec4 ScreenPos = Project * vec4(RayPos, 1.0); 
		ScreenPos.xyz /= ScreenPos.w; 

		float Depth = textureLod(ViewPosition,ScreenPos.xy * 0.5 + 0.5,0.).z; 

		if(abs(ScreenPos.x) > 1.0 || abs(ScreenPos.y) > 1.0) 
		return vec2(-1.0); 

		if(Depth > RayPos.z) {

			float StepSizeBinary = StepSize * 0.5; 
			
			for(int StepBinarySearch = 0; StepBinarySearch < int(mix(4,2,Roughness)); StepBinarySearch++) {
				
				ScreenPos = Project * vec4(RayPos, 1.0); 
				ScreenPos.xyz /= ScreenPos.w; 

				Depth = textureLod(ViewPosition,ScreenPos.xy * 0.5 + 0.5,0.).z; 

				if(Depth > RayPos.z) {
				Traversal -= StepSizeBinary; 
				RayPos -= Direction * StepSizeBinary; 
				}
				else {
				Traversal += StepSizeBinary; 
				RayPos += Direction * StepSizeBinary; 
				}

				StepSizeBinary*=0.5; 
			}


			if(abs(Depth - RayPos.z) < StepSizeBinary * 8.0) {
				
				return ScreenPos.xy; 
			}
		}

		
	}
	return vec2(-1.); 
}




vec4 CalculateViewSpacePosition(float z) {


    vec4 ClipSpace = vec4(TexCoord * 2.0 - 1.0, z * 2. - 1., 1.0);
    vec4 ViewSpace = InverseProject * ClipSpace;

	return ViewSpace / ViewSpace.w; 

}

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

vec4 SampleInterpolatied(sampler2DArray Sampler,vec3 Coord) {

	float BaseTime = mod(Coord.z, 119.); 

	int Coord1 = int(floor(BaseTime)); 
	int Coord2 = int(ceil(BaseTime))%119; 

	return mix(texture(Sampler, vec3(Coord.xy, Coord1)), texture(Sampler,vec3(Coord.xy, Coord2)), fract(BaseTime)); 


}


void main() {

	//STEP 1: grab important data 


	vec4 NormalRoughness = texture(Normal, TexCoord); 
	vec3 ViewPosition = texture(ViewPosition, TexCoord).xyz; 
	vec3 WorldPosition = (InverseView * vec4(ViewPosition, 1.0)).xyz;
	
	float Roughness = NormalRoughness.a; 
	bool Hit; 

	float DepthSampleWater = texture(WaterDepth, TexCoord).x; 
	
	IndirectSpecular.xyz = NormalRoughness.xyz;
	

	if(texture(Depth, TexCoord).x > DepthSampleWater) {
		ViewPosition = CalculateViewSpacePosition(DepthSampleWater).xyz; 

		WorldPosition = vec3(inverse(View) * vec4(ViewPosition,1.0)); 
		NormalRoughness.xyz = texture(WaterNormal, TexCoord).xyz; 
		Roughness = 0.0; 

		

	} 
	
	vec3 Incident = normalize(WorldPosition - CameraPosition); 
	vec3 ViewNormal = vec3(vec4(NormalRoughness.xyz, 0.0) * InverseView); 

	float Traversal; 

	Seed = gl_FragCoord.x * 1080 + gl_FragCoord.y + Frame; 

	//Start of with a screen space method 

	vec2 ScreenSpaceHit = ScreenSpaceTracing(ViewPosition, ViewNormal, Roughness); 

	//is there a hit? 

	//IndirectSpecular = vec3(Roughness); 


	if(abs(ScreenSpaceHit.x) < 1.0 && abs(ScreenSpaceHit.y) < 1.0) {
		ScreenSpaceHit = ScreenSpaceHit * 0.5 + 0.5; 

		IndirectSpecular.xyz = texture(FirstPassDiffuse, ScreenSpaceHit).xyz * texture(Albedo, ScreenSpaceHit).xyz; 

		
	}
	else {

		Seed = gl_FragCoord.x * 1080 + gl_FragCoord.y + Frame; 
	
		vec3 Direction = GetSpecularRayDirection(reflect(Incident, NormalRoughness.xyz),NormalRoughness.xyz,Incident,Roughness); 

		vec4 SpecularSample = texture(Test, TraceCubeMap(CubeMapWorldPosition,Direction , WorldPosition + NormalRoughness.xyz * 0.5, CameraPosition,0.0, mix(2.0,4.0,Roughness), mix(0.5,1.0,Roughness), int(mix(4,2,Roughness)), int(mix(12,6,Roughness)), Hit, Traversal, 1.25)); 
	
	

		vec2 Coord = SampleSphericalMap(Direction); 
		Coord.y = Coord.y * 2. - 1.; 
	


		vec4 SkySample = Coord.y < 0.0 ? vec4(0.0) : SampleInterpolatied(Sky, vec3(1.0 - Coord,mod(Time*10.0, 119.))); 



		vec3 BackGround = mix(vec3(0.0,0.3,0.6), (2.0 * max(dot(LightDirection,SkySample.xyz),0.) + vec3(0.0,0.3,0.6)*1.25),SkySample.w); 


		SpecularSample.xyz = max(SpecularSample.xyz, vec3(0.)); 
		SpecularSample.xyz += SpecularSample.aaa * BackGround; 

		IndirectSpecular.xyz  = SpecularSample.xyz; 

	}


	float Frame = 0.0; 

	if(!First) {

		vec4 NDC = PreviousCombined * vec4(WorldPosition, 1.0); 

		NDC.xyz /= NDC.w; 

		if (abs(NDC.x) < 1.0 && abs(NDC.y) < 1.0) {
			NDC.xy = NDC.xy * 0.5 + 0.5; 

			vec4 Value = texture(Previous, NDC.xy); 

			Frame = min(Value.w + 1.0,11.); 

			IndirectSpecular.xyz = mix(IndirectSpecular.xyz, Value.xyz, min(float(Frame) / float(Frame + 1.0), mix(0.0, 2.0, sqrt(Roughness)))); 


		}

	}

	IndirectSpecular.a = Frame; 

}