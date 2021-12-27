//
//  Texture.h
//  iff
//
//  Created by Fabien Sanglard on 12/20/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#pragma once

#include "ByteStream.h"

struct Texel {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

struct VGAPalette
{
    Texel colors[256];

	void SetColor(uint8_t value,Texel* texel)
	{
		colors[value] = *texel;
    }

	Texel* GetRGBColor(uint8_t value)
	{
        return &colors[value];
    }

	void Diff(const VGAPalette& other)
	{
        for (int i=0  ;i <256 ; i++){
			if(colors[i].r != other.colors[i].r ||
			   colors[i].g != other.colors[i].g ||
			   colors[i].b != other.colors[i].b ||
			   colors[i].a != other.colors[i].a
               )
                printf("diff: %d.\n",i);
        }
    }

	void ReadPatch(ByteStream* s)
	{
        int16_t offset = s->ReadShort();
        int16_t numColors = s->ReadShort();
        
        if (offset + numColors > 256){
            printf("VGAPalette::ReadPatch => Error, this will overflow.\n");
            return;
        }
        
        for (uint16_t i= 0 ; i < numColors ; i++){
            colors[offset+i].r = s->ReadByte() * 255/63.0f;
            colors[offset+i].g = s->ReadByte() * 255/63.0f;
            colors[offset+i].b = s->ReadByte() * 255/63.0f;
            colors[offset+i].a = 1 ;
        }
        
    }
};


class RSImage;

class Texture{
    
public:
    Texture();
    ~Texture();
    
    void Set(RSImage* image );
    size_t width;
    size_t height;
    char name[8];
    uint8_t* data;

    enum Location{ DISK=0x1,RAM=0x2,VRAM=0x4};
    uint8_t locFlag;
    
    //GPU stuff
    uint32_t id;
    uint32_t GetTextureID(void);

	void UpdateContent(RSImage* image);

private:
};
