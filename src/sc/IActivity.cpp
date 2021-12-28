//
//  IActivity.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/25/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "IActivity.h"

#include "precomp.h"

#include "main.h"

IActivity::IActivity()
{
}

IActivity::~IActivity()
{
}

void IActivity::SetTitle(const char* title){
	Screen.SetTitle(title);
}

void IActivity::Frame2D(std::initializer_list<RLEShape*> shapes)
{
	CheckButtons();
	VGA.Activate();
	VGA.Clear();

	VGA.SetPalette(this->palette);
	for (RLEShape* shape : shapes)
		VGA.DrawShape(*shape);

	DrawButtons();
	Mouse.Draw();
	VGA.VSync();
}

SCButton* IActivity::CheckButtons(void)
{
	for(SCButton* button : buttons) {
		if (!button->IsEnabled())
			continue;

		const Point2D mpos = Mouse.GetPosition();
		if (mpos.x < button->position.x ||
			mpos.x > button->position.x + button->dimension.x ||
			mpos.y < button->position.y ||
			mpos.y > button->position.y + button->dimension.y
			)
		{
			button->SetAppearance(SCButton::APR_UP);
			continue;
		}

		//HIT !
		Mouse.SetMode(SCMouse::VISOR);

		if (Mouse.buttons[MouseButton::LEFT].event == MouseButton::PRESSED)
			button->SetAppearance(SCButton::APR_DOWN);

		//If the mouse button has just been released: trigger action.
		if (Mouse.buttons[MouseButton::LEFT].event == MouseButton::RELEASED)
			button->OnAction();

		return button;
	}

	Mouse.SetMode(SCMouse::CURSOR);
	return NULL;
}

void IActivity::DrawButtons(void)
{
	for(SCButton* button : buttons) {
		if (button->IsEnabled())
			VGA.DrawShape(button->appearance[button->GetAppearance()]);
		else
			VGA.DrawShape(button->appearance[SCButton::Appearance::APR_DOWN]);
	}
}
