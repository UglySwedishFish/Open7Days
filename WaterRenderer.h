#pragma once

#include "Framebuffer.h"
#include "Shader.h"
#include "Camera.h"
#include "Mesh.h"

namespace Open7Days {
	namespace Rendering {
		struct WaterRenderer {
			unsigned int WaterNormalTexture; 
			Shader WaterDeferred; 
			FrameBufferObject WaterBuffer; 
			Model WaterMesh; 

			void PrepareWaterRenderer(Window & Window); 
			void ReloadWaterRenderer(); 
			void RenderWater(Camera & Camera, Window & Window); 


		};
	}
}
