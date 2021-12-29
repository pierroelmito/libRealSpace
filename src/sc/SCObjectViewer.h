//
//  NSObjectViewer.h
//  libRealSpace
//
//  Created by fabien sanglard on 1/3/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#pragma once

#include "IActivity.h"
#include "RSEntity.h"

class SCObjectViewer : public IActivity
{
public:
	SCObjectViewer();
	~SCObjectViewer();

	void Init( ) override;
	void RunFrame(const FrameParams& p) override;
	void NextObject();

protected:
	void ParseObjList(IffLexer* lexer);
	void ParseAssets(PakArchive* archive);

	struct RSShowCase
	{
		float cameraDist;
		RSEntity* entity;
		char displayName[20];
	};
	std::vector<RSShowCase> showCases;
	uint32_t currentObject;

	RLEShape bluePrint;
	RLEShape title;
	RLEShape board;
};
