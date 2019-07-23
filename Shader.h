#pragma once
#include "Core.h"

namespace Open7Days {
	namespace Rendering {
		unsigned int LoadShader(unsigned int ShaderType, const std::string & File, unsigned int &Buffer, unsigned int BaseID, bool ReCreate = true);

		struct Shader {
			unsigned int VertexShader = 10000;
			unsigned int FragmentShader = 10000;
			unsigned int GeometryShader = 10000;

			unsigned int VertexBuffer = 10000, FragmentBuffer = 10000, GeometryBuffer = 10000;
			unsigned int VertexID = 10000, FragmentID = 10000, GeometryID = 10000;

			unsigned int ShaderID = 10000;

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
