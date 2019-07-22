#include "Materials.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include "Dependencies/DependenciesMath.h"
#include "Dependencies/DependenciesRendering.h"

//if youre using an actually good IDE/compiler/are from the future, feel free to change this line to using std::filesystem
using namespace std::filesystem;


namespace Open7Days {
	namespace Materials {

		bool AtStart(std::string Line, std::string Key) {

			if (Key.size() > Line.size())
				return false; 

			for (int i = 0; i < Key.size(); i++)
				if (Key[i] != Line[i])
					return false; 
			return true; 
		}

		std::string Extract(std::string Line, int start, int end) {

			if (start < 0 || end > Line.length() || start > end)
				return ""; 

			std::string Result = ""; 

			for (int Character = start; Character < end; Character++) {
				Result += Line[Character]; 
			}
			return Result; 
		}
		

		Vector2i TransformToResolution(Vector2i CurrentPixel, Vector2i TransformToResolution) {

			Vector2f Normalized = Vector2f(CurrentPixel) / Vector2f(TEXTURE_RESOLUTION);

			Vector2i UnNormalized = Vector2i(Normalized * Vector2f(TransformToResolution)); 

			UnNormalized = glm::clamp(UnNormalized, Vector2i(0), TransformToResolution); 

			return UnNormalized; 


		}



		sf::Image* TryLoad(std::string Path) {

			sf::Image* TestImage = new sf::Image(); 

			for (auto& Format : SupportedFormats) {
				if (TestImage->loadFromFile(Path + Format))
					return TestImage; 
			}

			delete TestImage; 

			return nullptr; 

		}

		void MaterialList::Load()
		{

			std::vector<std::string> TexturePaths; 

			int SubID = 0; 
			int TextureAmount = 0; 

			

			for (const auto& Materials : directory_iterator("Textures/Materials")) {
				
				std::string FilePath = Materials.path().generic_string();

				//we only care about folders, not files. We also dont care about the "empty" directory 
				if (FilePath.find('.') != std::string::npos || FilePath.find("Empty") != std::string::npos)
					continue; 



				if (!exists(FilePath + "/Material.mat"))
					continue; //no Material.mat file exists, so we cant properly load the things that are required

				std::ifstream MaterialDataFile(FilePath + "/Material.mat"); 

				std::string Line = ""; 

				Material TemporaryMaterial; 
				std::string Name = ""; 

				while (std::getline(MaterialDataFile, Line)) {

					//TODO: make int conversions more safe. right now, if for instance health is set to be "1000 " the program will crash
					if (AtStart(Line, "name")) {
						Name = Extract(Line, 5, Line.size());
					}
					else if (AtStart(Line, "health")) {
						TemporaryMaterial.MaxHealth = std::stoi(Extract(Line, 7, Line.size()));
					}
					else if (AtStart(Line, "hardness")) {
						TemporaryMaterial.Hardness = std::stof(Extract(Line, 9, Line.size())); 
					}
					else if (AtStart(Line, "hasmet")) {
						TemporaryMaterial.HasMet = Extract(Line, 7, Line.size()) == "true"; 
					}
					else if (AtStart(Line, "type")) {

						std::string Syntax = Extract(Line, 5, Line.size()); 

						if (Syntax == "shovel")
							TemporaryMaterial.type = Shovel;
						else if (Syntax == "axe")
							TemporaryMaterial.type = Axe;
						else
							TemporaryMaterial.type = Pickaxe; 
					}
					else if (AtStart(Line, "icontype")) {
						std::string Syntax = Extract(Line, 9, Line.size());

						if (Syntax == "custom")
							TemporaryMaterial.iconType = Custom;
						else if (Syntax == "item")
							TemporaryMaterial.iconType = Item;
						else
							TemporaryMaterial.iconType = Block;
					}
					

				}



				//check if theres already a baked texture 
				if (!exists(FilePath + "/Combined.png")) {
					//TODO: bake together textures :) 

					sf::Image* ImageColor = TryLoad(FilePath + "/Color"), * ImageAO = TryLoad(FilePath + "/AO"), * ImageRoughness = TryLoad(FilePath + "/Roughness"), * ImageMetalness = TryLoad(FilePath + "/Metalness");

					//start with the required ones 
					
					if (ImageColor == nullptr || ImageRoughness == nullptr || (TemporaryMaterial.HasMet && ImageMetalness == nullptr))
						continue; 

					sf::Image *SaveImage = new sf::Image(); 
					SaveImage->create(TEXTURE_RESOLUTION, TEXTURE_RESOLUTION); 

					for (int x = 0; x < TEXTURE_RESOLUTION; x++) {
						for (int y = 0; y < TEXTURE_RESOLUTION; y++) {

							//for heavens sake, lets stick to floats for the colors aight mate 
							
							

							Vector2i CurrentPixel = Vector2i(x, y); 
							
							Vector2i TransformedColor = TransformToResolution(CurrentPixel, Vector2i(ImageColor->getSize().x, ImageColor->getSize().y)); 
							Vector2i TransformedRoughness = TransformToResolution(CurrentPixel, Vector2i(ImageRoughness->getSize().x, ImageRoughness->getSize().y)); 

							


							auto PixelColor = ImageColor->getPixel(TransformedColor.x, TransformedColor.y); 
							auto RoughessColor = ImageRoughness->getPixel(TransformedRoughness.x, TransformedRoughness.y); 
							
							auto AmbientOcclusionColor = sf::Color(255, 255, 255, 255); 
							auto MetalnessColor = sf::Color(255, 255, 255, 255);

							Vector4u CurrentColor = Vector4u(PixelColor.r, PixelColor.g, PixelColor.b, RoughessColor.r);

							if (ImageAO != nullptr) {

								Vector2i TransFormedAO = TransformToResolution(CurrentPixel, Vector2i(ImageAO->getSize().x, ImageAO->getSize().y));
								AmbientOcclusionColor = ImageAO->getPixel(TransFormedAO.x, TransFormedAO.y); 

								CurrentColor.x = (CurrentColor.x * AmbientOcclusionColor.r) / 255; 
								CurrentColor.y = (CurrentColor.y * AmbientOcclusionColor.r) / 255;
								CurrentColor.z = (CurrentColor.z * AmbientOcclusionColor.r) / 255;
							}

							if (TemporaryMaterial.HasMet) {

								Vector2i TransformedMet = TransformToResolution(CurrentPixel, Vector2i(ImageMetalness->getSize().x, ImageMetalness->getSize().y));
								MetalnessColor = ImageMetalness->getPixel(TransformedMet.x, TransformedMet.y);

								CurrentColor.a = (RoughessColor.r / 16) * 16 + MetalnessColor.r / 16; 


							}
							
							SaveImage->setPixel(x, y, sf::Color(CurrentColor.x, CurrentColor.y, CurrentColor.z, CurrentColor.w)); 





						}
					}

					SaveImage->saveToFile(FilePath + "/Combined.png"); 


				}
				
				//load the combined .png texture 

				TemporaryMaterial.SubTexture = TexturePaths.size(); 

				this->Materials[Name] = TemporaryMaterial;

				std::cout << Name << " subtexture: " << TemporaryMaterial.SubTexture << '\n'; 

				TexturePaths.push_back(FilePath + "/Combined.png"); 
				




			}

			

			unsigned int Id; 
			glGenTextures(1, &Id);

			std::cout << Id << std::endl;

			glBindTexture(GL_TEXTURE_2D_ARRAY, Id);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D_ARRAY,
				GL_TEXTURE_MIN_FILTER,
				GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D_ARRAY,
				GL_TEXTURE_MAG_FILTER,
				GL_LINEAR);
			glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, TEXTURE_RESOLUTION, TEXTURE_RESOLUTION, TexturePaths.size(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

			for (int SubImage = 0; SubImage < TexturePaths.size(); SubImage++) {
				sf::Image RawImage;

				if (!RawImage.loadFromFile(TexturePaths[SubImage])) {
					std::cout << "BAD!\n"; 
					//todo: add proper error handling 
				}

				glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
					0,
					0, 0, SubImage,
					TEXTURE_RESOLUTION, TEXTURE_RESOLUTION, 1,
					GL_RGBA,
					GL_UNSIGNED_BYTE,
					RawImage.getPixelsPtr());

			
			}

			glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

			glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

			TextureMaterials = Id; 


		}

	}
}
