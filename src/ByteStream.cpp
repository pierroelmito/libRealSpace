//
//  Stream.cpp
//  pak
//
//  Created by Fabien Sanglard on 12/23/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "ByteStream.h"

#include <cctype>

ByteStream::ByteStream(uint8_t* cursor)
{
	this->cursor = cursor;
}

ByteStream::ByteStream(ByteStream& stream)
{
	this->cursor = stream.cursor;
}

ByteStream::ByteStream()
{
}

ByteStream::~ByteStream()
{
}

void ByteStream::PrintBufStart(FILE* output, const uint8_t* buffer, int sz, int max)
{
	for (int i = 0; i < std::min(max, sz); ++i)
		fprintf(output, " %02x", buffer[i]);
	fprintf(output, " - ");
	for (int i = 0; i < std::min(max, sz); ++i) {
		uint8_t c = isprint(buffer[i]) ? buffer[i] : '?';
		fprintf(output, "%c", c);
	}
}
