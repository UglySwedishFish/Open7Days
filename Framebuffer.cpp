#include "framebuffer.h"
#include <iostream>




GLenum GetCorresponding(int x) {

	if (x == GL_RGBA32UI) {
		return GL_RGBA_INTEGER;
	}
	if (x == GL_RGBA32F || x == GL_RGBA16F || x == GL_RGBA8)
		return GL_RGBA; 
	if (x == GL_RGB32F || x == GL_RGB16F || x == GL_RGB8)
		return GL_RGB;
	if (x == GL_R32F || x == GL_R16F || x == GL_R8)
		return GL_RED;

}

std::string GetFormatText(int X) {

	if (X == GL_RGBA32F)
		return "GL_RGBA32F"; 
	if (X == GL_RGBA16F)
		return "GL_RGBA16F";
	if (X == GL_RGBA8)
		return "GL_RGBA8";
	if (X == GL_RGB32F)
		return "GL_RGB32F";
	if (X == GL_RGB16F)
		return "GL_RGB16F";
	if (X == GL_RGB8)
		return "GL_RGB8";
	if (X == GL_R32F)
		return "GL_R32F";
	if (X == GL_R16F)
		return "GL_R16F";
	if (X == GL_R8)
		return "GL_R8";
	if (X == GL_RGBA32UI)
		return "GL_RGBA32UI";

	if (X == GL_RGBA)
		return "GL_RGBA"; 
	if (X == GL_RGB)
		return "GL_RGB";
	if (X == GL_RED)
		return "GL_RED";
	if (X == GL_RGBA_INTEGER)
		return "GL_RGBA_INTEGER";

}


Open7Days::Rendering::FrameBufferObject::FrameBufferObject(glm::ivec2 Resolution, int Format, bool HasDepth, bool generatemip)
	: GenerateMip(generatemip), FrameBuffer(0), ColorBuffer(0), DepthBuffer(0), Resolution(Resolution)
{
	glGenFramebuffers(1, &FrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);

	glGenTextures(1, &ColorBuffer);
	glBindTexture(GL_TEXTURE_2D, ColorBuffer);

	glTexImage2D(GL_TEXTURE_2D, 0, Format, Resolution.x, Resolution.y, 0, GetCorresponding(Format), Format == GL_RGBA32UI ? GL_UNSIGNED_INT : GL_FLOAT, NULL);

	std::cout << "Frame buffer error: " << glGetError() << '\n'; 

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, generatemip ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ColorBuffer, 0); //attach it to the frame buffer
																								 //and depth buff
	if (HasDepth) {
		glGenTextures(1, &DepthBuffer);
		glBindTexture(GL_TEXTURE_2D, DepthBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, Resolution.x, Resolution.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, generatemip ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, DepthBuffer, 0);
	}

	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Open7Days::Rendering::FrameBufferObject::FrameBufferObject()
	: Resolution(0), FrameBuffer(0), ColorBuffer(0), DepthBuffer(0), GenerateMip(false)
{
}

void Open7Days::Rendering::FrameBufferObject::Bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);
	glViewport(0, 0, Resolution.x, Resolution.y);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Open7Days::Rendering::FrameBufferObject::UnBind(Window Window)
{
	glBindFramebuffer(GL_FRAMEBUFFER, NULL);
	if (GenerateMip) {
		glBindTexture(GL_TEXTURE_2D, ColorBuffer);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, NULL);
	}
	glViewport(0, 0, Window.GetResolution().x, Window.GetResolution().y);
}

Open7Days::Rendering::MultiPassFrameBufferObject::MultiPassFrameBufferObject(glm::ivec2 Resolution, int stages, std::vector<int> Formats, bool HasDepth, bool generatemip) :
	Resolution(Resolution), FrameBuffer(0), DepthBuffer(0), GenerateMip(generatemip), ColorBuffers{}
{
	glGenFramebuffers(1, &FrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);

	//now for the color buffer 
	for (int i = 0; i < stages; i++) {
		ColorBuffers.push_back(0);
		glGenTextures(1, &ColorBuffers[i]);
		glBindTexture(GL_TEXTURE_2D, ColorBuffers[i]);

		glTexImage2D(GL_TEXTURE_2D, 0, Formats[i], Resolution.x, Resolution.y, 0, GetCorresponding(Formats[i]), GL_FLOAT, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glBindTexture(GL_TEXTURE_2D, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, ColorBuffers[i], 0); //attach it to the frame buffer
	}
	//and depth buffer

	if (HasDepth) {
		glGenTextures(1, &DepthBuffer);
		glBindTexture(GL_TEXTURE_2D, DepthBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, Resolution.x, Resolution.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, DepthBuffer, 0);
	}

	GLenum DrawBuffers[30];
	for (int i = 0; i < stages; i++)
		DrawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;

	glDrawBuffers(stages, DrawBuffers);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Open7Days::Rendering::MultiPassFrameBufferObject::MultiPassFrameBufferObject() :
	Resolution(0), FrameBuffer(0), DepthBuffer(0), GenerateMip(false), ColorBuffers{}
{
}

void Open7Days::Rendering::MultiPassFrameBufferObject::Bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);
	glViewport(0, 0, Resolution.x, Resolution.y);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Open7Days::Rendering::MultiPassFrameBufferObject::UnBind(Window Window)
{
	glBindFramebuffer(GL_FRAMEBUFFER, NULL);

	if (GenerateMip) {
		glBindTexture(GL_TEXTURE_2D, DepthBuffer);
		glGenerateMipmap(GL_TEXTURE_2D);


		glBindTexture(GL_TEXTURE_2D, ColorBuffers[1]);
		glGenerateMipmap(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, NULL);
	}
	glViewport(0, 0, Window.GetResolution().x, Window.GetResolution().y);

}

unsigned int PPQuadVBO, PPQuadVAO, PPCubeVBO, PPCubeVAO, WaterVAO, WaterVBO;

void Open7Days::Rendering::PreparePostProcess()
{
	float vertices[] = {
		-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	};

	float WaterVertices[] = {
		-100.0f, 40.0f, 100.0f, 0.0f, 1.0f, 0.0, 1.0,0.0,
		-100.0f, 40.0f, -100.0f, 0.0f, 0.0f, 0.0, 1.0,0.0,
		100.0f, 40.0f, 100.0f, 1.0f, 1.0f, 0.0, 1.0, 0.0,
		100.0f, 40.0f,-100.0f, 1.0f, 0.0f, 0.0, 1.0, 0.0
	};

	float CubeVertices[] = {
		// back face
		-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
		 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
		 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
		 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
		-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
		-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
		// front face
		-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
		 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
		 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
		 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
		-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
		-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
		// left face
		-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
		-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
		-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
		-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
		-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
		-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
		// right face
		 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
		 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
		 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
		 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
		 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
		 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
		// bottom face
		-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
		 1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
		 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
		 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
		-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
		-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
		// top face
		-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
		 1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
		 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
		 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
		-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
		-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
	};

	glGenVertexArrays(1, &PPQuadVAO);
	glGenBuffers(1, &PPQuadVBO);
	glBindVertexArray(PPQuadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, PPQuadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), nullptr);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat),
		reinterpret_cast<void*>(3 * sizeof(GLfloat)));
	glBindVertexArray(0);

	glGenVertexArrays(1, &PPCubeVAO);
	glGenBuffers(1, &PPCubeVBO);
	// fill buffer
	glBindBuffer(GL_ARRAY_BUFFER, PPCubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(CubeVertices), CubeVertices, GL_STATIC_DRAW);
	// link vertex attributes
	glBindVertexArray(PPCubeVAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glGenVertexArrays(1, &WaterVAO);
	glGenBuffers(1, &WaterVBO);

	glBindBuffer(GL_ARRAY_BUFFER, WaterVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(WaterVertices), WaterVertices, GL_STATIC_DRAW);

	glBindVertexArray(WaterVAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);


}

void Open7Days::Rendering::DrawPostProcessQuad()
{
	glBindVertexArray(PPQuadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}
void Open7Days::Rendering::DrawPostProcessCube()
{
	glBindVertexArray(PPCubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

void Open7Days::Rendering::DrawWaterQuad()
{
	glBindVertexArray(WaterVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

Open7Days::Rendering::FrameBufferObjectPreviousData::FrameBufferObjectPreviousData(Vector2i Resolution, int Format, bool HasDepth, bool generatemip) : 
	Buffers{ FrameBufferObject(Resolution,Format,HasDepth,generatemip),FrameBufferObject(Resolution,Format,HasDepth,generatemip) }, Buffer(0)
{

}

void Open7Days::Rendering::FrameBufferObjectPreviousData::Bind(bool Swap)
{
	if (Swap)
		Buffer = !Buffer;

	Buffers[Buffer].Bind();
}

void Open7Days::Rendering::FrameBufferObjectPreviousData::BindImage(int Target)
{
	Buffers[Buffer].BindImage(Target);
}

void Open7Days::Rendering::FrameBufferObjectPreviousData::BindDepthImage(int Target)
{
	Buffers[Buffer].BindDepthImage(Target);
}

void Open7Days::Rendering::FrameBufferObjectPreviousData::BindImagePrevious(int Target)
{
	Buffers[!Buffer].BindImage(Target);
}

void Open7Days::Rendering::FrameBufferObjectPreviousData::BindDepthImagePrevious(int Target)
{
	Buffers[!Buffer].BindDepthImage(Target);
}

void Open7Days::Rendering::FrameBufferObjectPreviousData::UnBind(Window& Window)
{
	Buffers[Buffer].UnBind(Window);
}

void Open7Days::Rendering::FrameBufferObjectPreviousData::Swap()
{
	Buffer = !Buffer;
}


void Open7Days::Rendering::CubeMultiPassFrameBufferObject::Bind()
{
	glViewport(0, 0, Resolution.x, Resolution.y);

	glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);
}

void Open7Days::Rendering::CubeMultiPassFrameBufferObject::UnBind(Window Window)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, Window.GetResolution().x, Window.GetResolution().y);
	glBindTexture(GL_TEXTURE_CUBE_MAP, Texture[1]);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}


Open7Days::Rendering::CubeMultiPassFrameBufferObject::CubeMultiPassFrameBufferObject(Vector2i Resolution, int Targets, std::vector<int> Formats, bool HasDepth)
{
	this->Resolution = Resolution;
	glGenFramebuffers(1, &FrameBuffer);
	// create depth cubemap texture
	

	std::vector<unsigned int> ColorBuffers;

	if (HasDepth) {
		glGenTextures(1, &DepthTexture);
		glBindTexture(GL_TEXTURE_CUBE_MAP, DepthTexture);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		for (unsigned int i = 0; i < 6; ++i)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT32F, Resolution.x, Resolution.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	}

	for (int j = 0; j < Targets; j++) {
		Texture.push_back(0);
		glGenTextures(1, &Texture[j]);
		glBindTexture(GL_TEXTURE_CUBE_MAP, Texture[j]);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, j == 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		for (unsigned int i = 0; i < 6; ++i) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, Formats[j], Resolution.x, Resolution.y, 0, GetCorresponding(Formats[j]), GL_FLOAT, NULL);

			//std::cout << GetFormatText(Formats[j]) << ' ' << GetFormatText(GetCorresponding(Formats[j])) << '\n';

			std::cout << glGetError() << '\n'; 
		}
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + j, GL_TEXTURE_2D, Texture[j], 0); //attach it to the frame buffer

		ColorBuffers.push_back(GL_COLOR_ATTACHMENT0 + j);

	}

	

	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);

	glGenRenderbuffers(1, &DepthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, DepthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, Resolution.x, Resolution.y);
	glFramebufferRenderbuffer(GL_RENDERBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
		DepthBuffer);



	glDrawBuffers(ColorBuffers.size(), &ColorBuffers[0]);
	glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void Open7Days::Rendering::CubeFrameBufferObject::Bind()
{
	glViewport(0, 0, Resolution.x, Resolution.y);
	glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);
}

void Open7Days::Rendering::CubeFrameBufferObject::UnBind(Window Window)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, Window.GetResolution().x, Window.GetResolution().y);
	glBindTexture(GL_TEXTURE_CUBE_MAP, Texture);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

Open7Days::Rendering::CubeFrameBufferObject::CubeFrameBufferObject(Vector2i Resolution, int Format, bool HasDepth)
{
	this->Resolution = Resolution;
	glGenFramebuffers(1, &FrameBuffer);
	// create depth cubemap texture

	if (HasDepth) {
		glGenTextures(1, &DepthTexture);
		glBindTexture(GL_TEXTURE_CUBE_MAP, DepthTexture);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		for (unsigned int i = 0; i < 6; ++i)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT32F, Resolution.x, Resolution.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	}

	glGenTextures(1, &Texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, Texture);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	for (unsigned int i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, Format, Resolution.x, Resolution.y, 0, GetCorresponding(Format), GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Texture, 0); //attach it to the frame buffer
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	

	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);

	glGenRenderbuffers(1, &DepthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, DepthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, Resolution.x, Resolution.y);
	glFramebufferRenderbuffer(GL_RENDERBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
		DepthBuffer);



	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

