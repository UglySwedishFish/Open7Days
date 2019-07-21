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
uniform samplerCube Sky; 

uniform mat4 ViewMatrix; 
uniform mat4 ProjectionMatrix; 
uniform mat4 InverseView; 
uniform mat4 InverseProject; 

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
	return 0.04 + 0.96 * pow(max(1.0 - dot(Incident,Normal),0.0), 5.0);
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


			//if(abs(Depth - RayPos.z) < StepSizeBinary * 8.0) {
				
				return ScreenPos.xy; 
			//}
			
		}

		
	}
	return vec2(-1.); 
}


void main() {
	
	vec4 AlbedoSample = texture(Albedo, TexCoord); 
	vec4 RawNormalSample = texture(Normal, TexCoord); 
	float Roughness = RawNormalSample.a; 
	vec3 ActualAlbedo = AlbedoSample.xyz; 
	float Metalness = AlbedoSample.a; 

	vec3 Diffuse = texture(FirstPassDiffuse, TexCoord).xyz; 
	vec3 Specular = texture(IndirectSpecular, TexCoord).xyz; 

	vec3 Incident = -normalize(vec3(inverse(ProjectionMatrix * mat4(mat3(ViewMatrix))) * vec4(TexCoord * 2. - 1., 1., 1.))); 

	vec3 SpecularColor = Fresnel(Incident,RawNormalSample.xyz, mix(vec3(0.04), ActualAlbedo, Metalness), Roughness); 
	vec3 DiffuseColor = mix(ActualAlbedo, vec3(0.0), Metalness); 


	//check if were looking up at the sky 

	float DepthSample = texture(Depth, TexCoord).x; 
	float WaterDepth = texture(WaterDepth, TexCoord).x; 

	RawNormalSample.xyz = WaterDepth > DepthSample ? RawNormalSample.xyz : texture(WaterNormal, TexCoord).xyz; 

	if(length(RawNormalSample.xyz) < 0.5) {
		Lighting = texture(Sky, -Incident).xyz * 3.; 
	}
	else {
		if(WaterDepth > DepthSample) 
		Lighting = Diffuse * DiffuseColor + Specular * SpecularColor;
		else {

			vec3 BackGroundLighting = ActualAlbedo.xyz * Diffuse; 

			vec3 WaterViewPosition = CalculateViewSpacePosition(WaterDepth).xyz; 
			vec3 WorldPosition = vec3(InverseView * CalculateViewSpacePosition(DepthSample)); 

			WaterDepth = LinearlizeDepth(WaterDepth); 
			DepthSample = LinearlizeDepth(DepthSample); 

			vec2 RefractedLightingCoord = ScreenSpaceTracing(WaterViewPosition, vec3(vec4(RawNormalSample.xyz,0.0) * InverseView)); 

			RefractedLightingCoord = (abs(RefractedLightingCoord.x) < 1.0 && abs(RefractedLightingCoord.y) < 1.0) ? (RefractedLightingCoord * 0.5 + 0.5) : TexCoord; 

			vec3 RefractedLighting = texture(Albedo, RefractedLightingCoord).xyz * texture(FirstPassDiffuse, RefractedLightingCoord).xyz; 
			vec3 HitWorldPosition = vec3(InverseView * vec4(CalculateViewSpacePosition(texture(Depth, RefractedLightingCoord).x).xyz,1.0)); 
			RefractedLighting /= clamp(pow(max(abs(HitWorldPosition.y - 40.0)-.333,0.0),2.0) * 5,1.0,10000.); 



			RefractedLighting = mix(RefractedLighting * vec3(0.0, 0.35, 0.5 ) * 2.0, Specular, FresnelWater(Incident, RawNormalSample.xyz)); 

			Lighting = mix(BackGroundLighting,RefractedLighting,clamp(abs(40.0-WorldPosition.y), 0.0, 1.0)); 
			

		}
	}


	Lighting = Uncharted2Tonemap(Lighting); 
	Lighting = pow(Lighting, vec3(0.454545)); 


}