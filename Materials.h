#pragma once

#include <map>
#include <string>
#include <array>

//todo: probably another place where u wanna have settings but idk tbh 
#define TEXTURE_RESOLUTION 1024

const std::array<std::string,5> SupportedFormats = { ".png", ".jpg", ".jpeg", ".gif", ".bmp" }; 


namespace Open7Days {
	namespace Materials {

		

		//Specifies what type of tool is primarily used to break the material 
		enum Type {
			Shovel,
			Axe,
			Pickaxe
		};

		enum IconType {
			Block,
			Item,
			Custom
		};

		struct Material {
			Type type; 
			IconType iconType; 
			unsigned short MaxHealth; 
			float Hardness; 
			bool HasMet; 
			unsigned short SubTexture; 
			


		};

		struct MaterialList {
			unsigned int TextureMaterials = 0, TextureIcons = 0; 

			//specifies the sub-texture 
			
			std::map<std::string, Material> Materials; 

			void Load(); 




		};

		




	}
}
