#pragma once

#include "Shadows.h"
#include "DeferredRenderer.h"
#include "Texture.h"

namespace Open7Days {

	namespace Rendering {

		#define CUBEMAPRESOLUTION 128


		struct CubeMapRenderer {

			CubeMultiPassFrameBufferObject RawDeferred;
			CubeFrameBufferObject Lighting; 
			Shader CubeLighting, CubeDeferred; 
			Frustum* ViewFrustum; 

			void PrepareCubeMapRenderer(); 
			void RenderToCubeMap(Window& Window, Camera& Camera, DeferredRenderer& Deferred, ShadowMapper& Shadows, ChunkContainer& Chunks); 

			
			

		};

	}

}
