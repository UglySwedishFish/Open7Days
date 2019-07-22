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
			FirstPassLightingShader.SetUniform("WaterDepth", 5);
			FirstPassLightingShader.SetUniform("WaterNormal", 6);
			FirstPassLightingShader.SetUniform("WaterNormalMap", 7);

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
			FirstPassLightingShader.SetUniform("WaterDepth", 5);
			FirstPassLightingShader.SetUniform("WaterNormal", 6);
			FirstPassLightingShader.SetUniform("WaterNormalMap", 7);


			FirstPassLightingShader.UnBind();
		}
		void FirstPassLighting::RenderFirstLightingPass(Window& Window, Camera& Camera, DeferredRenderer &Deferred, ShadowMapper &Shadows, IndirectDiffuseLighting& Diffuse, CubeMapRenderer& CubeMap, WaterRenderer& Water)
		{

			FirstPassLightingBuffer.Bind(); 

			FirstPassLightingShader.Bind(); 

			FirstPassLightingShader.SetUniform("InverseView", glm::inverse(Camera.View)); 
			FirstPassLightingShader.SetUniform("InverseProject", glm::inverse(Camera.Project));

			FirstPassLightingShader.SetUniform("CameraPosition", Camera.Position);
			FirstPassLightingShader.SetUniform("Time", Window.GetTimeOpened());

			Deferred.DeferredUnwrappedBuffer.BindImage(1, 1); 
			Deferred.DeferredUnwrappedBuffer.BindImage(2, 0);
			BlueNoise.Bind(2); 
			Diffuse.IndirectDiffusePongBuffer.BindImage(3); 
			Deferred.DeferredRawBuffer.BindDepthImage(4);
			Water.WaterBuffer.BindDepthImage(5); 
			Water.WaterBuffer.BindImage(6); 


			//CubeMap.RawDeferred;

			Shadows.BindAndSetUniforms(FirstPassLightingShader, 8); 

			DrawPostProcessQuad(); 

			FirstPassLightingShader.UnBind(); 

			FirstPassLightingBuffer.UnBind(Window);


		}
	}
}
