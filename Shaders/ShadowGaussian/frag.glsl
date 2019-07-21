#version 330

//super hardcoded for performance reasons 

//"magic" constants explained: 

//-0.005859375 = -3/512
//0.001953125 = 1/512
//0.07894736842 = (1/4)/3.16666667
//0.10526315789 = (1/2)/3.16666667
//0.15789473684 = (1/2)/3.16666667
//0.31578947368 = 1/3.16666667
//3.16666667 = 2*(1/4) + 2*(1/3) + 2*(1/2) + 1, which is the entire "sum" of the blur, 
//and because we want the sum of the blur to be 1 we divide by this number

in vec2 TexCoord; 
out float Color; 

uniform sampler2D Image; 
uniform bool Vertical; 
uniform float TexelSize; 

uniform int Size; 


void main() {	
	vec2 TC = TexCoord - vec2(Vertical ? 0.0 : TexelSize*3., Vertical ? TexelSize*Size : 0.0); 

	float Weight = 0.0; 
	
	for(int i=-Size;i<=Size;i++) {
		float cweight = 1.0 / (abs(i)+1.0); 
		Weight += cweight; 
		Color += texture(Image, TC).x * cweight; TC += vec2(Vertical ? 0.0 : TexelSize, Vertical ? TexelSize : 0.0); 
	}
	Color /= Weight; 

}