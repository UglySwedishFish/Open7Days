#version 330

//texture coordinate is calculated from world position + normal later
//specific details about material are stored in OutNormal.w 
//[3 8 bit values]
//materialprimary, materialsecondary, mixfactor
layout(location = 0) out vec3 OutNormal; 
layout(location = 1) out vec4 OutPosition;

in vec3 Position; 
in vec3 Normal; 
in vec2 TexCoord; 

flat in uvec3 Materials; 

in vec3 Flag; 


float PackMaterialData() {
	
	float Smallest; 
	vec2 Largest; 
	uvec2 PrimaryMaterials; 

	if(Materials.x == Materials.y) {
		
		Largest = vec2(Flag.x+Flag.y,Flag.z); 
		PrimaryMaterials = Materials.xz; 

	}
	else if(Materials.x == Materials.z) {
		Largest = vec2(Flag.x+Flag.z,Flag.y); 
		PrimaryMaterials = Materials.xy; 
	}
	else if(Materials.y == Materials.z) {
		Largest = vec2(Flag.x, Flag.y+Flag.z); 
		PrimaryMaterials = Materials.xz; 
	}
	else {
		if(Flag.x < Flag.y && Flag.x < Flag.z) {
			Largest = Flag.yz; 
			PrimaryMaterials = Materials.yz; 
		}
		else if(Flag.z < Flag.y) {
			Largest = Flag.xy; 
			PrimaryMaterials = Materials.xy; 
		}
		else {
			Largest = Flag.xz; 
			PrimaryMaterials = Materials.xz; 
		}
	}

	//Largest += Smallest / 2.f; 

	Largest = Largest / ((Largest.x+Largest.y)/2.f); 
	

	if(Largest.y > Largest.x) {
		Largest = Largest.yx; 
		PrimaryMaterials = PrimaryMaterials.yx; 
	}

	uint Packed = PrimaryMaterials.x * 65536u + PrimaryMaterials.y * 256u + clamp(uint(Largest.y*255.),0u,255u); 
	
	return uintBitsToFloat(Packed); 


}

float PackMaterialDataNew() {
	

	return uintBitsToFloat(Materials.x * 16777216u + Materials.y * 65536u + Materials.z * 256u + clamp(uint(Flag.x*15.),0u,15u) * 16u + clamp(uint(Flag.y*15.),0u,15u)); 



}


void main() {


	OutPosition = vec4(Position, PackMaterialDataNew()); 
	OutNormal = Normal; 
}