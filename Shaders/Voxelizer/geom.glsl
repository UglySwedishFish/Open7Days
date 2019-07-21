#version 330

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in GeometryData {
	 vec3 Normal; 
	 vec3 WorldPosition; 
}GS_IN[]; 

out FragmentData {
	out vec3 WorldPosition; 
	out vec3 Normal; 
}GS_OUT; 

uniform float Size; 

void main(){
	vec3 p1 = GS_IN[1].WorldPosition - GS_IN[0].WorldPosition;
	vec3 p2 = GS_IN[2].WorldPosition - GS_IN[0].WorldPosition;
	vec3 p = abs(cross(p1, p2)); 
	for(int i = 0; i < 3; i++){  //the actual voxelisation 
		GS_OUT.WorldPosition = GS_IN[i].WorldPosition;
		GS_OUT.Normal =  GS_IN[i].Normal;

		if(p.z > p.x && p.z > p.y){
			gl_Position = vec4(GS_OUT.WorldPosition.x, GS_OUT.WorldPosition.y, 0, 1) / vec4(Size/2,Size/2,1,1);
		} else if (p.x > p.y && p.x > p.z){
			gl_Position = vec4(GS_OUT.WorldPosition.y, GS_OUT.WorldPosition.z, 0, 1) / vec4(Size/2,Size/2,1,1);
		} else {
			gl_Position = vec4(GS_OUT.WorldPosition.x, GS_OUT.WorldPosition.z, 0, 1) / vec4(Size/2,Size/2,1,1);
		}
		EmitVertex();
	}
    EndPrimitive();
}