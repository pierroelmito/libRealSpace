//
//  SCMouse.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/25/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#pragma once

#include "Math.h"

class RLEShape;

struct SCMouseButton
{
	enum BUTTON_ID {LEFT, MIDDLE, RIGHT} ;
	enum EventType{NONE, PRESSED, RELEASED} ;
	EventType event;
};

class SCMouse
{
public:
	SCMouse();
	~SCMouse();

	void Init(void);

	inline bool IsVisible(void){ return this->visible ; }
	void SetVisible(bool visible){ this->visible = visible; }

	inline void SetPosition(Point2D position){ this->position = position;}
	inline Point2D GetPosition(void) { return this->position ; }

	void Draw(void);

	enum Mode {CURSOR, VISOR };
	void SetMode(Mode mode){this->mode = mode;}

	void FlushEvents(void);
	SCMouseButton buttons[3];

private:
	RLEShape* appearances[4];
	Mode mode{ CURSOR };

	//The cursor position in VGA 320x200 coordinates.
	Point2D position;

	bool visible{ true };
};
