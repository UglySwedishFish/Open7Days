#pragma once

#include <IndirectSpecular.h>

namespace Open7Days {
	namespace Rendering {


		struct LightCombiner {

			Shader LightCombinerShader; 

			void PrepareLightCombiner(); 
			void CombineLighting(Window & Window, Camera & Camera, DeferredRenderer & Deferred, FirstPassLighting & First, IndirectSpecularLighting & IndirectSpecular, WaterRenderer & Water, TextureCubeMap & Sky); 
			void ReloadLightCombiner(); 
		};


	}
}