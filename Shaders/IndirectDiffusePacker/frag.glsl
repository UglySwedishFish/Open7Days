#version 420


in vec2 TexCoord; 

layout(location = 0) out vec4 PackedOut; 

uniform vec2 TexelSize; 
uniform sampler2D PackedData; 
uniform int Size; 
uniform mat4 InverseProject; 
uniform mat3 InverseView; 
uniform bool Vertical; 

vec4 PackData(vec3 Normal, vec3 Diffuse, float Depth) {
	
	vec4 Packed; 

	Packed.x = uintBitsToFloat(packHalf2x16(Diffuse.xy)); 
	Packed.y = uintBitsToFloat(packHalf2x16(vec2(Diffuse.z, Normal.x))); 
	Packed.z = uintBitsToFloat(packHalf2x16(Normal.yz)); 
	Packed.w = Depth; 

	return Packed; 

}

void UnPackData(vec4 Packed, out vec3 Diffuse, out vec3 Normal, out float Depth) {
	
	vec2 UnPacked1 = unpackHalf2x16(floatBitsToUint(Packed.x)); 
	vec2 UnPacked2 = unpackHalf2x16(floatBitsToUint(Packed.y)); 
	vec2 UnPacked3 = unpackHalf2x16(floatBitsToUint(Packed.z)); 

	Diffuse = vec3(UnPacked1, UnPacked2.x); 
	Normal = vec3(UnPacked2.y, UnPacked3.xy); 
	Depth = Packed.w; 

}

vec3 GetIncident(){
	
    vec4 device_normal = vec4(TexCoord * 2. - 1., 0.0, 1.0);
    vec3 eye_normal = normalize((InverseProject * device_normal).xyz);
    vec3 world_normal = normalize(InverseView*eye_normal);
    return world_normal;
}

void main() {
	
	vec3 Diffuse, Normal; 
	float Depth; 

	ivec2 BaseCoord = ivec2(gl_FragCoord.xy);

	UnPackData(texelFetch(PackedData, BaseCoord,0), Diffuse, Normal, Depth);

	float Weight = 0.2; 

	Diffuse *= Weight; 

	vec3 IncidentVector = GetIncident(); 

	for(int x = -4; x <= 4; x++) {
		
		if(x != 0) {

			vec3 SampleNormal, SampleDiffuse; 
			float SampleDepth; 

			ivec2 NewCoord = BaseCoord + ivec2(!Vertical ? x : 0, Vertical ? x : 0) * Size;

			UnPackData(texelFetch(PackedData, NewCoord,0),SampleDiffuse, SampleNormal, SampleDepth); 

			if(abs(SampleDepth-Depth) < mix(0.1,1.0,pow(1.-abs(dot(Normal.xyz, IncidentVector)),2.))) {
				
				float SampleWeight = pow(max(dot(SampleNormal, Normal),0.0), 3.0) * (1.0 / (abs(x)+5.0)); 

				Diffuse += SampleDiffuse * SampleWeight; 
				
				Weight += SampleWeight; 

			}
		}

	}
	
	PackedOut = PackData(Normal, Diffuse / Weight, Depth); 


}