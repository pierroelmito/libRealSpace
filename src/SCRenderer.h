//
//  gfx.h
//  iff
//
//  Created by Fabien Sanglard on 12/21/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#pragma once

#include <cstddef>
#include <cstdint>

#include <functional>

#include "Camera.h"
#include "Texture.h"

class RSEntity;
class Triangle;
class RSArea;
class MapVertex;

class SCRenderer {
public:
	SCRenderer();
	~SCRenderer();

	void Prepare();
	void Init();
	void Release();
	void ClearCache();
	void DrawModel(const RSEntity* object, size_t lodLevel, const Matrix& world);
	bool UploadTextureContentToGPU(RSTexture* texture);

	static void UpdateBitmapQuad(Color* data, uint32_t width, uint32_t height,
		float fade);

	VGAPalette& GetPalette() { return palette; }

#if USE_SHADER_PIPELINE != 1
	// Map Rendering
	// For research methods: Those should be deleted soon:
	void RenderObjects(const RSArea& area, size_t blockID);
	void RenderVerticeField(Vector3* vertices, int numVertices);
	void RenderWorldPoints(const RSArea& area, int LOD, int verticesPerBlock);
#endif

	struct Render3DParams {
		enum Flags {
			CLEAR_COLORS = 1,
			SKY = 2,
			CLOUDS = 4,
		};
		Vector3 camPos {};
		Camera3D camera {};
		uint32_t flags { CLEAR_COLORS };
	};

	using AddVertex = std::function<void(Texture&, uint8_t tritype, const Vector3&, const Vector3&, Color, const Vector2&)>;

	bool IsTextured(const MapVertex* tri0, const MapVertex* tri1, const MapVertex* tri2);
	void RenderTexturedTriangle(const AddVertex& vfunc, const RSArea& area, const MapVertex& tri0, const MapVertex& tri1, const MapVertex& tri2, int triangleType);
	void RenderColoredTriangle(const AddVertex& vfunc, const MapVertex& tri0, const MapVertex& tri1, const MapVertex& tri2);
	void RenderQuad(const AddVertex& vfunc, const RSArea& area, const MapVertex& currentVertex, const MapVertex& rightVertex, const MapVertex& bottomRightVertex, const MapVertex& bottomVertex, bool renderTexture);
	void RenderBlock(const AddVertex& vfunc, const RSArea& area, int LOD, int blockID, bool renderTexture);
	void RenderWorldSolid(const Render3DParams& params,const RSArea& area, int LOD, double gtime);
	void RenderWorldGround(const Render3DParams& params,const RSArea& area, int LOD, double gtime);
	void RenderWorldModels(const Render3DParams& params,const RSArea& area, int LOD, double gtime);

	void Draw3D(const Render3DParams& params, std::function<void(const Render3DParams& params)>&& f);

	RSCamera& GetCamera() { return camera; }
	void SetLight(const Vector3& position);

	bool IsPaused() const { return paused; }
	void Pause() { paused = true; }

	void Prepare(RSEntity* object);

	static Vector3 GetNormal(const RSEntity* object, const Triangle* triangle);
	static Vector3 GetNormal(const Vector3& v0, const Vector3& v1, const Vector3& v2);

	static void Log(const char* tag, uint32_t log_level, uint32_t log_item_id, const char* message_or_null, uint32_t line_nr, const char* filename_or_null, void* user_data);

private:
	VGAPalette palette;
	RSCamera camera;
	Vector3 lightDir;

	bool paused {};
};

using R3Dp = SCRenderer::Render3DParams;
