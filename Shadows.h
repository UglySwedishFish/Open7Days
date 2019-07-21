#pragma once

#include <Framebuffer.h>
#include <Shader.h>
#include <Camera.h>
#include <Chunk.h>
#include <array>
#include <Texture.h>

//TODO: maybe at some point add settings for this. 
#define SHADOW_MAP_RES 1024

namespace Open7Days {
	namespace Rendering {

		const unsigned char UpdateQueue[] = { 0,0,1,0,1,0,2 }; 
		const float Ranges[] = { 10.0,50.0,150. }; 

		struct ShadowMapper {
			FrameBufferObject Cascades[3], PingPongBuffer; 
			Matrix4f ViewMatrices[3], ProjectionMatrices[3]; 

			Frustum ShadowFrustums[3]; 

			Vector2f Direction = Vector2f(0.0); 
			Vector3f Orientation = Vector3f(0.0); 

			Shader DeferredShadow, ShadowGaussian; 
			
			void Prepare(Camera Camera); 
			void UpdateShadowMapCascades(Window & Window, Camera & Camera, ChunkContainer & Chunks); 
			void BindAndSetUniforms(Shader & Shader, int StartingTexture = 0); 

		};

	}
}

