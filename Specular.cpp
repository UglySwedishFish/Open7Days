#include "IndirectSpecular.h"

namespace Zombiez {
	namespace Rendering {

		

		void IndirectSpecularLighting::PrepareIndirectSpecularLighting(Window& Window)
		{
			IndirectSpecularShader = Shader("Shaders/IndirectSpecular"); 

			IndirectSpecularBuffer = FrameBufferObjectPreviousData(Window.GetResolution(), GL_RGBA16F,false);

		}

		void IndirectSpecularLighting::RenderIndirectSpecularLighting(Window& Window, Camera& Camera, DeferredRenderer& Deferred, CubeMapRenderer& CubeMap, FirstPassLighting& FirstPassLighting)
		{
			
			
			


		}

	}
}