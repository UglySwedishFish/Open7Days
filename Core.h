#pragma once
#include <Dependencies\DependenciesMath.h> 
#include <Window.h> 
constexpr double PI = 3.14159265;
#define Open7Days_DEBUG

namespace Open7Days {
	namespace Core {
		template<typename T>
		inline T mix(T A, T B, float Mix) {
			return A * static_cast<T>(Mix) + B * static_cast<T>(1.0 - Mix);
		}



		struct BoundingBox {
			Vector3f Min, Max;
			inline Vector3f Center() { return (Min + Max) / 2.f; }
			inline Vector3f Size() {
				return Max - Min;
			}
			inline int LongestAxis() {
				Vector3f Length = Size();
				return Length.x > Length.y && Length.x > Length.z ? 0 : Length.y > Length.z ? 1 : 2;
			}
			inline float Area() {
				return (Size().x*Size().y + Size().x*Size().z + Size().y * Size().z) * 2.;
			}
			inline void ResizeToFit(BoundingBox Box) {
				Max = glm::max(Max, Box.Max);
				Min = glm::min(Min, Box.Min);
			}
			inline void ResizeToFit(Vector3f Point) {
				Max = glm::max(Max, Point);
				Min = glm::min(Min, Point);
			}

			BoundingBox(Vector3f Min = Vector3f(100000.), Vector3f Max = Vector3f(-100000.)) : Min(Min), Max(Max) {}

		};
		template<typename T>
		T min3(T a, T b, T c) {
			return std::min(a, std::min(b, c));
		}
		template<typename T>
		T max3(T a, T b, T c) {
			return std::max(a, std::max(b, c));
		}
		template<typename T>
		T min4(T a, T b, T c, T d) {
			return std::min(std::min(a, b), std::min(c, d));
		}
		template<typename T>
		T max4(T a, T b, T c, T d) {
			return std::max(std::max(a, b), std::max(c, d));
		}



		template<typename T>
		T Interpolate(T A, T B, T X);
		template<typename T, typename V>
		void Move_2DXY(V & Pos, T Speed, T Rotation, T FrameTime) {
			Pos.x -= cos(Rotation*T(PI / 180.))*Speed*FrameTime;
			Pos.y -= sin(Rotation*T(PI / 180.))*Speed*FrameTime;

		}
		template<typename T, typename V>
		void Move_2DXZ(V & Pos, T Speed, T Rotation, T FrameTime) {
			Pos.x -= cos(Rotation*T(PI / 180.))*Speed*FrameTime;
			Pos.z -= sin(Rotation*T(PI / 180.))*Speed*FrameTime;
		}
		template<typename T, typename V>
		void Move_3D(V & Pos, T Speed, T RotationX, T RotationY, T FrameTime, bool Forward = false) {

			float HMultiplier = Forward ? -cos(RotationX*(PI / 180.)) : cos(RotationX*(PI / 180.));

			Pos.x -= (cos(RotationY*(PI / 180.)) * HMultiplier)*Speed*FrameTime;
			Pos.y += sin(RotationX*(PI / 180.))*Speed*FrameTime;
			Pos.z -= (sin(RotationY*(PI / 180.)) * HMultiplier)*Speed*FrameTime;
		}
		template<class T>
		bool inRange(T value, T min, T max) { //is a value in a certain range
			return value > min && value < max;
		}

		template<typename T, typename F>
		inline void HandleInput(T & Camera, F MovementSpeed, F MouseSense, Window & Window, bool Rotation, bool Position) {
			if (Position) {

				if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
					Move_2DXZ(Camera.Position, MovementSpeed, static_cast<F>(Camera.Rotation.y - 180.0), static_cast<F>(Window.GetFrameTime()));
					//Window.FrameCount = 0; 
				}
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
					Move_2DXZ(Camera.Position, MovementSpeed, static_cast<F>(Camera.Rotation.y), static_cast<F>(Window.GetFrameTime()));
					//Window.FrameCount = 0;
				}
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
					Move_3D(Camera.Position, MovementSpeed, static_cast<F>(Camera.Rotation.x), static_cast<F>(Camera.Rotation.y - 90.0), static_cast<F>(Window.GetFrameTime()));
					//Window.FrameCount = 0;
				}
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
					Move_3D(Camera.Position, MovementSpeed, static_cast<F>(Camera.Rotation.x + 180.0), static_cast<F>(Camera.Rotation.y + 90.0), static_cast<F>(Window.GetFrameTime()), true);
					//Window.FrameCount = 0;
				}
			}
			if (Rotation) {
				float RelativeMouseX = sf::Mouse::getPosition(*Window.GetRawWindow()).x - Window.GetResolution().x / 2;
				float RelativeMouseY = sf::Mouse::getPosition(*Window.GetRawWindow()).y - Window.GetResolution().y / 2;


				sf::Mouse::setPosition(sf::Vector2i(Window.GetResolution().x / 2, Window.GetResolution().y / 2), *Window.GetRawWindow());

				Vector2f PreviousCameraRotation = Vector2f(Camera.Rotation.x, Camera.Rotation.y);

				Vector3f TempRotation = Camera.Rotation;

				Camera.Rotation.y += RelativeMouseX * MouseSense;
				Camera.Rotation.x += static_cast<float>(RelativeMouseY*MouseSense);

				Camera.Rotation.y = Camera.Rotation.y > 360.0 ? Camera.Rotation.y - 360.0 : Camera.Rotation.y < 0.0 ? Camera.Rotation.y + 360.0 : Camera.Rotation.y;




			}

			//Window.FrameCountOpposite++;

			//if (Window.FrameCount != 0)
			//	Window.FrameCountOpposite = 0;
		}

		Matrix4f ViewMatrix(Vector3f Position, Vector3f Rotation);
		Matrix4f ModelMatrix(Vector3f Position, Vector3f Rotation);
		Matrix4f ShadowOrthoMatrix(float edge, float znear, float zfar);
		template<typename T>
		const T Lerp(T A, T B, T F) {
			return A + F * (B - A);
		}

		enum class TAAQuality : int {
			TAA2X, TAA8X, TAA4X
		};

		Matrix4f JitterMatrix(int Sample, Vector2i Resolution, TAAQuality Quality);



	}
}