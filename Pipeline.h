#pragma once

#include <LightCombiner.h>


namespace Open7Days {
	struct Pipeline {
		Rendering::ChunkContainer Chunks; 
		Rendering::DeferredRenderer Deferred; 
		Rendering::ShadowMapper Shadows; 
		Rendering::FirstPassLighting FirstPassLighting; 
		Rendering::Voxelizer Voxelizer; 
		Rendering::IndirectDiffuseLighting IndirectDiffuse; 
		Rendering::IndirectSpecularLighting IndirectSpecular; 
		Rendering::LightCombiner LightCombiner;
		Rendering::CubeMapRenderer CubeMap; 
		Rendering::TextureCubeMap Sky; 
		Rendering::Shader EquirectangularToCubeMapShader; 
		Rendering::WaterRenderer WaterRenderer; 

		void Prepare(Window & Window, Camera & Camera); 
		void Run(Window & Window, Camera & Camera); 
	};
}