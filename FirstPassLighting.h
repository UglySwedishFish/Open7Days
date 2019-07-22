#pragma once

#include "IndirectDiffuse.h"
#include "CubeMapRenderer.h"
#include "WaterRenderer.h"

namespace Open7Days {
	namespace Rendering {

		struct FirstPassLighting {

			Shader FirstPassLightingShader; 
			MultiPassFrameBufferObject FirstPassLightingBuffer; 
			TextureGL BlueNoise;
			


			void Prepare(Window &Window);
			void ReloadShader(); 
			void RenderFirstLightingPass(Window& Window, Camera& Camera, DeferredRenderer &Deferred, ShadowMapper &Shadows, IndirectDiffuseLighting & Diffuse, CubeMapRenderer & CubeMap, WaterRenderer & Water); 



		};



	}
}
