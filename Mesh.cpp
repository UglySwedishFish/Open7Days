#include "Mesh.h"
#include <iostream>

namespace Open7Days {

	namespace Rendering {

		void Mesh::ClearVectors() {
			Vertices.clear();
			Normals.clear();
			TexCoords.clear();
			Indices.clear(); 
			Materials.clear(); 
			MeshEntries.clear(); 
		}

		Mesh::~Mesh() {
			//ClearVectors(); 
		}

		void LoadMeshData(const char* file, Mesh& Model) {
			Assimp::Importer ObjectImporter;

			const aiScene* Scene = ObjectImporter.ReadFile(file, aiProcess_Triangulate | aiProcess_CalcTangentSpace);
			Mesh TemporaryModel;

			if (Scene) {

				TemporaryModel.MeshEntries.resize(Scene->mNumMeshes);


				unsigned int NumVertices = 0;
				unsigned int NumIndices = 0;

				// Count the number of vertices and indices
				for (unsigned int i = 0; i < TemporaryModel.MeshEntries.size(); i++) {
					TemporaryModel.MeshEntries[i].MaterialIndex = Scene->mMeshes[i]->mMaterialIndex;
					TemporaryModel.MeshEntries[i].NumIndices = Scene->mMeshes[i]->mNumFaces * 3;
					TemporaryModel.MeshEntries[i].BaseVertex = NumVertices;
					TemporaryModel.MeshEntries[i].BaseIndex = NumIndices;

					NumVertices += Scene->mMeshes[i]->mNumVertices;
					NumIndices += TemporaryModel.MeshEntries[i].NumIndices;
				}


				for (unsigned int i = 0; i < TemporaryModel.MeshEntries.size(); i++) {
					const aiMesh* aiMesh = Scene->mMeshes[i];
					InitMesh(aiMesh, TemporaryModel);
				}

				int CurrentGlobalIndicie = 0, CurrentIndicieRefractive = 0, CurrentIndicieNonRefractive = 0;

				std::cout << "Meshentry size: " << TemporaryModel.MeshEntries.size() << '\n';

				for (int MeshEntry = 0; MeshEntry < TemporaryModel.MeshEntries.size(); MeshEntry++) {

					std::cout << MeshEntry << '\n';

					for (int Indicie = 0; Indicie < TemporaryModel.MeshEntries[MeshEntry].NumIndices; Indicie++) {

						Model.Vertices.push_back(TemporaryModel.Vertices[CurrentGlobalIndicie]);
						Model.Normals.push_back(TemporaryModel.Normals[CurrentGlobalIndicie]);
						Model.TexCoords.push_back(Vector3f(TemporaryModel.TexCoords[CurrentGlobalIndicie].x, TemporaryModel.TexCoords[CurrentGlobalIndicie].y, float(TemporaryModel.Materials[CurrentGlobalIndicie]) + 0.1));
						Model.Indices.push_back(CurrentIndicieNonRefractive++);
						CurrentGlobalIndicie++;

					}

				}




			}
			else {
				std::cout << "Failed to load model: " << file << " (Error: " << ObjectImporter.GetErrorString() << ") \n";
			}
		}

		bool InitMesh(const aiMesh* aiMesh, Mesh& mesh) {
			for (unsigned int i = 0; i < aiMesh->mNumVertices; i++) {
				const aiVector3D* VertexPos = &(aiMesh->mVertices[i]);
				const aiVector3D* VertexNormal = &(aiMesh->mNormals[i]);
				const aiVector3D* VertexTextureCoordinate = aiMesh->HasTextureCoords(0) ? &(aiMesh->mTextureCoords[0][i]) : nullptr;

				//if (!aiMesh->HasTangentsAndBitangents())
				//	std::cout << "Warning: model does not have proper tangents!\n"; 



				mesh.Vertices.push_back(glm::vec3(VertexPos->x, VertexPos->y, VertexPos->z));
				mesh.Normals.push_back(glm::vec3(VertexNormal->x, VertexNormal->y, VertexNormal->z));


				if(VertexTextureCoordinate)
				{
					mesh.TexCoords.push_back(Vector3f(VertexTextureCoordinate->x, VertexTextureCoordinate->y, float(aiMesh->mMaterialIndex) + 0.1));
				}
				else
				{
					mesh.TexCoords.push_back(Vector3f(aiVector3D(0).x, aiVector3D(0).y, float(aiMesh->mMaterialIndex) + 0.1f));
				}
				mesh.Materials.push_back(aiMesh->mMaterialIndex);
			}

			for (unsigned int i = 0; i < aiMesh->mNumFaces; i++) {
				const aiFace& Face = aiMesh->mFaces[i];
				if (Face.mNumIndices == 3) { //if it isn't a triangle, skip this face
					mesh.Indices.push_back(Face.mIndices[0]);
					mesh.Indices.push_back(Face.mIndices[1]);
					mesh.Indices.push_back(Face.mIndices[2]);
				}
				else {
					mesh.Indices.push_back(Face.mIndices[0]);
					mesh.Indices.push_back(Face.mIndices[0]);
					mesh.Indices.push_back(Face.mIndices[0]);
				}
			}
			return true;
		}

		void Model::PrepareForRendering() {
			glGenVertexArrays(1, &VAO);
			glBindVertexArray(VAO);
			glGenBuffers(4, VBO);

			glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(ModelData.Vertices[0]) * ModelData.Vertices.size(), &ModelData.Vertices[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

			glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(ModelData.TexCoords[0]) * ModelData.TexCoords.size(), &ModelData.TexCoords[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

			glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(ModelData.Normals[0]) * ModelData.Normals.size(), &ModelData.Normals[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);


			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBO[0]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ModelData.Indices[0]) * ModelData.Indices.size(), &ModelData.Indices[0], GL_STATIC_DRAW);



			glBindVertexArray(0);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}

		void Model::Draw() {
			glBindVertexArray(VAO);

			glDrawElements(GL_TRIANGLES, ModelData.Vertices.size(), GL_UNSIGNED_INT, nullptr);

			glBindVertexArray(0);
		}

	}

}
