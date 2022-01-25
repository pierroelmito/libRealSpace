//
//  Screen.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/27/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#pragma once

#include <cstdint>

class RSScreen
{
public:
	RSScreen();
	~RSScreen();

	void Init(int32_t zoomFactor);
	void Release();

	void SetTitle(const char* title);
	bool StartFrame();
	void EndFrame();
	void Refresh(void);

	int32_t width;
	int32_t height;
	int32_t scale;

private:
};

