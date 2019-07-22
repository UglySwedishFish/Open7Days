#include "WaterRenderer.h"
#include <iostream>

namespace Open7Days {
	namespace Rendering {
		void WaterRenderer::PrepareWaterRenderer(Window& Window)
		{
			WaterDeferred = Shader("Shaders/WaterDeferred"); 
			WaterBuffer = FrameBufferObject(Window.GetResolution(), GL_RGB16F);
			WaterMesh = Model("Models/WaterPlane.obj"); 

			glGenTextures(1, &WaterNormalTexture); 

			glBindTexture(GL_TEXTURE_2D_ARRAY, WaterNormalTexture);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D_ARRAY,
				GL_TEXTURE_MIN_FILTER,
				GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D_ARRAY,
				GL_TEXTURE_MAG_FILTER,
				GL_LINEAR);
			glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, 1024, 1024, 120, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
			
			for (int i = 0; i < 120; i++) {

				std::string Path = "Textures/Water/"; 

				if (i < 9)
					Path += "000" + std::to_string(i + 1);
				else if (i < 99)
					Path += "00" + std::to_string(i + 1);
				else
					Path += "0" + std::to_string(i + 1); 

				Path += ".png"; 

				std::cout << Path << '\n'; 

				sf::Image RawImage;

				RawImage.loadFromFile(Path); 

				glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
					0,
					0, 0, i,
					1024, 1024, 1,
					GL_RGBA,
					GL_UNSIGNED_BYTE,
					RawImage.getPixelsPtr());

			}

			glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

			glBindTexture(GL_TEXTURE_2D_ARRAY, 0);


		}
		void WaterRenderer::ReloadWaterRenderer()
		{
			WaterDeferred.Reload("Shaders/WaterDeferred"); 
		}
		void WaterRenderer::RenderWater(Camera& Camera, Window& Window)
		{

			WaterBuffer.Bind(); 

			WaterDeferred.Bind(); 

			Matrix4f View = Core::ViewMatrix(Vector3f(0.0, Camera.Position.y, 0.0), Camera.Rotation); 



			WaterDeferred.SetUniform("IdentityMatrix", Camera.Project * View);
			WaterDeferred.SetUniform("Time", Window.GetTimeOpened());
			WaterDeferred.SetUniform("CameraPosition", Camera.Position);

			glActiveTexture(GL_TEXTURE0); 
			glBindTexture(GL_TEXTURE_2D_ARRAY, WaterNormalTexture); 

			WaterMesh.Draw(); 

			WaterDeferred.UnBind(); 

			WaterBuffer.UnBind(Window);

		}
	}
}