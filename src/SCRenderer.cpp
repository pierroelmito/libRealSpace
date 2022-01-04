 //
//  gfx.cpp
//  iff
//
//  Created by Fabien Sanglard on 12/21/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

#include <cassert>

#include <algorithm>

#define SOKOL_GLCORE33
#include "sokol_gfx.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#define TINYDDSLOADER_IMPLEMENTATION
#include "tinyddsloader.h"

#include "shaders/shaders.h"

extern GLFWwindow* win;

int ComputeMipCount(int w, int h)
{
	const int m1 = 32;
	const int m0 = std::min(__builtin_clz(w), __builtin_clz(h));
	return m1 - m0;
}

sg_filter GetMinFilter(bool linear, bool mipmaps)
{
	if (linear)
		return mipmaps ? SG_FILTER_LINEAR_MIPMAP_LINEAR : SG_FILTER_LINEAR;
	return mipmaps ? SG_FILTER_NEAREST_MIPMAP_NEAREST : SG_FILTER_NEAREST;
}

sg_filter GetMagFilter(bool linear)
{
	if (linear)
		return SG_FILTER_LINEAR;
	return SG_FILTER_NEAREST;
}

sg_pixel_format GetFormatFromDDS(tinyddsloader::DDSFile::DXGIFormat fmt)
{
	if (fmt == tinyddsloader::DDSFile::DXGIFormat::B8G8R8A8_UNorm)
		return SG_PIXELFORMAT_BGRA8;
	if (fmt == tinyddsloader::DDSFile::DXGIFormat::BC1_UNorm)
		return SG_PIXELFORMAT_BC1_RGBA;
	return SG_PIXELFORMAT_NONE;
}

std::optional<sg_image> LoadDDS(const char* path)
{
	tinyddsloader::DDSFile dds;
	tinyddsloader::Result result = dds.Load(path);
	if (result != tinyddsloader::Result::Success)
		return {};

	sg_pixel_format fmt = GetFormatFromDDS(dds.GetFormat());
	if (fmt == SG_PIXELFORMAT_NONE)
		return {};

	const auto mips = dds.GetMipCount();

	sg_image_desc idesc{};

	idesc.type = SG_IMAGETYPE_2D;
	idesc.render_target = false;
	idesc.width = dds.GetWidth();
	idesc.height = dds.GetHeight();
	idesc.pixel_format = fmt;
	idesc.usage = SG_USAGE_IMMUTABLE;
	idesc.mag_filter = GetMagFilter(true);
	idesc.min_filter = GetMinFilter(true, mips > 1);
	idesc.max_anisotropy = 4;
	idesc.num_mipmaps = mips;
	idesc.wrap_u = SG_WRAP_REPEAT;
	idesc.wrap_v = SG_WRAP_REPEAT;

	for (int i = 0; i < mips; ++i) {
		const tinyddsloader::DDSFile::ImageData* data = dds.GetImageData(i);
		idesc.data.subimage[0][i] = { data->m_mem, data->m_memSlicePitch };
	}

	return sg_make_image(&idesc);
}

template <typename T>
sg_image MakeImage(int w, int h, sg_pixel_format fmt, sg_usage usage, bool mipmaps, const std::vector<T>& data = {})
{
	const int mipcount = mipmaps ? ComputeMipCount(w, h) : 1;

	sg_image_desc idesc{};

	idesc.width = w;
	idesc.height = h;
	idesc.pixel_format = fmt;
	idesc.usage = usage;
	idesc.mag_filter = GetMagFilter(false);
	idesc.min_filter = GetMinFilter(false, mipmaps);
	idesc.num_mipmaps = mipcount;
	idesc.data.subimage[0][0] = { &data[0], w * h * sizeof(T) };

	return sg_make_image(&idesc);
}

sg_image MakeImage(int w, int h, sg_pixel_format fmt, sg_usage usage, bool mipmaps)
{
	const int mipcount = mipmaps ? ComputeMipCount(w, h) : 1;

	sg_image_desc idesc{};

	idesc.width = w;
	idesc.height = h;
	idesc.pixel_format = fmt;
	idesc.usage = usage;
	idesc.mag_filter = GetMagFilter(false);
	idesc.min_filter = GetMinFilter(false, mipmaps);
	idesc.num_mipmaps = mipcount;

	return sg_make_image(&idesc);
}

template <typename T>
sg_buffer MakeBuffer(sg_buffer_type bt, sg_usage usage, const std::vector<T>& data)
{
	sg_buffer_desc vdesc{};
	vdesc.type = bt;
	vdesc.usage = usage;
	vdesc.data = { &data[0], data.size() * sizeof(T) };
	vdesc.size = vdesc.data.size;
	return sg_make_buffer(vdesc);
}

struct ModelRenderData
{
	sg_shader shd{};
	sg_pipeline pip_opaque{};
	sg_pipeline pip_blend{};

	struct MeshItem
	{
		sg_image texture{};
		sg_buffer vbuf{};
		sg_buffer ibuf{};
		int pcount{};
		bool blend{ false };
	};
	using Mesh = std::vector<MeshItem>;

	ModelRenderData()
	{
		shd = sg_make_shader(model_shader_desc(sg_query_backend()));

		{
			sg_pipeline_desc pdesc{};
			pdesc.shader = shd;
			pdesc.layout.attrs[ATTR_model_vs_position].format = SG_VERTEXFORMAT_FLOAT3;
			pdesc.layout.attrs[ATTR_model_vs_normal].format = SG_VERTEXFORMAT_FLOAT3;
			pdesc.layout.attrs[ATTR_model_vs_texcoord].format = SG_VERTEXFORMAT_FLOAT2;
			pdesc.layout.attrs[ATTR_model_vs_vcolor].format = SG_VERTEXFORMAT_UBYTE4N;
			pdesc.index_type = SG_INDEXTYPE_UINT16;
			pdesc.depth.write_enabled = true;
			pdesc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;
			pdesc.cull_mode = SG_CULLMODE_BACK;

			pip_opaque = sg_make_pipeline(&pdesc);

			pdesc.depth.write_enabled = false;
			pdesc.colors[0].blend = {
				true,
				SG_BLENDFACTOR_SRC_ALPHA,
				SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
				SG_BLENDOP_ADD,
				SG_BLENDFACTOR_ZERO,
				SG_BLENDFACTOR_ONE,
				SG_BLENDOP_ADD,
			};

			pip_blend = sg_make_pipeline(&pdesc);
		}
	}
};

struct GroundRenderData
{
	sg_shader shd{};
	sg_pipeline pip{};

	struct MeshItem
	{
		sg_image texture{};
		sg_buffer vbuf{};
		int pcount{};
	};
	using Mesh = std::vector<MeshItem>;

	GroundRenderData()
	{
		shd = sg_make_shader(ground_shader_desc(sg_query_backend()));

		{
			sg_pipeline_desc pdesc{};
			pdesc.shader = shd;
			pdesc.depth.write_enabled = true;
			pdesc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;
			pdesc.layout.attrs[ATTR_ground_vs_position].format = SG_VERTEXFORMAT_FLOAT3;
			pdesc.layout.attrs[ATTR_ground_vs_texcoord].format = SG_VERTEXFORMAT_FLOAT2;
			pdesc.layout.attrs[ATTR_ground_vs_vcolor].format = SG_VERTEXFORMAT_UBYTE4N;
			pdesc.cull_mode = SG_CULLMODE_BACK;
			pip = sg_make_pipeline(&pdesc);
		}
	}
};

struct FullscreenSky
{
	sg_buffer vbuf{};
	sg_shader shd{};
	sg_pipeline pip{};
	sg_bindings bind{};

	FullscreenSky()
	{
		shd = sg_make_shader(sky_shader_desc(sg_query_backend()));

		{
			const float N = -1.0f;
			const float P = -N;
			const float Z = 1.0f;
			const float vertices[] = {
				N, P, Z,
				P, P, Z,
				N, N, Z,
				P, P, Z,
				P, N, Z,
				N, N, Z,
			};
			sg_buffer_desc bdesc{};
			bdesc.data = SG_RANGE(vertices);
			vbuf = sg_make_buffer(&bdesc);
		}

		{
			sg_pipeline_desc pdesc{};
			pdesc.shader = shd;
			pdesc.depth.write_enabled = false;
			pdesc.depth.compare = SG_COMPAREFUNC_EQUAL;
			pdesc.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT3;
			pip = sg_make_pipeline(&pdesc);
		}

		bind.vertex_buffers[0] = vbuf;
	}
};

struct FullscreenBitmapData
{
	sg_buffer vbuf{};
	sg_image img{};
	sg_shader shd{};
	sg_pipeline pip{};
	sg_bindings bind{};

	FullscreenBitmapData(uint32_t width, uint32_t height)
	{
		shd = sg_make_shader(bitmap_shader_desc(sg_query_backend()));

		{
			const float N = -1.0f;
			const float P = -N;
			const float Z = 0.5f;
			const float vertices[] = {
				N, P, Z,
				P, P, Z,
				N, N, Z,
				P, P, Z,
				P, N, Z,
				N, N, Z,
			};
			sg_buffer_desc bdesc{};
			bdesc.data = SG_RANGE(vertices);
			vbuf = sg_make_buffer(&bdesc);
		}

		img = MakeImage(width, height, SG_PIXELFORMAT_RGBA8, SG_USAGE_STREAM, false);

		{
			sg_pipeline_desc pdesc{};
			pdesc.shader = shd;
			pdesc.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT3;
			pip = sg_make_pipeline(&pdesc);
		}

		bind.vertex_buffers[0] = vbuf;
		bind.fs_images[SLOT_fs_bitmap] = img;
	}
};

sg_image noise{};
sg_image white{};

std::optional<FullscreenBitmapData> FbdRender;
std::optional<FullscreenSky> FullscreenSky;
std::optional<ModelRenderData> modelRender;
std::optional<GroundRenderData> groundRender;

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
			fn(ptr, allData.emplace_back());
		}
		return allData[dataIndex - 1];
	}
	const std::vector<V>& AllData() const { return allData; }
protected:
	std::map<const K*, uint32_t> objectToDataIndex;
	std::vector<V> allData;
};

using ModelData = ModelRenderData::Mesh;
using GroundData = GroundRenderData::Mesh;

ObjectCacheManager<RSEntity, ModelData> cacheEntityToModel;
ObjectCacheManager<AreaBlock, GroundData> cacheBlockToModel;

// DIRTY...
const uint32_t PASS_VCOLOR = 0x12345678;
const uint32_t PASS_BLEND = 0x12345679;

SCRenderer::SCRenderer()
: initialized(false)
{
}

SCRenderer::~SCRenderer(){
}

void
SCRenderer::Draw3D(const Render3DParams& params, std::function<void()>&& f)
{
	sg_pass_action pass_action = {0};
	if (!params.clearColors)
		pass_action.colors[0].action = SG_ACTION_DONTCARE;
	int cur_width{}, cur_height{};
	glfwGetFramebufferSize(win, &cur_width, &cur_height);
	sg_begin_default_pass(&pass_action, cur_width, cur_height);

	f();

	sg_end_pass();
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

	camera.SetPersective(50.0f, width / (float)height, 1.0f, 20000.0f);

	lightDir = HMM_NormalizeVec3({ 1, 1, 1 });

	std::vector<uint32_t> pixels = { 0xffffffffu };
	white = MakeImage(1, 1, SG_PIXELFORMAT_RGBA8, SG_USAGE_IMMUTABLE, false, pixels);
	noise = LoadDDS("../assets/noise.dds").value_or(white);
	//noise = white;

	initialized = true;
}

void SCRenderer::UpdateBitmapQuad(Texel* data, uint32_t width, uint32_t height)
{
	if (!FbdRender)
		FbdRender.emplace(width, height);

	sg_image_data idata{};
	idata.subimage[0][0] = { data, width * height * sizeof(Texel) };
	sg_update_image(FbdRender->img, idata);

	int cur_width{}, cur_height{};
	glfwGetFramebufferSize(win, &cur_width, &cur_height);
	sg_pass_action pass_action = {0};
	sg_begin_default_pass(&pass_action, cur_width, cur_height);
	sg_apply_pipeline(FbdRender->pip);
	sg_apply_bindings(&FbdRender->bind);
	sg_draw(0, 6, 1);
	sg_end_pass();
}

void SCRenderer::CreateTextureInGPU(RSTexture* texture)
{
	if (!initialized)
		return;

	sg_image img = MakeImage(texture->width, texture->height, SG_PIXELFORMAT_RGBA8, SG_USAGE_DYNAMIC, false);
	texture->id = img.id;
}

void SCRenderer::UploadTextureContentToGPU(RSTexture* texture)
{
	if (!initialized)
		return;

	const int mips = ComputeMipCount(texture->width, texture->height);

	sg_image_data data;
	std::vector<std::vector<uint32_t>> mipmapData;
	mipmapData.resize(mips);

	data.subimage[0][0] = { texture->data, texture->width * texture->height * 4 };
	/*
	for (int i = 1; i < mips; ++i)
	{
		const size_t nw = std::max<size_t>(1, texture->width >> i);
		const size_t nh = std::max<size_t>(1, texture->height >> i);
		mipmapData[i - 1].resize(nw * nh, 0xffffffffu);
		data.subimage[0][i] = { &mipmapData[0], nw * nh * 4 };
	}
	*/

	sg_update_image({ texture->id }, data);
}

void SCRenderer::DeleteTextureInGPU(RSTexture* texture)
{
	if (!initialized)
		return;
	sg_destroy_image({ texture->id });
}

void SCRenderer::RenderSky()
{
	if (!FullscreenSky)
		FullscreenSky.emplace();

	RSMatrix view = camera.getView();
	RSMatrix proj = camera.getProj();

	sg_apply_pipeline(FullscreenSky->pip);
	sky_vs_params_t params;
	memcpy(&params.view, &view, 64);
	memcpy(&params.proj, &proj, 64);
	memcpy(&params.plightdir, &lightDir, 12);
	sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, { &params, sizeof(params) });
	sg_apply_bindings(&FullscreenSky->bind);
	sg_draw(0, 6, 1);
}

RSVector3 SCRenderer::GetNormal(const RSEntity* object, const Triangle* triangle)
{
	return GetNormal(object->vertices[triangle->ids[0]], object->vertices[triangle->ids[1]], object->vertices[triangle->ids[2]]);
}

RSVector3 SCRenderer::GetNormal(const RSVector3& v0, const RSVector3& v1, const RSVector3& v2)
{
	const RSVector3 edge1 = v0 - v1;
	const RSVector3 edge2 = v2 - v1;
	return HMM_NormalizeVec3(HMM_Cross(edge1, edge2));
}

void PrepareModel(SCRenderer& r, const RSEntity* object, size_t lodLevel, ModelData& mdata)
{
	const Lod& lod = object->lods[lodLevel];

	struct ObjVertex
	{
		RSVector3 pos;
		RSVector3 normal;
		hmm_vec2 uv;
		std::array<uint8_t, 4> col;
	};

	struct VKey
	{
		hmm_vec2 uv;
		uint32_t id;
		uint8_t col;
		uint8_t prop;
		uint8_t usetex;
		uint8_t flipN;
		bool operator< (const VKey& other) const
		{
			if (id != other.id) return id < other.id;
			if (col != other.col) return col < other.col;
			if (prop != other.prop) return prop < other.prop;
			if (usetex != other.usetex) return usetex < other.usetex;
			if (flipN != other.flipN) return flipN < other.flipN;
			if (uv.X != other.uv.X) return uv.X < other.uv.X;
			if (uv.Y != other.uv.Y) return uv.Y < other.uv.Y;
			return false;
		}
	};

	struct MeshData {
		std::map<VKey, uint16_t> lookup;
		std::vector<ObjVertex> vertice;
		std::vector<uint16_t> indice;
		size_t total = 0;
	};

	const auto computeNormals = [] (MeshData& data) {
		std::vector<float> count(data.vertice.size(), 0.0f);
		for (int i = 0; i < data.indice.size(); i += 3) {
			const uint16_t v0 = data.indice[i + 0];
			const uint16_t v1 = data.indice[i + 1];
			const uint16_t v2 = data.indice[i + 2];
			const RSVector3 n = SCRenderer::GetNormal(data.vertice[v0].pos, data.vertice[v1].pos, data.vertice[v2].pos);
			data.vertice[v0].normal += n;
			data.vertice[v1].normal += n;
			data.vertice[v2].normal += n;
			count[v0] += 1.0f;
			count[v1] += 1.0f;
			count[v2] += 1.0f;
		}
		for (int i = 0; i < data.vertice.size(); ++i) {
			data.vertice[i].normal = HMM_NormalizeVec3(data.vertice[i].normal / count[i]);
		}
	};

	const auto colFromProp = [&] (uint8_t prop, uint8_t useTex, const Texel& tx) -> std::array<uint8_t, 4> {
		if (!useTex) {
			if (prop == 6 || prop == 9)
				return { 255, 0, 255, 0 };
		}
		if (useTex)
			return { 255, 255, 255, 255 };
		if (prop == 2)
			return { tx.r, tx.g, tx.b, 64 };
		return { tx.r, tx.g, tx.b, tx.a };
	};

	const auto clock0FromProp = [&] (uint8_t prop) {
		if ((prop & 2) != 0)
			return true;
		return true; // really???
	};

	const auto clock1FromProp = [&] (uint8_t prop) {
		if ((prop & 2) != 0)
			return true;
		return (prop & 1) != 0;
	};

	auto resolveVertex = [&] (MeshData& currentData, uint32_t index, uint8_t colIdx, uint8_t useTex, uint8_t prop, uint8_t flipN, hmm_vec2 uv) -> uint16_t {
		++currentData.total;
		uint16_t& res = currentData.lookup[{ uv, index, colIdx, prop, useTex, flipN }];
		if (res == 0) {
			res = currentData.vertice.size() + 1;
			const Texel* tx = r.GetPalette().GetRGBColor(colIdx);
			currentData.vertice.push_back({ object->vertices[index], { 0.0f, 0.0f, 0.0f }, uv, colFromProp(prop, useTex, *tx) });
		}
		assert(currentData.vertice[res - 1].uv == uv);
		return res - 1;
	};

	mdata.emplace_back();

	int propCount0[256]{};

	if (lodLevel == 0){
		std::map<uint32_t, MeshData> textureData;
		for (const uvxyEntry& textInfo : object->uvs) {
			//Seems we have a textureID that we don't have :( !
			if (textInfo.textureID >= object->images.size())
				continue;

			RSImage* image = object->images[textInfo.textureID];
			const RSTexture* texture = image->GetTexture();
			auto& d = textureData[texture->id];
			const Triangle& tri = object->triangles[textInfo.triangleID];
			++propCount0[tri.property];

			const bool clock0 = clock0FromProp(tri.property);
			const bool clock1 = clock1FromProp(tri.property);

			hmm_vec2 uvs[3];
			for(int j = 0; j < 3; j++){
				const float u = textInfo.uvs[j].u / (float)texture->width;
				const float v = textInfo.uvs[j].v / (float)texture->height;
				uvs[j] = { u, v };
			}

			if (clock0) {
				uint16_t vid[3];
				for(int j = 0; j < 3; j++)
					vid[j] = resolveVertex(d, tri.ids[j], tri.color, 1, tri.property, 0, uvs[j]);
				d.indice.push_back(vid[0]);
				d.indice.push_back(vid[1]);
				d.indice.push_back(vid[2]);
			}

			if (clock1) {
				uint16_t vid[3];
				for(int j = 0; j < 3; j++)
					vid[j] = resolveVertex(d, tri.ids[j], tri.color, 1, tri.property, 1, uvs[j]);
				d.indice.push_back(vid[0]);
				d.indice.push_back(vid[2]);
				d.indice.push_back(vid[1]);
			}
		}

		for (auto& kv : textureData) {
			if (!kv.second.indice.empty()) {
				const bool opt = kv.second.total != kv.second.vertice.size();
				//printf("opt: %s...\n", opt ? "yes" : "no");
				auto& msh = mdata.emplace_back();
				computeNormals(kv.second);
				msh.texture = { kv.first };
				msh.vbuf = MakeBuffer(SG_BUFFERTYPE_VERTEXBUFFER, SG_USAGE_IMMUTABLE, kv.second.vertice);
				msh.ibuf = MakeBuffer(SG_BUFFERTYPE_INDEXBUFFER, SG_USAGE_IMMUTABLE, kv.second.indice);
				msh.pcount = kv.second.indice.size();
			}
		}
	}

	int propCount1[256]{};

	MeshData opaque;
	MeshData blend;
	for (int i = 0 ; i < lod.numTriangles ; i++) {
		uint16_t triangleID = lod.triangleIDs[i];
		const Triangle& tri = object->triangles[triangleID];
		MeshData& d = tri.property == RSEntity::TRANSPARENT ? blend : opaque;
		++propCount1[tri.property];

		const bool clock0 = clock0FromProp(tri.property);
		const bool clock1 = clock1FromProp(tri.property);

		if (clock0) {
			uint16_t vid[3];
			for(int j = 0; j < 3; j++)
				vid[j] = resolveVertex(d, tri.ids[j], tri.color, 0, tri.property, 0, { 0.5f, 0.5f });
			d.indice.push_back(vid[0]);
			d.indice.push_back(vid[1]);
			d.indice.push_back(vid[2]);
		}

		if (clock1) {
			uint16_t vid[3];
			for(int j = 0; j < 3; j++)
				vid[j] = resolveVertex(d, tri.ids[j], tri.color, 0, tri.property, 1, { 0.5f, 0.5f });
			d.indice.push_back(vid[0]);
			d.indice.push_back(vid[2]);
			d.indice.push_back(vid[1]);
		}
	}

	auto& mshOpaque = mdata[0];
	if (!opaque.indice.empty()) {
		const bool opt = opaque.total != opaque.vertice.size();
		//printf("opt: %s...\n", opt ? "yes" : "no");
		computeNormals(opaque);
		mshOpaque.texture = white;
		mshOpaque.vbuf = MakeBuffer(SG_BUFFERTYPE_VERTEXBUFFER, SG_USAGE_IMMUTABLE, opaque.vertice);
		mshOpaque.ibuf = MakeBuffer(SG_BUFFERTYPE_INDEXBUFFER, SG_USAGE_IMMUTABLE, opaque.indice);
		mshOpaque.pcount = opaque.indice.size();
	}

	auto& mshBlend = mdata.emplace_back();
	if (!blend.indice.empty()) {
		const bool opt = blend.total != blend.vertice.size();
		//printf("opt: %s...\n", opt ? "yes" : "no");
		computeNormals(blend);
		mshBlend.texture = white;
		mshBlend.vbuf = MakeBuffer(SG_BUFFERTYPE_VERTEXBUFFER, SG_USAGE_IMMUTABLE, blend.vertice);
		mshBlend.ibuf = MakeBuffer(SG_BUFFERTYPE_INDEXBUFFER, SG_USAGE_IMMUTABLE, blend.indice);
		mshBlend.pcount = blend.indice.size();
		mshBlend.blend = true;
	}

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

void SCRenderer::DrawModel(const RSEntity* object, size_t lodLevel, const RSVector3& pos, float scale, const RSQuaternion& orientation)
{
	if (!initialized)
		return;

	if (object == nullptr)
		return;

	if (lodLevel >= object->lods.size()) {
		const auto maxl = std::min(0UL, object->lods.size() - 1);
		printf("Unable to render this Level Of Details (out of range): Max level is  %lu\n", maxl);
		return;
	}

	RSMatrix world = HMM_QuaternionToMat4(orientation) * HMM_Scale({ scale, scale, scale });
	world.Elements[3][0] = pos.X;
	world.Elements[3][1] = pos.Y;
	world.Elements[3][2] = pos.Z;

	return DrawModel(object, lodLevel, world);
}

void SCRenderer::DrawModel(const RSEntity* object, size_t lodLevel, const RSMatrix world)
{
	if (!initialized)
		return;

	if (object == nullptr)
		return;

	if (lodLevel >= object->lods.size()) {
		const auto maxl = std::min(0UL, object->lods.size() - 1);
		printf("Unable to render this Level Of Details (out of range): Max level is  %lu\n", maxl);
		return;
	}

	const RSVector3 lighDirection = HMM_NormalizeVec3({ 10, 30, 10 });

	if (!modelRender)
		modelRender.emplace();

	ModelData& meshes = cacheEntityToModel.GetData(object, [&] (const RSEntity* o, ModelData& tmp) {
		PrepareModel(*this, o, lodLevel, tmp);
	});

	const RSMatrix& view = camera.getView();
	const RSMatrix& proj = camera.getProj();
	model_vs_params_t params;
	memcpy(params.proj, &proj, 64);
	memcpy(params.view, &view, 64);
	memcpy(params.world, &world, 64);
	memcpy(params.camPos, &camera.getPosition(), 12);
	memcpy(params.lightDir, &lightDir, 12);

	sg_apply_pipeline(modelRender->pip_opaque);
	sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, { &params, sizeof(params) });
	sg_apply_pipeline(modelRender->pip_blend);
	sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, { &params, sizeof(params) });

	for (const auto& msh : meshes) {
		if (msh.pcount != 0) {
			if (msh.blend)
				sg_apply_pipeline(modelRender->pip_blend);
			else
				sg_apply_pipeline(modelRender->pip_opaque);
			sg_bindings bind{};
			bind.vertex_buffers[0] = msh.vbuf;
			bind.index_buffer = msh.ibuf;
			bind.fs_images[SLOT_model_bitmap] = msh.texture;
			sg_apply_bindings(bind);
			sg_draw(0, msh.pcount, 1);
		}
	}
}

void SCRenderer::SetLight(const RSVector3& l){
	this->lightDir = l;
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

	const uint32_t texId = image->GetTexture()->GetTextureID();
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
	if (i % 18 != 17){
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
	if (i / 18 != 17) {
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
	if (i % 18 != 17 && i / 18 != 17) {
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
		DrawModel(entity, LOD_LEVEL_MAX, entity->position, OBJECT_SCALE, entity->orientation);
}

void SCRenderer::RenderWorldSolid(const RSArea& area, int LOD, int verticesPerBlock, double gtime)
{
	running = true;

#if 0
	// Camera
	const RSVector3 lookAt{ 3856, 0, 2856};
	RSVector3 newPosition{ 4100, 100, 3000 };
	uint32_t currentTime = SDL_GetTicks();
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

	if (!groundRender)
		groundRender.emplace();

	RSMatrix world = HMM_Mat4d(1.0f);
	const RSMatrix& view = camera.getView();
	const RSMatrix& proj = camera.getProj();
	ground_vs_params_t params;
	memcpy(params.proj, &proj, 64);
	memcpy(params.view, &view, 64);
	memcpy(params.world, &world, 64);
	params.gtime = gtime;
	sg_apply_pipeline(groundRender->pip);
	sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, { &params, sizeof(params) });

	for(int i = 0; i < BLOCKS_PER_MAP; i++) {
		const AreaBlock& block = area.GetAreaBlockByID(LOD, i);
		const GroundData& meshes = cacheBlockToModel.GetData(&block, [&] (const AreaBlock* block, GroundData& meshes) {
			//struct PointComparator
			//{
			//	bool operator() (const RSVector3& a, const RSVector3& b) const {
			//		if (a.X != b.X) return a.X < b.X;
			//		if (a.Y != b.Y) return a.Y < b.Y;
			//		return a.Z < b.Z;
			//	}
			//};
			//std::map<RSVector3, uint32_t, PointComparator> pointToIndex;
			struct AreaVertex
			{
				RSVector3 pos;
				hmm_vec2 uv;
				std::array<uint8_t, 4> col;
			};
			using BlockCache = std::map<uint32_t, std::vector<AreaVertex>>;
			BlockCache tmp;
			AddVertex vadd = [&] (uint32_t texId, const RSVector3& pos, const float* col, const float* uv) {
				//uint32_t& idx = pointToIndex[pos];
				//if (idx == 0)
				//	idx = pointToIndex.size();
				auto& vert = tmp[texId];
				const auto r = col[0];
				const auto g = col[1];
				const auto b = col[2];
				//const bool useVertex = std::abs(pos.Y) > 0.01f || texId != PASS_VCOLOR || b < r || b < g;
				const bool useVertex = true;
				auto toByte = [] (float v) { return uint8_t(v * 255.99f); };
				if (useVertex)
					vert.push_back({ pos, { uv[0], uv[1] }, { toByte(r), toByte(g), toByte(b), toByte(col[3]) } });
			};
			RenderBlock(vadd, area, LOD, i, false);
			RenderBlock(vadd, area, LOD, i, true);
			const auto& data = tmp[PASS_VCOLOR];
			if (data.size() != 0) {
				auto& msh = meshes.emplace_back();
				msh.texture = white;
				msh.vbuf = MakeBuffer(SG_BUFFERTYPE_VERTEXBUFFER, SG_USAGE_IMMUTABLE, data);
				msh.pcount = data.size();
			}
			for (const auto& kv : tmp) {
				if (kv.first == PASS_VCOLOR)
					continue;
				const auto& data = kv.second;
				if (data.size() != 0) {
					auto& msh = meshes.emplace_back();
					msh.texture = { kv.first };
					msh.vbuf = MakeBuffer(SG_BUFFERTYPE_VERTEXBUFFER, SG_USAGE_IMMUTABLE, data);
					msh.pcount = data.size();
				}
			}
		});

		for (const auto& msh : meshes) {
			sg_bindings bind{};
			bind.vertex_buffers[0] = msh.vbuf;
			bind.fs_images[SLOT_ground_bitmap] = msh.texture;
			bind.fs_images[SLOT_water] = noise;
			sg_apply_bindings(bind);
			sg_draw(0, msh.pcount, 1);
		}
	}

	//Render objects on the map
	//for(int i=97 ; i < 98 ; i++)
	for(int id = 0; id < BLOCKS_PER_MAP; id++) {
		const std::vector<MapObject>& objects = area.objects[id];
		for (const MapObject& object : objects)
		{
			const float bx = float(id % 18);
			const float by = float(id / 18);

			float offset[3];
			offset[0] = bx * BLOCK_WIDTH;
			offset[1] = 0; //area.elevation[id];
			offset[2] = by * BLOCK_WIDTH;

			float localDelta[3];
			localDelta[0] = object.position[0] / 65355.0f * BLOCK_WIDTH;
			localDelta[1] = object.position[1] / (float)HEIGHT_DIVIDER;
			localDelta[2] = object.position[2] / 65355.0f * BLOCK_WIDTH;

			float toDraw[3];
			toDraw[0] = localDelta[0] + offset[0];
			toDraw[1] = localDelta[1] + offset[1];
			toDraw[2] = localDelta[2] + offset[2];

			const RSVector3 worldPos{ toDraw[0], toDraw[1], toDraw[2] };

			RSMatrix mworld = HMM_Mat4d(1);
			mworld.Elements[0][0] = OBJECT_SCALE * object.transform[0][0];
			mworld.Elements[0][1] = OBJECT_SCALE * object.transform[0][1];
			mworld.Elements[0][2] = OBJECT_SCALE * object.transform[0][2];
			mworld.Elements[1][0] = OBJECT_SCALE * object.transform[1][0];
			mworld.Elements[1][1] = OBJECT_SCALE * object.transform[1][1];
			mworld.Elements[1][2] = OBJECT_SCALE * object.transform[1][2];
			mworld.Elements[2][0] = OBJECT_SCALE * object.transform[2][0];
			mworld.Elements[2][1] = OBJECT_SCALE * object.transform[2][1];
			mworld.Elements[2][2] = OBJECT_SCALE * object.transform[2][2];
			mworld.Elements[3][0] = worldPos.X;
			mworld.Elements[3][1] = worldPos.Y;
			mworld.Elements[3][2] = worldPos.Z;

			DrawModel(object.entity, LOD_LEVEL_MAX, mworld);
		}
	}

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
