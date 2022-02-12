//
//  Stream.h
//  pak
//
//  Created by Fabien Sanglard on 12/23/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#pragma once

#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <array>

/*

 This is the class used to parse all byte streams.
 It is just a class which encapsulate convenient operations
 and can read byte,short,int as Little or Big Endian.

*/
class ByteStream
{
public:
	ByteStream(uint8_t* cursor);
	ByteStream(ByteStream& stream);
	ByteStream();
	~ByteStream();

	static void PrintBufStart(FILE* output, const uint8_t* buffer, int sz, int max);

	inline void Set(uint8_t* cursor)
	{
		this->cursor = cursor;
	}

	inline void MoveForward(size_t bytes)
	{
		this->cursor += bytes;
	}

	const uint8_t* Cursor() const { return cursor; }

	template <typename T>
	inline T ReadT()
	{
		T* v = (T*)this->cursor;
		this->cursor+=sizeof(T);
		return *v;
	}

	template <size_t N>
	inline std::array<uint8_t, N> ReadBytes()
	{
		std::array<uint8_t, N> r;
		memcpy(&r[0], this->cursor, N);
		this->cursor += N;
		return r;
	}

	inline uint8_t ReadByte(void)
	{
		return *this->cursor++;
	}

	inline uint8_t PeekByte(void)
	{
		return *(this->cursor+1);
	}

	inline uint16_t ReadUShort(void)
	{
		return ReadT<uint16_t>();
	}

	inline int16_t ReadShort(void)
	{
		return ReadT<int16_t>();
	}

	inline uint8_t* GetPosition(void)
	{
		return this->cursor;
	}

	inline uint32_t ReadUInt32LE(void)
	{
		return ReadT<uint32_t>();
	}

	inline int32_t ReadInt32LE(void)
	{
		return ReadT<int32_t>();
	}

	inline uint32_t ReadUInt32BE(void)
	{
		uint32_t toLittleEndian = 0;
		toLittleEndian |= *(cursor++)   << 24 ;
		toLittleEndian |= *(cursor++)   << 16 ;
		toLittleEndian |= *(cursor++)   <<  8 ;
		toLittleEndian |= *(cursor++)   <<  0 ;
		return toLittleEndian;
	}

	inline uint32_t ReadInt32BE(void)
	{
		uint32_t toLittleEndian = 0;
		toLittleEndian |= *(cursor++)   << 24 ;
		toLittleEndian |= *(cursor++)   << 16 ;
		toLittleEndian |= *(cursor++)   <<  8 ;
		toLittleEndian |= *(cursor++)   <<  0 ;
		return *reinterpret_cast<int32_t*>(&toLittleEndian);
	}

private:
	uint8_t* cursor{ nullptr };
};
