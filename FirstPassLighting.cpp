#include "FirstPassLighting.h"

namespace Open7Days {
	namespace Rendering {
		void FirstPassLighting::Prepare(Window& Window)
		{

			FirstPassLightingBuffer = MultiPassFrameBufferObject(Window.GetResolution(), 2, { GL_RGB16F, GL_RGB16F }, false); 
			FirstPassLightingShader = Shader("Shaders/FirstPassLighting"); 

			FirstPassLightingShader.Bind(); 

			FirstPassLightingShader.SetUniform("ViewPosition", 0); 
			FirstPassLightingShader.SetUniform("Normal", 1);
			FirstPassLightingShader.SetUniform("BlueNoise", 2);
			FirstPassLightingShader.SetUniform("IndirectDiffuse", 3);
			FirstPassLightingShader.SetUniform("Depth", 4);
			FirstPassLightingShader.SetUniform("Albedo", 5);
			FirstPassLightingShader.SetUniform("Test", 6);
			FirstPassLightingShader.SetUniform("CubeMapWorldPosition", 7);

			FirstPassLightingShader.UnBind(); 

			BlueNoise = LoadTextureGL("Textures/BlueNoise.png");



		}
		void FirstPassLighting::ReloadShader()
		{
			FirstPassLightingShader.Reload("Shaders/FirstPassLighting");

			FirstPassLightingShader.Bind();

			FirstPassLightingShader.SetUniform("ViewPosition", 0);
			FirstPassLightingShader.SetUniform("Normal", 1);
			FirstPassLightingShader.SetUniform("BlueNoise", 2);
			FirstPassLightingShader.SetUniform("IndirectDiffuse", 3);
			FirstPassLightingShader.SetUniform("Depth", 4);
			FirstPassLightingShader.SetUniform("Albedo", 5);
			FirstPassLightingShader.SetUniform("Test", 6);
			FirstPassLightingShader.SetUniform("CubeMapWorldPosition", 7);

			FirstPassLightingShader.UnBind();
		}
		void FirstPassLighting::RenderFirstLightingPass(Window& Window, Camera& Camera, DeferredRenderer &Deferred, ShadowMapper &Shadows, IndirectDiffuseLighting& Diffuse, CubeMapRenderer& CubeMap)
		{

			FirstPassLightingBuffer.Bind(); 

			FirstPassLightingShader.Bind(); 

			FirstPassLightingShader.SetUniform("InverseView", glm::inverse(Camera.View)); 
			FirstPassLightingShader.SetUniform("CameraPosition", Camera.Position);
			FirstPassLightingShader.SetUniform("Time", Window.GetTimeOpened());

			Deferred.DeferredUnwrappedBuffer.BindImage(1, 1); 
			Deferred.DeferredUnwrappedBuffer.BindImage(2, 0);
			BlueNoise.Bind(2); 
			Diffuse.IndirectDiffusePongBuffer.BindImage(3); 
			Deferred.DeferredRawBuffer.BindDepthImage(4);
			Deferred.DeferredUnwrappedBuffer.BindImage(0, 5);

			glActiveTexture(GL_TEXTURE6); 
			glBindTexture(GL_TEXTURE_CUBE_MAP, CubeMap.Lighting.Texture); 

			glActiveTexture(GL_TEXTURE7);
			glBindTexture(GL_TEXTURE_CUBE_MAP, CubeMap.RawDeferred.Texture[1]);

			//CubeMap.RawDeferred;

			Shadows.BindAndSetUniforms(FirstPassLightingShader, 8); 

			DrawPostProcessQuad(); 

			FirstPassLightingShader.UnBind(); 

			FirstPassLightingBuffer.UnBind(Window);


		}
	}
}
