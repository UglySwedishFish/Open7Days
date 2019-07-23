#include "Shadows.h"
#include "Core.h"
#include <iostream>

namespace Open7Days {
	namespace Rendering {

		void Move(Vector3f & Pos, float Speed, float RotationX, float RotationY) {
			Pos.x -= (cos(RotationY*(PI / 180.)) * cos(RotationX*(PI / 180.)))*Speed;
			Pos.y += sin(RotationX*(PI / 180.))*Speed;
			Pos.z -= (sin(RotationY*(PI / 180.)) * cos(RotationX*(PI / 180.)))*Speed;
		}

		void ShadowMapper::Prepare(Camera Camera) {

			Orientation = Vector3f(0.);

			Move(Orientation, 1.0, Direction.x, Direction.y - 90.0);

			PingPongBuffer = FrameBufferObject(Vector2i(SHADOW_MAP_RES), GL_R32F, false);

			for (int i = 0; i < 3; i++) {
				Cascades[i] = FrameBufferObject(Vector2i(SHADOW_MAP_RES), GL_R32F , true);
				
				ProjectionMatrices[i] = Core::ShadowOrthoMatrix(Ranges[i], 100.f, 1000.); 

				ViewMatrices[i] = Core::ViewMatrix(Camera.Position + Orientation, Vector3f(Direction.x, Direction.y, 0.));
			}

			DeferredShadow = Shader("Shaders/DeferredShadow"); 
			ShadowGaussian = Shader("Shaders/ShadowGaussian"); 
		}

		int BlurSize[] = { 3,2,1 }; 
		
		void ShadowMapper::UpdateShadowMapCascades(Window & Window, Camera & Camera, ChunkContainer & Chunks) {

			int ToUpdate = UpdateQueue[Window.GetFrameCount() % 7]; 
			
			Orientation = Vector3f(0.);

			Move(Orientation, 1.0, Direction.x, Direction.y - 90.0);

			ViewMatrices[ToUpdate] = Core::ViewMatrix(Camera.Position + Orientation * 500.0f, Vector3f(Direction.x, Direction.y, 0.));

			ShadowFrustums[ToUpdate].Update(ProjectionMatrices[ToUpdate] * ViewMatrices[ToUpdate]); 
			//temporary

			DeferredShadow.Bind(); 

			Cascades[ToUpdate].Bind(); 


			Chunks.RenderChunks(ViewMatrices[ToUpdate], ProjectionMatrices[ToUpdate], DeferredShadow, -1, Vector2f(0.0), &ShadowFrustums[ToUpdate]); 


			Cascades[ToUpdate].UnBind(Window);


				DeferredShadow.UnBind();


				Cascades[ToUpdate].BindDepthImage(0);

				ShadowGaussian.Bind();

				ShadowGaussian.SetUniform("TexelSize", 1.0f / SHADOW_MAP_RES);
				ShadowGaussian.SetUniform("Size", BlurSize[ToUpdate]);

				PingPongBuffer.Bind();

				ShadowGaussian.SetUniform("Vertical", false);

				DrawPostProcessQuad();

				PingPongBuffer.UnBind(Window);

				PingPongBuffer.BindImage(0);

				Cascades[ToUpdate].Bind();

				ShadowGaussian.SetUniform("Vertical", true);

				DrawPostProcessQuad();

				Cascades[ToUpdate].UnBind(Window);

				ShadowGaussian.UnBind();
			

			
		}
		void ShadowMapper::BindAndSetUniforms(Shader & Shader, int StartingTexture) {

			Shader.SetUniform("ShadowDirection", glm::normalize(Orientation)); 
			Shader.SetUniform("SunColor", Vector3f(0.988, 0.831, 0.251));

			for (int i = 0; i < 3; i++) {

				Shader.SetUniform("ShadowCombined[" + std::to_string(i) + "]", ProjectionMatrices[i] * ViewMatrices[i]); 
				Shader.SetUniform("ShadowMaps[" + std::to_string(i) + "]", StartingTexture+i);

				Cascades[i].BindImage(StartingTexture + i); 

			}

		}

	}
}
