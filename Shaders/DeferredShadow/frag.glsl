#version 330

out float Depth; 

void main() {
	Depth = pow(gl_FragCoord.z,1.0); 
}
