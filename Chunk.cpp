#include "Chunk.h"
#include <noise.h>
#include <iostream>
#include <thread>

double FindNoise1(int n, int seed) {
	n += seed;
	n = (n << 13) ^ n;
	int nn = (n * (n * n * 60493 + 19990303) + 1376312589) & 0x7fffffff;
	return 1.0 - ((double)nn / 1073741824.0);
}
double FindNoise2(double x, double y, int seed) {

	int n = (int)x + (int)y * 57;
	return FindNoise1(n, seed);
}
double interpolate(double a, double b, double x) {
	double ft = x * 3.1415;
	double f = (1.0 - cos(ft)) * 0.5;
	return a * (1.0 - f) + b * f;
}
double noise(double x, double y, int seed) {
	double floorx = (double)((int)x);//This is kinda a cheap way to floor a double integer.
	double floory = (double)((int)y);
	double s, t, u, v;//Integer declaration
	s = FindNoise2(floorx, floory, seed);
	t = FindNoise2(floorx + 1, floory, seed);
	u = FindNoise2(floorx, floory + 1, seed);//Get the surrounding pixels to calculate the transition.
	v = FindNoise2(floorx + 1, floory + 1, seed);
	double int1 = interpolate(s, t, x - floorx);//Interpolate between the values.
	double int2 = interpolate(u, v, x - floorx);//Here we use x-floorx, to get 1st dimension. Don't mind the x-floorx thingie, it's part of the cosine formula.
	return interpolate(int1, int2, y - floory);//Here we use y-floory, to get the 2nd dimension.

}

Vector4f interpolateVerts(Vector3f Vertex1, Vector3f Vertex2, float Weight1, float Weight2) {
	float t = glm::clamp((-Weight1) / (Weight2 - Weight1),1e-9f,1-(1e-9f));
	return Vector4f(Vertex1 + t * (Vertex2 - Vertex1),t);
}

Vector3f CalculateTangent(Vector3f Normal) {

	Vector3f MajorAxis; 

	if (glm::abs(Normal.x) < 0.57735026919f) {
		MajorAxis = Vector3f(1, 0, 0);
	}
	else if (glm::abs(Normal.y) < 0.57735026919f) {
		MajorAxis = Vector3f(0, 1, 0);
	}
	else {
		MajorAxis = Vector3f(0, 0, 1);
	}

	return glm::normalize(glm::cross(Normal, MajorAxis)); 

}


namespace Open7Days {

	namespace Rendering {

		siv::PerlinNoise Noise; 

		double WorldGenNoise(double X, double Y, double Z) {



			double Density = (Y / CHUNKHEIGHT) * 2. - 1.;

			Density *= 64.0; 
			
			X = X / 16;
			Y = Y / 256;
			Z = Z / 16;

			//for (float x = 0.4; x <= 4.0; x+= 0.05) {
			//	float actualx = x * x; 
			Density += Noise.noise(X, Y, Z);
				
			//}

			return Density; 

			//return int(Y * 16) == 128 ? -1.0 : Density;

		}
		
		#define OCTAVES 6
		float fbm(Vector2f st) {
			// Initial values
			float value = 0.0;
			float amplitude = .5;
			float frequency = 0.;
			//
			// Loop of octaves
			for (int i = 0; i < OCTAVES; i++) {
				value += amplitude * Noise.noise(st.x, st.y, 0);
				st *= 2.;
				amplitude *= .5;
			}
			return value;
		}

		float Warped(Vector2f x) {

			return fbm(x + Vector2f(fbm(x), fbm(Vector2f(x.y, x.x))));

		}

		float TerrainHeightGeneration(Vector2f x) {
			float BaseTerrain = Warped(x * 0.25f) * 0.5 + 0.5;

			//SECONDARY. GENERATE VALLEYS 

			float Valleys = pow(1.0 - abs(fbm(Vector2f(x.y * 0.05, x.x * 0.05))), 4.);

			//THIRDLY. GENERATE RIVERS 

			float Rivers = pow(1.0 - abs(Noise.noise(-x.x * 0.025, -x.y * 0.025, 0)), 2.0) * 0.65;

			return glm::clamp((BaseTerrain) - Rivers, -0.25f, 1.5f);
		}


		void Chunk::Generate(Materials::MaterialList* Materials)
		{


			glGenVertexArrays(1, &VAO);

			glBindVertexArray(VAO);
			glGenBuffers(3, VBOs);

			//could prob use memset for this but eh 



			//generate a heightgrid

			std::array<std::array<double, CHUNKSIZE + 1>, CHUNKSIZE + 1> HeightGrid; 
			std::array<std::array<std::array<double,CHUNKSIZE + 1>, CHUNKHEIGHT + 1>, CHUNKSIZE + 1> HeightGridCaves;
			std::array<std::array<unsigned char, CHUNKSIZE + 1>, CHUNKSIZE + 1> BlockTypes;

			for (int x = 0; x < CHUNKSIZE + 1; x++) {
				for (int z = 0; z < CHUNKSIZE + 1; z++) {

					Vector2f ActualPosition = Vector2f(double(x + X * CHUNKSIZE) / 25, double(z + Y * CHUNKSIZE) / 25); 

					HeightGrid[x][z] = TerrainHeightGeneration(ActualPosition) * 100.f + 50.f;

				//	HeightGrid[x][z] = glm::max(HeightGrid[x][z], 75.); 

					for (int y = 0; y < CHUNKHEIGHT + 1; y++) {
						//HeightGridCaves[x][y][z] = (Noise.octaveNoise(double(x + X * CHUNKSIZE + 12321) / 10,double(y)/10., double(z + Y * CHUNKSIZE - 9123712) / 10., 2) * 0.5 + 0.5) * 4.0;
					}
					

					float RawBlockNoise = (Noise.octaveNoise(double(x + X * CHUNKSIZE + 123) / 256, double(z + Y * CHUNKSIZE - 91273) / 256, 0, 12) * 0.5 + 0.5);

				//	std::cout << Materials->Materials["dirt"].SubTexture << '\n'; 


					float Roads = pow(1.0 - abs(Noise.noise(ActualPosition.x * 0.025, ActualPosition.y * 0.025)), 4.0); 

					

					BlockTypes[x][z] = HeightGrid[x][z] > 60.0f ? Materials->Materials["snow"].SubTexture : (HeightGrid[x][z] < 45.0 ? Materials->Materials["sand"].SubTexture : Materials->Materials["dirt"].SubTexture);

					if (Roads > 0.99)
						BlockTypes[x][z] = Materials->Materials["asphalt"].SubTexture; 

					




				}
			}




			for (std::uint8_t x = 0; x < CHUNKSIZE; x++) {

				for (std::uint8_t z = 0; z < CHUNKSIZE; z++) {

					//generate terrain 

				

					for (std::uint16_t y = 0; y < CHUNKHEIGHT; y++) {


						for (int Point = 0; Point < 8; Point++) {

							unsigned short PosX = x + BlockPointPositions[Point].x;
							unsigned short PosZ = z + BlockPointPositions[Point].z;
							unsigned short PosY = y + BlockPointPositions[Point].y;


							double Density = double(PosY) - HeightGrid[PosX][PosZ];

						//	Density += HeightGridCaves[PosX][PosY][PosZ]; 

							//auto& Block = Blocks[x * CHUNKHEIGHT * CHUNKSIZE + y * CHUNKSIZE + z]; 

							//Block.Weights[Point] = Density;
							//Block.Materials[Point] = BlockTypes[PosX][PosZ]; 

						}


						

					}

				}
			}

			for (unsigned short x = 0;x < (CHUNKSIZE + 1); x++) {
				for (unsigned short z = 0; z < (CHUNKSIZE + 1); z++) {
					for (unsigned short y = 0; y < (CHUNKHEIGHT + 1); y++) {

						double Density = double(y) - HeightGrid[x][z];

						//Density += HeightGridCaves[x][y][z];

						BlockWeights[x * (CHUNKHEIGHT+1) * (CHUNKSIZE+1) + y * (CHUNKSIZE+1) + z] = Density; 
						BlockMaterials[x * (CHUNKHEIGHT + 1) * (CHUNKSIZE + 1) + y * (CHUNKSIZE + 1) + z] = BlockTypes[x][z];

					}
				}
			}

		}

		

		float DistanceSquared(Vector3f A, Vector3f B) {
			return glm::dot(A - B, A - B); 
		}

		int ConvertToMaterialData(unsigned char Material1, unsigned char Material2, unsigned char Material3, unsigned char Flag) {
			return Material1 * 262144 + Material2 * 1024 + Material3 * 4 + Flag %4;
		}

		void Chunk::UpdateMesh(Materials::MaterialList* Materials) {


			//IDEA! brute force normal smoothing 

			//layout: 0-p1, 1-p2, 2-p3, 3-normal, 4-tangent [for x amount of triangles]
			auto BlockRelatedVertices = new std::vector<Vector3f>[CHUNKSIZE*CHUNKSIZE*CHUNKHEIGHT]; 
			auto BlockRelatedMaterialData = new std::vector<Vector3u>[CHUNKSIZE * CHUNKSIZE * CHUNKHEIGHT]; 

			std::vector<int> Indicies;
			std::vector<unsigned int> MaterialData; 

			unsigned int Indicie = 0;

			for (int x = 0; x < CHUNKSIZE; x++) {
				for (int y = 0; y < CHUNKHEIGHT; y++) {




					for (int z = 0; z < CHUNKSIZE; z++) {

						BlockRelatedVertices[x * CHUNKHEIGHT * CHUNKSIZE + y * CHUNKSIZE + z] = std::vector<Vector3f>(0);

						//Block CurrentState = Blocks[x * CHUNKHEIGHT * CHUNKSIZE + y * CHUNKSIZE + z];

						int CubeIndex = 0; 

						float Weights[8]; 
						unsigned char Materials[8]; 

						for (int Point = 0; Point < 8; Point++) {

							unsigned short PosX = x + BlockPointPositions[Point].x;
							unsigned short PosZ = z + BlockPointPositions[Point].z;
							unsigned short PosY = y + BlockPointPositions[Point].y;

							Weights[Point] = BlockWeights[PosX * (CHUNKHEIGHT + 1) * (CHUNKSIZE + 1) + PosY * (CHUNKSIZE + 1) + PosZ];
							Materials[Point] = BlockMaterials[PosX * (CHUNKHEIGHT + 1) * (CHUNKSIZE + 1) + PosY * (CHUNKSIZE + 1) + PosZ];
						}

						for (int point = 0; point < 8; point++) {
							CubeIndex += (Weights[point] < 0.0 ? 1 << point : 0);
						}


						for (int triangle = 0; triangle < 15; triangle += 3) {
							if (triangulation[CubeIndex][triangle] != -1) {

								
								Vector3f TriangleVertices[3]; 

								Vector3u CurrentMaterials; 

								for (int vertice = 0; vertice < 3; vertice++) {
									int Edge1 = cornerIndexAFromEdge[triangulation[CubeIndex][triangle + vertice]];
									int Edge2 = cornerIndexBFromEdge[triangulation[CubeIndex][triangle + vertice]];

									Vector4f Raw = interpolateVerts(BlockPointPositions[Edge1], BlockPointPositions[Edge2], Weights[Edge1], Weights[Edge2]);

									TriangleVertices[vertice] = Vector3f(Raw); 

									CurrentMaterials[vertice] = Materials[Raw.w < 0.5 ? Edge1 : Edge2];

								}





								//avoid stupid geometry. I:e lines / points
								if (glm::distance(TriangleVertices[0], TriangleVertices[1]) < 1e-7f || glm::distance(TriangleVertices[0], TriangleVertices[2]) < 1e-7f || glm::distance(TriangleVertices[1], TriangleVertices[2]) < 1e-7f)
									continue; 

								Vector3f Normal = glm::normalize(glm::cross(TriangleVertices[1] - TriangleVertices[0], TriangleVertices[2] - TriangleVertices[0]));

								for (int vertice = 0; vertice < 3; vertice++) {

									
									//compute the texture coordinate 
									
									Vector3f CurrentNormal = Normal; 
									Vector3f CurrentVertex = TriangleVertices[vertice]; 

									Vector2f YCentric = Vector2f(CurrentVertex.z, CurrentVertex.x);
									Vector2f XCentric = Vector2f(CurrentVertex.y, CurrentVertex.z);
									Vector2f ZCentric = Vector2f(CurrentVertex.x, CurrentVertex.y);

									Vector3f Blending = glm::max((glm::abs(Normal) - 0.2f) * 7.0f, 0.f); 
									Blending /= (Blending.x + Blending.y + Blending.z); 

									Vector2f TextureCoordinate = XCentric * Blending.x + YCentric * Blending.y + ZCentric * Blending.z; 

									

									BlockRelatedVertices[x*CHUNKHEIGHT*CHUNKSIZE + y * CHUNKSIZE + z].push_back(Vector3f(x,y,z) + CurrentVertex); 


									//Vertices.push_back(Vector4f(Vector3f(x, y, z) + CurrentVertex, glm::uintBitsToFloat(ConvertToMaterialData(Materials.x, Materials.y, Materials.z, vertice))));



								}
								//...filler 

								BlockRelatedMaterialData[x * CHUNKHEIGHT * CHUNKSIZE + y * CHUNKSIZE + z].push_back(CurrentMaterials);



								BlockRelatedVertices[x*CHUNKHEIGHT*CHUNKSIZE + y * CHUNKSIZE + z].push_back(glm::normalize(Normal));
								BlockRelatedVertices[x*CHUNKHEIGHT*CHUNKSIZE + y * CHUNKSIZE + z].push_back(CalculateTangent(Normal));

							}
						}



					}
				}

			}

			//secondary pass, used for smoothing out normals 

			for (int x = 0; x < CHUNKSIZE; x++) {
				for (int y = 0; y < CHUNKHEIGHT; y++) {
					for (int z = 0; z < CHUNKSIZE; z++) {


						//IDEA: for each vertex, create a weight for the normal thats decided by the distance from that vertex to the center
						//then go through all neighboring blocks, and find an identical vertex
						//now add that vertex' normal to your normal and weight it by that vertex' distance to the center
						

						auto CurrentBlockTriangles = BlockRelatedVertices[x*CHUNKHEIGHT*CHUNKSIZE + y * CHUNKSIZE + z]; 
						auto CurrentBlockMaterials = BlockRelatedMaterialData[x * CHUNKHEIGHT * CHUNKSIZE + y * CHUNKSIZE + z];

						
						

						for (int Triangle = 0; Triangle < CurrentBlockTriangles.size(); Triangle += 5) {

							Vector3f TriangleCenter = (CurrentBlockTriangles[Triangle] + CurrentBlockTriangles[Triangle + 1] + CurrentBlockTriangles[Triangle + 2]) / 3.f; 

							Vector3u MaterialData;

							Vector3f TempVertices[3]; 
							Vector4f TempNormals[3];

							for (int vertice = 0; vertice < 3; vertice++) {

								float Weight = 1.0f / DistanceSquared(CurrentBlockTriangles[Triangle + vertice], TriangleCenter) ;

								float MaterialWeight = 1.0f; 

								Vector3f Normal = CurrentBlockTriangles[Triangle + 3] * Weight;
								Vector3f Tangent = CurrentBlockTriangles[Triangle + 4] * Weight; 

								float Material = float(CurrentBlockMaterials[Triangle/5][vertice]); 
								


								for (int XOffset = -1; XOffset <= 1; XOffset++) {
									for (int YOffset = -1; YOffset <= 1; YOffset++) {
										for (int ZOffset = -1; ZOffset <= 1; ZOffset++) {

											if (x+XOffset < 0 || x+XOffset >= CHUNKSIZE || y+YOffset < 0 || y+YOffset >= CHUNKHEIGHT || z+ZOffset < 0 || z+ZOffset >= CHUNKSIZE)
												continue; 



											//go through this blocks triangles 

											auto TemporaryBlockTriangles = BlockRelatedVertices[(x+XOffset)*CHUNKHEIGHT*CHUNKSIZE + (y+YOffset) * CHUNKSIZE + (z+ZOffset)];
											auto TemporaryBlockMaterials = BlockRelatedMaterialData[(x + XOffset) * CHUNKHEIGHT * CHUNKSIZE + (y + YOffset) * CHUNKSIZE + (z + ZOffset)];

											for (int TriangleTemporary = 0; TriangleTemporary < TemporaryBlockTriangles.size(); TriangleTemporary += 5) {

												if (XOffset == 0 && YOffset == 0 && ZOffset == 0 && TriangleTemporary == Triangle)
													continue; 

												Vector3f TriangleCenterTemporary = (TemporaryBlockTriangles[TriangleTemporary] + TemporaryBlockTriangles[TriangleTemporary + 1] + TemporaryBlockTriangles[TriangleTemporary + 2]) / 3.f;

												for (int TemporaryVertex = 0; TemporaryVertex < 3; TemporaryVertex++) {

													Vector3f Difference = glm::abs(CurrentBlockTriangles[Triangle + vertice] - TemporaryBlockTriangles[TriangleTemporary + TemporaryVertex]); 

													if (Difference.x < .1 && Difference.y < .1 && Difference.z < .1) {
														float TemporaryWeight = 1.0 / DistanceSquared(CurrentBlockTriangles[Triangle + vertice], TriangleCenterTemporary);
														//MaterialWeight += 1.0; 

														Weight += TemporaryWeight;
														
														Normal += TemporaryBlockTriangles[TriangleTemporary + 3] * TemporaryWeight;

														//Material += TemporaryBlockMaterials[TriangleTemporary / 5][TemporaryVertex]; 

														//Tangent += TemporaryBlockTriangles[TriangleTemporary + 4] * TemporaryWeight;
														
														
													}

												}

											}


										}
									}
								}
								
								Normal = Normal / Weight;
								Tangent = Tangent / Weight; 
							//	Material = (abs(Normal.y) > 0.55 ? Material / MaterialWeight : Materials->Materials["stone"].SubTexture);

								Tangent = CalculateTangent(Normal); 

								Matrix3f TBN = Matrix3f(Tangent, glm::cross(Normal, Tangent), Normal); 


								Vector3f CurrentNormal = Normal;
								Vector3f CurrentVertex = CurrentBlockTriangles[Triangle+vertice];

								MaterialData[vertice] = (Material + .01f); 


								TempNormals[vertice]=(Vector4f(CurrentNormal, 0.0));
								TempVertices[vertice] = CurrentBlockTriangles[Triangle + vertice]; 

							}

							for (int vertice = 0; vertice < 3; vertice++) {
								Vertices.push_back(Vector4f(TempVertices[vertice], glm::uintBitsToFloat(ConvertToMaterialData(MaterialData.x, MaterialData.y, MaterialData.z, vertice))));
								Normals.push_back(TempNormals[vertice]); 
								Indicies.push_back(Indicie++);
							}
							

						}

					}
				}
			}

			glBindVertexArray(VAO);

			glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices[0]) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

			glBindBuffer(GL_ARRAY_BUFFER, VBOs[2]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(Normals[0]) * Normals.size(), &Normals[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBOs[0]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indicies[0]) * Indicies.size(), &Indicies[0], GL_STATIC_DRAW);

			glBindVertexArray(0);

			glFinish(); 

			
			Indicies.clear(); 
			MaterialData.clear(); 
			
			for (int x = 0; x < CHUNKSIZE * CHUNKSIZE * CHUNKHEIGHT; x++) {
				BlockRelatedVertices->clear(); 
				BlockRelatedMaterialData->clear(); 
			}

			delete[] BlockRelatedVertices; 
			delete[] BlockRelatedMaterialData;


		}

		void Chunk::Draw(Matrix4f View, Matrix4f Project, Shader & Shader) {

			Shader.SetUniform("IdentityMatrix", Project * View * Model);
			Shader.SetUniform("ModelMatrix", Model);


			glBindVertexArray(VAO);

			glDrawElements(GL_TRIANGLES, Vertices.size(), GL_UNSIGNED_INT, nullptr);

			glBindVertexArray(0);
		}

		Chunk::~Chunk() {

			Vertices.clear();
			Normals.clear();


			delete[] BlockWeights;
			delete[] BlockMaterials; 

			//delete Blocks; 

			glDeleteVertexArrays(1, &VAO); 
			glDeleteBuffers(3, VBOs);

			std::cout << "yeet\n"; 

		}

		void ChunkContainer::GenerateChunks(Camera& Camera, Materials::MaterialList* Materials) {

			//step one, delete chunks outside of the players view

			bool UpdateDrawChunks = false; 

			for (auto& X : ChunkContainer) {
				int64_t x = X.first;
				for (auto& Y : X.second) {

					
					int64_t y = Y.first; 

					//todo: camera should be split into two elements

					double Distance = glm::distance(Vector2f(Camera.Position.x, Camera.Position.z), Vector2f(x * CHUNKSIZE, y * CHUNKSIZE)); 

					if (Distance / CHUNKSIZE > RENDER_DISTANCE) {
						delete ChunkContainer[x][y]; 
						ChunkContainer[x].erase(y); 
						UpdateDrawChunks = true; 


					}



				}

				if (ChunkContainer[x].size() == 0)
					ChunkContainer.erase(x); 
					
			}

			//step two, add ungenerated chunks to the queue 

			for (int x = -RENDER_DISTANCE; x <= RENDER_DISTANCE; x++) {

				for (int y = -RENDER_DISTANCE; y <= RENDER_DISTANCE; y++) {

					int ActualX = std::floor(Camera.Position.x / float(CHUNKSIZE)) + x;
					int ActualY = std::floor(Camera.Position.z / float(CHUNKSIZE)) + y;

					if (ChunkContainer.find(ActualX) != ChunkContainer.end()) {
						if (ChunkContainer[ActualX].find(ActualY) != ChunkContainer[ActualX].end())
							continue; 
					}


					if (glm::distance(Vector2f(Camera.Position.x, Camera.Position.z),Vector2f(ActualX * CHUNKSIZE, ActualY * CHUNKSIZE)) < RENDER_DISTANCE*CHUNKSIZE) {
						ChunksToBeGenerated.push_back(ChunkLocation(ActualX, ActualY));
						ChunkContainer[ActualX][ActualY] = new Chunk(ActualX, ActualY);

					}

				}

			}
			
			//step 3, generate one chunk 

			while (ChunksToBeGenerated.size() != 0) {

				auto Location = ChunksToBeGenerated.front(); 
				ChunksToBeGenerated.pop_front();

				if (ChunkContainer.find(Location.X) != ChunkContainer.end()) {
					if (ChunkContainer[Location.X].find(Location.Y) != ChunkContainer[Location.X].end()) {


						auto Update = [&]() {

							ChunkContainer[Location.X][Location.Y]->Generate(Materials);
							ChunkContainer[Location.X][Location.Y]->UpdateMesh(Materials);

						}; 

						Update(); 

						UpdateDrawChunks = true;
					}
						
				}
				

				

				
			}

			if (UpdateDrawChunks) {
				DrawChunks.clear();
				for (auto& X : ChunkContainer) {
					for (auto& Y : X.second) {
						DrawChunks.push_back(Y.second);
					}
				}
			}




		}

		void ChunkContainer::RenderChunks(Matrix4f View, Matrix4f Project, Shader& Shader, int RenderDistanceOverload, Vector2f Position, Frustum* ViewFrustum) {

			for (auto& Chunk : DrawChunks) {

				bool Draw = ViewFrustum == nullptr; 

				if (!Draw) {

					Vector3f ChunkMin = Vector3f(Chunk->X * CHUNKSIZE, 0.0, Chunk->Y * CHUNKSIZE); 
					Vector3f ChunkMax = Vector3f((Chunk->X + 1) * CHUNKSIZE, CHUNKHEIGHT, (Chunk->Y + 1) * CHUNKSIZE); 

					Draw = ViewFrustum->InFrustum(FrustumAABB(ChunkMin, ChunkMax)); 
				}

				if (Draw) {

					if (RenderDistanceOverload == -1)
						Chunk->Draw(View, Project, Shader);
					else {

						float Distance = glm::distance(Position, Vector2f(Chunk->X * CHUNKSIZE, Chunk->Y * CHUNKSIZE));

						if (Distance < RenderDistanceOverload * CHUNKSIZE)
							Chunk->Draw(View, Project, Shader);



					}

				}
			}


		}

}


}
