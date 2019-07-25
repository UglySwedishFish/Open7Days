#include "Pipeline.h"

int main() {
	Window Screen = Window(Vector2i(1920, 1080), true); 
	Open7Days::Camera Camera = Open7Days::Camera(90.0, 0.1, 1000., Vector3f(16.,128.,16.), Vector3f(0.), Screen); 
	Open7Days::Pipeline Pipeline; 
	Pipeline.Prepare(Screen, Camera); 
	Pipeline.Run(Screen, Camera); 
}
