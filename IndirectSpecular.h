#pragma once

#include <FirstPassLighting.h>

namespace Open7Days {
	namespace Rendering {
		
		struct IndirectSpecularLighting {

			Shader IndirectSpecularShader; 
			FrameBufferObjectPreviousData IndirectSpecularBuffer; 

			bool First = true; 

			void PrepareIndirectSpecularLighting(Window& Window); 
			void RenderIndirectSpecularLighting(Window& Window, Camera& Camera, DeferredRenderer& Deferred, CubeMapRenderer& CubeMap, FirstPassLighting& FirstPassLighting,WaterRenderer & Water, TextureCubeMap& Sky);
			void ReloadIndirectSpecular(); 

		};

	}
}