#include "Voxelizer.h"
#include <iostream>

namespace Open7Days {
	namespace Rendering {
		void Voxelizer::PrepareVoxelizer() {

			for(int i=0;i<6;i++)
			Texture[i] = VoxelTexture(VOXEL_VOLUME_RESOLUTION, 4);

			VoxelizingShader = Shader("Shaders/Voxelizer", true); 


			VoxelizingShader.Bind(); 

			VoxelizingShader.SetUniform("Voxels", 0); 
			VoxelizingShader.SetUniform("PreviousVolume", 1);

			VoxelizingShader.UnBind(); 

		}
		void Voxelizer::VoxelizeScene(Camera& Camera, Window& Window, ShadowMapper &Shadows, ChunkContainer& Chunks) {

			int ToUpdate = UpdateQueue[Window.GetFrameCount() % 7]; 
			int ToUpdateActual = ToUpdate * 2 + ActiveTexture[ToUpdate]; 
			int PreviousActual = ToUpdate * 2 + (!ActiveTexture[ToUpdate]);		

			Texture[ToUpdateActual].Clear(Vector4u(0.));

			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
			glDisable(GL_CULL_FACE);
			glDisable(GL_DEPTH_TEST);
			glDisable(GL_BLEND);
			
			VoxelizingShader.Bind(); 



			glViewport(0, 0, VOXEL_VOLUME_RESOLUTION, VOXEL_VOLUME_RESOLUTION);



			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_3D, Texture[PreviousActual].ID);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_3D, Texture[ToUpdateActual].ID);
			glBindImageTexture(0, Texture[ToUpdateActual].ID, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);

			

			VoxelizingShader.SetUniform("CameraPosition", Camera.Position); 
			VoxelizingShader.SetUniform("CameraPositionPrevious", CameraPositions[ToUpdate]);

			VoxelizingShader.SetUniform("Size", VoxelVolumeSizes[ToUpdate]);
			VoxelizingShader.SetUniform("Resolution", VOXEL_VOLUME_RESOLUTION);

			Shadows.BindAndSetUniforms(VoxelizingShader, 2); 

			Chunks.RenderChunks(Camera.View, Camera.Project, VoxelizingShader, 4, Vector2f(Camera.Position.x, Camera.Position.z)); 


			VoxelizingShader.UnBind();

			glEnable(GL_DEPTH_TEST);
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		//	glEnable(GL_CULL_FACE);

			CameraPositions[ToUpdate] = Camera.Position;

			ActiveTexture[ToUpdate] = !ActiveTexture[ToUpdate]; 


		}
		void Voxelizer::SetUniforms(Shader& Shader, int StartingTexture) {

			for (int i = 0; i < 3; i++) {

				Shader.SetUniform("Size[" + std::to_string(i) + "]", VoxelVolumeSizes[i]);
				Shader.SetUniform("CameraPositionLight[" + std::to_string(i) + "]", CameraPositions[i]);
				Shader.SetUniform("Voxels[" + std::to_string(i) + "]", StartingTexture + i);

				glActiveTexture(GL_TEXTURE0 + StartingTexture + i); 
				glBindTexture(GL_TEXTURE_3D, Texture[i*2+ActiveTexture[i]].ID); 

			}

		}
		void Voxelizer::ReloadVoxelizer() {
			VoxelizingShader.Reload("Shaders/Voxelizer", true);


			VoxelizingShader.Bind();

			VoxelizingShader.SetUniform("Voxels", 0);
			VoxelizingShader.SetUniform("PreviousVolume", 1);

			VoxelizingShader.UnBind();
		}
	}
}