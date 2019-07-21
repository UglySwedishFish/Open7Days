#include "DeferredRenderer.h"

namespace Open7Days {
	namespace Rendering {
		void DeferredRenderer::PrepareDeferredRendering(Window & Window, Camera & Camera)
		{
			DeferredRawBuffer = FrameBufferObject(Window.GetResolution(), GL_RGBA32F); 
			DeferredUnwrappedBuffer = MultiPassFrameBufferObject(Window.GetResolution(), 3, { GL_RGBA8, GL_RGBA16F, GL_RGB32F }, false); 
			
			DeferredRaw = Shader("Shaders/Deferred");
			DeferredUnwrappedShader = Shader("Shaders/DeferredUnwrapper"); 

			DeferredUnwrappedShader.Bind(); 

			DeferredUnwrappedShader.SetUniform("DeferredNormal", 0); 
			DeferredUnwrappedShader.SetUniform("DeferredDepth", 1);
			DeferredUnwrappedShader.SetUniform("Textures", 2);

			DeferredUnwrappedShader.UnBind();

			Materials = new Materials::MaterialList(); 
			Materials->Load(); 


		}
		void DeferredRenderer::RenderToDeffered(Window & Window, Camera & Camera, ChunkContainer& Chunks)
		{

			DeferredRawBuffer.Bind(); 

			DeferredRaw.Bind();

			Chunks.RenderChunks(Camera.View, Camera.Project, DeferredRaw, -1, Vector2f(0.0), &Camera.CameraFrustum); 

			DeferredRaw.UnBind();

			DeferredRawBuffer.UnBind(Window); 
	
			DeferredRawBuffer.BindImage(0); 

			DeferredRawBuffer.BindDepthImage(1);

			glActiveTexture(GL_TEXTURE2); 
			glBindTexture(GL_TEXTURE_2D_ARRAY, Materials->TextureMaterials); 


			DeferredUnwrappedBuffer.Bind(); 

			DeferredUnwrappedShader.Bind();

			DeferredUnwrappedShader.SetUniform("InverseView", glm::inverse(Camera.View)); 
			DeferredUnwrappedShader.SetUniform("InverseProjection", glm::inverse(Camera.Project));

			DrawPostProcessQuad(); 

			DeferredUnwrappedShader.UnBind();

			DeferredUnwrappedBuffer.UnBind(Window);


		}
	}
}