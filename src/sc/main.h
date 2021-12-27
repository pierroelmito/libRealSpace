//
//  main.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/25/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#pragma once

#include "SCMouse.h"
#include "AssetManager.h"
#include "ConvAssetManager.h"
#include "GameEngine.h"
#include "RSScreen.h"
#include "RSVGA.h"
#include "RSFontManager.h"
#include "SCRenderer.h"

extern GameEngine Game;
extern SCMouse Mouse;
extern RSScreen Screen;
extern RSVGA VGA;
extern AssetManager Assets;
extern SCRenderer Renderer;
extern ConvAssetManager ConvAssets;
extern RSFontManager FontManager;
