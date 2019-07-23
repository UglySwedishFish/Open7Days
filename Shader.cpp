#include "Shader.h"
#include "GL/glad.h"
#include <string> 
#include <fstream> 
#include <iostream> 

#define throwerror(s) std::cout << s

#define GLOBAL_SHADER_DEFINITIONS "#define half lowp float\n#define double highp float\n"




unsigned int Open7Days::Rendering::LoadShader(unsigned int ShaderType, const std::string & File, unsigned int & Buffer, unsigned int BaseID, bool ReCreate) {
	std::string source = ""; //all the shader code
	std::ifstream file(File);

	if (!file.is_open()) {
		throwerror(static_cast<std::string>("Failed to open file: ") + File);
		return 0; //-1 hurr durr
	}


	while (!file.eof()) {
		char line[50000];
		file.getline(line, 50000);
		std::string Line = line;

		if (source.size() == 0) {
			source += Line + '\n';
			source += GLOBAL_SHADER_DEFINITIONS;
		}
		else {
			source += Line + '\n';
		}




	}

	std::cout << "BaseID: " << BaseID << '\n';

	if (ReCreate)
		BaseID = glCreateShader(ShaderType); //compile the shader 

	const char* csource = source.c_str();
	if (ReCreate)
		glGenBuffers(1, &Buffer);
	glShaderSource(BaseID, 1, &csource, NULL);
	glCompileShader(BaseID);
	char error[15000];
	glGetShaderInfoLog(BaseID, 15000, NULL, error);


	throwerror(error);
	std::string s = error;

	if (s.length() > 3)
		std::cout << "Shader: " << File << " compiled with either errors or warnings\n";


	return BaseID;
}

Open7Days::Rendering::Shader::Shader(const std::string & vertex, const std::string & fragment) :
	VertexShader(LoadShader(GL_VERTEX_SHADER, vertex, VertexBuffer, VertexShader)),
	FragmentShader(LoadShader(GL_FRAGMENT_SHADER, fragment, FragmentBuffer, FragmentShader)),
	ShaderID(glCreateProgram()) {
	glAttachShader(ShaderID, VertexShader);
	glAttachShader(ShaderID, FragmentShader);
	glLinkProgram(ShaderID);
	glUseProgram(ShaderID);
}
Open7Days::Rendering::Shader::Shader(const std::string & vertex, const std::string & geometry, const std::string & fragment) :
	VertexShader(LoadShader(GL_VERTEX_SHADER, vertex, VertexBuffer, VertexShader)),
	GeometryShader(LoadShader(GL_GEOMETRY_SHADER, geometry, GeometryBuffer, GeometryShader)),
	FragmentShader(LoadShader(GL_FRAGMENT_SHADER, fragment, FragmentBuffer, FragmentShader)),
	ShaderID(glCreateProgram()) {
	glAttachShader(ShaderID, VertexShader);
	glAttachShader(ShaderID, GeometryShader);
	glAttachShader(ShaderID, FragmentShader);
	glLinkProgram(ShaderID);
	glUseProgram(ShaderID);
}

Open7Days::Rendering::Shader::Shader(const std::string & Directory, bool HasGeometryShader) {
	if (HasGeometryShader)
		*this = Shader(Directory + "/vert.glsl", Directory + "/geom.glsl", Directory + "/frag.glsl");
	else
		*this = Shader(Directory + "/vert.glsl", Directory + "/frag.glsl");
}

Open7Days::Rendering::Shader::Shader() :
	VertexShader(NULL),
	FragmentShader(NULL),
	GeometryShader(NULL),
	ShaderID(NULL) {
}

void Open7Days::Rendering::Shader::Bind() {
	glUseProgram(ShaderID);
}

void Open7Days::Rendering::Shader::UnBind() {
	glUseProgram(NULL);
}

void Open7Days::Rendering::Shader::SetUniform(const std::string & Name, int Value) {
	glUniform1i(glGetUniformLocation(ShaderID, Name.c_str()), Value);
}

void Open7Days::Rendering::Shader::SetUniform(const std::string & Name, size_t Value) {
	glUniform1i(glGetUniformLocation(ShaderID, Name.c_str()), Value);
}

void Open7Days::Rendering::Shader::SetUniform(const std::string & Name, float Value) {
	glUniform1f(glGetUniformLocation(ShaderID, Name.c_str()), Value);
}

void Open7Days::Rendering::Shader::SetUniform(const std::string & Name, bool Value) {
	glUniform1i(glGetUniformLocation(ShaderID, Name.c_str()), Value);
}

void Open7Days::Rendering::Shader::SetUniform(const std::string & Name, Vector2f Value) {
	glUniform2f(glGetUniformLocation(ShaderID, Name.c_str()), Value.x, Value.y);
}

void Open7Days::Rendering::Shader::SetUniform(const std::string & Name, Vector3f Value) {
	glUniform3f(glGetUniformLocation(ShaderID, Name.c_str()), Value.x, Value.y, Value.z);
}

void Open7Days::Rendering::Shader::SetUniform(const std::string & Name, Vector4f Value) {
	glUniform4f(glGetUniformLocation(ShaderID, Name.c_str()), Value.x, Value.y, Value.z, Value.w);
}

void Open7Days::Rendering::Shader::SetUniform(const std::string & Name, Vector2i Value) {
	glUniform2i(glGetUniformLocation(ShaderID, Name.c_str()), Value.x, Value.y);
}

void Open7Days::Rendering::Shader::SetUniform(const std::string & Name, Vector3i Value) {
	glUniform3i(glGetUniformLocation(ShaderID, Name.c_str()), Value.x, Value.y, Value.z);
}

void Open7Days::Rendering::Shader::SetUniform(const std::string & Name, Vector4i Value) {
	glUniform4i(glGetUniformLocation(ShaderID, Name.c_str()), Value.x, Value.y, Value.z, Value.w);
}

void Open7Days::Rendering::Shader::SetUniform(const std::string & Name, Matrix4f Value, bool Transpose) {
	glUniformMatrix4fv(glGetUniformLocation(ShaderID, Name.c_str()), 1, Transpose, glm::value_ptr(Value));
}

void Open7Days::Rendering::Shader::SetUniform(const std::string& Name, Matrix3f Value, bool Transpose) {
	glUniformMatrix3fv(glGetUniformLocation(ShaderID, Name.c_str()), 1, Transpose, glm::value_ptr(Value));
}

void Open7Days::Rendering::Shader::Reload(const std::string& vertex, const std::string& fragment) {


	glDetachShader(ShaderID, VertexShader);
	glDetachShader(ShaderID, FragmentShader);

	glDeleteShader(VertexShader);
	glDeleteShader(FragmentShader);

	glDeleteProgram(ShaderID);

	ShaderID = glCreateProgram();


	VertexShader = LoadShader(GL_VERTEX_SHADER, vertex, VertexBuffer, VertexShader, true);
	FragmentShader = LoadShader(GL_FRAGMENT_SHADER, fragment, FragmentBuffer, FragmentShader, true);

	glAttachShader(ShaderID, VertexShader);
	glAttachShader(ShaderID, FragmentShader);
	glLinkProgram(ShaderID);
	glUseProgram(ShaderID);

}

void Open7Days::Rendering::Shader::Reload(const std::string& vertex, const std::string& fragment, const std::string& geometry) {
	glDetachShader(ShaderID, VertexShader);
	glDetachShader(ShaderID, FragmentShader);
	glDetachShader(ShaderID, GeometryShader);

	glDeleteShader(VertexShader);
	glDeleteShader(FragmentShader);
	glDeleteShader(GeometryShader);

	glDeleteProgram(ShaderID);

	ShaderID = glCreateProgram();


	VertexShader = LoadShader(GL_VERTEX_SHADER, vertex, VertexBuffer, VertexShader, true);
	GeometryShader = LoadShader(GL_GEOMETRY_SHADER, geometry, GeometryBuffer, GeometryShader, true); 
	FragmentShader = LoadShader(GL_FRAGMENT_SHADER, fragment, FragmentBuffer, FragmentShader, true);

	glAttachShader(ShaderID, VertexShader);
	glAttachShader(ShaderID, GeometryShader);
	glAttachShader(ShaderID, FragmentShader);
	glLinkProgram(ShaderID);
	glUseProgram(ShaderID);
}

void Open7Days::Rendering::Shader::Reload(const std::string& Directory) {
	Reload(Directory + "/vert.glsl", Directory + "/frag.glsl");
}

void Open7Days::Rendering::Shader::Reload(const std::string& Directory, bool HasGeometryShader) {
	if(HasGeometryShader)
		Reload(Directory + "/vert.glsl", Directory + "/frag.glsl", Directory + "/geom.glsl");
	else 
		Reload(Directory + "/vert.glsl", Directory + "/frag.glsl", Directory + "/geom.glsl");

}

Open7Days::Rendering::Shader::~Shader() {
	//TODO: free up shader
}

/*

Open7Days::Rendering::Shader::Shader(const std::string & vertex, const std::string & fragment) :
	VertexShader(LoadShader(GL_VERTEX_SHADER, vertex, VertexBuffer, VertexShader)),
	FragmentShader(LoadShader(GL_FRAGMENT_SHADER, fragment, FragmentBuffer, FragmentShader)),
	ShaderID(glCreateProgram())
{
	glAttachShader(ShaderID, VertexShader);
	glAttachShader(ShaderID, FragmentShader);
	glLinkProgram(ShaderID);
	glUseProgram(ShaderID);
}*/
