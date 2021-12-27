//
//  SCMainMenu.h
//  libRealSpace
//
//  Created by fabien sanglard on 1/3/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#pragma once

#include "IActivity.h"

class SCMainMenu : public IActivity
{
public:
    SCMainMenu();
    ~SCMainMenu();

	virtual void Init(void) override;
	virtual void RunFrame(void) override;

private:
	void LoadButtons(void);
    void LoadBoard(void);
    void LoadBackgrounds(void);
    void LoadPalette(void);

	RLEShape board;
    RLEShape sky;
    RLEShape mountain;
    RLEShape cloud;
    
    PakArchive mainMenupak;
    
    void DrawMenu(void);
};
