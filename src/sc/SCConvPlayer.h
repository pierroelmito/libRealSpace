//
//  SCConvPlayer.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/31/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#pragma once

#include "IActivity.h"

class RSFont;
class CharFigure;
class CharFace;

class ConvFrame
{
public:
	RSFont* font;
	char* text;
	uint8_t textColor;

	enum ConvMode{ CONV_WIDE, CONV_CLOSEUP, CONV_WINGMAN_CHOICE, CONV_CONTRACT_CHOICE};
	ConvMode mode;

	enum FacePos{ FACE_DEF=0x0, FACE_LEFT=0x82, FACE_RIGHT=0xBE, FACE_CENTER=0xA0};
	FacePos facePosition;

	//If we are in a wide of chose wingman mode
	std::vector<CharFigure*> participants;

	//If we are in close up mode
	CharFace* face;
	int8_t facePaletteID;

	std::vector<RLEShape*>* bgLayers;
	std::vector<uint8_t*> * bgPalettes;

	GTime creationTime; // Used to check when a frame expires.

	inline void SetExpired(bool exp){ this->expired = exp;}
	inline bool IsExpired(void){ return this->expired;}

private:
	bool expired;
};


class SCConvPlayer: public IActivity
{
public:
	SCConvPlayer();
	~SCConvPlayer();

	void Init( ) override;
	void RunFrame(const FrameParams& p) override;
	void SetID(int32_t id);
	virtual void Focus(void) override;

private:
	void ReadNextFrame(const FrameParams& p);
	void SetArchive(const PakEntry* conv);
	void ReadtNextFrame(void);
	void DrawText(void);
	void CheckFrameExpired(const FrameParams& p);

	int32_t conversationID{ 0 };
	ByteStream conv ;
	size_t size; //In bytes
	uint8_t* end; //In bytes
	ConvFrame currentFrame;
	bool initialized{ false };
};
