#include "IndirectSpecular.h"

namespace Open7Days {
	namespace Rendering {

		

		void IndirectSpecularLighting::PrepareIndirectSpecularLighting(Window& Window) {
			IndirectSpecularBuffer = FrameBufferObjectPreviousData(Window.GetResolution(), GL_RGBA16F,false);

			IndirectSpecularShader = Shader("Shaders/IndirectSpecular");

			IndirectSpecularShader.Bind(); 
			
			IndirectSpecularShader.SetUniform("ViewPosition", 0); 
			IndirectSpecularShader.SetUniform("Normal", 1);
			IndirectSpecularShader.SetUniform("FirstPassDiffuse", 2);
			IndirectSpecularShader.SetUniform("Test", 3);
			IndirectSpecularShader.SetUniform("CubeMapWorldPosition", 4);
			IndirectSpecularShader.SetUniform("Previous", 5);
			IndirectSpecularShader.SetUniform("Albedo", 6);
			IndirectSpecularShader.SetUniform("Sky", 7);
			IndirectSpecularShader.SetUniform("Depth", 8);
			IndirectSpecularShader.SetUniform("WaterDepth", 9);
			IndirectSpecularShader.SetUniform("WaterNormal", 10);

			IndirectSpecularShader.UnBind(); 


		}

		void IndirectSpecularLighting::RenderIndirectSpecularLighting(Window& Window, Camera& Camera, DeferredRenderer& Deferred, CubeMapRenderer& CubeMap, FirstPassLighting& FirstPassLighting, WaterRenderer& Water, ShadowMapper& Shadows,TextureCubeMap& Sky) {
			
			IndirectSpecularBuffer.Bind(); 

			IndirectSpecularShader.Bind(); 

			IndirectSpecularShader.SetUniform("PreviousCombined", Camera.Project * Camera.PrevView);
			IndirectSpecularShader.SetUniform("InverseView", glm::inverse(Camera.View)); 
			IndirectSpecularShader.SetUniform("View", Camera.View);
			IndirectSpecularShader.SetUniform("Frame", (Window.GetFrameCount() % 16)*919);

			IndirectSpecularShader.SetUniform("Project", (Camera.Project));
			IndirectSpecularShader.SetUniform("InverseProject", glm::inverse(Camera.Project));

			IndirectSpecularShader.SetUniform("CameraPosition", Camera.Position);
			IndirectSpecularShader.SetUniform("Time", Window.GetTimeOpened());
			IndirectSpecularShader.SetUniform("LightDirection", glm::normalize(Shadows.Orientation));


			Deferred.DeferredUnwrappedBuffer.BindImage(1, 1);
			Deferred.DeferredUnwrappedBuffer.BindImage(2, 0);
			FirstPassLighting.FirstPassLightingBuffer.BindImage(0, 2); 

			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_CUBE_MAP, CubeMap.Lighting.Texture);

			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_CUBE_MAP, CubeMap.RawDeferred.Texture[1]);

			IndirectSpecularBuffer.BindImagePrevious(5); 

			Deferred.DeferredUnwrappedBuffer.BindImage(0, 6); 

			glActiveTexture(GL_TEXTURE7);	
			glBindTexture(GL_TEXTURE_2D_ARRAY, Water.SkyTexture);

			Deferred.DeferredRawBuffer.BindDepthImage(8); 
			Water.WaterBuffer.BindDepthImage(9); 
			Water.WaterBuffer.BindImage(10); 

			DrawPostProcessQuad(); 

			IndirectSpecularShader.UnBind(); 

			IndirectSpecularBuffer.UnBind(Window); 
			
		}

		void IndirectSpecularLighting::ReloadIndirectSpecular() {
			IndirectSpecularShader.Reload("Shaders/IndirectSpecular");

			IndirectSpecularShader.Bind();

			IndirectSpecularShader.SetUniform("ViewPosition", 0);
			IndirectSpecularShader.SetUniform("Normal", 1);
			IndirectSpecularShader.SetUniform("FirstPassDiffuse", 2);
			IndirectSpecularShader.SetUniform("Test", 3);
			IndirectSpecularShader.SetUniform("CubeMapWorldPosition", 4);
			IndirectSpecularShader.SetUniform("Previous", 5);
			IndirectSpecularShader.SetUniform("Albedo", 6);
			IndirectSpecularShader.SetUniform("Sky", 7);
			IndirectSpecularShader.SetUniform("Depth", 8);
			IndirectSpecularShader.SetUniform("WaterDepth", 9);
			IndirectSpecularShader.SetUniform("WaterNormal", 10);

			IndirectSpecularShader.UnBind();
		}

	}
}