//
//  gfx.cpp
//  iff
//
//  Created by Fabien Sanglard on 12/21/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "SCRenderer.h"

#include <cassert>

#include <algorithm>
#include <format>
#include <set>

#include <assimp/Exporter.hpp>
#include <assimp/scene.h>

// #include "Math.h"
#include "RSArea.h"
#include "RSImage.h"
#include "RSPalette.h"
#include "UserProperties.h"
#include "main.h"
#include "rltools.hpp"

#define TINYDDSLOADER_IMPLEMENTATION
#include "tinyddsloader.h"

typedef enum {
	START = SHADER_LOC_BONE_MATRICES,
	SHADER_LOC_CAMERA_INFO
} UserLocationIndex;

struct ObjVertexData {
	std::vector<Vector3> pos;
	std::vector<Vector3> normal;
	std::vector<Vector2> uv;
	std::vector<Color> col;
};

struct BaseMeshData {
	ObjVertexData vertexData {};
	std::vector<uint16_t> indice {};
};

struct CompTexture {
	bool operator()(Texture* a, Texture* b) const { return a->id < b->id; }
};

RenderTexture rtScene {};
Texture rtScene_Depth {};

Texture texNoise {};
Texture texSkydome {};
Texture texScreen {};
Texture texWhite {};
Texture texWater {};
Texture texGrass {};
Texture texCloudDensity {};
Texture texDetails {};

Mesh mshFsq {};

Shader shdModel {};
Shader shdGroundColor {};
Shader shdGroundTex {};
Shader shdSky {};

// int locYolo {};

Vector3 DecodeColor(const std::string& col)
{
	if (col.size() != 6)
		return Vector3 { 1.0f, 0.0f, 1.0f };
	const auto charToColor = [](char c) {
		if (c >= '0' && c <= '9')
			return c - '0';
		if (c >= 'a' && c <= 'f')
			return 10 + c - 'a';
		if (c >= 'A' && c <= 'F')
			return 10 + c - 'A';
		return 0;
	};
	union {
		uint32_t v;
		uint8_t a[4];
	};
	v = 0;
	for (char c : col)
		v = (v << 4) | charToColor(c);
	return { a[2] / 255.0f, a[1] / 255.0f, a[0] / 255.0f };
}

std::optional<PixelFormat> GetFormatFromDDS(tinyddsloader::DDSFile::DXGIFormat fmt)
{
	if (fmt == tinyddsloader::DDSFile::DXGIFormat::B8G8R8A8_UNorm)
		return PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
	if (fmt == tinyddsloader::DDSFile::DXGIFormat::BC1_UNorm)
		return PIXELFORMAT_COMPRESSED_DXT1_RGB;
	assert(false);
	return {};
}

std::optional<Texture> LoadDDS(const char* path)
{
	tinyddsloader::DDSFile dds;
	tinyddsloader::Result result = dds.Load(path);
	if (result != tinyddsloader::Result::Success)
		return {};

	auto fmt = GetFormatFromDDS(dds.GetFormat());
	if (!fmt)
		return {};

#if 0
	const auto mips = dds.GetMipCount();

	sg_image_desc idesc {};

	idesc.type = SG_IMAGETYPE_2D;
	idesc.render_target = false;
	idesc.width = dds.GetWidth();
	idesc.height = dds.GetHeight();
	idesc.pixel_format = fmt;
	idesc.usage = SG_USAGE_IMMUTABLE;
	// idesc.mag_filter = GetMagFilter(true);
	// idesc.min_filter = GetMinFilter(true, mips > 1);
	// idesc.max_anisotropy = 4;
	idesc.num_mipmaps = mips;
	// idesc.wrap_u = SG_WRAP_REPEAT;
	// idesc.wrap_v = SG_WRAP_REPEAT;

	for (auto i = 0u; i < mips; ++i) {
		const tinyddsloader::DDSFile::ImageData* data = dds.GetImageData(i);
		idesc.data.subimage[0][i] = { data->m_mem, data->m_memSlicePitch };
	}

	return std::make_optional<Texture>(idesc.width, idesc.height,
		sg_make_image(&idesc));
#else
	return {};
#endif
}

template <typename TO>
TO* allocBuffer(size_t sz)
{
	TO* r = new TO[sz];
	return r;
}

template <typename TO, typename FROM>
TO* allocFromVec(const std::vector<FROM>& from, TO (*f)(const FROM&))
{
	TO* r = allocBuffer<TO>(from.size());
	for (size_t i = 0; i < from.size(); ++i)
		r[i] = f(from[i]);
	return r;
}

template <typename T>
T* allocFromVec(const std::vector<T>& from)
{
	T* r = allocBuffer<T>(from.size());
	for (size_t i = 0; i < from.size(); ++i)
		r[i] = from[i];
	return r;
}

void TestSaveModel(const std::string& name, const BaseMeshData& data)
{
	std::vector<aiMesh*> meshes;
	std::vector<unsigned int> meshIndex;
	meshIndex.push_back(meshes.size());

	aiMesh* msh = meshes.emplace_back(new aiMesh {});
	msh->mPrimitiveTypes = aiPrimitiveType_TRIANGLE;
	msh->mNumVertices = data.vertexData.pos.size();
	msh->mVertices = allocFromVec<aiVector3D, Vector3>(data.vertexData.pos, [](const auto& p) -> aiVector3D { return { p.x, p.y, p.z }; });
	msh->mNormals = allocFromVec<aiVector3D, Vector3>(data.vertexData.normal, [](const auto& p) -> aiVector3D { return { p.x, p.y, p.z }; });
	msh->mTextureCoords[0] = allocFromVec<aiVector3D, Vector2>(data.vertexData.uv, [](const auto& p) -> aiVector3D { return { p.x, p.y, 0.0f }; });
	msh->mColors[0] = allocFromVec<aiColor4D, Color>(data.vertexData.col, [](const auto& p) -> aiColor4D { return { p.r / 255.0f, p.g / 255.0f, p.b / 255.0f, p.a / 255.0f }; });
	msh->mNumFaces = data.indice.size() / 3;
	msh->mFaces = allocBuffer<aiFace>(msh->mNumFaces);
	for (int i = 0; i < msh->mNumFaces; ++i) {
		aiFace& f = msh->mFaces[i];
		f.mNumIndices = 3;
		const auto i0 = data.indice[i * 3 + 0];
		const auto i1 = data.indice[i * 3 + 1];
		const auto i2 = data.indice[i * 3 + 2];
		const std::vector<uint16_t> face { i0, i1, i2 };
		f.mIndices = allocFromVec<unsigned int, uint16_t>(face, [](const uint16_t& idx) -> unsigned int { return idx; });
	}

	aiNode* root = new aiNode {};
	aiScene scene {};
	scene.mRootNode = root;
	if (meshes.empty())
		return;

	scene.mNumMeshes = meshes.size();
	scene.mMeshes = allocFromVec(meshes);
	root->mNumMeshes = meshIndex.size();
	root->mMeshes = allocFromVec(meshIndex);

	const auto path = std::format("data/models/model_{}.gltf", name);
	Assimp::Exporter exporter;
	aiReturn result = exporter.Export(&scene, "gltf2", path.c_str());
}

std::vector<uint32_t> ComputeSkyDome(
	int w, int h,
	const std::function<std::array<uint8_t, 4>(const Vector3& dir)>& fn)
{
	std::vector<uint32_t> data;
	data.resize(w * h);

	for (int y = 0; y < h; ++y) {
		const float ry = (y + 0.5f - 0.5f * h) / (0.5f * h);
		for (int x = 0; x < w; ++x) {
			const float rx = (x + 0.5f - 0.5f * w) / (0.5f * w);
			const float d = rx * rx + ry * ry;
			if (d <= 1.001f) {
				const std::array<uint8_t, 4> c = fn({ rx, ry, 1 - sqrtf(d) });
				memcpy(&data[y * w + x], &c[0], 4);
			}
		}
	}

	return data;
}

Texture MakeSkyDome(
	int w, int h,
	const std::function<std::array<uint8_t, 4>(const Vector3& dir)>& fn)
{
	/*
	return MakeImage(w, h, SG_PIXELFORMAT_RGBA8, SG_USAGE_IMMUTABLE, IFLinear,
		ComputeSkyDome(w, h, fn));
	*/
	return Texture {};
}

/*
fog.fogColor = DecodeColor(UserProperties::Get().Strings.Get("FogColor", "1b669b"));
fog.thickNess = UserProperties::Get().Floats.Get("FogThickness", 0.0002);
*/

template <class K, class V>
class ObjectCacheManager {
public:
	void Clear(const std::function<void(V&)>& fn)
	{
		for (V& v : allData)
			fn(v);
		allData.clear();
	}
	template <typename FN>
	V& GetData(const K* ptr, FN&& fn)
	{
		uint32_t& dataIndex = objectToDataIndex[ptr];
		if (dataIndex == 0) {
			dataIndex = allData.size() + 1;
			fn(ptr, allData.emplace_back());
		}
		return allData[dataIndex - 1];
	}
	const std::vector<V>& AllData() const { return allData; }

protected:
	std::map<const K*, uint32_t> objectToDataIndex;
	std::vector<V> allData;
};

ObjectCacheManager<RSEntity, Model> cacheEntityToModel;
ObjectCacheManager<AreaBlock, Model> cacheBlockToModel;

SCRenderer::SCRenderer()
{
}

SCRenderer::~SCRenderer() { }

#if 0
template <size_t N, typename S, typename... PARAMS>
std::array<uint8_t, 4>
FractalNoiseSkyDome(const Vector3& v, const std::array<Vector2, N>& seeds,
	const S& sampler, const PARAMS&... params)
{
	float tc = 0.0f;
	float tw = 0.0f;
	for (int i = 0; i < N; ++i) {
		const float m = powf(1.65f, i);
		const float cw = 1.0f / powf(1.5f, i);
		const float div = 20.0f * (0.1f + v.Z);
		const float fx = HMM_DotVec2({ v.X, v.Y }, seeds[i]) / div;
		const float fy = HMM_DotVec2({ v.X, v.Y }, { seeds[i].Y, -seeds[i].X }) / div;
		const float cc = sampler(m * fx, m * fy, params...);
		assert(cc >= 0.0f && cc <= 1.0f);
		tw += cw;
		tc += cw * cc;
	}
	const float fc = SmoothStep(Ratio(0.48f, 0.99f, tc / tw));
	assert(fc >= 0.0f && fc <= 1.0f);
	const uint8_t c = uint8_t(fc * 255.9f);
	return { 255, 255, 255, c };
}
#endif

void SCRenderer::Init()
{
	int32_t width = 320;
	int32_t height = 200;

	std::vector<std::pair<size_t, const char*>> shdUniforms = {
		{ SHADER_LOC_MAP_OCCLUSION, "cloudDensity" },
		{ SHADER_LOC_MAP_HEIGHT, "depthMap" },
		{ SHADER_LOC_CAMERA_INFO, "camInfo" },
	};

	shdModel = rlt::MakeShader("default", "model", shdUniforms);
	shdGroundColor = rlt::MakeShader("default", "ground_color", shdUniforms);
	shdGroundTex = rlt::MakeShader("default", "ground_tex", shdUniforms);
	shdSky = rlt::MakeShader("fsq", "sky", shdUniforms);

	mshFsq = rlt::FSQ();

	// Load the default palette
	palette = *RSPalette::LoadFromFile("PALETTE.IFF").GetColorPalette();

	lightDir = Vector3Normalize({ 1, 1, 1 });

	texWhite = rlt::CreateColorTexture(4, 4, WHITE);
	texWater = rlt::CreateColorTexture(4, 4, BLUE);
	texGrass = rlt::CreateColorTexture(4, 4, GREEN);

	{
		texCloudDensity = LoadTexture("assets/clouddensity00.png");
		GenTextureMipmaps(&texCloudDensity);
		SetTextureFilter(texCloudDensity, TEXTURE_FILTER_BILINEAR);
		SetTextureWrap(texCloudDensity, TEXTURE_WRAP_REPEAT);
	}

	{
		texDetails = LoadTexture("assets/clouddensity00.png");
		GenTextureMipmaps(&texDetails);
		SetTextureFilter(texDetails, TEXTURE_FILTER_BILINEAR);
		SetTextureWrap(texDetails, TEXTURE_WRAP_REPEAT);
	}

	/*
	std::vector<uint32_t> pixels = { 0xffffffffu };
	white = MakeImage(1, 1, SG_PIXELFORMAT_RGBA8, SG_USAGE_IMMUTABLE, 0, pixels);
	noise = LoadDDS("assets/noise.dds").value_or(white);
	*/

	/*
	std::array<Vector2, 7> seeds;
	for (Vector2& v : seeds) {
		const float a = HMM_PI32 * 2.0f * float(rand() % 1024) / 1023.0f;
		v = HMM_NormalizeVec2({ cosf(a), sinf(a) });
	}

	const size_t sz = 32;
	const float cf = 1.0f / sqrtf(2.0f);
	std::array<Vector2, sz * sz> gradients {};
	for (Vector2& g : gradients) {
		const float a = HMM_PI32 * 2.0f * float(rand() % 1024) / 1023.0f;
		g = cf * HMM_NormalizeVec2({ cosf(a), sinf(a) });
	}
	skydome = MakeSkyDome(1024, 1024, [&](const Vector3& d) {
		return FractalNoiseSkyDome(d, seeds, PerlinNoise<sz>, gradients);
	});
	*/
}

void SCRenderer::ClearCache()
{
	/*
	cacheEntityToModel.Clear(&ModelRenderData::ReleaseMesh);
	cacheBlockToModel.Clear(&GroundRenderData::ReleaseMesh);
	*/
}

void SCRenderer::Release()
{
	ClearCache();
}

void SCRenderer::Log(const char* tag, uint32_t log_level, uint32_t log_item_id,
	const char* message_or_null, uint32_t line_nr,
	const char* filename_or_null, void* user_data)
{
	printf("toto: %d %s %d %s\n", log_item_id,
		message_or_null ? message_or_null : "null", line_nr,
		filename_or_null ? filename_or_null : "null");
	exit(-1);
}

namespace {
RenderTexture2D LoadRenderTextureDepthTex(int width, int height, Texture& depth)
{
	RenderTexture2D target = { 0 };

	target.id = rlLoadFramebuffer();

	if (target.id > 0) {
		rlEnableFramebuffer(target.id);

		target.texture.id = rlLoadTexture(0, width, height, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, 1);
		target.texture.width = width;
		target.texture.height = height;
		target.texture.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
		target.texture.mipmaps = 1;

		depth.id = rlLoadTexture(0, width, height, PIXELFORMAT_UNCOMPRESSED_R32, 1);
		depth.width = width;
		depth.height = height;
		depth.format = PIXELFORMAT_UNCOMPRESSED_R32;
		depth.mipmaps = 1;

		target.depth.id = rlLoadTextureDepth(width, height, false);
		target.depth.width = width;
		target.depth.height = height;
		target.depth.format = 19; // DEPTH_COMPONENT_24BIT: Not defined in raylib
		target.depth.mipmaps = 1;

		rlActiveDrawBuffers(2);

		rlFramebufferAttach(target.id, target.texture.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);
		rlFramebufferAttach(target.id, depth.id, RL_ATTACHMENT_COLOR_CHANNEL1, RL_ATTACHMENT_TEXTURE2D, 0);
		rlFramebufferAttach(target.id, target.depth.id, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_TEXTURE2D, 0);

		if (rlFramebufferComplete(target.id))
			TRACELOG(LOG_INFO, "FBO: [ID %i] Framebuffer object created successfully", target.id);

		rlDisableFramebuffer();
	} else {
		TRACELOG(LOG_WARNING, "FBO: Framebuffer object can not be created");
	}

	return target;
}

void UnloadRenderTextureDepthTex(RenderTexture2D target, Texture& depth)
{
	if (depth.id > 0) {
		rlUnloadTexture(depth.id);
		depth = {};
	}
	if (target.id > 0) {
		rlUnloadTexture(target.texture.id);
		rlUnloadTexture(target.depth.id);
		rlUnloadFramebuffer(target.id);
		target = {};
	}
}

}

void SCRenderer::Draw3D(const Render3DParams& params,
	std::function<void(const Render3DParams& params)>&& f)
{
	const auto [w, h] = std::pair { GetScreenWidth(), GetScreenHeight() };

	if (rtScene.texture.width != w || rtScene.texture.height != h) {
		if (rtScene.texture.width != 0 && rtScene.texture.height != 0)
			UnloadRenderTextureDepthTex(rtScene, rtScene_Depth);
		rtScene = LoadRenderTextureDepthTex(w, h, rtScene_Depth);
	}

	BeginShaderMode(shdModel);
	rlt::SetUniform(shdModel, SHADER_LOC_CAMERA_INFO, &params.camPos, SHADER_UNIFORM_VEC3);
	EndShaderMode();
	BeginShaderMode(shdGroundColor);
	rlt::SetUniform(shdGroundColor, SHADER_LOC_CAMERA_INFO, &params.camPos, SHADER_UNIFORM_VEC3);
	EndShaderMode();
	BeginShaderMode(shdGroundTex);
	rlt::SetUniform(shdGroundTex, SHADER_LOC_CAMERA_INFO, &params.camPos, SHADER_UNIFORM_VEC3);
	EndShaderMode();

	BeginTextureMode(rtScene);
	BeginMode3D(params.camera);

	if (params.flags & Render3DParams::CLEAR_COLORS) {
		ClearBackground({ 255, 0, 0, 0 });
	}

	f(params);

	EndMode3D();
	EndTextureMode();

	if (params.flags & Render3DParams::SKY) {
		Camera3D cpp = params.camera;
		// cpp.target += params.camPos;
		// cpp.position += params.camPos;
		BeginMode3D(cpp);
		const auto t = rlt::GetCameraTransform(params.camera, w, h);
		auto m = LoadMaterialDefault();
		m.shader = shdSky;
		m.maps[MATERIAL_MAP_OCCLUSION].texture = texCloudDensity;
		m.maps[MATERIAL_MAP_ALBEDO].texture = rtScene.texture;
		m.maps[MATERIAL_MAP_HEIGHT].texture = rtScene_Depth;
		BeginShaderMode(shdSky);
		rlt::SetUniform(shdSky, SHADER_LOC_CAMERA_INFO, &params.camPos, SHADER_UNIFORM_VEC3);
		EndShaderMode();
		DrawMesh(mshFsq, m, t);
		EndMode3D();
	} else {
		rlt::DrawRt(rtScene, w, h);
	}
}

void SCRenderer::UpdateBitmapQuad(Color* data, uint32_t width, uint32_t height, float fade)
{
	if (texScreen.width != width || texScreen.height != height) {
		if (texScreen.width != 0)
			UnloadTexture(texScreen);
		Image img = GenImageColor(width, height, WHITE);
		texScreen = LoadTextureFromImage(img);
		UnloadImage(img);
	}
	UpdateTexture(texScreen, data);
	DrawTexturePro(texScreen, { 0, 0, float(texScreen.width), float(texScreen.height) }, { 0, 0, float(GetScreenWidth()), float(GetScreenHeight()) }, {}, {}, WHITE);
}

bool SCRenderer::UploadTextureContentToGPU(RSTexture* texture)
{
	UpdateTexture(texture->tex, texture->img.data);
	return true;
}

Vector3 SCRenderer::GetNormal(const RSEntity* object,
	const Triangle* triangle)
{
	return GetNormal(object->vertices[triangle->ids[0]],
		object->vertices[triangle->ids[1]],
		object->vertices[triangle->ids[2]]);
}

Vector3 SCRenderer::GetNormal(const Vector3& v0, const Vector3& v1, const Vector3& v2)
{
	const Vector3 edge1 = v0 - v1;
	const Vector3 edge2 = v2 - v1;
	return Vector3Normalize(Vector3CrossProduct(edge1, edge2));
}

void PrepareModel(SCRenderer& r, const RSEntity* object, size_t lodLevel, Model& mdata)
{
	const Lod& lod = object->lods[lodLevel];

	struct VKey {
		Vector2 uv {};
		uint32_t id {};
		uint8_t col {};
		uint8_t prop {};
		uint8_t usetex {};
		uint8_t flipN {};
		bool operator<(const VKey& other) const
		{
			if (id != other.id)
				return id < other.id;
			if (col != other.col)
				return col < other.col;
			if (prop != other.prop)
				return prop < other.prop;
			if (usetex != other.usetex)
				return usetex < other.usetex;
			if (flipN != other.flipN)
				return flipN < other.flipN;
			if (uv.x != other.uv.x)
				return uv.x < other.uv.x;
			if (uv.y != other.uv.y)
				return uv.y < other.uv.y;
			return false;
		}
	};

	struct MeshData : public BaseMeshData {
		std::map<VKey, uint16_t> lookup {};
		size_t total = 0;
	};

	const auto computeNormals = [](MeshData& data) {
		const auto& pos = data.vertexData.pos;
		auto& normal = data.vertexData.normal;
		std::vector<float> count(pos.size(), 0.0f);
		for (int i = 0; i < data.indice.size(); i += 3) {
			const uint16_t v0 = data.indice.at(i + 0);
			const uint16_t v1 = data.indice.at(i + 1);
			const uint16_t v2 = data.indice.at(i + 2);
			const Vector3 n = SCRenderer::GetNormal(pos[v0], pos[v1], pos[v2]);
			normal[v0] += n;
			normal[v1] += n;
			normal[v2] += n;
			count[v0] += 1.0f;
			count[v1] += 1.0f;
			count[v2] += 1.0f;
		}
		for (int i = 0; i < normal.size(); ++i) {
			Vector3 an = normal[i] / count[i];
			// assert(HMM_LengthVec3(an) > 0.001);
			normal[i] = Vector3Normalize(an);
		}
	};

	const auto colFromProp = [&](uint8_t prop, uint8_t useTex, const Color& tx) -> Color {
		if (!useTex) {
			if (prop == 6 || prop == 9)
				return { 255, 0, 255, 0 };
		}
		if (useTex)
			return { 255, 255, 255, 255 };
		if (prop == 2)
			return { tx.r, tx.g, tx.b, 128 };
		return { tx.r, tx.g, tx.b, tx.a };
	};

	const auto clock0FromProp = [&](uint8_t prop) {
		if ((prop & 2) != 0)
			return true;
		return true; // really???
	};

	const auto clock1FromProp = [&](uint8_t prop) {
		if ((prop & 2) != 0)
			return true;
		return (prop & 1) != 0;
	};

	auto resolveVertex = [&](MeshData& currentData, uint32_t index,
							 uint8_t colIdx, uint8_t useTex, uint8_t prop,
							 uint8_t flipN, Vector2 uv) -> uint16_t {
		++currentData.total;
		const VKey key { uv, index, colIdx, prop, useTex, flipN };
		uint16_t& res = currentData.lookup[key];
		if (res == 0) {
			res = currentData.vertexData.pos.size() + 1;
			const Color& tx = r.GetPalette().GetRGBColor(colIdx);
			currentData.vertexData.pos.push_back(object->vertices.at(index));
			currentData.vertexData.normal.push_back(Vector3 { 0, 0, 0 });
			currentData.vertexData.uv.push_back(uv);
			currentData.vertexData.col.push_back(colFromProp(prop, useTex, tx));
		}
		assert(currentData.vertexData.uv[res - 1] == uv);
		return res - 1;
	};

	std::vector<std::pair<Mesh, Texture*>> meshes;
	meshes.emplace_back(); // always reserve vertex color at index 0

	std::array<int, 256> propCount0 {};

	if (lodLevel == 0) {
		std::map<Texture*, std::unique_ptr<MeshData>, CompTexture> textureData;
		for (const uvxyEntry& textInfo : object->uvs) {
			// Seems we have a textureID that we don't have :( !
			if (textInfo.textureID >= object->images.size())
				continue;

			auto& image = object->images[textInfo.textureID];
			RSTexture* texture = image->GetTexture();
			auto& d = textureData[&(texture->tex)];
			if (!d)
				d = std::make_unique<MeshData>();
			const Triangle& tri = object->triangles[textInfo.triangleID];
			++propCount0.at(tri.property);

			const bool clock0 = clock0FromProp(tri.property);
			const bool clock1 = clock1FromProp(tri.property);

			std::array<Vector2, 3> uvs;
			for (int j = 0; j < 3; j++) {
				const float u = (textInfo.uvs.at(j).u + 0.5f) / (float)(texture->img.width);
				const float v = (textInfo.uvs.at(j).v + 0.5f) / (float)(texture->img.height);
				uvs.at(j) = { u, v };
			}

			if (clock0) {
				std::array<uint16_t, 3> vid;
				for (int j = 0; j < 3; j++)
					vid.at(j) = resolveVertex(*d, tri.ids.at(j), tri.color, 1, tri.property, 0, uvs.at(j));
				d->indice.push_back(vid[0]);
				d->indice.push_back(vid[2]);
				d->indice.push_back(vid[1]);
			}

			if (clock1) {
				std::array<uint16_t, 3> vid;
				for (int j = 0; j < 3; j++)
					vid.at(j) = resolveVertex(*d, tri.ids.at(j), tri.color, 1, tri.property, 1, uvs.at(j));
				d->indice.push_back(vid[0]);
				d->indice.push_back(vid[1]);
				d->indice.push_back(vid[2]);
			}
		}

		for (auto& kv : textureData) {
			if (!kv.second->indice.empty()) {
				// const bool opt = kv.second->total != kv.second->vertexData.pos.size();
				//  printf("opt: %s...\n", opt ? "yes" : "no");
				auto& [msh, tex] = meshes.emplace_back();
				tex = kv.first;
				computeNormals(*kv.second);
				msh = rlt::MakeMesh(
					kv.second->vertexData.pos,
					kv.second->vertexData.normal,
					kv.second->vertexData.uv,
					kv.second->vertexData.col,
					kv.second->indice);
				// msh.texture = { kv.first };
			}
		}
	}

	int propCount1[256] {};

	MeshData opaque;
	MeshData blend;
	for (int i = 0; i < lod.numTriangles; i++) {
		const uint16_t triangleID = lod.triangleIDs[i];
		const Triangle& tri = object->triangles[triangleID];
		MeshData& d = tri.property == RSEntity::TRANSPARENT ? blend : opaque;
		++propCount1[tri.property];

		const bool clock0 = clock0FromProp(tri.property);
		const bool clock1 = clock1FromProp(tri.property);

		if (clock0) {
			std::array<uint16_t, 3> vid;
			for (int j = 0; j < 3; j++)
				vid.at(j) = resolveVertex(d, tri.ids.at(j), tri.color, 0, tri.property, 0, { 0.5f, 0.5f });
			d.indice.push_back(vid[0]);
			d.indice.push_back(vid[2]);
			d.indice.push_back(vid[1]);
		}

		if (clock1) {
			std::array<uint16_t, 3> vid;
			for (int j = 0; j < 3; j++)
				vid.at(j) = resolveVertex(d, tri.ids.at(j), tri.color, 0, tri.property, 1, { 0.5f, 0.5f });
			d.indice.push_back(vid[0]);
			d.indice.push_back(vid[1]);
			d.indice.push_back(vid[2]);
		}
	}

	{
		auto& [mshOpaque, tex] = meshes[0];
		if (!opaque.indice.empty()) {
			// const bool opt = opaque.total != opaque.vertexData.pos.size();
			//  printf("opt: %s...\n", opt ? "yes" : "no");
			computeNormals(opaque);

			if (!object->name.empty())
				TestSaveModel(object->name, opaque);

			mshOpaque = rlt::MakeMesh(
				opaque.vertexData.pos,
				opaque.vertexData.normal,
				opaque.vertexData.uv,
				opaque.vertexData.col,
				opaque.indice);
			// mshOpaque.texture = white.img;
		}
	}

	{
		auto& [mshBlend, tex] = meshes.emplace_back();
		if (!blend.indice.empty()) {
			// const bool opt = blend.total != blend.vertexData.pos.size();
			//  printf("opt: %s...\n", opt ? "yes" : "no");
			computeNormals(blend);
			mshBlend = rlt::MakeMesh(
				blend.vertexData.pos,
				blend.vertexData.normal,
				blend.vertexData.uv,
				blend.vertexData.col,
				blend.indice);
			// mshBlend.texture = white.img;
		}
	}

	std::vector<Mesh> finalMeshes;
	std::vector<Material> materials;
	std::vector<int> mshMat;
	for (const auto& [msh, tex] : meshes) {
		finalMeshes.push_back(msh);
		Material& m = materials.emplace_back(LoadMaterialDefault());
		m.shader = shdModel;
		if (tex)
			m.maps[MATERIAL_MAP_ALBEDO].texture = *tex;
		mshMat.push_back(int(mshMat.size()));
	}

	mdata = {};
	mdata.transform = MatrixIdentity();
	mdata.meshCount = meshes.size();
	mdata.materialCount = materials.size();
	mdata.meshes = rlt::AllocCopy<Mesh>(finalMeshes);
	mdata.materials = rlt::AllocCopy<Material>(materials);
	mdata.meshMaterial = rlt::AllocCopy<int>(mshMat);

	/*
	printf("prop0\n");
	for (int i = 0; i < 256; ++i) {
			int c = propCount0[i];
			if (c != 0)
					printf("\t- %d / count: %d\n", i, c);
	}

	printf("prop1\n");
	for (int i = 0; i < 256; ++i) {
			int c = propCount1[i];
			if (c != 0)
					printf("\t- %d / count: %d\n", i, c);
	}
	*/
}

void SCRenderer::DrawModel(const RSEntity* object, size_t lodLevel, const Matrix& world)
{
	if (object == nullptr || lodLevel >= object->lods.size())
		return;

	Model& model = cacheEntityToModel.GetData(
		object, [&](const RSEntity* o, Model& tmp) {
			PrepareModel(*this, o, lodLevel, tmp);
		});

	for (int i = 0; i < model.meshCount; ++i)
		DrawMesh(model.meshes[i], model.materials[model.meshMaterial[i]], world);
}

void SCRenderer::SetLight(const Vector3& l)
{
	this->lightDir = l;
}

void SCRenderer::Prepare(RSEntity* object)
{
	for (auto&& img : object->images)
		img->SyncTexture();
	object->prepared = true;
}

bool SCRenderer::IsTextured(const MapVertex* tri0, const MapVertex* tri1,
	const MapVertex* tri2)
{
	return
		// tri0->type != tri1->type ||
		// tri0->type != tri2->type ||
		tri0->upperImageID == 0xFF || tri0->lowerImageID == 0xFF;
}

constexpr int LOWER_TRIANGE = 0;
constexpr int UPPER_TRIANGE = 1;

void SCRenderer::RenderTexturedTriangle(
	const AddVertex& vfunc, const RSArea& area, const MapVertex& tri0,
	const MapVertex& tri1, const MapVertex& tri2, int triangleType)
{
	constexpr float TEX_ZERO = 0.0f;
	constexpr float TEX_ONE = 1.0f;
	// What is this offset ? It is used to get rid of the red delimitations
	// in the 64x64 textures.
	constexpr float OFFSET = (1.1f / 64.0f);
	constexpr Vector2 textTrianCoo64[2][3] = {
		{
			{ TEX_ZERO, TEX_ZERO + OFFSET },
			{ TEX_ONE - 2 * OFFSET, TEX_ONE - OFFSET },
			{ TEX_ZERO, TEX_ONE - OFFSET },
		}, // LOWER_TRIANGE
		{
			{ TEX_ZERO + 2 * OFFSET, TEX_ZERO + OFFSET },
			{ TEX_ONE, TEX_ZERO + OFFSET },
			{ TEX_ONE, TEX_ONE - OFFSET },
		} // UPPER_TRIANGE
	};
	constexpr Vector2 textTrianCoo[2][3] = {
		{
			{ TEX_ZERO, TEX_ZERO },
			{ TEX_ONE, TEX_ONE },
			{ TEX_ZERO, TEX_ONE },
		}, // LOWER_TRIANGE
		{
			{ TEX_ZERO, TEX_ZERO },
			{ TEX_ONE, TEX_ZERO },
			{ TEX_ONE, TEX_ONE },
		} // UPPER_TRIANGE
	};

	RSImage* image = NULL;
	if (triangleType == LOWER_TRIANGE)
		image = area.GetImageByID(tri0.lowerImageID);
	if (triangleType == UPPER_TRIANGE)
		image = area.GetImageByID(tri0.upperImageID);

	if (image == NULL) {
		assert(false);
		printf("This should never happen: Put a break point here.\n");
		return;
	}

	// switch tex coord depending on texture size
	const bool is64 = image->width == 64;
	const auto& ttc = is64 ? textTrianCoo64 : textTrianCoo;

	auto& tex = image->GetTexture()->tex;
	vfunc(tex, 0, tri0.v, tri0.n, WHITE, ttc[triangleType][0]);
	vfunc(tex, 0, tri1.v, tri1.n, WHITE, ttc[triangleType][1]);
	vfunc(tex, 0, tri2.v, tri2.n, WHITE, ttc[triangleType][2]);
}

void SCRenderer::RenderColoredTriangle(const AddVertex& vfunc,
	const MapVertex& tri0,
	const MapVertex& tri1,
	const MapVertex& tri2)
{
	const Vector2 noUv { 0.5f, 0.5f };
	if (tri0.type != tri1.type || tri0.type != tri2.type) {
		const MapVertex* tri {};
		if (tri1.type > tri0.type)
			if (tri1.type > tri2.type)
				tri = &tri1;
			else
				tri = &tri2;
		else if (tri0.type > tri2.type)
			tri = &tri0;
		else
			tri = &tri2;
		Texture* tex = &texWhite;
		vfunc(*tex, tri->type, tri0.v, tri0.n, tri->color, noUv);
		vfunc(*tex, tri->type, tri1.v, tri1.n, tri->color, noUv);
		vfunc(*tex, tri->type, tri2.v, tri2.n, tri->color, noUv);
	} else {
		Texture* tex = &texWhite;
		vfunc(*tex, tri0.type, tri0.v, tri0.n, tri0.color, noUv);
		vfunc(*tex, tri0.type, tri1.v, tri1.n, tri1.color, noUv);
		vfunc(*tex, tri0.type, tri2.v, tri2.n, tri2.color, noUv);
	}
}

void SCRenderer::RenderQuad(const AddVertex& vfunc, const RSArea& area,
	const MapVertex& currentVertex,
	const MapVertex& rightVertex,
	const MapVertex& bottomRightVertex,
	const MapVertex& bottomVertex, bool renderTexture)
{
	if (!renderTexture) {
		// if (currentVertex.lowerImageID == 0xFF)
		RenderColoredTriangle(vfunc, currentVertex, bottomRightVertex, bottomVertex);
		// if (currentVertex.upperImageID == 0xFF )
		RenderColoredTriangle(vfunc, currentVertex, rightVertex, bottomRightVertex);
	} else {
		if (currentVertex.lowerImageID != 0xFF)
			RenderTexturedTriangle(vfunc, area, currentVertex, bottomRightVertex, bottomVertex, LOWER_TRIANGE);
		if (currentVertex.upperImageID != 0xFF)
			RenderTexturedTriangle(vfunc, area, currentVertex, rightVertex, bottomRightVertex, UPPER_TRIANGE);
	}
}

void SCRenderer::RenderBlock(const AddVertex& vfunc, const RSArea& area,
	int LOD, int i, bool renderTexture)
{
	const AreaBlock& block = area.GetAreaBlockByID(LOD, i);
	const uint32_t sideSize = block.sideSize;

	for (size_t x = 0; x < sideSize - 1; x++) {
		for (size_t y = 0; y < sideSize - 1; y++) {
			const MapVertex& currentVertex = block.vertice[x + y * sideSize];
			const MapVertex& rightVertex = block.vertice[(x + 1) + y * sideSize];
			const MapVertex& bottomRightVertex = block.vertice[(x + 1) + (y + 1) * sideSize];
			const MapVertex& bottomVertex = block.vertice[x + (y + 1) * sideSize];
			RenderQuad(vfunc, area, currentVertex, rightVertex, bottomRightVertex, bottomVertex, renderTexture);
		}
	}

	// Inter-block right side
	if (i % 18 != 17) {
		const AreaBlock& currentBlock = block;
		const AreaBlock& rightBlock = area.GetAreaBlockByID(LOD, i + 1);
		for (int y = 0; y < sideSize - 1; y++) {
			const MapVertex& currentVertex = *currentBlock.GetVertice(currentBlock.sideSize - 1, y);
			const MapVertex& rightVertex = *rightBlock.GetVertice(0, y);
			const MapVertex& bottomRightVertex = *rightBlock.GetVertice(0, y + 1);
			const MapVertex& bottomVertex = *currentBlock.GetVertice(currentBlock.sideSize - 1, y + 1);
			RenderQuad(vfunc, area, currentVertex, rightVertex, bottomRightVertex, bottomVertex, renderTexture);
		}
	}

	// Inter-block bottom side
	if (i / 18 != 17) {
		const AreaBlock& currentBlock = block;
		const AreaBlock& bottomBlock = area.GetAreaBlockByID(LOD, i + BLOCK_PER_MAP_SIDE);
		for (int x = 0; x < sideSize - 1; x++) {
			const MapVertex& currentVertex = *currentBlock.GetVertice(x, currentBlock.sideSize - 1);
			const MapVertex& rightVertex = *currentBlock.GetVertice(x + 1, currentBlock.sideSize - 1);
			const MapVertex& bottomRightVertex = *bottomBlock.GetVertice(x + 1, 0);
			const MapVertex& bottomVertex = *bottomBlock.GetVertice(x, 0);
			RenderQuad(vfunc, area, currentVertex, rightVertex, bottomRightVertex, bottomVertex, renderTexture);
		}
	}

	// Inter bottom-right quad
	if (i % 18 != 17 && i / 18 != 17) {
		const AreaBlock& currentBlock = block;
		const AreaBlock& rightBlock = area.GetAreaBlockByID(LOD, i + 1);
		const AreaBlock& rightBottonBlock = area.GetAreaBlockByID(LOD, i + 1 + BLOCK_PER_MAP_SIDE);
		const AreaBlock& bottomBlock = area.GetAreaBlockByID(LOD, i + BLOCK_PER_MAP_SIDE);
		const MapVertex& currentVertex = *currentBlock.GetVertice(currentBlock.sideSize - 1, currentBlock.sideSize - 1);
		const MapVertex& rightVertex = *rightBlock.GetVertice(0, currentBlock.sideSize - 1);
		const MapVertex& bottomRightVertex = *rightBottonBlock.GetVertice(0, 0);
		const MapVertex& bottomVertex = *bottomBlock.GetVertice(currentBlock.sideSize - 1, 0);
		RenderQuad(vfunc, area, currentVertex, rightVertex, bottomRightVertex, bottomVertex, renderTexture);
	}
}

void SCRenderer::RenderWorldSolid(const Render3DParams& params, const RSArea& area, int LOD, double gtime)
{
	RenderWorldModels(params, area, LOD, gtime);
	RenderWorldGround(params, area, LOD, gtime);
}

void SCRenderer::RenderWorldGround(const Render3DParams& params, const RSArea& area, int LOD, double gtime)
{
	std::set<uint8_t> triTypes;

	static std::vector<Model> ground;
	ground.resize(0);
	for (int i = 0; i < BLOCKS_PER_MAP; i++) {
		const AreaBlock& block = area.GetAreaBlockByID(LOD, i);
		const Model& model = cacheBlockToModel.GetData(
			&block, [&](const AreaBlock* block, Model& mdata) {
				struct AreaVertex {
					Vector3 pos;
					Vector3 normal;
					Vector2 uv;
					std::array<uint8_t, 4> col;
				};

				const float maxv = std::numeric_limits<float>::max();
				const float minv = std::numeric_limits<float>::min();
				Vector4 aabb { maxv, maxv, minv, minv };

				using BlockCache = std::map<Texture*, ObjVertexData>;
				BlockCache tmp;
				AddVertex vadd = [&](Texture& tex, uint8_t tritype, const Vector3& pos, const Vector3& n, Color col, const Vector2& uv) {
					auto& vert = tmp[&tex];
					const bool useVertex = true;
					if (useVertex) {
						triTypes.insert(tritype);
						aabb.x = std::min(aabb.x, pos.x);
						aabb.y = std::min(aabb.y, pos.z);
						aabb.z = std::min(aabb.z, pos.x);
						aabb.w = std::min(aabb.w, pos.z);
						vert.pos.push_back(pos);
						vert.normal.push_back(n);
						vert.uv.push_back(uv);
						vert.col.push_back({ col.r, col.g, col.b, col.a });
					}
				};

				RenderBlock(vadd, area, LOD, i, false);
				RenderBlock(vadd, area, LOD, i, true);

				const Vector3 center { 0.5f * (aabb.x + aabb.z), 0, 0.5f * (aabb.y + aabb.w) };

				for (auto& kv : tmp) {
					auto& data = kv.second;
					for (Vector3& p : data.pos)
						p -= center;
				}

				std::vector<Mesh> finalMeshes;
				std::vector<Material> materials;
				std::vector<int> mshMat;

				{
					const auto& data = tmp[&texWhite];
					if (data.pos.size() != 0) {
						std::vector<uint16_t> indices(data.pos.size());
						for (size_t i = 0; i < indices.size(); i += 3) {
							indices[i] = uint16_t(i + 1);
							indices[i + 1] = uint16_t(i);
							indices[i + 2] = uint16_t(i + 2);
						}
						finalMeshes.push_back(rlt::MakeMesh(data.pos, data.normal, data.uv, data.col, indices));
						Material& m = materials.emplace_back(LoadMaterialDefault());
						m.shader = shdGroundColor;
						m.maps[MATERIAL_MAP_ALBEDO].texture = texWhite;
						m.maps[MATERIAL_MAP_OCCLUSION].texture = texDetails;
						mshMat.push_back(int(mshMat.size()));
					}
				}

				for (const auto& kv : tmp) {
					if (kv.first == &texWhite)
						continue;
					const auto& data = kv.second;
					if (data.pos.size() != 0) {
						std::vector<uint16_t> indices(data.pos.size());
						for (size_t i = 0; i < indices.size(); i += 3) {
							indices[i] = uint16_t(i + 1);
							indices[i + 1] = uint16_t(i);
							indices[i + 2] = uint16_t(i + 2);
						}
						finalMeshes.push_back(rlt::MakeMesh(data.pos, data.normal, data.uv, data.col, indices));
						Material& m = materials.emplace_back(LoadMaterialDefault());
						m.shader = shdGroundTex;
						m.maps[MATERIAL_MAP_ALBEDO].texture = *kv.first;
						mshMat.push_back(int(mshMat.size()));
					}
				}

				mdata = {};
				mdata.transform = MatrixTranslate(center.x, center.y, center.z);
				mdata.meshCount = finalMeshes.size();
				mdata.materialCount = materials.size();
				mdata.meshes = rlt::AllocCopy<Mesh>(finalMeshes);
				mdata.materials = rlt::AllocCopy<Material>(materials);
				mdata.meshMaterial = rlt::AllocCopy<int>(mshMat);
			});

		ground.push_back(model);
	}

	if (!ground.empty()) {
		auto p = params.camPos * -1;
		auto world = MatrixTranslate(p.x, p.y, p.z);
		for (const auto& model : ground) {
			for (int i = 0; i < model.meshCount; ++i) {
				auto& mat = model.materials[model.meshMaterial[i]];
				DrawMesh(model.meshes[i], mat, model.transform * world);
			}
		}
	}
}

void SCRenderer::RenderWorldModels(const Render3DParams& params, const RSArea& area, int LOD, double gtime)
{
#if 1
	const auto& userInts = UserProperties::Get().Ints;
	const auto& userFloats = UserProperties::Get().Floats;

	const float ofs0 = userFloats.Get("BlockObjOfsX", 1.0f);
	const float ofs1 = userFloats.Get("BlockObjOfsZ", 1.0f);
	const float factorX = userFloats.Get("BlockObjFactorX", -1.0f);
	const float factorZ = userFloats.Get("BlockObjFactorZ", -1.0f);
	const float objScale = userFloats.Get("BlockObjScale", OBJECT_SCALE);

	const int axisX = userInts.Get("BlockObjX", 1);
	const int axisZ = userInts.Get("BlockObjZ", 0);

	// Render objects on the map
	for (int id = 0; id < BLOCKS_PER_MAP; id++) {
		const std::vector<MapObject>& objects = area.objects[id];

		const float bx = float(id % 18) + ofs0;
		const float by = float(id / 18) + ofs1;

		const Vector3 offset = {
			bx * BLOCK_WIDTH,
			// area.elevation[id] / (2 * (float)HEIGHT_DIVIDER),
			2.0f * area.elevation[id] / (float)(1 << 8),
			by * BLOCK_WIDTH,
		};

		for (const MapObject& object : objects) {
			// const float factorXZ = BLOCK_WIDTH;
			const Vector3 localDelta = {
				object.position[axisX] * factorX,
				0.5f * object.position[2] / (float)HEIGHT_DIVIDER,
				object.position[axisZ] * factorZ,
			};
			auto p = params.camPos * -1;
			const Vector3 wp = localDelta + offset + p;
			const auto& t = object.transform;
			const auto s = objScale;
			const Matrix mworld = rlt::MakeMat4({
				{ s * t[0][0], s * t[1][0], s * t[2][0], wp.x },
				{ s * t[0][1], s * t[1][1], s * t[2][1], wp.y },
				{ s * t[0][2], s * t[1][2], s * t[2][2], wp.z },
				{ 0, 0, 0, 1 },
			});
			DrawModel(object.entity, LOD_LEVEL_MAX, mworld);
		}
	}
#endif
}

#if USE_SHADER_PIPELINE != 1

void SCRenderer::RenderObjects(const RSArea& area, size_t blockID)
{
	float color[3] = { 1, 0, 0 };

	const std::vector<MapObject>* objects = &area.objects[blockID];

	glColor3fv(color);
	glPointSize(3);
	glDisable(GL_DEPTH_TEST);
	glBegin(GL_POINTS);

	for (size_t i = 8; i < 9; i++) {
		MapObject object = objects->at(i);

		int32_t offset[3];
		offset[0] = blockID % 18 * BLOCK_WIDTH;
		offset[1] = area.elevation[blockID];
		offset[2] = (int32_t)blockID / 18 * BLOCK_WIDTH;

		/*
		glVertex3d(object.position[0]/255*BLOCK_WIDTH+offset[0],
						   object.position[1]+offset[1],
						   object.position[2]/255*BLOCK_WIDTH+offset[2]);
		*/

		int32_t localDelta[3];
		localDelta[0] = object.position[0] / 65355.0f * BLOCK_WIDTH;
		localDelta[1] = object.position[1]; /// HEIGHT_DIVIDER                   ;
		localDelta[2] = object.position[2] / 65355.0f * BLOCK_WIDTH;

		size_t toDraw[3];
		toDraw[0] = localDelta[0] + offset[0];
		toDraw[1] = offset[1];
		toDraw[2] = localDelta[2] + offset[2];

		glVertex3d(toDraw[0], toDraw[1], toDraw[2]);
	}

	glEnd();
}

void SCRenderer::RenderVerticeField(Vector3* vertices, int numVertices)
{
	SetProj(camera.proj);

	running = true;
	float counter = 0;
	while (running) {
		const Vector3 newPosition { 256 * cos(counter), 0, 256 * sin(counter) };
		counter += 0.02;

		camera.SetPosition(newPosition);
		SetView(camera.getView());

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glPointSize(5);
		glBegin(GL_POINTS);
		for (int i = 0; i < numVertices; i++)
			glVertex(vertices[i]);
		glEnd();
	}
}

void SCRenderer::RenderWorldPoints(const RSArea& area, int LOD,
	int verticesPerBlock)
{
	SetProj(camera.proj);

	glPointSize(4);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	running = true;

	static float counter = 0;

	while (running) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glMatrixMode(GL_MODELVIEW);

		Vector3 lookAt = { 256 * 16, 100, 256 * 16 };

		Renderer.GetCamera().LookAt(lookAt);

		Vector3 newPosition = camera.position;
		newPosition.X = lookAt.X + 5256 * cos(counter / 2);
		newPosition.Y = 3700;
		newPosition.Z = lookAt.Z + 5256 * sin(counter / 2);
		camera.SetPosition(newPosition);

		// Matrix* modelViewMatrix = camera.GetViewMatrix();
		// glLoadMatrixf(modelViewMatrix->ToGL());
		glLoadMatrixHMM(camera.getView());

		glBegin(GL_POINTS);
		for (int i = 0; i < 324; i++) {
			// for(int i=96 ; i < 99 ; i++) {
			const AreaBlock& block = area.GetAreaBlockByID(LOD, i);
			for (size_t i = 0; i < verticesPerBlock; i++) {
				const MapVertex* v = &block.vertice[i];
				glColor3fv(v->color);
				glVertex(v->v);
			}
		}
		glEnd();

		// Render objects on the map
		for (int i = 0; i < 324; i++)
			RenderObjects(area, i);
	}
}

#endif
