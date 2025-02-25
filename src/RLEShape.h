//
//  rle.h
//  pak
//
//  Created by Fabien Sanglard on 12/23/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#pragma once

#include <cstdint>

#include "Math.h"
#include "ByteStream.h"
#include "PakArchive.h"

class RLEShape
{
public:
	RLEShape();
	~RLEShape();

	bool Init(const ByteSlice& bytes);
	bool Init(uint8_t* data, size_t size);
	void InitWithPositionOld(uint8_t* data, size_t size, const Point2D& position );
	void InitWithPosition(const ByteSlice& bytes, const Point2D& position );
	bool Expand(uint8_t* dst, size_t* byteRead);

	inline void SetPosition(const Point2D& position) {
		this->position = position;
	}

	inline void SetPositionX(int32_t x) {
		this->position.x = x;
	}

	static RLEShape* GetStaticEmptyShape();
	static RLEShape* GetNewEmptyShape();

	int32_t GetWidth(void){ return leftDist + this->rightDist;}
	int32_t GetHeight(void){ return topDist+botDist;}
	int32_t GetTop(void){ return topDist;}
	int32_t GetBottom(void){ return botDist;}

	void SetColorOffset(uint8_t offset){ this->colorOffset = offset;}

private:
	ByteStream stream;
	size_t size;
	Point2D position{ 0, 0 };

	uint8_t* data{ nullptr };

	enum FragmentType {FRAG_END,FRAG_COMPOSITE,FRAG_RAW} ;
	enum FragmentSubType {SUB_FRAG_RAW =0x0, SUB_FRAG_COMPRESSED=0x1} ;

	struct RLEFragment{
		FragmentType type;
		bool isCompressed;
		uint16_t numTexels;

		int16_t dx;
		int16_t dy;
	};

	void ReadFragment(RLEFragment* frag);
	bool ExpandFragment(RLEFragment* frag, uint8_t* dst );

	int16_t leftDist;
	int16_t topDist;
	int16_t rightDist;
	int16_t botDist;

	uint8_t colorOffset{ 0 };
	bool WriteColor(uint8_t* dst,int16_t dx, int16_t dy, uint8_t color);
};

