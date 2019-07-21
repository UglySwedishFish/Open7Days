#include "Core.h"
#include <iostream> 


template<typename T>
T Open7Days::Core::Interpolate(T A, T B, T X)
{
	T FT = X * static_cast<T>(3.1415);
}







Matrix4f Open7Days::Core::ViewMatrix(Vector3f Position, Vector3f Rotation)
{
	Matrix4f Temp;
	Temp = glm::rotate(Temp, glm::radians(Rotation.x), { 1, 0, 0 });
	Temp = glm::rotate(Temp, glm::radians(Rotation.y), { 0, 1, 0 });
	Temp = glm::rotate(Temp, glm::radians(Rotation.z), { 0, 0, 1 });

	Temp = glm::translate(Temp, Vector3f(-Position.x, -Position.y, -Position.z));

	return Temp;
}

Matrix4f Open7Days::Core::ModelMatrix(Vector3f Position, Vector3f Rotation)
{
	Matrix4f Temp;

	Temp = glm::translate(Temp, Vector3f(-Position.x, -Position.y, -Position.z));
	Temp = glm::rotate(Temp, glm::radians(Rotation.x), { 1, 0, 0 });
	Temp = glm::rotate(Temp, glm::radians(Rotation.y), { 0, 1, 0 });
	Temp = glm::rotate(Temp, glm::radians(Rotation.z), { 0, 0, 1 });

	return Temp;
}

Matrix4f Open7Days::Core::ShadowOrthoMatrix(float edge, float znear, float zfar)
{
	return glm::ortho(-edge, edge, -edge, edge, znear, zfar);
}

const Vector2f TAA8X[8] = {
	Vector2f(-7.0f, 1.0f) / 8.0f,
	Vector2f(-5.0f, -5.0f) / 8.0f,
	Vector2f(-1.0f, -3.0f) / 8.0f,
	Vector2f(3.0f, -7.0f) / 8.0f,
	Vector2f(5.0f, -1.0f) / 8.0f,
	Vector2f(7.0f, 7.0f) / 8.0f,
	Vector2f(1.0f, 3.0f) / 8.0f,
	Vector2f(-3.0f, 5.0f) / 8.0f };

const Vector2f TAA4X[4] = {
	Vector2f(-7.0f, 1.0f) / 8.0f,
	Vector2f(-1.0f, -3.0f) / 8.0f,
	Vector2f(5.0f, -1.0f) / 8.0f,
	Vector2f(1.0f, 3.0f) / 8.0f };


Vector2f TAA2X[] = {
	Vector2f(-0.25f,  -0.25f),
	Vector2f(0.25f, 0.25f)
};

Matrix4f Open7Days::Core::JitterMatrix(int Sample, Vector2i Resolution, TAAQuality Quality)
{
	Vector2f Jitter = TAA4X[Sample % 4];
	Vector2f TexelSize = 1.0f / Vector2f(Resolution);
	return glm::translate(Matrix4f(), glm::vec3(2.0 * Jitter.x * TexelSize.x, 2.0 * Jitter.y * TexelSize.y, 0.0f));
}
