#include "CubeMapRenderer.h"

namespace Open7Days {

	namespace Rendering {
		void CubeMapRenderer::PrepareCubeMapRenderer()
		{
			CubeDeferred = Shader("Shaders/CubeMapDeferred"); 
			CubeLighting = Shader("Shaders/CubeMapPostProcess");

			RawDeferred = CubeMultiPassFrameBufferObject(Vector2i(CUBEMAPRESOLUTION), 2, { GL_RGB16F, GL_RGBA32F }, true); 
			Lighting = CubeFrameBufferObject(Vector2i(CUBEMAPRESOLUTION), GL_RGBA16F, false);

			ViewFrustum = new Frustum(); 

			CubeLighting.Bind(); 

			CubeLighting.SetUniform("Normal", 0); 
			CubeLighting.SetUniform("WorldPosition", 1);
			CubeLighting.SetUniform("Textures", 2);

			CubeLighting.UnBind();

		}
		void CubeMapRenderer::RenderToCubeMap(Window& Window, Camera& Camera, DeferredRenderer& Deferred, ShadowMapper& Shadows, ChunkContainer& Chunks) {
			
			RawDeferred.Bind(); 

			CubeDeferred.Bind(); 

			for (int i = 0; i < 6; i++) {

				Matrix4f ViewMatrix = glm::translate(CubeViews[i], -Camera.Position); 

				ViewFrustum->Update(CubeProjection * ViewMatrix);

				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, RawDeferred.DepthTexture, 0);
				for (int _i = 0; _i < RawDeferred.Texture.size(); _i++)
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + _i,
						GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, RawDeferred.Texture[_i], 0);

				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				Chunks.RenderChunks(ViewMatrix, CubeProjection, CubeDeferred, -1, Vector2f(0.), ViewFrustum); 
				
			}

			CubeDeferred.UnBind(); 

			RawDeferred.UnBind(Window); 
			
			CubeLighting.Bind(); 

			Lighting.Bind(); 

			for (int i = 0; i < 6; i++) {

				//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
				//	GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, Lighting.DepthTexture, 0);
				
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 ,
						GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, Lighting.Texture, 0);

				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
					

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_CUBE_MAP, RawDeferred.Texture[0]);

				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_CUBE_MAP, RawDeferred.Texture[1]);

				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D_ARRAY, Deferred.Materials->TextureMaterials);

				Shadows.BindAndSetUniforms(CubeLighting, 3);

				CubeLighting.SetUniform("IdentityMatrix", CubeProjection * CubeViews[i]); 

				DrawPostProcessCube(); 
		
			}

			Lighting.UnBind(Window); 

			CubeLighting.UnBind();

		}
	}

}