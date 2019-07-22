#version 420

in vec2 TexCoord; 
layout(location = 0) out vec3 Diffuse;
layout(location = 1) out vec3 Specular;

//TemporarySpecular will be used later in the combining stage 

//samplers: (ie textures)  
uniform sampler2D ViewPosition; 

uniform sampler2D Normal; 
uniform sampler2D BlueNoise; 
uniform sampler2D IndirectDiffuse; 
uniform sampler2D Depth; 
uniform sampler2D WaterDepth; 
uniform sampler2D WaterNormal;
uniform sampler2DArray WaterNormalMap; 
uniform sampler2D ShadowMaps[3]; 


uniform float Time; 


uniform vec3 CameraPositionLight[3]; 
uniform float Size[3]; 

//matrices		

uniform mat4 ShadowCombined[3]; 
uniform mat4 InverseView;
uniform mat4 InverseProject;

//vectors 

uniform vec3 ShadowDirection; 
uniform vec3 CameraPosition; 
uniform vec3 SunColor; 

vec4 SampleInterpolatied(sampler2DArray Sampler,vec3 Coord) {

	float BaseTime = mod(Coord.z, 119.); 

	int Coord1 = int(floor(BaseTime)); 
	int Coord2 = int(ceil(BaseTime))%119; 

	return mix(texture(Sampler, vec3(Coord.xy, Coord1)), texture(Sampler,vec3(Coord.xy, Coord2)), fract(BaseTime)); 


}


void UnPackData(vec4 Packed, out vec3 Diffuse, out vec3 Normal, out float Depth) {
	
	vec2 UnPacked1 = unpackHalf2x16(floatBitsToUint(Packed.x)); 
	vec2 UnPacked2 = unpackHalf2x16(floatBitsToUint(Packed.y)); 
	vec2 UnPacked3 = unpackHalf2x16(floatBitsToUint(Packed.z)); 
	vec2 UnPacked4 = unpackHalf2x16(floatBitsToUint(Packed.w)); 

	Diffuse = vec3(UnPacked1, UnPacked2.x); 
	Normal = vec3(UnPacked2.y, UnPacked3.xy); 
	Depth = UnPacked4.x; 

}

vec3 ImportanceGGX(vec2 xi, float roughness)
{
    float r_square = roughness * roughness;
    float phi = 6.2831 * xi.x;
    float cos_theta = sqrt((1 - xi.y) / (1 + (r_square * r_square - 1) * xi.y));
    float sin_theta = sqrt(1 - cos_theta * cos_theta);

    return vec3(sin_theta * cos(phi), sin_theta * sin(phi), cos_theta);
}

vec2 hash2(float seed) {
	return fract(vec2(sin(vec2(seed, seed + 0.1f))*vec2(43758.5453123f, 22578.1459123f)));
}




float ShadowsESM(sampler2D ShadowMap, vec3 Coord) {

	//return (Coord.z > (texture(ShadowMap, Coord.xy).x+0.0003) ? 0. : 1.0); 

	return clamp(exp(-10000.0*(Coord.z-(texture(ShadowMap, Coord.xy).x)-.0015)), 0.0,1.0); 
}


vec2 GetDirectionalShadows(vec3 WorldPosition, vec3 Normal, vec3 Incident, float Roughness, float Noise) {
	vec3 ShadowNDC = vec3(0.); 

	int Index = -1; 

	float Factor = 1.0 - 0.25 * Noise; 

	for(int try = 0; try < 3; try++) {

	vec4 ShadowCoord = ShadowCombined[try] * vec4(WorldPosition, 1.0); 

	ShadowNDC = ShadowCoord.xyz / ShadowCoord.w; 




	if(abs(ShadowNDC.x) < Factor && abs(ShadowNDC.y) < Factor) {
		Index = try; 
		break; 
	}

	}

	


	if(Index == -1) 
		return vec2(1.); 

	float ShadowMapSample = ShadowsESM((Index == 0 ? ShadowMaps[0] : Index == 1 ? ShadowMaps[1] : ShadowMaps[2]), ShadowNDC.xyz * 0.5 + 0.5) * 100.;

	return vec2(ShadowMapSample * max(pow(dot(Normal,normalize(ShadowDirection)),3.),0.),
					ShadowMapSample * pow(max(dot(reflect(Incident,Normal), ShadowDirection),0.0),32.0 + 500.0 * (1.0-Roughness))); 

}


vec3 TransformToWorld(vec3 Direction, vec3 Normal) {
    // Find an axis that is not parallel to normal
    vec3 majorAxis;
    if (abs(Normal.x) < 0.57735026919f ) {
        majorAxis = vec3(1, 0, 0);
    } else if (abs(Normal.y) < 0.57735026919f ) {
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


float A = 0.15;
float B = 0.50;
float C = 0.10;
float D = 0.20;
float E = 0.02;
float F = 0.30;
float W = 11.2;

vec3 Uncharted2Tonemap(vec3 x)
{
   return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}


vec3 GetUpscaledIndirectDiffuse(vec3 Normal, float Depth) {
	//(1.0-dot(N, SN)) + abs(D-SD)
	float Closest = -1.0; 
	vec3 Result = vec3(0.0); 

	ivec2 BaseCoord = ivec2(gl_FragCoord.xy / 4); 

	for(int x = -1; x<=1;x++) {
		for (int y = -1; y<=1;y++) {
			
			vec4 Sample = texelFetch(IndirectDiffuse, BaseCoord + ivec2(x,y),0); 

			vec3 SampleNormal, Lighting; 
			float SampleDepth; 

			UnPackData(Sample, Lighting, SampleNormal, SampleDepth); 

			float Weight = (1.0 - dot(Normal, SampleNormal)) + abs(Depth-SampleDepth); 

			if(Weight < Closest || Closest < 0) {
				Closest = Weight; 
				Result = Lighting; 
			}


		}
	}

	return Result; 
}


float zNear = 0.1; 
float zFar = 1000.; 

float LinearlizeDepth(float z) {
	
	return 2.0 * zNear * zFar / (zFar + zNear - (z*2.-1.) * (zFar - zNear)); 

}



vec3 Volumetric(vec3 Direction, vec3 Origin, float DistanceToWorldPosition, float DitherOffset) {

	//grab dither offset 




	float StepSize = DistanceToWorldPosition / 12.0f;

	float StepLength = DitherOffset * StepSize;

	vec3 OriginActual = Origin + Direction * DitherOffset * StepSize;

	vec3 Position = OriginActual;

	vec3 Result = vec3(0.);

	float Weight = 0.f;

	float Dot = max(dot(Direction, ShadowDirection), 0.);

	float StepLengthActualPrevious = 0.0;

	while (StepLength < DistanceToWorldPosition) {

		

		StepLength += StepSize;

		float StepLengthActual = StepLength / DistanceToWorldPosition;
		//if(Fake) 
		//StepLengthActual = StepLengthActual * StepLengthActual; 
		StepLengthActual = StepLengthActual * DistanceToWorldPosition;




		Position = OriginActual + Direction * StepLengthActual;

		float StepSizeActual = StepLengthActual - StepLengthActualPrevious;


		//grab shadow sample 

		vec3 ShadowNDC = vec3(0.);

		int Index = -1;

		for (int try = 0; try < 5; try++) {

			vec4 ShadowCoord = ShadowCombined[try] * vec4(Position, 1.0);

			ShadowNDC = ShadowCoord.xyz / ShadowCoord.w;

			if (abs(ShadowNDC.x) < 1. && abs(ShadowNDC.y) < 1.) {
				Index = try;
				break;
			}

		}

		if (Index == -1)
			continue; 

		ShadowNDC = ShadowNDC * .5 + .5; 
		

		float CurrentWeight = StepSizeActual;

		Result += vec3(1.0, 0.6, 0.3) * 2.25f * ShadowsESM(ShadowMaps[Index], ShadowNDC.xyz) * CurrentWeight;
		Weight += CurrentWeight;

		StepLengthActualPrevious = StepLengthActual;




	}

	return Result / Weight;

}


float GetWaves(vec2 Point, float Time, int Iterations) {
    
    float Result = 0.0; 
    float Speed = 1.0; 
    float Multiplier = 1.0; 
    float Weight = 1.0; 
    float TotalWeight = 0.0; 
    vec2 Position = Point; 
    
    
    for(int x = 0; x < Iterations; x++) {
    	vec2 Direction = vec2(sin(float(x)), cos(float(x)));
        
        float Height = exp2(sin(dot(Direction, Position) * Multiplier + Speed * Time) - 1.0); 
        
        Position += Direction * Height * Weight * 0.05;  
        
        Result += Height * Weight; 
        TotalWeight += Weight; 
        
        Weight = Weight * 0.8; 
        Multiplier *= 1.25; 
        Speed *= 1.1; 
        
        
    }
    
    return Result / TotalWeight; 
    
}

float Caustics(vec2 P, float Time, int Iterations) {
    
    float RawX = 1.0 - abs(GetWaves(P, Time, Iterations) * 2. -1.0); 
    
	return RawX * RawX * RawX;  
}

float ActualCaustics(vec2 P, float Time, int Iterations) {
	return Caustics(P, Time * 0.912, Iterations) * Caustics(P, Time* 0.959, Iterations) * Caustics(P, -Time * 0.6237, Iterations) * Caustics(P, Time, Iterations);
}

vec4 CalculateViewSpacePosition(float z) {


    vec4 ClipSpace = vec4(TexCoord * 2.0 - 1.0, z * 2. - 1., 1.0);
    vec4 ViewSpace = InverseProject * ClipSpace;

	return ViewSpace / ViewSpace.w; 

}

void main() {

	//STEP 1: grab important data 

	float Seed = gl_FragCoord.x * 1920 + gl_FragCoord.y; 


	vec4 NormalRoughness = texture(Normal, TexCoord); 
	vec3 ViewPosition = texture(ViewPosition, TexCoord).xyz; 
	vec3 WorldPosition = (InverseView * vec4(ViewPosition, 1.0)).xyz;
	vec3 Incident = normalize(WorldPosition - CameraPosition); 

	vec2 TexCoordNoise = (TexCoord / 128.) * textureSize(Normal, 0);  

	vec4 NoiseFactor = texture(BlueNoise, TexCoordNoise); 
	
	//START WITH DIRECT LIGHTING 

	vec2 DirectionalShadows = GetDirectionalShadows(WorldPosition, NormalRoughness.xyz, Incident, NormalRoughness.a,NoiseFactor.x); 

	Diffuse =  SunColor * DirectionalShadows.x; 

	float WaterHeight = 40.0 + SampleInterpolatied(WaterNormalMap, vec3(WorldPosition.xz*0.05,mod(Time*10.0, 119.))).w * 2.1; 
	
	Diffuse += (clamp(WaterHeight - WorldPosition.y, 0.0, 5.0) * (ActualCaustics(WorldPosition.xz * 10.0, Time, 10) + vec3(0.0, 0.35, 0.5 )) * 5.0); 


	
	float DepthSample = texture(Depth, TexCoord).x; 
	float LinearDepthSample = LinearlizeDepth(DepthSample); 

	Diffuse += GetUpscaledIndirectDiffuse(NormalRoughness.xyz, LinearDepthSample);
	
	Diffuse /= WorldPosition.y > WaterHeight ? 1.0 : clamp(pow(max(abs(WorldPosition.y - WaterHeight)-.333,0.0),2.0) * 5,1.0,10000.);


	float WaterDepthSample = texture(WaterDepth, TexCoord).x; 

	if(WaterDepthSample > DepthSample)
		Specular = SunColor * DirectionalShadows.y; 
	else {
		//do it for the water now! yay! 

		Specular = SunColor * GetDirectionalShadows(vec3(InverseView * CalculateViewSpacePosition(WaterDepthSample)), texture(WaterNormal, TexCoord).xyz, Incident, 0.0,NoiseFactor.x).y; 


	}
	
}