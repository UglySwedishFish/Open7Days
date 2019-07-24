#pragma once
#include "Core.h"

namespace Open7Days {
	namespace Rendering {
		unsigned int LoadShader(unsigned int ShaderType, const std::string & File, unsigned int &Buffer, unsigned int BaseID, bool ReCreate = true);

		struct Shader {
			unsigned int VertexShader = 0;
			unsigned int FragmentShader = 0;
			unsigned int GeometryShader = 0;

			unsigned int VertexBuffer = 0, FragmentBuffer = 0, GeometryBuffer = 0;
			unsigned int VertexID = 0, FragmentID = 0, GeometryID = 0;

			unsigned int ShaderID = 0;

			Shader(const std::string & vertex, const std::string & fragment);
			Shader(const std::string & vertex, const std::string & geometry, const std::string & fragment);
			Shader(const std::string & Directory, bool HasGeometryShader = false);
			Shader();
			void Bind();
			void UnBind(); 

			void SetUniform(const std::string & Name, int Value);
			void SetUniform(const std::string & Name, size_t Value);
			void SetUniform(const std::string & Name, float Value);
			void SetUniform(const std::string & Name, bool Value);
			void SetUniform(const std::string & Name, Vector2f Value);
			void SetUniform(const std::string & Name, Vector3f Value);
			void SetUniform(const std::string & Name, Vector4f Value);
			void SetUniform(const std::string & Name, Vector2i Value);
			void SetUniform(const std::string & Name, Vector3i Value);
			void SetUniform(const std::string & Name, Vector4i Value);
			void SetUniform(const std::string & Name, Matrix4f Value, bool Transpose = false);
			void SetUniform(const std::string& Name, Matrix3f Value, bool Transpose = false);

			void Reload(const std::string& vertex, const std::string& fragment);
			void Reload(const std::string& vertex, const std::string& fragment, const std::string & geometry);

			void Reload(const std::string& Directory);
			void Reload(const std::string& Directory, bool HasGeometryShader);

			~Shader();
		};

	}
}
