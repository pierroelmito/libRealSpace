//
//  main.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/25/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

#include "SCMainMenu.h"
#include "RSFontManager.h"
#include "RSAudio.h"
#include "UserProperties.h"

//Here are all the mean subsystems interacting together
GameEngine Game;
RSScreen Screen;
RSVGA VGA;
RSAudio Audio;
SCMouse Mouse;
AssetManager Assets;
SCRenderer Renderer;
ConvAssetManager ConvAssets;
RSFontManager FontManager;

int main(int argc, char* argv[])
{
	UserProperties::Get().ReadFromFile("assets/properties.def");

	Assets.SetBase("./");
	Game.Init();
	//Add MainMenu activity on the game stack.
	Game.MakeActivity<SCMainMenu>();
	Game.Run();
	Game.Release();

	return EXIT_SUCCESS;
}
