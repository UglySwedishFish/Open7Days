#include "IndirectDiffuse.h"

namespace Open7Days {
	namespace Rendering {



		void IndirectDiffuseLighting::PrepareIndirectDiffuseLighting(Window &Window)
		{

			//IndirectDiffusePongBuffer = MultiPassFrameBufferObject(Window.GetResolution() / 2, 3, { GL_RGB32F, GL_RGB16F, GL_RGB16F }); //useful for linear interpolation of the final image 

			IndirectDiffuseBuffer = FrameBufferObject(Window.GetResolution() / 4, GL_RGBA32F, false);
			IndirectDiffusePongBuffer = FrameBufferObject(Window.GetResolution() / 4, GL_RGBA32F,false);
			IndirectDiffusePingBuffer = FrameBufferObject(Window.GetResolution() / 4, GL_RGBA32F, false);
			IndirectDiffuseTemporalBuffer = FrameBufferObjectPreviousData(Window.GetResolution() / 4, GL_RGBA32F, false); 

			IndirectDiffuseShader = Shader("Shaders/IndirectDiffuse");
			IndirectDiffuseSpatialShader = Shader("Shaders/IndirectDiffuseSpatial");
			IndirectDiffusePacker = Shader("SHaders/IndirectDiffusePacker"); 

			IndirectDiffuseShader.Bind(); 

			IndirectDiffuseShader.SetUniform("ViewPosition", 0);
			IndirectDiffuseShader.SetUniform("Normal", 1);
			IndirectDiffuseShader.SetUniform("BlueNoise", 2);
			IndirectDiffuseShader.SetUniform("Depth", 3);
			IndirectDiffuseShader.SetUniform("Previous", 4);
			IndirectDiffuseShader.SetUniform("Sky", 5); 
			IndirectDiffuseShader.SetUniform("Resolution", Window.GetResolution() / 4);

			IndirectDiffuseShader.UnBind(); 

			IndirectDiffuseSpatialShader.Bind(); 

			IndirectDiffuseSpatialShader.SetUniform("TexelSize", 1.0f / Vector2f(Window.GetResolution() / 4)); 
			IndirectDiffuseSpatialShader.SetUniform("PackedData", 0);

			IndirectDiffuseSpatialShader.UnBind();

			BlueNoise = LoadTextureGL("Textures/BlueNoise.png");

		}

		void IndirectDiffuseLighting::RenderIndirectDiffuseLighting(Window &Window, Camera &Camera, DeferredRenderer & Deferred, ShadowMapper & Shadows, Voxelizer & Voxels, TextureCubeMap& Sky)
		{
			
			IndirectDiffuseTemporalBuffer.Bind();

			IndirectDiffuseShader.Bind(); 

			IndirectDiffuseShader.SetUniform("InverseView", glm::inverse(Camera.View));
			IndirectDiffuseShader.SetUniform("PreviousCombined", Camera.Project * Camera.PrevView);

			IndirectDiffuseShader.SetUniform("CameraPosition", Camera.Position);
			IndirectDiffuseShader.SetUniform("First", First);
			IndirectDiffuseShader.SetUniform("Frame", Window.GetFrameCount()%10);

			Deferred.DeferredUnwrappedBuffer.BindImage(1, 1);
			Deferred.DeferredUnwrappedBuffer.BindImage(2, 0);
			BlueNoise.Bind(2);
			Deferred.DeferredRawBuffer.BindDepthImage(3);
			IndirectDiffuseTemporalBuffer.BindImagePrevious(4); 
			Sky.Bind(5); 

			Shadows.BindAndSetUniforms(IndirectDiffuseShader, 6);
			Voxels.SetUniforms(IndirectDiffuseShader, 9);

			DrawPostProcessQuad();

			IndirectDiffuseShader.UnBind(); 

			IndirectDiffuseTemporalBuffer.UnBind(Window);
			
			IndirectDiffuseSpatialShader.Bind(); 

			IndirectDiffuseSpatialShader.SetUniform("InverseProject", glm::inverse(Camera.Project));
			IndirectDiffuseSpatialShader.SetUniform("InverseView", glm::inverse(Matrix3f(Camera.View)));



			for (int i = 0; i < 2; i++) {
				IndirectDiffuseSpatialShader.SetUniform("Size", BlurSize[i]);

				if (i == 0)
					IndirectDiffuseTemporalBuffer.BindImage(0); 
				else 
					IndirectDiffusePongBuffer.BindImage(0);

				IndirectDiffusePingBuffer.Bind(); 

				IndirectDiffuseSpatialShader.SetUniform("Vertical", false);

				DrawPostProcessQuad(); 

				IndirectDiffusePingBuffer.UnBind(Window);

				IndirectDiffusePingBuffer.BindImage(0);

				IndirectDiffusePongBuffer.Bind();

				IndirectDiffuseSpatialShader.SetUniform("Vertical", true);

				DrawPostProcessQuad();

				IndirectDiffusePongBuffer.UnBind(Window);


			}

			IndirectDiffuseSpatialShader.UnBind();

			First = false; 


		}

		void IndirectDiffuseLighting::Reload(Window & Window)
		{

			First = true; 

			IndirectDiffuseShader.Reload("Shaders/IndirectDiffuse");
			IndirectDiffuseSpatialShader.Reload("Shaders/IndirectDiffuseSpatial");

			IndirectDiffuseShader.Bind();

			IndirectDiffuseShader.SetUniform("ViewPosition", 0);
			IndirectDiffuseShader.SetUniform("Normal", 1);
			IndirectDiffuseShader.SetUniform("BlueNoise", 2);
			IndirectDiffuseShader.SetUniform("Depth", 3);
			IndirectDiffuseShader.SetUniform("Previous", 4);
			IndirectDiffuseShader.SetUniform("Sky", 5); 
			IndirectDiffuseShader.SetUniform("Resolution", Window.GetResolution() / 4);

			IndirectDiffuseShader.UnBind();

			IndirectDiffuseSpatialShader.Bind();

			IndirectDiffuseSpatialShader.SetUniform("TexelSize", 1.0f / Vector2f(Window.GetResolution() / 4));
			IndirectDiffuseSpatialShader.SetUniform("PackedData", 0);

			IndirectDiffuseSpatialShader.UnBind();

		}

	}
}	