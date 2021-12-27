//
//  Button.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/26/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#pragma once

#include <functional>

#include "Math.h"
#include "RLEShape.h"

class SCButton
{
public:
	using ActionFunction = std::function<void()>;

	SCButton();
    ~SCButton();
    
    Point2D position;
    Point2D dimension;
    
	void InitBehavior(Point2D position, Point2D dimension, ActionFunction&& fct); // functor last for readibility

    void OnAction(void);

	enum Appearance { APR_UP, APR_DOWN};

    RLEShape appearance[2];

	inline bool IsEnabled(void){ return this->enabled; }
    inline void SetEnable(bool enabled){ this->enabled = enabled;}
    
    inline void SetAppearance(Appearance app){ this->apre = app;}
    inline Appearance GetAppearance(void){ return this->apre; }

private:
	bool enabled{ true };
	Appearance apre{ APR_UP };
	ActionFunction onClick{};
};

