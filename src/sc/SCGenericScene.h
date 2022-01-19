//
//  WildCatBase.h
//  libRealSpace
//
//  Created by fabien sanglard on 1/31/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#pragma once

#include "IActivity.h"

enum class Scene {
	WildcatBaseHangar,
	WildcatBaseOffice,
	WildcatBaseChangeroom,
	WildcatBasePinupF,
	WildcatBasePinupM,
};

class RSFont;

class SCGenericScene : public IActivity
{
public:
	SCGenericScene();
	virtual ~SCGenericScene();

	struct Area
	{
		int x0, y0, x1, y1;
	};

	void AddInteraction(Area area, Scene sc);

	virtual void Init(Scene sc);
	virtual void RunFrame(const FrameParams& p) override;

protected:
	std::vector<std::pair<Area, std::function<void(SCGenericScene*)>>> _interactions;
	RSFont* _font;
};
