#pragma once

#include "Shader.h"
#include <assimp/cimport.h>
#include <assimp/mesh.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Open7Days {
	namespace Rendering {

		

		struct Mesh { //note; this is NOT a model. It is only a collection of the data required to construct a model 

			struct MeshEntry {
				unsigned int NumIndices;
				unsigned int BaseVertex;
				unsigned int BaseIndex;
				unsigned int MaterialIndex;
				inline MeshEntry() :
					NumIndices(0),
					BaseVertex(0),
					BaseIndex(0),
					MaterialIndex(0)
				{}
			};

			std::vector<MeshEntry> MeshEntries;
			std::vector<unsigned int> Materials;

			std::vector<Vector3f> Vertices;
			std::vector<Vector3f> Normals;
			std::vector<Vector3f> TexCoords;
			std::vector<unsigned int> Indices;

			std::vector<unsigned int> SingleMeshIndicies;
			void ClearVectors();
			~Mesh();

		};

		void LoadMeshData(const char* file, Mesh& Model);
		bool InitMesh(const aiMesh* aiMesh, Mesh& mesh);


		struct Model {
			Mesh ModelData; 

			unsigned int VAO, VBO[4]; 

			inline Model(const char* Model) : 
				VAO(),
				VBO{},
				ModelData(Mesh()) {
				LoadMeshData(Model, ModelData); 
				PrepareForRendering(); 
			}

			inline Model() :
				VAO(),
				VBO{},
				ModelData(Mesh()) {
			}

			void PrepareForRendering(); 
			void Draw(); 

		};


	}
}
