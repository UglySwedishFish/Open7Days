#pragma once

#include "Shadows.h"
#include "DeferredRenderer.h"
#include "Voxelizer.h"

namespace Open7Days {
	namespace Rendering {
		
		int const BlurSize[3] = { 1,3,9 }; 

		struct IndirectDiffuseLighting {
			
			Shader IndirectDiffuseShader, IndirectDiffuseSpatialShader, IndirectDiffusePacker; 

			FrameBufferObject IndirectDiffuseBuffer, IndirectDiffusePongBuffer, IndirectDiffusePingBuffer; //pong! 
			FrameBufferObjectPreviousData IndirectDiffuseTemporalBuffer;

			TextureGL BlueNoise;

			bool First = true;

			void PrepareIndirectDiffuseLighting(Window & Window); 
			void RenderIndirectDiffuseLighting(Window & Window, Camera& Camera,DeferredRenderer &Deferred,ShadowMapper &Shadows, Voxelizer &Voxels, TextureCubeMap& Sky);
			void Reload(Window & Window); 
		};

	}
}
