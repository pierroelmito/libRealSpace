//
//  gfx.h
//  iff
//
//  Created by Fabien Sanglard on 12/21/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#pragma once

#include <cstdint>
#include <cstddef>

#include <functional>
#include <optional>
#include <map>
#include <string>

#include "Matrix.h"
#include "Quaternion.h"
#include "Texture.h"
#include "Camera.h"

class RSEntity;
class Triangle;
class RSArea;
class MapVertex;

class SCRenderer
{
public:
	 SCRenderer();
	~SCRenderer();

	void Prepare();
	void Init(int32_t zoom);
	void Release();
	void MakeContext();
	void DrawModel(const RSEntity* object, size_t lodLevel, const RSMatrix& world);
	bool CreateTextureInGPU(RSTexture* texture);
	bool UploadTextureContentToGPU(RSTexture* texture);
	void DeleteTextureInGPU(RSTexture* texture);

	static void UpdateBitmapQuad(Texel* data, uint32_t width, uint32_t height);

	VGAPalette& GetPalette() { return palette; }

#if USE_SHADER_PIPELINE != 1
	//Map Rendering
	//For research methods: Those should be deleted soon:
	void RenderObjects(const RSArea& area,size_t blockID);
	void RenderVerticeField(RSVector3* vertices, int numVertices);
	void RenderWorldPoints(const RSArea& area, int LOD, int verticesPerBlock);
#endif

	using AddVertex = std::function<void(uint32_t, const RSVector3&, const RSVector3&, const float*, const float*)>;

	bool IsTextured(const MapVertex* tri0,const MapVertex* tri1,const MapVertex* tri2);
	void RenderTexturedTriangle(const AddVertex& vfunc, const RSArea& area,const MapVertex& tri0,const MapVertex& tri1,const MapVertex& tri2,int triangleType);
	void RenderColoredTriangle (const AddVertex& vfunc, const MapVertex& tri0,const MapVertex& tri1,const MapVertex& tri2);
	void RenderQuad(const AddVertex& vfunc, const RSArea& area, const MapVertex& currentVertex, const MapVertex& rightVertex, const MapVertex& bottomRightVertex, const MapVertex& bottomVertex, bool renderTexture);
	void RenderBlock(const AddVertex& vfunc, const RSArea& area,int LOD, int blockID,bool renderTexture);
	void RenderWorldSolid(const RSArea& area, int LOD, double gtime);
	void RenderWorldGround(const RSArea& area, int LOD, double gtime);
	void RenderWorldModels(const RSArea& area, int LOD, double gtime);
	void RenderJets(const RSArea& area);
	void RenderSky();
	void RenderClouds();

	struct Render3DParams {
		enum Flags {
			CLEAR_COLORS = 1,
			SKY = 2,
			CLOUDS = 4,
		};
		uint32_t flags{ CLEAR_COLORS };
	};
	void Draw3D(const Render3DParams& params, std::function<void()>&& f);

	RSCamera& GetCamera() { return camera; }
	void SetLight(const RSVector3& position);

	bool IsPaused() const { return paused; }
	void Pause(){ paused = true; }
	void Prepare(RSEntity* object);
	static RSVector3 GetNormal(const RSEntity* object, const Triangle* triangle);
	static RSVector3 GetNormal(const RSVector3& v0, const RSVector3& v1, const RSVector3& v2);

private:
	VGAPalette palette;
	RSCamera camera;
	RSVector3 lightDir;
	bool initialized{ false };
	bool running;
	bool paused;
};

using R3Dp = SCRenderer::Render3DParams;

/*
void IMG_Init();
void IMG_ShowPalette(Palette* palette,int cellSize);
void IMG_ShowImage(uint8_t* image, uint16_t width, uint16_t height,Palette* palette,int zoom,bool wait);
void IMG_ShowModel(RealSpaceObject* object,Palette* palette );
*/
