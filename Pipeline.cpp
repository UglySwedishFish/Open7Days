#include "Pipeline.h"
#include <iostream>

namespace Open7Days {
	void Pipeline::Prepare(Window & Window, Camera & Camera) {

		Rendering::PreparePostProcess(); 

		Shadows.Prepare(Camera);
		Deferred.PrepareDeferredRendering(Window, Camera); 
		FirstPassLighting.Prepare(Window); 
		Voxelizer.PrepareVoxelizer(); 
		IndirectDiffuse.PrepareIndirectDiffuseLighting(Window); 
		IndirectSpecular.PrepareIndirectSpecularLighting(Window); 
		LightCombiner.PrepareLightCombiner(); 
		CubeMap.PrepareCubeMapRenderer(); 
		WaterRenderer.PrepareWaterRenderer(Window); 

		EquirectangularToCubeMapShader = Rendering::Shader("Shaders/EquirectangularToCubeMapShader");

		Sky = Rendering::LoadHDRI("Textures/sky.hdr", true, true, EquirectangularToCubeMapShader);

		glClearColor(0.0,0.0,0.0, 1.0);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); 

		//glEnable(GL_CULL_FACE); 
		//glCullFace(GL_BACK); 

	}
	void Pipeline::Run(Window & Window, Camera & Camera) {

		bool Running = true;
		sf::Event Event;
		sf::Clock GameClock;
		int Frame = 0;
		float T = 0.;
		float Speed = 18.; 

		int FramesPerSecond = 0; 
		float TimeElapsed = 0.0; 

		while (Running) {
			while (Window.GetRawWindow()->pollEvent(Event)) {}
			Window.SetFrameTime(GameClock.getElapsedTime().asSeconds());
			GameClock.restart();
			T += Window.GetFrameTime();
			Frame++;
			Window.SetTimeOpened(T);
			Window.SetFrameCount(Frame); 
			
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
				Shadows.Direction.x += Speed * Window.GetFrameTime();

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
				Shadows.Direction.x -= Speed * Window.GetFrameTime();

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
				Shadows.Direction.y += Speed * Window.GetFrameTime();

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
				Shadows.Direction.y -= Speed * Window.GetFrameTime();

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R) && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl)) {
				FirstPassLighting.ReloadShader();
				Voxelizer.ReloadVoxelizer(); 
				IndirectDiffuse.Reload(Window); 
				WaterRenderer.ReloadWaterRenderer(); 
				IndirectSpecular.ReloadIndirectSpecular(); 
				LightCombiner.ReloadLightCombiner(); 
			}
			Camera.PrevView = Camera.View; 
			Core::HandleInput(Camera, 10.f, 0.15f, Window, sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle), true);
			Camera.View = Core::ViewMatrix(Camera.Position, Camera.Rotation);
			Camera.UpdateFrustum(); 

			FramesPerSecond++; 
			if (TimeElapsed + 1.0 < T) {
				std::cout << FramesPerSecond << '\n'; 
				TimeElapsed = T; 
				FramesPerSecond = 0; 
			}

			Chunks.GenerateChunks(Camera, Deferred.Materials); 

			Shadows.UpdateShadowMapCascades(Window, Camera, Chunks);
			Deferred.RenderToDeffered(Window, Camera, Chunks);
			WaterRenderer.RenderWater(Camera, Window);
			Voxelizer.VoxelizeScene(Camera, Window, Shadows, Chunks);
			CubeMap.RenderToCubeMap(Window, Camera, Deferred, Shadows, Chunks);
			IndirectDiffuse.RenderIndirectDiffuseLighting(Window, Camera, Deferred, Shadows, Voxelizer, Sky);
			FirstPassLighting.RenderFirstLightingPass(Window, Camera, Deferred, Shadows, IndirectDiffuse, CubeMap, WaterRenderer);
			IndirectSpecular.RenderIndirectSpecularLighting(Window, Camera, Deferred, CubeMap, FirstPassLighting, WaterRenderer, Sky);

			glViewport(0, 0, Window.GetResolution().x, Window.GetResolution().y); 

			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT); 
			
			LightCombiner.CombineLighting(Window, Camera, Deferred, FirstPassLighting, IndirectSpecular, WaterRenderer, Sky); 
			
			Window.GetRawWindow()->display(); 

		}


	}
}
