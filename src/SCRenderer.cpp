 //
//  gfx.cpp
//  iff
//
//  Created by Fabien Sanglard on 12/21/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

#include <algorithm>
#include <cassert>

#if USE_RAYLIB
#include <raylib.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#endif

template <class K, class V>
class ObjectCacheManager
{
public:
	template <typename FN>
	V& GetData(const K* ptr, FN&& fn)
	{
		uint32_t& dataIndex = objectToDataIndex[ptr];
		if (dataIndex == 0) {
			dataIndex = allData.size() + 1;
			allData.push_back({});
			V& tmp = allData[dataIndex - 1];
			fn(ptr, tmp);
		}
		return allData[dataIndex - 1];
	}
protected:
	std::map<const K*, uint32_t> objectToDataIndex;
	std::vector<V> allData;
};

struct AreaVertex
{
	RSVector3 pos;
	hmm_vec2 uv;
	hmm_vec4 color;
};

struct ObjVertex
{
	RSVector3 pos;
	RSVector3 normal;
	std::array<float, 2> uv;
	std::array<uint8_t, 4> col;
};

#if USE_RAYLIB
using ModelData = Model;
#else
template <typename T> using GenericMeshData = std::pair<std::vector<T>, std::vector<uint16_t>>;
using MeshData = GenericMeshData<ObjVertex>;
using ModelData = std::map<uint32_t, MeshData>;
#endif
using AreaCache = std::map<uint32_t, std::vector<AreaVertex>>;

ObjectCacheManager<RSEntity, ModelData> cacheEntityToModel;
ObjectCacheManager<RSArea, AreaCache> cacheAreaToModel;

// DIRTY...
const uint32_t PASS_VCOLOR = 0x12345678;
const uint32_t PASS_BLEND = 0x12345679;

#if !USE_RAYLIB

void glVertex(const RSVector3& p)
{
	glVertex3f(p.X, p.Y, p.Z);
}

void glLoadMatrixHMM(const RSMatrix& m)
{
	glLoadMatrixf((float*)m.Elements);
}

#endif

SCRenderer::SCRenderer()
: initialized(false)
{
}

SCRenderer::~SCRenderer(){
}

#if !USE_RAYLIB

void
SCRenderer::SetProj(const RSMatrix& m)
{
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixHMM(m);
}

void
SCRenderer::SetView(const RSMatrix& m)
{
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixHMM(m);
}

#endif

void
SCRenderer::Draw3D(const Render3DParams& params, std::function<void()>&& f)
{
#if USE_RAYLIB
	auto toVec3 = [] (auto v) -> Vector3 {
		return { v.X, v.Y, v.Z };
	};

	Camera3D cam{
		toVec3(camera.position),
		toVec3(camera.lookAt),
		{ 0, 1, 0 },
		50,
		CAMERA_PERSPECTIVE
	};
	BeginMode3D(cam);

	f();

	EndMode3D();
#else
	Renderer.SetProj(camera.proj);
	Renderer.SetView(camera.getView());

	glClearDepth(1.0f);
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	f();

	glDisable(GL_DEPTH_TEST);
#endif
}

void SCRenderer::Init(int32_t zoomFactor)
{
	int32_t width  = 320 * zoomFactor;
	int32_t height = 200 * zoomFactor;

	//Load the default palette
	IffLexer lexer ;
	lexer.InitFromFile("PALETTE.IFF");
	//lexer.List(stdout);

	RSPalette palette;
	palette.InitFromIFF(&lexer);

	this->palette = *palette.GetColorPalette();

#if !USE_RAYLIB
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);				// Black Background
	//glClearDepth(1.0f);								// Depth Buffer Setup
	glDisable(GL_DEPTH_TEST);							// Disable Depth Testing
#endif

	camera.SetPersective(50.0f, width / (float)height, 10.0f, 12000.0f);

	light = { 300, 300, 300 };

	initialized = true;
}

void SCRenderer::SetClearColor(uint8_t red, uint8_t green, uint8_t blue){
	if (!initialized)
		return;
#if !USE_RAYLIB
	glClearColor(red/255.0f, green/255.0f, blue/255.0f, 1.0f);
#endif
}

void SCRenderer::Clear()
{
	if (!initialized)
		return;
#if !USE_RAYLIB
	glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
	glColor4f(1, 1, 1, 1);
#endif
}

void* SCRenderer::MakeTexture(uint32_t w, uint32_t h, bool nearest)
{
#if USE_RAYLIB
	Texture* ptextureID = new Texture();
	Texture& textureID = *ptextureID;
	Image img = GenImageColor(w, h, BLUE);
	textureID = LoadTextureFromImage(img);
	UnloadImage(img);
	return ptextureID;
#else
	uint32_t* ptextureID = new uint32_t();
	uint32_t& textureID = *ptextureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glEnable(GL_TEXTURE_2D);
	//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	if (nearest) {
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	} else {
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}

	glBindTexture(GL_TEXTURE_2D, textureID);
	uint8_t* data = (uint8_t* )calloc(1, 320*200*4);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	free(data);
	return ptextureID;
#endif
}

void SCRenderer::UpdateBitmapQuad(void* ptextureID, Texel* data)
{
#if USE_RAYLIB
	Texture& textureID = *((Texture*)ptextureID);
	UpdateTexture(textureID, data);
	Camera2D cam{
		{ 0, 0 },
		{ 0, 0 },
		0,
		3.0f
	};
	BeginMode2D(cam);
	DrawTexture(textureID, 0, 0, RAYWHITE);
	EndMode2D();
#else
	uint32_t textureID = *((uint32_t*)ptextureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, 320, 200, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 1);
	glVertex2d(0,0);
	glTexCoord2f(1, 1);
	glVertex2d(320,0);
	glTexCoord2f(1, 0);
	glVertex2d(320, 200);
	glTexCoord2f(0, 0);
	glVertex2d(0,200);
	glEnd();
#endif
}

void SCRenderer::ResetState()
{
#if !USE_RAYLIB
	glMatrixMode(GL_PROJECTION); // Select The Projection Matrix
	glLoadIdentity(); // Reset The Projection Matrix
	glOrtho(0, 320, 0, 200, -10, 10) ;
	glMatrixMode(GL_MODELVIEW); // Select The Modelview Matrix
	glLoadIdentity();
	glColor4f(1, 1, 1,1);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST); // Disable Depth Testing
	glDisable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
#endif
}

void SCRenderer::CreateTextureInGPU(RSTexture* texture)
{
	if (!initialized)
		return;

#if !USE_RAYLIB
	glGenTextures(1, &texture->id);
	glBindTexture(GL_TEXTURE_2D, texture->id);
	glEnable(GL_TEXTURE_2D);
	//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glDisable(GL_TEXTURE_2D);
#endif
}

void SCRenderer::UploadTextureContentToGPU(RSTexture* texture)
{
	if (!initialized)
		return;
#if !USE_RAYLIB
	glBindTexture(GL_TEXTURE_2D, texture->id);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)texture->width, (GLsizei)texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture->data);
#endif
}

void SCRenderer::DeleteTextureInGPU(RSTexture* texture)
{
	if (!initialized)
		return;
#if !USE_RAYLIB
	if (texture->id)
		glDeleteTextures(1, &texture->id);
#endif
}


RSVector3 SCRenderer::GetNormal(const RSEntity* object, const Triangle* triangle) const
{
	//Calculate the normal for this triangle
	const RSVector3 edge1 = object->vertices[triangle->ids[0]] - object->vertices[triangle->ids[1]];
	const RSVector3 edge2 = object->vertices[triangle->ids[2]] - object->vertices[triangle->ids[1]];
	RSVector3 normal = HMM_NormalizeVec3(HMM_Cross(edge1, edge2));

#if 0
	// All normals are supposed to point outward in modern GPU but SC triangles
	// don't have consistent winding. They can be CW or CCW (the back governal of a jet  is
	// typically one triangle that must be visible from both sides !
	// As a result, gouraud shading was probably performed in screen space.
	// How can we replicate this ?
	// - Take the normal and compare it to the sign of the direction to the camera.
	// - If the signs don't match: reverse the normal.
	const RSVector3& cameraPosition = camera.position;
	const RSVector3& vertexOnTriangle = object->vertices[triangle->ids[0]];
	const RSVector3 cameraDirection = HMM_NormalizeVec3(cameraPosition - vertexOnTriangle);
	if (HMM_Dot(cameraDirection, normal) < 0)
		normal *= -1.0f;
#endif

	return normal;
}

#if USE_RAYLIB

void PrepareModel(SCRenderer& r, const RSEntity* object, size_t lodLevel, ModelData& mdata)
{
#if 1
	const Lod& lod = object->lods[lodLevel];

	struct MeshData {
		std::map<std::tuple<uint32_t, uint8_t, uint8_t>, uint16_t> lookup;
		std::vector<ObjVertex> vertice;
		std::vector<uint16_t> indice;
		int total = 0;
	};

	auto resolveVertex = [&] (MeshData& data, uint32_t index, uint8_t colIdx, uint8_t texSet) -> uint16_t {
		++data.total;
		uint16_t& res = data.lookup[{ index, colIdx, texSet }];
		if (res == 0) {
			res = data.vertice.size() + 1;
			const Texel* tx = r.GetPalette().GetRGBColor(colIdx);
			data.vertice.push_back({ object->vertices[index], { 0, 1, 0 }, { 0.5f, 0.5f }, { tx->r, tx->g, tx->b, tx->a } });
		}
		return res - 1;
	};

	MeshData d;

	for (int i = 0 ; i < lod.numTriangles ; i++) {
		uint16_t triangleID = lod.triangleIDs[i];
		const Triangle& tri = object->triangles[triangleID];
		if (tri.property == RSEntity::TRANSPARENT)
			continue;
		const uint16_t v0 = resolveVertex(d, tri.ids[0], tri.color, 255);
		const uint16_t v1 = resolveVertex(d, tri.ids[1], tri.color, 255);
		const uint16_t v2 = resolveVertex(d, tri.ids[2], tri.color, 255);
		d.indice.push_back(v0);
		d.indice.push_back(v1);
		d.indice.push_back(v2);
		d.indice.push_back(v0);
		d.indice.push_back(v2);
		d.indice.push_back(v1);
	}

	Mesh msh;
	memset(&msh, 0, sizeof(msh));
	msh.vertexCount = d.vertice.size();
	msh.triangleCount = d.indice.size() / 3;
	msh.vertices = (float*)RL_MALLOC(sizeof(float) * 3 * msh.vertexCount);
	msh.colors = (uint8_t*)RL_MALLOC(4 * msh.vertexCount);
	msh.indices = (uint16_t*)RL_MALLOC(sizeof(uint16_t) * 3 * msh.triangleCount);
	for (int i = 0; i < msh.vertexCount; ++i) {
		const auto& vert = d.vertice[i];
		memcpy(&msh.vertices[3 * i], vert.pos.Elements, 3 * 4);
		memcpy(&msh.colors[4 * i], &vert.col[0], 4);
	}
	for (int i = 0; i < msh.triangleCount * 3; ++i) {
		msh.indices[i] = d.indice[i];
	}
	UploadMesh(&msh, true);
#else
	Mesh msh = GenMeshCube(10, 2, 10);
#endif

	mdata = LoadModelFromMesh(msh);
}

#else

void PrepareModel(SCRenderer& r, const RSEntity* object, size_t lodLevel, ModelData& mdata)
{
	const Lod& lod = object->lods[lodLevel];
	const std::array<uint8_t, 4> white{ 0xff, 0xff, 0xff, 0xff };
	const std::array<float, 2> noUv{ 0.5f, 0.5f };

	if (lodLevel == 0){
		for (const uvxyEntry& textInfo : object->uvs) {
			//Seems we have a textureID that we don't have :( !
			if (textInfo.textureID >= object->images.size())
				continue;
			RSImage* image = object->images[textInfo.textureID];
			const RSTexture* texture = image->GetTexture();
			auto& verts = mdata[texture->id];
			const Triangle& tri = object->triangles[textInfo.triangleID];
			const RSVector3 normal = r.GetNormal(object, &tri);
			for(int j = 0; j < 3; j++){
				RSVector3 vertice = object->vertices[tri.ids[j]];
				const float u = textInfo.uvs[j].u / (float)texture->width;
				const float v = textInfo.uvs[j].v / (float)texture->height;
				verts.first.push_back({ vertice, normal, { u, v }, white });
			}
		}
	}

	{
		ModelData::mapped_type blend;
		for(int i = 0 ; i < lod.numTriangles ; i++) {
			uint16_t triangleID = lod.triangleIDs[i];
			const Triangle& tri = object->triangles[triangleID];
			if (tri.property != RSEntity::TRANSPARENT)
				continue;
			const RSVector3 normal = r.GetNormal(object, &tri);
			for (int j = 0; j < 3; j++) {
				RSVector3 vertice = object->vertices[tri.ids[j]];
				blend.first.push_back({ vertice, normal, noUv, white });
			}
		}
		if (!blend.first.empty())
			mdata[PASS_BLEND] = std::move(blend);
	}

	{
		ModelData::mapped_type opaque;
		for (int i = 0 ; i < lod.numTriangles ; i++) {
			//for(int i = 60 ; i < 62 ; i++){  //Debug purpose only back governal of F-16 is 60-62
			uint16_t triangleID = lod.triangleIDs[i];
			const Triangle& tri = object->triangles[triangleID];
			if (tri.property == RSEntity::TRANSPARENT)
				continue;
			const RSVector3 normal = r.GetNormal(object, &tri);
			for(int j=0 ; j < 3 ; j++) {
				RSVector3 vertice = object->vertices[tri.ids[j]];
				const Texel* texel = r.GetPalette().GetRGBColor(tri.color);
				opaque.first.push_back({ vertice, normal, noUv, { texel->r, texel->g, texel->b, texel->a } });
			}
		}
		if (!opaque.first.empty())
			mdata[PASS_VCOLOR] = std::move(opaque);
	}
}

#endif

void SCRenderer::DrawModel(const RSEntity* object, size_t lodLevel, const RSVector3& pos, const RSQuaternion& orientation)
{
	if (!initialized)
		return;

	if (lodLevel >= object->lods.size()){
		const auto maxl = std::min(0UL, object->lods.size() - 1);
		printf("Unable to render this Level Of Details (out of range): Max level is  %lu\n", maxl);
		return;
	}

#if !USE_RAYLIB
	const RSVector3 lighDirection = HMM_NormalizeVec3({ 10, 30, 10 });

	auto pushTransform = [&] () {
		glMatrixMode(GL_MODELVIEW);
		RSMatrix objMatrix = HMM_QuaternionToMat4(orientation);
		objMatrix.Elements[3][0] = pos.X;
		objMatrix.Elements[3][1] = pos.Y;
		objMatrix.Elements[3][2] = pos.Z;
		glPushMatrix();
		glMultMatrixf((float*)objMatrix.Elements);
	};

	auto popTransform = [&] () {
		glPopMatrix();
	};
#endif

#if 1

	ModelData& vertice = cacheEntityToModel.GetData(object, [&] (const RSEntity* o, ModelData& mdl) {
		PrepareModel(*this, o, lodLevel, mdl);
	});

#if USE_RAYLIB

	// need to fix normals... flip in shader??
	RSMatrix objMatrix = HMM_QuaternionToMat4(orientation);
	objMatrix.Elements[3][0] = pos.X;
	objMatrix.Elements[3][1] = pos.Y;
	objMatrix.Elements[3][2] = pos.Z;
	auto tmp = HMM_Transpose(objMatrix);
	DrawMesh(vertice.meshes[0], vertice.materials[0], *((Matrix*)&tmp));

#else

	const auto renderObj = [&] (const MeshData& mdata) {
		const auto& vert = mdata.first;
		glBegin(GL_TRIANGLES);
		for (const ObjVertex& v : vert) {
			const float l = std::max(0.2f, HMM_Dot(v.normal, lighDirection));
			const float col[4] = {
				l * (v.col[0] / 255.0f),
				l * (v.col[1] / 255.0f),
				l * (v.col[2] / 255.0f),
				1.0f
			};
			glTexCoord2f(v.uv[0], v.uv[1]);
			glColor4fv(col);
			glVertex(v.pos);
		}
		glEnd();
	};

	pushTransform();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_ALPHA_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glDisable(GL_CULL_FACE);

	if (auto itVColor = vertice.find(PASS_VCOLOR); itVColor != vertice.end()) {
		renderObj(itVColor->second);
	}

	glEnable(GL_TEXTURE_2D);
	glAlphaFunc(GL_GREATER, 0.0f);
	glEnable(GL_ALPHA_TEST);

	for (const auto& [k, vert] : vertice)
	{
		if (k == PASS_BLEND || k == PASS_VCOLOR)
			continue;
		glBindTexture(GL_TEXTURE_2D, k);
		renderObj(vert);
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glBlendFunc(GL_ONE, GL_ONE);
	glBlendEquation(GL_ADD);

	if (auto itBlend = vertice.find(PASS_BLEND); itBlend != vertice.end()) {
		renderObj(itBlend->second);
	}

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	popTransform();

#endif

#else

	const float ambientLamber = 0.4f;

	const Lod* lod = &object->lods[lodLevel];

	pushTransform();

	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	//Texture pass
	if (lodLevel == 0){
		glEnable(GL_TEXTURE_2D);
		//glDepthFunc(GL_EQUAL);
		glAlphaFunc ( GL_GREATER, 0.0 ) ;
		glEnable ( GL_ALPHA_TEST ) ;

		for (const uvxyEntry& textInfo : object->uvs) {
			//Seems we have a textureID that we don't have :( !
			if (textInfo.textureID >= object->images.size())
				continue;

			RSImage* image = object->images[textInfo.textureID];

			const Texture* texture = image->GetTexture();

			glBindTexture(GL_TEXTURE_2D, texture->id);

			const Triangle* triangle = &object->triangles[textInfo.triangleID];

			const RSVector3 normal = GetNormal(object, triangle);

			glBegin(GL_TRIANGLES);
			for(int j=0 ; j < 3 ; j++){
				RSVector3 vertice = object->vertices[triangle->ids[j]];
				RSVector3 lighDirection = HMM_NormalizeVec3(light - vertice);

				float lambertianFactor = HMM_Dot(lighDirection, normal);
				if (lambertianFactor < 0  )
					lambertianFactor = 0;
				lambertianFactor+= ambientLamber;
				if (lambertianFactor > 1)
					lambertianFactor = 1;

				glColor4f(lambertianFactor, lambertianFactor, lambertianFactor,1);
				glTexCoord2f(textInfo.uvs[j].u/(float)texture->width, textInfo.uvs[j].v/(float)texture->height);
				glVertex(object->vertices[triangle->ids[j]]);
			}
			glEnd();
		}
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
	}

	//Pass 3: Let's draw the transparent stuff render RSEntity::TRANSPARENT)
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glBlendEquation(GL_ADD);
	//glDepthFunc(GL_LESS);

	for(int i = 0 ; i < lod->numTriangles ; i++) {
		uint16_t triangleID = lod->triangleIDs[i];
		Triangle* triangle = &object->triangles[triangleID];

		if (triangle->property != RSEntity::TRANSPARENT)
			continue;

		const RSVector3 normal = GetNormal(object,triangle);

		glBegin(GL_TRIANGLES);

		for(int j=0 ; j < 3 ; j++) {
			RSVector3 vertice = object->vertices[triangle->ids[j]];
			RSVector3 sunDirection = HMM_NormalizeVec3(light - vertice);

			float lambertianFactor = HMM_Dot(sunDirection, normal);
			if (lambertianFactor < 0  )
				lambertianFactor = 0;
			lambertianFactor=0.2f;

			//int8_t gouraud = 255 * lambertianFactor;
			//gouraud = 255;
			glColor4f(lambertianFactor, lambertianFactor, lambertianFactor,1);
			glVertex(vertice);
		}

		glEnd();
	}

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	//Pass 1, draw color
	for(int i = 0 ; i < lod->numTriangles ; i++){
		//for(int i = 60 ; i < 62 ; i++){  //Debug purpose only back governal of F-16 is 60-62

		uint16_t triangleID = lod->triangleIDs[i];

		Triangle* triangle = &object->triangles[triangleID];

		if (triangle->property == RSEntity::TRANSPARENT)
			continue;

		const RSVector3 normal = GetNormal(object, triangle);

		glBegin(GL_TRIANGLES);
		for(int j=0 ; j < 3 ; j++){
			RSVector3 vertice = object->vertices[triangle->ids[j]];
			RSVector3 lighDirection = HMM_NormalizeVec3(light - vertice);

			float lambertianFactor = HMM_Dot(lighDirection, normal);
			if (lambertianFactor < 0  )
				lambertianFactor = 0;
			lambertianFactor+= ambientLamber;
			if (lambertianFactor > 1)
				lambertianFactor = 1;

			const Texel* texel = palette.GetRGBColor(triangle->color);

			glColor4f(texel->r/255.0f*lambertianFactor, texel->g/255.0f*lambertianFactor, texel->b/255.0f*lambertianFactor,1);
			//glColor4f(texel->r/255.0f, texel->g/255.0f, texel->b/255.0f,1);

			glVertex(object->vertices[triangle->ids[j]]);
		}
		glEnd();
	}

	popTransform();

#endif
}

void SCRenderer::SetLight(const RSVector3& l){
	this->light = l;
}

void SCRenderer::Prepare(RSEntity* object)
{
	for (RSImage* img : object->images)
		img->SyncTexture();
	object->prepared = true;
}

#define TEX_ZERO (0/64.0f)
#define TEX_ONE (64/64.0f)

// What is this offset ? It is used to get rid of the red delimitations
// in the 64x64 textures.
#define OFFSET (1/64.0f)

const float textTrianCoo64[2][3][2] = {
	{{TEX_ZERO,TEX_ZERO+OFFSET},    {TEX_ONE-2*OFFSET,TEX_ONE-OFFSET},    {TEX_ZERO,TEX_ONE-OFFSET} }, // LOWER_TRIANGE
	{{TEX_ZERO+2*OFFSET,TEX_ZERO+OFFSET},    {TEX_ONE,TEX_ZERO+OFFSET},    {TEX_ONE,TEX_ONE-OFFSET} }  //UPPER_TRIANGE
};

const float textTrianCoo[2][3][2] = {
	{{TEX_ZERO,TEX_ZERO},    {TEX_ONE,TEX_ONE},    {TEX_ZERO,TEX_ONE} }, // LOWER_TRIANGE
	{{TEX_ZERO,TEX_ZERO},    {TEX_ONE,TEX_ZERO},    {TEX_ONE,TEX_ONE} }  //UPPER_TRIANGE
};

#define LOWER_TRIANGE 0
#define UPPER_TRIANGE 1

bool SCRenderer::IsTextured(const MapVertex* tri0, const MapVertex* tri1, const MapVertex* tri2)
{
	return
		// tri0->type != tri1->type ||
		//tri0->type != tri2->type ||
		tri0->upperImageID == 0xFF || tri0->lowerImageID == 0xFF ;
}

void SCRenderer::RenderTexturedTriangle(
	const AddVertex& vfunc,
	const RSArea& area,
	const MapVertex* tri0,
	const MapVertex* tri1,
	const MapVertex* tri2,
	int triangleType)
{
	const float white[4] { 1, 1, 1, 1 };

	RSImage* image = NULL;
	if (triangleType == LOWER_TRIANGE)
		image = area.GetImageByID(tri0->lowerImageID);
	if (triangleType == UPPER_TRIANGE)
		image = area.GetImageByID(tri0->upperImageID);

	if (image == NULL){
		assert(false);
		printf("This should never happen: Put a break point here.\n");
		return;
	}

	// switch tex coord depending on texture size
	const bool is64 = image->width == 64;
	const auto& ttc = is64 ? textTrianCoo64 : textTrianCoo;

#if USE_RAYLIB
	const uint32_t texId = 0;
#else
	const uint32_t texId = image->GetTexture()->GetTextureID();
#endif
	vfunc(texId, tri0->v, white, ttc[triangleType][0]);
	vfunc(texId, tri1->v, white, ttc[triangleType][1]);
	vfunc(texId, tri2->v, white, ttc[triangleType][2]);
}

void SCRenderer::RenderColoredTriangle(
	const AddVertex& vfunc,
	const MapVertex* tri0,
	const MapVertex* tri1,
	const MapVertex* tri2)
{
	const float noUv[2] = { 0.5f, 0.5f };
	if (tri0->type != tri1->type || tri0->type != tri2->type) {
		const MapVertex* tri{};
		if (tri1->type > tri0->type)
			if (tri1->type > tri2->type)
				tri = tri1;
			else
				tri = tri2;
		else
			if (tri0->type > tri2->type)
				tri = tri0;
			else
				tri = tri2;
		vfunc(PASS_VCOLOR, tri0->v, tri->color, noUv);
		vfunc(PASS_VCOLOR, tri1->v, tri->color, noUv);
		vfunc(PASS_VCOLOR, tri2->v, tri->color, noUv);
	} else{
		vfunc(PASS_VCOLOR, tri0->v, tri0->color, noUv);
		vfunc(PASS_VCOLOR, tri1->v, tri1->color, noUv);
		vfunc(PASS_VCOLOR, tri2->v, tri2->color, noUv);
	}
}

void SCRenderer::RenderQuad(
	const AddVertex& vfunc,
	const RSArea& area,
	const MapVertex* currentVertex,
	const MapVertex* rightVertex,
	const MapVertex* bottomRightVertex,
	const MapVertex* bottomVertex,
	bool renderTexture)
{
	if (!renderTexture){
		//if (currentVertex->lowerImageID == 0xFF )
		RenderColoredTriangle(vfunc,currentVertex,bottomRightVertex,bottomVertex);
		// if (currentVertex->upperImageID == 0xFF )
		RenderColoredTriangle(vfunc,currentVertex,rightVertex,bottomRightVertex);
	} else{
		if (currentVertex->lowerImageID != 0xFF)
			RenderTexturedTriangle(vfunc,area,currentVertex,bottomRightVertex,bottomVertex,LOWER_TRIANGE);
		if (currentVertex->upperImageID != 0xFF)
			RenderTexturedTriangle(vfunc,area,currentVertex,rightVertex,bottomRightVertex,UPPER_TRIANGE);
	}
}

void SCRenderer::RenderBlock(const AddVertex& vfunc, const RSArea& area, int LOD, int i, bool renderTexture)
{
	const AreaBlock& block = area.GetAreaBlockByID(LOD, i);
	const uint32_t sideSize = block.sideSize;

	for (size_t x=0 ; x < sideSize-1 ; x ++){
		for (size_t y=0 ; y < sideSize-1 ; y ++){
			const MapVertex* currentVertex     =   &block.vertice[x+y*sideSize];
			const MapVertex* rightVertex       =   &block.vertice[(x+1)+y*sideSize];
			const MapVertex* bottomRightVertex =   &block.vertice[(x+1)+(y+1)*sideSize];
			const MapVertex* bottomVertex      =   &block.vertice[x+(y+1)*sideSize];
			RenderQuad(vfunc,area,currentVertex,rightVertex, bottomRightVertex, bottomVertex,renderTexture);
		}
	}

	//Inter-block right side
	if ( i % 18 != 17){
		const AreaBlock& currentBlock = block;
		const AreaBlock& rightBlock = area.GetAreaBlockByID(LOD, i+1);
		for (int y=0 ; y < sideSize-1 ; y ++){
			const MapVertex* currentVertex     =   currentBlock.GetVertice(currentBlock.sideSize-1, y);
			const MapVertex* rightVertex       =   rightBlock.GetVertice(0, y);
			const MapVertex* bottomRightVertex =   rightBlock.GetVertice(0, y+1);
			const MapVertex* bottomVertex      =   currentBlock.GetVertice(currentBlock.sideSize-1, y+1);
			RenderQuad(vfunc,area,currentVertex,rightVertex, bottomRightVertex, bottomVertex,renderTexture);
		}
	}

	//Inter-block bottom side
	if ( i / 18 != 17) {
		const AreaBlock& currentBlock = block;
		const AreaBlock& bottomBlock = area.GetAreaBlockByID(LOD, i+BLOCK_PER_MAP_SIDE);
		for (int x=0 ; x < sideSize-1 ; x++){
			const MapVertex* currentVertex     =   currentBlock.GetVertice(x,currentBlock.sideSize-1);
			const MapVertex* rightVertex       =   currentBlock.GetVertice(x+1,currentBlock.sideSize-1);
			const MapVertex* bottomRightVertex =   bottomBlock.GetVertice(x+1,0);
			const MapVertex* bottomVertex      =   bottomBlock.GetVertice(x,0);
			RenderQuad(vfunc,area,currentVertex,rightVertex, bottomRightVertex, bottomVertex,renderTexture);
		}
	}

	//Inter bottom-right quad
	if ( i % 18 != 17 && i / 18 != 17) {
		const AreaBlock& currentBlock = block;
		const AreaBlock& rightBlock = area.GetAreaBlockByID(LOD, i+1);
		const AreaBlock& rightBottonBlock = area.GetAreaBlockByID(LOD, i+1+BLOCK_PER_MAP_SIDE);
		const AreaBlock& bottomBlock = area.GetAreaBlockByID(LOD, i+BLOCK_PER_MAP_SIDE);
		const MapVertex* currentVertex     =   currentBlock.GetVertice(currentBlock.sideSize-1,currentBlock.sideSize-1);
		const MapVertex* rightVertex       =   rightBlock.GetVertice(0,currentBlock.sideSize-1);
		const MapVertex* bottomRightVertex =   rightBottonBlock.GetVertice(0,0);
		const MapVertex* bottomVertex      =   bottomBlock.GetVertice(currentBlock.sideSize-1,0);
		RenderQuad(vfunc,area,currentVertex,rightVertex, bottomRightVertex, bottomVertex,renderTexture);
	}
}

void SCRenderer::RenderJets(const RSArea& area)
{
	for(RSEntity* entity : area.GetJets())
		DrawModel(entity, LOD_LEVEL_MAX, entity->position, entity->orientation);
}

void SCRenderer::RenderWorldSolid(const RSArea& area, int LOD, int verticesPerBlock)
{
	running = true;

#if 0
	// Camera
	const RSVector3 lookAt{ 3856, 0, 2856};
	RSVector3 newPosition{ 4100, 100, 3000 };
#if USE_RAYLIB
	double currentTime = GetTime();
#else
	uint32_t currentTime = SDL_GetTicks();
#endif
	newPosition.X =  lookAt.X + 300 * cos(currentTime/2000.0f);
	newPosition.Z =  lookAt.Z + 300 * sin(currentTime/2000.0f);
	camera.SetPosition(newPosition);
	camera.LookAt(lookAt);
	SetProj(camera.proj);
	SetView(camera.getView());

	//Island
	/*
	newPosition[0]=  2500;//lookAt[0] + 5256*cos(counter/2);
	newPosition[1]= 350;
	newPosition[2]=  600;//lookAt[2];// + 5256*sin(counter/2);
	vec3_t lookAt = {2456,0,256};
	*/

	//City Top

	//City view on mountains
	/*
	counter = 23;
	vec3_t lookAt = {3856,30,2856};
	newPosition[0]=  lookAt[0] + 256*cos(counter/2);
	newPosition[1]= 60;
	newPosition[2]=  lookAt[2] + 256*sin(counter/2);
	*/

	//Canyon
	///*

	//*/
#endif

	//counter += 0.02;

	const AreaCache& areaCache = cacheAreaToModel.GetData(&area, [&] (const RSArea* a, AreaCache& cache) {
		//struct PointComparator
		//{
		//	bool operator() (const RSVector3& a, const RSVector3& b) const {
		//		if (a.X != b.X) return a.X < b.X;
		//		if (a.Y != b.Y) return a.Y < b.Y;
		//		return a.Z < b.Z;
		//	}
		//};
		//std::map<RSVector3, uint32_t, PointComparator> pointToIndex;
		AreaCache tmp;
		AddVertex vadd = [&] (uint32_t texId, const RSVector3& pos, const float* col, const float* uv) {
			//uint32_t& idx = pointToIndex[pos];
			//if (idx == 0)
			//	idx = pointToIndex.size();
			auto& vert = tmp[texId];
			const auto r = col[0];
			const auto g = col[1];
			const auto b = col[2];
			const bool useVertex = std::abs(pos.Y) > 0.01f || texId != PASS_VCOLOR || b < r || b < g;
			if (useVertex)
				vert.push_back({ pos, { uv[0], uv[1] }, { r, g, b, col[3] } });
		};
		for(int i = 0; i < BLOCKS_PER_MAP; i++)
			RenderBlock(vadd, area, LOD, i, false);
		for(int i = 0; i < BLOCKS_PER_MAP; i++)
			RenderBlock(vadd, area, LOD, i, true);
		cache = std::move(tmp);
	});

#if USE_RAYLIB

	DrawPlane({ 0, 0, 0 }, { 3000, 3000 }, GREEN);

#else

	const GLuint fogMode[]= { GL_EXP, GL_EXP2, GL_LINEAR };   // Storage For Three Types Of Fog
	const GLuint fogfilter= 0;                    // Which Fog To Use
	const GLfloat fogColor[4]= {1.0f, 1.0f, 1.0f, 1.0f};
	glFogi(GL_FOG_MODE, fogMode[fogfilter]);        // Fog Mode
	glFogfv(GL_FOG_COLOR, fogColor);            // Set Fog Color
	glFogf(GL_FOG_DENSITY, 0.0002f);              // How Dense Will The Fog Be
	glHint(GL_FOG_HINT, GL_DONT_CARE);          // Fog Hint Value
	glFogf(GL_FOG_START, 600.0f);             // Fog Start Depth
	glFogf(GL_FOG_END, 8000.0f);               // Fog End Depth
	glEnable(GL_FOG);
	glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	if (auto itV = areaCache.find(PASS_VCOLOR); itV != areaCache.end()) {
		const auto& vert = itV->second;
		glBegin(GL_TRIANGLES);
		for (const AreaVertex& v : vert) {
			glTexCoord2fv(v.uv.Elements);
			glColor4fv(v.color.Elements);
			glVertex3fv(v.pos.Elements);
		}
		glEnd();
	}
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glDepthFunc(GL_EQUAL);
	for (const auto& [k, vert] : areaCache) {
		if (k == PASS_VCOLOR)
			continue;
		glBindTexture(GL_TEXTURE_2D, k);
		glBegin(GL_TRIANGLES);
		for (const AreaVertex& v : vert) {
			glTexCoord2fv(v.uv.Elements);
			glColor4fv(v.color.Elements);
			glVertex3fv(v.pos.Elements);
		}
		glEnd();
	}
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);

#endif

	//Render objects on the map
	//for(int i=97 ; i < 98 ; i++)
	//for(int i=0 ; i < BLOCKS_PER_MAP ; i++)
	//   RenderObjects(area,i);
	RenderJets(area);
}

#if USE_SHADER_PIPELINE != 1

void SCRenderer::DisplayModel(RSEntity* object,size_t lodLevel)
{
	if (!initialized)
		return;

	if (object->IsPrepared())
		Prepare(object);

	SetProj(camera.proj);

	running = true;
	float counter = 0;
	while (running) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		light = { 20 * cos(counter), 10, 20 * sin(counter) };
		counter += 0.02;

		//camera.SetPosition(position);

		SetView(camera.getView());

		DrawModel(object, lodLevel);

		//Render light
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		glPointSize(6);
		glBegin(GL_POINTS);
		glColor4f(1, 1,0 , 1);
		glVertex(light);
		glEnd();
	}
}

void SCRenderer::RenderObjects(const RSArea& area,size_t blockID)
{
	float color[3] = { 1, 0, 0 };

	const std::vector<MapObject> *objects = &area.objects[blockID];

	glColor3fv(color);
	glPointSize(3);
	glDisable(GL_DEPTH_TEST);
	glBegin(GL_POINTS);

	const uint32_t BLOCK_WIDTH = 512;

	for (size_t i =8 ; i < 9; i++) {
		MapObject object = objects->at(i);

		int32_t offset[3];
		offset[0] = blockID % 18 * BLOCK_WIDTH ;
		offset[1] = area.elevation[blockID];
		offset[2] = (int32_t)blockID / 18 * BLOCK_WIDTH;

		/*
		glVertex3d(object.position[0]/255*BLOCK_WIDTH+offset[0],
				   object.position[1]+offset[1],
				   object.position[2]/255*BLOCK_WIDTH+offset[2]);
		*/

		int32_t localDelta[3];
		localDelta[0] = object.position[0]/65355.0f*BLOCK_WIDTH;
		localDelta[1] = object.position[1];/// HEIGHT_DIVIDER                   ;
		localDelta[2] = object.position[2]/65355.0f*BLOCK_WIDTH;

		size_t toDraw[3];
		toDraw[0] = localDelta[0]+offset[0];
		toDraw[1] = offset[1];
		toDraw[2] = localDelta[2]+offset[2];

		glVertex3d(toDraw[0], toDraw[1], toDraw[2]);
	}

	glEnd();
}

void SCRenderer::RenderVerticeField(RSVector3* vertices, int numVertices)
{
	SetProj(camera.proj);

	running = true;
	float counter = 0;
	while (running) {
		const RSVector3 newPosition{ 256 * cos(counter), 0, 256 * sin(counter) };
		counter += 0.02;

		camera.SetPosition(newPosition);
		SetView(camera.getView());

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glPointSize(5);
		glBegin(GL_POINTS);
		for(int i=0; i < numVertices ; i ++)
			glVertex(vertices[i]);
		glEnd();
	}
}

void SCRenderer::RenderWorldPoints(const RSArea& area, int LOD, int verticesPerBlock)
{
	SetProj(camera.proj);

	glPointSize(4);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	running = true;

	static float counter = 0;

	while (running) {
		glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);

		glMatrixMode(GL_MODELVIEW);

		RSVector3 lookAt = { 256*16,100,256*16 };

		Renderer.GetCamera().LookAt(lookAt);

		RSVector3 newPosition = camera.position;
		newPosition.X = lookAt.X + 5256*cos(counter/2);
		newPosition.Y = 3700;
		newPosition.Z = lookAt.Z + 5256*sin(counter/2);
		camera.SetPosition(newPosition);

		//Matrix* modelViewMatrix = camera.GetViewMatrix();
		//glLoadMatrixf(modelViewMatrix->ToGL());
		glLoadMatrixHMM(camera.getView());

		glBegin(GL_POINTS);
		for(int i=0 ; i < 324 ; i++) {
		//for(int i=96 ; i < 99 ; i++) {
			const AreaBlock& block = area.GetAreaBlockByID(LOD, i);
			for (size_t i=0 ; i < verticesPerBlock ; i ++){
				const MapVertex* v = &block.vertice[i];
				glColor3fv(v->color);
				glVertex(v->v);
			}
		}
		glEnd();

		//Render objects on the map
		for(int i=0 ; i < 324 ; i++)
			RenderObjects(area,i);
	}
}

#endif
