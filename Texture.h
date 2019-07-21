#pragma once

#include <Dependencies/DependenciesMath.h>
#include <string>
#include <Shader.h>

const Matrix4f CubeProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 1000.0f);
const Matrix4f CubeViews[] =
{
	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
};


namespace Open7Days {
	namespace Rendering {
		struct TextureGL {
			unsigned int ID;
			Vector2i Resolution;
			TextureGL(unsigned int id = 4294967295, Vector2i Resolution = Vector2i(0));
			void Bind(unsigned int target);
		};

		struct TextureCubeMap {
			unsigned int ID;
			glm::ivec2 Resolution;
			TextureCubeMap(unsigned int id = 4294967295, glm::ivec2 res = glm::ivec2(0));
			void Bind(unsigned int target);
		};

		TextureGL LoadTextureGL(const std::string & Path);
		TextureCubeMap LoadHDRI(const char* name, bool linear, bool mipmaps, Shader& EquirectangularToCubeMapShader);
	}
}