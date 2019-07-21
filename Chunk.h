#pragma once

#include <Core.h>
#include <Framebuffer.h>
#include <stdint.h>
#include <array>
#include <tuple>
#include <vector>
#include <unordered_map>
#include <queue>
#include <Camera.h>
#include <Shader.h>
#include <Materials.h>

#define CHUNKSIZE 16
#define CHUNKHEIGHT 256

//TODO: this should absolutely be a setting 
#define RENDER_DISTANCE 6

namespace Open7Days {

	namespace Rendering {

		inline std::tuple<std::uint8_t, std::uint8_t> ExtractFrom(std::uint16_t Block) {
			return std::make_tuple<std::uint8_t, std::uint8_t>(Block & 0xFF, Block >> 8);
		}

		inline std::uint16_t ExtractTo(std::uint16_t State, std::uint16_t Type) {
			return ((Type << 8) & 0xFF) | (State & 0xFF);
		}



		struct Chunk {
			std::int64_t X, Y;
			std::vector<Vector4f> Vertices, Normals;
			Matrix4f Model; 

			float* BlockWeights;
			unsigned char* BlockMaterials; 


			GLuint VAO, VBOs[3];

			Chunk(std::int64_t X = 0, std::int64_t Y = 0) :
				X(X), Y(Y),BlockWeights(new float[(CHUNKSIZE+1)*(CHUNKHEIGHT+1)*(CHUNKSIZE+1)]), BlockMaterials(new unsigned char[(CHUNKSIZE + 1) * (CHUNKHEIGHT + 1) * (CHUNKSIZE + 1)]), Vertices{}, Normals{}, VAO(0), VBOs{ 0,0,0 } {
				Model = Core::ModelMatrix(Vector3f(-X * CHUNKSIZE, 0.0, -Y * CHUNKSIZE), Vector3f(0.)); 
			}

			void Generate(Materials::MaterialList * Materials); 

			void UpdateMesh(Materials::MaterialList* Materials);
			
			void Draw(Matrix4f View, Matrix4f Project, Shader& Shader);

			//destructor, important 
			~Chunk();


		};

		struct ChunkContainer {

			std::vector<Chunk*> DrawChunks; //for faster iteration
			std::map<std::int64_t, std::map<std::int64_t, Chunk*>> ChunkContainer;
			

			struct ChunkLocation {
				std::int64_t X, Y; 

				ChunkLocation(std::int64_t X, std::int64_t Y): X(X), Y(Y) {
				}

				bool operator<(ChunkLocation& a) {
					//empty for now, will be used for sorting later
				}
			};

			std::list<ChunkLocation> ChunksToBeGenerated; 
			
			void GenerateChunks(Camera& Camera, Materials::MaterialList* Materials);
			void RenderChunks(Matrix4f View, Matrix4f Project, Shader & Shader, int RenderDistanceOverload = -1, Vector2f Position = Vector2f(0.), Frustum * ViewFrustum = nullptr);


		};

	}

}