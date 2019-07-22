#include "Texture.h"
#include "Dependencies/DependenciesRendering.h"
#include "Framebuffer.h"

namespace Open7Days {
	namespace Rendering {

		TextureGL::TextureGL(unsigned int id, Vector2i Resolution) :
			ID(id), Resolution(Resolution) {

		}
		void TextureGL::Bind(unsigned int target) {
			glActiveTexture(GL_TEXTURE0 + target);
			glBindTexture(GL_TEXTURE_2D, ID);
		}

		TextureGL LoadTextureGL(const std::string & Path) {
			unsigned int id;
			sf::Image Image;

			if (Image.loadFromFile(Path)) {

				Image.flipVertically();
				glGenTextures(1, &id);
				glBindTexture(GL_TEXTURE_2D, id);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Image.getSize().x, Image.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, Image.getPixelsPtr());
				glBindTexture(GL_TEXTURE_2D, 0);
				return TextureGL(id, glm::ivec2(Image.getSize().x, Image.getSize().y));
			}
			else {
				return TextureGL();
			}
		}

		TextureCubeMap LoadHDRI(const char* name, bool linear, bool mipmaps, Shader& EquirectangularToCubeMapShader) {


			TextureGL RawHDRIMap = LoadTextureGL(name);
			//get a somewhat accurate resolution to use for cubemap: 
			unsigned int PixelCount = RawHDRIMap.Resolution.x * RawHDRIMap.Resolution.y;

			unsigned int Resolution = sqrt(static_cast<float>(PixelCount) / 6.); //obtain a resolution
			unsigned int ResolutionPower2 = pow(2, round(log2(Resolution) + .25)); //force it to be a power of 2

			unsigned int TemporaryFrameBuffer, TemporaryRenderBuffer, FinalImage;

			glGenFramebuffers(1, &TemporaryFrameBuffer);
			glGenRenderbuffers(1, &TemporaryRenderBuffer);

			glBindFramebuffer(GL_FRAMEBUFFER, TemporaryFrameBuffer);
			glBindRenderbuffer(GL_RENDERBUFFER, TemporaryRenderBuffer);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, ResolutionPower2, ResolutionPower2);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, TemporaryRenderBuffer);

			glGenTextures(1, &FinalImage);
			glBindTexture(GL_TEXTURE_CUBE_MAP, FinalImage);
			for (unsigned int i = 0; i < 6; ++i)
			{
				// note that we store each face with 16 bit floating point values
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F,
					ResolutionPower2, ResolutionPower2, 0, GL_RGB, GL_FLOAT, nullptr);
			}
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, linear ? GL_LINEAR : GL_NEAREST);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, linear ? GL_LINEAR : GL_NEAREST);

			glViewport(0, 0, ResolutionPower2, ResolutionPower2); // don't forget to configure the viewport to the capture dimensions.
			glBindFramebuffer(GL_FRAMEBUFFER, TemporaryFrameBuffer);

			EquirectangularToCubeMapShader.Bind();

			glUniformMatrix4fv(glGetUniformLocation(EquirectangularToCubeMapShader.ShaderID, "ProjectionMatrix"), 1, false, glm::value_ptr(CubeProjection));
			glUniform1i(glGetUniformLocation(EquirectangularToCubeMapShader.ShaderID, "EquirectangularMap"), 0);
			RawHDRIMap.Bind(0);

			for (unsigned int i = 0; i < 6; ++i)
			{
				glUniformMatrix4fv(glGetUniformLocation(EquirectangularToCubeMapShader.ShaderID, "ViewMatrix"), 1, false, glm::value_ptr(CubeViews[i]));
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, FinalImage, 0);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				DrawPostProcessCube(); // renders a 1x1 cube
			}

			EquirectangularToCubeMapShader.UnBind();

			return TextureCubeMap(FinalImage, Vector2i(ResolutionPower2));
		}


		TextureCubeMap::TextureCubeMap(unsigned int id, glm::ivec2 res) : 
			ID(id), Resolution(res) {
		}

		void TextureCubeMap::Bind(unsigned int target) {
			glActiveTexture(GL_TEXTURE0 + target);
			glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
		}

	}
}
