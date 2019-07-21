#include "LightCombiner.h"

namespace Open7Days {
	namespace Rendering {



		void LightCombiner::PrepareLightCombiner()
		{

			LightCombinerShader = Shader("Shaders/LightCombiner"); 

			LightCombinerShader.Bind(); 
			
			LightCombinerShader.SetUniform("FirstPassDiffuse", 0); 
			LightCombinerShader.SetUniform("FirstPassSpecular", 1);
			LightCombinerShader.SetUniform("IndirectSpecular", 2);
			LightCombinerShader.SetUniform("Normal", 3);
			LightCombinerShader.SetUniform("Albedo", 4);
			LightCombinerShader.SetUniform("Sky", 5); 
			LightCombinerShader.SetUniform("Depth", 6);
			LightCombinerShader.SetUniform("WaterDepth", 7);
			LightCombinerShader.SetUniform("WaterNormal", 8);

			LightCombinerShader.UnBind();


		}

		void LightCombiner::CombineLighting(Window& Window, Camera& Camera, DeferredRenderer& Deferred, FirstPassLighting& First, IndirectSpecularLighting& IndirectSpecular, WaterRenderer & Water,TextureCubeMap& Sky)
		{

			LightCombinerShader.Bind(); 

			LightCombinerShader.SetUniform("ViewMatrix", Camera.View); 
			LightCombinerShader.SetUniform("ProjectionMatrix", Camera.Project);
			LightCombinerShader.SetUniform("InverseView", glm::inverse(Camera.View));
			LightCombinerShader.SetUniform("InverseProject", glm::inverse(Camera.Project));

			
			First.FirstPassLightingBuffer.BindImage(0, 0); 
			First.FirstPassLightingBuffer.BindImage(1, 1);
			IndirectSpecular.IndirectSpecularBuffer.BindImage(2); 
			Deferred.DeferredUnwrappedBuffer.BindImage(1, 3); 
			Deferred.DeferredUnwrappedBuffer.BindImage(0, 4);
			Sky.Bind(5); 
			Deferred.DeferredRawBuffer.BindDepthImage(6); 
			Water.WaterBuffer.BindDepthImage(7); 
			Water.WaterBuffer.BindImage(8); 

			DrawPostProcessQuad(); 

			LightCombinerShader.UnBind(); 

		}

		void LightCombiner::ReloadLightCombiner()
		{
			LightCombinerShader.Reload("Shaders/LightCombiner");

			LightCombinerShader.Bind();

			LightCombinerShader.SetUniform("FirstPassDiffuse", 0);
			LightCombinerShader.SetUniform("FirstPassSpecular", 1);
			LightCombinerShader.SetUniform("IndirectSpecular", 2);
			LightCombinerShader.SetUniform("Normal", 3);
			LightCombinerShader.SetUniform("Albedo", 4);
			LightCombinerShader.SetUniform("Sky", 5);
			LightCombinerShader.SetUniform("Depth", 6);
			LightCombinerShader.SetUniform("WaterDepth", 7);
			LightCombinerShader.SetUniform("WaterNormal", 8);

			LightCombinerShader.UnBind();
		}
		
	}
}