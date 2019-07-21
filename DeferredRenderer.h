#pragma once

//renders + unwraps the required terrain 

#include <Shader.h>
#include <Framebuffer.h>
#include <Camera.h>
#include <Chunk.h>
#include <Materials.h>


namespace Open7Days {
	namespace Rendering {

		struct DeferredRenderer {
			FrameBufferObject DeferredRawBuffer;
			MultiPassFrameBufferObject DeferredUnwrappedBuffer; 

			Shader DeferredUnwrappedShader, DeferredRaw;

			Materials::MaterialList *Materials; 

			void PrepareDeferredRendering(Window & Window, Camera & Camera); 
			void RenderToDeffered(Window & Window, Camera & Camera, ChunkContainer & Chunks); 

		};

	}
}