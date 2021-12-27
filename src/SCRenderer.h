//
//  gfx.h
//  iff
//
//  Created by Fabien Sanglard on 12/21/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#pragma once

#include <cstdint>
#include <functional>

#include "Matrix.h"
#include "Texture.h"
#include "Camera.h"

class RSEntity;

class Triangle;
class RSArea;
class MapVertex;
class Texture;

class SCRenderer
{
public:
	 SCRenderer();
	~SCRenderer();

	void Prepare(void);
	void Init(int32_t zoom);
	void Clear(void);
	void DrawModel(RSEntity* object, size_t lodLevel);
	void DisplayModel(RSEntity* object,size_t lodLevel);
	void CreateTextureInGPU(Texture* texture);
	void UploadTextureContentToGPU(Texture* texture);
	void DeleteTextureInGPU(Texture* texture);

	VGAPalette& GetPalette(void) { return palette; }

	//Map Rendering
	//For research methods: Those should be deleted soon:
	void RenderVerticeField(Point3D* vertices, int numVertices);

	void RenderWorldPoints(const RSArea& area, int LOD, int verticesPerBlock);

	void RenderTexturedTriangle(const MapVertex* tri0,const MapVertex* tri1,const MapVertex* tri2,const RSArea& area,int triangleType);
	void RenderColoredTriangle (const MapVertex* tri0,const MapVertex* tri1,const MapVertex* tri2);
	bool IsTextured(const MapVertex* tri0,const MapVertex* tri1,const MapVertex* tri2);
	void RenderQuad(
		const MapVertex* currentVertex,
		const MapVertex* rightVertex,
		const MapVertex* bottomRightVertex,
		const MapVertex* bottomVertex,
		const RSArea& area,
		bool renderTexture
	);

	void RenderBlock(const RSArea& area,int LOD, int blockID,bool renderTexture);
	void RenderWorldSolid(const RSArea& area, int LOD, int verticesPerBlock);
	void RenderObjects(const RSArea& area,size_t blockID);
	void RenderJets(const RSArea& area);

	struct Render3DParams {};
	void SetProj(const Matrix& m);
	void SetView(const Matrix& m);
	void Draw3D(const Render3DParams& params, std::function<void()>&& f);

	Camera& GetCamera(void) { return camera; }
	void SetLight(const Point3D& position);

	inline bool IsPaused(void) const {
		return paused;
	}

	inline void Pause(void){
		paused = true;
	}

	void SetClearColor(uint8_t red, uint8_t green, uint8_t blue);
	void Prepare(RSEntity* object);

private:

	bool initialized{ false };

	Vector3D GetNormal(RSEntity* object, const Triangle* triangle);

	VGAPalette palette;
	bool running;
	bool paused;

	Camera camera;
	Point3D light;
};

/*
void IMG_Init(void);
void IMG_ShowPalette(Palette* palette,int cellSize);
void IMG_ShowImage(uint8_t* image, uint16_t width, uint16_t height,Palette* palette,int zoom,bool wait);
void IMG_ShowModel(RealSpaceObject* object,Palette* palette );
*/
