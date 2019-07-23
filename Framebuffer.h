#pragma once
#pragma once
#include "Dependencies/DependenciesRendering.h"
#include "Dependencies/DependenciesMath.h"
#include "Window.h"

namespace Open7Days {
	namespace Rendering {
		struct FrameBufferObject {
			GLuint FrameBuffer, ColorBuffer, DepthBuffer;
			glm::ivec2 Resolution;
			bool GenerateMip;
			FrameBufferObject(glm::ivec2 Resolution, int Format = GL_RGB32F, bool HasDepth = true, bool generatemip = false);
			FrameBufferObject();
			void Bind();
			void BindImage(int Target) { glActiveTexture(GL_TEXTURE0 + Target); glBindTexture(GL_TEXTURE_2D, ColorBuffer); }
			void BindDepthImage(int Target) { glActiveTexture(GL_TEXTURE0 + Target); glBindTexture(GL_TEXTURE_2D, DepthBuffer); }
			void UnBind(Window Window);

		};

		class FrameBufferObjectPreviousData { //also stores the previous data! 
			bool Buffer;
		public:
			FrameBufferObject Buffers[2];


			FrameBufferObjectPreviousData(Vector2i Resolution, int Format = GL_RGB32F, bool HasDepth = true, bool generatemip = false);
			FrameBufferObjectPreviousData() : Buffers{ FrameBufferObject(),FrameBufferObject() }, Buffer(0) {}

			FrameBufferObject& GetCurrent() {
				return Buffers[Buffer];
			}


			void Bind(bool Swap = true);
			void BindImage(int Target);
			void BindDepthImage(int Target);

			void BindImagePrevious(int Target);
			void BindDepthImagePrevious(int Target);

			void UnBind(Window& Window);
			void Swap();

		};
		
		struct MultiPassFrameBufferObject {
			GLuint FrameBuffer, DepthBuffer;
			std::vector<GLuint> ColorBuffers;
			glm::ivec2 Resolution;
			bool GenerateMip;
			MultiPassFrameBufferObject(glm::ivec2 Resolution, int stages, std::vector<int> Formats, bool HasDepth = true, bool generatemip = false);
			MultiPassFrameBufferObject();
			void Bind();
			void BindImage(int Target, int TargetImage) { glActiveTexture(GL_TEXTURE0 + TargetImage); glBindTexture(GL_TEXTURE_2D, ColorBuffers[Target]); }
			void BindDepthImage(int Target) { glActiveTexture(GL_TEXTURE0 + Target); glBindTexture(GL_TEXTURE_2D, DepthBuffer); }
			void UnBind(Window Window);
		};

		struct CubeMultiPassFrameBufferObject {
			unsigned int DepthTexture, DepthBuffer, FrameBuffer;
			std::vector<unsigned int> Texture;
			Vector2i Resolution;
			inline CubeMultiPassFrameBufferObject() :
				DepthTexture(0),
				DepthBuffer(0),
				Texture{},
				FrameBuffer(0),
				Resolution(0){
			
			}

			void Bind();
			void UnBind(Window Window);
			CubeMultiPassFrameBufferObject(Vector2i Resolution, int Targets, std::vector<int> Formats, bool HasDepth);
		};
		struct CubeFrameBufferObject {
			unsigned int DepthTexture, DepthBuffer, FrameBuffer, Texture;
			Vector2i Resolution;
			inline CubeFrameBufferObject() :
				DepthTexture(0),
				DepthBuffer(0),
				Texture{},
				FrameBuffer(0),
				Resolution(0){
			
			}

			void Bind();
			void UnBind(Window Window);
			CubeFrameBufferObject(Vector2i Resolution, int Format, bool HasDepth);
		};
		

		void PreparePostProcess();
		void DrawPostProcessQuad();
		void DrawPostProcessCube();
		void DrawWaterQuad(); 

	}

}

