//
//  Button.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/26/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "SCButton.h"

#include "precomp.h"

SCButton::SCButton()
{
}

SCButton::~SCButton()
{
}

void SCButton::InitBehavior(Point2D position, Point2D dimension, ActionFunction&& fct){
	this->onClick = fct;
	this->position = position;
	this->dimension = dimension;
}

void SCButton::OnAction(void){
    this->onClick();
}
