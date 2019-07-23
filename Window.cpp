#include "Window.h"
#include <iostream>
#include <string>

void APIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void*)
{
	std::string msg(message, length);
	std::ostream &ostr = (type == GL_DEBUG_TYPE_ERROR ? std::cerr : std::cout);
	ostr << "gl";
	switch(severity)
	{
		case GL_DEBUG_SEVERITY_HIGH:
			ostr << "ERROR";
			break;
		case GL_DEBUG_SEVERITY_MEDIUM:
			ostr << "Warning";
			break;
		case GL_DEBUG_SEVERITY_LOW:
			ostr << "Alert";
			break;
		case GL_DEBUG_SEVERITY_NOTIFICATION:
			ostr << "Notify";
			break;
		default:
			ostr << "Unknown";
			break;
	}
	ostr << " (" << id << "): ";
	switch(source)
	{
		case GL_DEBUG_SOURCE_API:
			ostr << "API";
			break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
			ostr << "Window system";
			break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER:
			ostr << "Shader compiler";
			break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:
			ostr << "Third party";
			break;
		case GL_DEBUG_SOURCE_APPLICATION:
			ostr << "Application";
			break;
		case GL_DEBUG_SOURCE_OTHER:
			ostr << "Unknown";
			break;
		default:
			ostr << "???";
			break;
	}
	ostr << " reported ";
	switch(type)
	{
		case GL_DEBUG_TYPE_ERROR:
			ostr << "error";
			break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
			ostr << "deprecated behaviour";
			break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
			ostr << "undefined behaviour";
			break;
		case GL_DEBUG_TYPE_PORTABILITY:
			ostr << "portability issue";
			break;
		case GL_DEBUG_TYPE_PERFORMANCE:
			ostr << "performance issue";
			break;
		case GL_DEBUG_TYPE_MARKER:
			ostr << "annotation";
			break;
		case GL_DEBUG_TYPE_PUSH_GROUP:
			ostr << "debug group push";
			break;
		case GL_DEBUG_TYPE_POP_GROUP:
			ostr << "debug group pop";
			break;
		case GL_DEBUG_TYPE_OTHER:
		default:
			ostr << "unknown";
			break;
	}
	ostr << ": " << msg << std::endl;
}

void Window::SetResolution(Vector2i Resolution) {
	this->Resolution = Resolution;
}

Vector2i Window::GetResolution() {
	return Resolution;
}

void Window::SetFullscreen(bool Fullscreen) {
	this->Fullscreen = Fullscreen;
}

bool Window::GetFullscreen() {
	return Fullscreen;
}

void Window::SetTitle(const char * Title) {
	this->Title = Title;
}

const char * Window::GetTitle() {
	return Title;
}

float Window::GetFrameTime() {
	return FrameTime;
}

void Window::SetFrameTime(float _FrameTime) {
	FrameTime = _FrameTime;
}

void Window::SetTimeOpened(float _TimeOpened) {
	TimeOpened = _TimeOpened;
}

float Window::GetTimeOpened() {
	return TimeOpened;
}

void Window::SetFrameCount(int _FrameCount) {
	FrameCount = _FrameCount;
}

int Window::GetFrameCount() {
	return FrameCount;
}

void Window::CursorVisible(bool Visible) {
	RawWindow->setMouseCursorVisible(Visible);
}

sf::RenderWindow * Window::GetRawWindow() {
	return RawWindow;
}

Window::Window(Vector2i Resolution, bool Fullscreen) {

	this->Resolution = Resolution;
	this->Fullscreen = Fullscreen;
	this->FrameCount = 0;

	sf::ContextSettings settings;
	settings.depthBits = 8;
	settings.stencilBits = 8;
	settings.antialiasingLevel = 0;
	settings.majorVersion = 4;
	settings.minorVersion = 3;
#ifndef NDEBUG
	settings.attributeFlags |= sf::ContextSettings::Debug;
#endif//NDEBUG

	RawWindow = new sf::RenderWindow(sf::VideoMode(Resolution.x, Resolution.y), "Open7Days", Fullscreen ? sf::Style::Fullscreen : sf::Style::Resize, settings);

	//LOAD ICON 

	sf::Image Icon; 
	Icon.loadFromFile("Textures/Icon.png"); 

	RawWindow->setIcon(256, 256, Icon.getPixelsPtr()); 

	gladLoadGL(); //prepare glad (needs to be AFTER window creation) 
#ifndef NDEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif//NDEBUG
	glDebugMessageCallback(MessageCallback, NULL);
}

Window::~Window() {
	//RawWindow->close(); 
	//delete RawWindow; 
}
