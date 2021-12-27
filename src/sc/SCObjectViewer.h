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
    
    void Init( );
    void RunFrame(void);
    void NextObject(void);

private:

	struct RSShowCase
	{
        float cameraDist;
        RSEntity* entity;
        char displayName[20];
	};
    
    std::vector<RSShowCase> showCases;
    
    void ParseObjList(IffLexer* lexer);
    void ParseAssets(PakArchive* archive);
    
    RLEShape bluePrint;
    RLEShape title;
    RLEShape board;
    
    uint32_t currentObject;

	//For rotating the object
    uint32_t startTime;
};
