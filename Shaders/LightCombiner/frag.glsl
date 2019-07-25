#version 420

in vec2 TexCoord; 
layout(location = 0) out vec3 Lighting;

uniform sampler2D FirstPassDiffuse; //indirect + direct diffuse
uniform sampler2D FirstPassSpecular; //direct specular
uniform sampler2D IndirectSpecular; //indirect specular
uniform sampler2D Normal; //contains roughness 
uniform sampler2D Albedo; //contains albedo + metalness 
uniform sampler2D ViewPosition; 
uniform sampler2D Depth; 
uniform sampler2D WaterDepth; 
uniform sampler2D WaterNormal;
uniform sampler2DArray WaterNormalMap; 
uniform sampler2DArray Sky; 
uniform float Time; 
uniform vec3 LightDirection; 

uniform mat4 ViewMatrix; 
uniform mat4 ProjectionMatrix; 
uniform mat4 InverseView; 
uniform mat4 InverseProject; 
uniform vec3 CameraPosition; 
in float WaterHeightCamera; 

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

vec3 Fresnel(vec3 Incident, vec3 Normal, vec3 Specular, float Roughness) {
	return Specular + (max(vec3(1-Roughness) - Specular,vec3(0.))) * pow(max(1.0 - abs(dot(Incident,Normal)),0.0), 5.0);
}

float FresnelWater(vec3 Incident, vec3 Normal) {

	return 0.04 + 0.96 * pow(max(1.0 - abs(dot(Incident,Normal)),0.0), 5.0);
}

vec4 CalculateViewSpacePosition(float z) {


    vec4 ClipSpace = vec4(TexCoord * 2.0 - 1.0, z * 2. - 1., 1.0);
    vec4 ViewSpace = InverseProject * ClipSpace;

	return ViewSpace / ViewSpace.w; 

}

float zNear = 0.1; 
float zFar = 1000.; 

float LinearlizeDepth(float z) {
	
	return 2.0 * zNear * zFar / (zFar + zNear - (z*2.-1.) * (zFar - zNear)); 

}

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}




vec2 ScreenSpaceTracing(vec3 ViewPos, vec3 Normal) {

	vec3 Incident = normalize(ViewPos); 
	vec3 Direction = refract(Incident, Normal,1/1.33333); 

	float StepSize = 2.0; 

	float Traversal = 0.0; 

	vec3 RayPos = ViewPos; 

	for(int Step = 0; Step < 6; Step++) {
		Traversal += StepSize; 
		StepSize *= 1.4; 
		RayPos += Direction * StepSize;  
		
		vec4 ScreenPos = ProjectionMatrix * vec4(RayPos, 1.0); 
		ScreenPos.xyz /= ScreenPos.w; 

		float Depth = textureLod(ViewPosition,ScreenPos.xy * 0.5 + 0.5,0.).z; 

		if(abs(ScreenPos.x) > 1.0 || abs(ScreenPos.y) > 1.0) 
		return vec2(-1.0); 

		if(Depth > RayPos.z) {

			float StepSizeBinary = StepSize * 0.5; 
						
			for(int StepBinarySearch = 0; StepBinarySearch < 4; StepBinarySearch++) {
				
				ScreenPos = ProjectionMatrix * vec4(RayPos, 1.0); 
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


				
				return ScreenPos.xy; 
			
			
		}

		
	}
	return vec2(-1.); 
}


float Fog(vec3 WorldPos, vec3 CamPos) {

	float BaseDistance = min(distance(WorldPos, CamPos) / 90.,1.0); 

	return pow(BaseDistance,4.0); 

}

vec4 SampleInterpolatied(sampler2DArray Sampler,vec3 Coord) {

	float BaseTime = mod(Coord.z, 119.); 

	int Coord1 = int(floor(BaseTime)); 
	int Coord2 = int(ceil(BaseTime))%119; 

	return mix(texture(Sampler, vec3(Coord.xy, Coord1)), texture(Sampler,vec3(Coord.xy, Coord2)), fract(BaseTime)); 


}

const float LN2 = 0.693147181; 

vec3 AnalyticalWaterDarkness(float Traversal, vec3 Incident) {
	
	float X = Traversal; 

	vec3 A = vec3(1.0,0.65,0.5); 
	vec3 B = vec3(1.0,0.5,0.65); 

	if(X<=1) {
		return 0.4 * ((B-2*A)/(X+1.)+(B-A)*log(X+1)); 
	}
	else {
		return 0.4 * ((B*(X*X+X*LN2+LN2))/((X)+1.) - A*(1 + LN2)); 
	}

}

#define PI 3.14159265359
vec3 decode (vec2 enc)
{
    vec2 fenc = enc*4-2;
    float f = dot(fenc,fenc);
    float g = sqrt(1-f/4);
    vec3 n;
    n.xy = fenc*g;
    n.z = 1-f/2;
    return normalize(n);
}
void main() {
	
	vec4 AlbedoSample = texture(Albedo, TexCoord); 
	vec4 RawNormalSample = texture(Normal, TexCoord); 
	float Roughness = RawNormalSample.a; 
	vec3 ActualAlbedo = AlbedoSample.xyz; 
	float Metalness = AlbedoSample.a; 

	vec3 Diffuse = texture(FirstPassDiffuse, TexCoord).xyz; 
	vec3 Specular = texture(IndirectSpecular, TexCoord).xyz + texture(FirstPassSpecular, TexCoord).xyz; 

	vec3 Incident = -normalize(vec3(inverse(ProjectionMatrix * mat4(mat3(ViewMatrix))) * vec4(TexCoord * 2. - 1., 1., 1.))); 

	vec3 SpecularColor = Fresnel(Incident,RawNormalSample.xyz, mix(vec3(0.04), ActualAlbedo, Metalness), Roughness); 
	vec3 DiffuseColor = mix(ActualAlbedo, vec3(0.0), Metalness); 


	float DepthSample = texture(Depth, TexCoord).x; 
	float WaterDepth = texture(WaterDepth, TexCoord).x; 

	vec3 WorldPosition = vec3(InverseView * CalculateViewSpacePosition(DepthSample)); 

	vec3 WaterViewPosition = CalculateViewSpacePosition(WaterDepth).xyz; 

	vec3 WaterWorldPosition = vec3(InverseView * vec4(WaterViewPosition,1.0)); 

	RawNormalSample.xyz = WaterDepth > DepthSample ? RawNormalSample.xyz : texture(WaterNormal, TexCoord).xyz; 
	
	vec2 Coord = SampleSphericalMap(-Incident); 
	Coord.y = Coord.y * 2. - 1.; 

	vec4 SkyTextureSample = SampleInterpolatied(Sky, vec3(1.0 - Coord,mod(Time*10.0, 119.))); ; 
	
	if(abs(Coord.x*2.-1.) > 0.998) {
		
		SkyTextureSample = mix(SampleInterpolatied(Sky, vec3(0.999,1.0-Coord.y,mod(Time*10.0, 119.))),SampleInterpolatied(Sky, vec3(0.001,1.0-Coord.y,mod(Time*10.0, 119.))),0.5); 


	}
		

	
	vec4 SkySample = Coord.y < 0.01 ? vec4(0.0) :SkyTextureSample; 



	vec3 BackGround = mix(vec3(0.0,0.3,0.6), (2.0 * max(dot(LightDirection,SkySample.xyz),0.) + vec3(0.0,0.3,0.6)*1.25),SkySample.w); 

	vec3 RelativeCameraPosition = CameraPosition - Incident * 0.15;

	float ActualRelativeHeight = 40.0 + SampleInterpolatied(WaterNormalMap, vec3(RelativeCameraPosition.xz * 0.05,mod(Time*10.0, 119.))).w * 2.1; 


	if(length(RawNormalSample.xyz) < 0.5) {

		

		Lighting = BackGround; 

		if(RelativeCameraPosition.y < ActualRelativeHeight) 
			Lighting = vec3(0.0); 

	}
	else {

		bool FragmentUnderWater = RelativeCameraPosition.y < ActualRelativeHeight; 
		bool FragmentWaterPlane = WaterDepth < DepthSample; 

		if(!FragmentWaterPlane) {
		Lighting = FragmentUnderWater ? Diffuse * AlbedoSample.xyz : Diffuse * DiffuseColor + Specular * SpecularColor;

		Lighting = mix(Lighting, BackGround, Fog(CameraPosition,WorldPosition)); 

		}
		else {

			vec3 NormalSampleRefraction = normalize(RawNormalSample.xyz * vec3(1.0,0.125,1.0)); 


			vec3 BackGroundLighting = ActualAlbedo.xyz * Diffuse; 

			


			WaterDepth = LinearlizeDepth(WaterDepth); 
			DepthSample = LinearlizeDepth(DepthSample); 

			vec2 RefractedLightingCoord = ScreenSpaceTracing(WaterViewPosition, vec3(vec4(FragmentUnderWater ? -NormalSampleRefraction.xyz : NormalSampleRefraction.xyz,0.0) * InverseView)); 

			RefractedLightingCoord = (abs(RefractedLightingCoord.x) < 1.0 && abs(RefractedLightingCoord.y) < 1.0) ? (RefractedLightingCoord * 0.5 + 0.5) : TexCoord; 

			vec3 RefractedLightingRaw = texture(Albedo, RefractedLightingCoord).xyz * texture(FirstPassDiffuse, RefractedLightingCoord).xyz; 
			vec3 RefractedLighting = RefractedLightingRaw; 
			vec3 HitWorldPosition = vec3(InverseView * vec4(CalculateViewSpacePosition(texture(Depth, RefractedLightingCoord).x).xyz,1.0)); 
			if(!FragmentUnderWater) 
				RefractedLighting /= clamp(pow(max(abs(HitWorldPosition.y - WaterWorldPosition.y)-.333,0.0),2.0) * 5,1.0,10000.); 



			RefractedLighting = mix(RefractedLighting * mix(vec3(0.0, 0.35, 0.5 ),vec3(0.0,0.5,0.35),clamp(abs(WaterWorldPosition.y-WorldPosition.y),0.0,2.0)*0.5)  * 2.0, Specular, FresnelWater(Incident, RawNormalSample.xyz)); 

			Lighting = mix(RefractedLightingRaw,RefractedLighting,clamp(abs(WaterWorldPosition.y-WorldPosition.y), 0.0, 1.0)); 
			Lighting = mix(BackGroundLighting,Lighting,clamp(abs(WaterWorldPosition.y-WorldPosition.y), 0.0, .25)*4.); 

			Lighting = mix(Lighting, BackGround, Fog(CameraPosition,WaterWorldPosition)); 


		

		}

		if(FragmentUnderWater) {

		float WorldDistance = distance(FragmentWaterPlane ? WaterWorldPosition : WorldPosition, CameraPosition); 

		vec3 LightingRaw = Lighting; 

		//use an analytical solution of an integral describing the water darkness at a point 

		float T = WorldDistance; 

		vec3 WaterDarkness = 1.0- clamp((AnalyticalWaterDarkness(WorldDistance*2.0, -Incident)),vec3(0.0),vec3(1.)); 

		Lighting *= WaterDarkness; 

		}

	}

	
	Lighting = Uncharted2Tonemap(Lighting); 
	Lighting = pow(Lighting, vec3(0.454545)); 


}