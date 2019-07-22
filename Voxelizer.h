#pragma once

#include <Shadows.h>

//todo: maybe a setting? :S
#define VOXEL_VOLUME_RESOLUTION 64 

namespace Open7Days {
	namespace Rendering {
		struct VoxelTexture {
			unsigned int ID;
			unsigned int Resolution;
			std::vector<GLfloat> Data;
			inline VoxelTexture() :
				ID(4294967295),
				Resolution(4294967295),
				Data(std::vector<GLfloat>()) {
			}



			inline VoxelTexture(unsigned int resolution, unsigned int maxMipMap = 1) {
				Resolution = resolution;
				Data = std::vector<GLfloat>(4 * Resolution * Resolution * Resolution, 0.0); //the 4* is to make sure every pixel has 4 values (RGBA) 

				glGenTextures(1, &ID);
				glBindTexture(GL_TEXTURE_3D, ID);

				glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
				glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
				glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

				glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				const int levels = maxMipMap;
				glTexStorage3D(GL_TEXTURE_3D, levels, GL_RGBA8, Resolution, Resolution, Resolution);
				glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, Resolution, Resolution, Resolution, 0, GL_RGBA, GL_FLOAT, &Data[0]);
				glBindTexture(GL_TEXTURE_3D, 0);
			}

			inline void Clear(Vector4u ClearColor) {
				unsigned char CCF[] = { ClearColor.r,ClearColor.g,ClearColor.b,ClearColor.a };
				glBindTexture(GL_TEXTURE_3D, ID);
				glClearTexImage(ID, 0, GL_RGBA, GL_FLOAT, 0);
				glBindTexture(GL_TEXTURE_3D, 0);
			}

		};

		const float VoxelVolumeSizes[] = { 32.0,64.0,128.0 }; 

		struct Voxelizer {

			VoxelTexture Texture[6]; 
			int ActiveTexture[3] = { 0,0,0 }; 
			
			Vector3f CameraPositions[3] = { Vector3f(-100000000.0),Vector3f(-100000000.0),Vector3f(-100000000.0) };
			Shader VoxelizingShader; 

			void PrepareVoxelizer(); 
			void VoxelizeScene(Camera& Camera, Window & Window, ShadowMapper &Shadows, ChunkContainer& Chunks); 
			void SetUniforms(Shader &Shader, int StartingTexture = 0);
			void ReloadVoxelizer(); 

		};

	}
}