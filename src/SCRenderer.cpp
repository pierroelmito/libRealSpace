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

#include "UserProperties.h"

extern GLFWwindow* win;

const sg_pixel_format RSColorFormat = SG_PIXELFORMAT_RGB10A2;
const sg_pixel_format RSDepthFormat = SG_PIXELFORMAT_DEPTH;
const int RSSampleCount = 4;

struct SgTexture
{
	int w{ 0 };
	int h{ 0 };
	sg_image img{ 0 };
};

RSVector3 DecodeColor(const std::string& col)
{
	if (col.size() != 6)
		return { 1, 0, 1 };
	const auto charToColor = [] (char c) {
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
	assert(false);
	return SG_PIXELFORMAT_NONE;
}

std::optional<SgTexture> LoadDDS(const char* path)
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

	for (auto i = 0u; i < mips; ++i) {
		const tinyddsloader::DDSFile::ImageData* data = dds.GetImageData(i);
		idesc.data.subimage[0][i] = { data->m_mem, data->m_memSlicePitch };
	}

	return std::make_optional<SgTexture>(idesc.width, idesc.height, sg_make_image(&idesc));
}

enum ImageFlags
{
	IFMipMaps = 1,
	IFLinear = 2,
	IFRenderTarget = 4,
};

sg_image_desc MakeImageDesc(int w, int h, sg_pixel_format fmt, sg_usage usage, uint32_t flags)
{
	const bool mipmaps = (flags & IFMipMaps) != 0;
	const bool linear = (flags & IFLinear) != 0;
	const bool rt = (flags & IFRenderTarget) != 0;
	const int mipcount = mipmaps ? ComputeMipCount(w, h) : 1;

	sg_image_desc idesc{};

	idesc.width = w;
	idesc.height = h;
	idesc.render_target = rt;
	idesc.pixel_format = fmt;
	idesc.usage = usage;
	idesc.mag_filter = GetMagFilter(linear);
	idesc.min_filter = GetMinFilter(linear, mipmaps);
	idesc.num_mipmaps = mipcount;
	idesc.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
	idesc.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
	idesc.max_anisotropy = rt ? 1 : 1;
	idesc.sample_count = rt ? RSSampleCount: 1;

	return idesc;
}

template <typename T>
SgTexture MakeImage(int w, int h, sg_pixel_format fmt, sg_usage usage, uint32_t flags, const std::vector<T>& data)
{
	sg_image_desc idesc = MakeImageDesc(w, h, fmt, usage, flags);
	idesc.data.subimage[0][0] = { &data[0], w * h * sizeof(T) };
	return { w, h, sg_make_image(&idesc) };
}

SgTexture MakeImage(int w, int h, sg_pixel_format fmt, sg_usage usage, uint32_t flags)
{
	sg_image_desc idesc = MakeImageDesc(w, h, fmt, usage, flags);
	return { w, h, sg_make_image(&idesc) };
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

std::vector<uint32_t> ComputeSkyDome(int w, int h, const std::function<std::array<uint8_t, 4>(const RSVector3& dir)>& fn)
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

SgTexture MakeSkyDome(int w, int h, const std::function<std::array<uint8_t, 4>(const RSVector3& dir)>& fn)
{
	return MakeImage(w, h, SG_PIXELFORMAT_RGBA8, SG_USAGE_IMMUTABLE, IFLinear, ComputeSkyDome(w, h, fn));
}

template <class T>
T GetFogParams()
{
	T fog;
	fog.fogColor = DecodeColor(UserProperties::Get().Strings.Get("FogColor", "1b669b"));
	fog.thickNess = UserProperties::Get().Floats.Get("FogThickness", 0.0002);
	return fog;
}

struct ModelRenderData
{
	sg_shader shd{};
	sg_pipeline pip_opaque{};
	sg_pipeline pip_blend{};
	bool dirtyGlobals{ false };

	struct MeshItem
	{
		sg_image texture{};
		sg_buffer vbuf{};
		sg_buffer ibuf{};
		int pcount{};
		bool blend{ false };
	};
	using Mesh = std::vector<MeshItem>;

	bool Init()
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
			pdesc.color_count = 2;
			pdesc.colors[1].pixel_format = SG_PIXELFORMAT_R32F;

			pip_opaque = sg_make_pipeline(&pdesc);

			pdesc.depth.write_enabled = false;
			pdesc.colors[0].blend = {
				true,
				SG_BLENDFACTOR_SRC_ALPHA,
				SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
				SG_BLENDOP_ADD,
				SG_BLENDFACTOR_ONE,
				SG_BLENDFACTOR_ZERO,
				SG_BLENDOP_ADD,
			};
			pdesc.colors[1].blend = {
				true,
				SG_BLENDFACTOR_ONE,
				SG_BLENDFACTOR_ZERO,
				SG_BLENDOP_ADD,
				SG_BLENDFACTOR_ONE,
				SG_BLENDFACTOR_ZERO,
				SG_BLENDOP_ADD,
			};

			pip_blend = sg_make_pipeline(&pdesc);
		}

		return true;
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

	bool Init()
	{
		shd = sg_make_shader(ground_shader_desc(sg_query_backend()));

		{
			sg_pipeline_desc pdesc{};
			pdesc.shader = shd;
			pdesc.depth.write_enabled = true;
			pdesc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;
			pdesc.layout.attrs[ATTR_ground_vs_position].format = SG_VERTEXFORMAT_FLOAT3;
			pdesc.layout.attrs[ATTR_ground_vs_normal].format = SG_VERTEXFORMAT_FLOAT3;
			pdesc.layout.attrs[ATTR_ground_vs_texcoord].format = SG_VERTEXFORMAT_FLOAT2;
			pdesc.layout.attrs[ATTR_ground_vs_vcolor].format = SG_VERTEXFORMAT_UBYTE4N;
			pdesc.cull_mode = SG_CULLMODE_BACK;
			pdesc.color_count = 2;
			pdesc.colors[1].pixel_format = SG_PIXELFORMAT_R32F;
			pip = sg_make_pipeline(&pdesc);
		}

		return true;
	}
};

struct FullscreenSky
{
	sg_buffer vbuf{};
	sg_shader shd{};
	sg_pipeline pip{};
	sg_bindings bind{};

	bool Init()
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

		return true;
	}
};

struct FullscreenClouds
{
	sg_buffer vbuf{};
	sg_shader shd{};
	sg_pipeline pip{};
	sg_bindings bind{};

	bool Init()
	{
		sg_shader_desc sd = *clouds_shader_desc(sg_query_backend());
		shd = sg_make_shader(sd);

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
			pdesc.depth.pixel_format = SG_PIXELFORMAT_NONE;
			pdesc.depth.write_enabled = false;
			pdesc.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT3;
			pdesc.colors[0].blend = {
				true,
				SG_BLENDFACTOR_SRC_ALPHA,
				SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
				SG_BLENDOP_ADD,
				SG_BLENDFACTOR_ZERO,
				SG_BLENDFACTOR_ONE,
				SG_BLENDOP_ADD,
			};
			pip = sg_make_pipeline(&pdesc);
		}

		bind.vertex_buffers[0] = vbuf;

		return true;
	}
};

struct FullscreenBitmapData
{
	sg_buffer vbuf{};
	sg_shader shd{};
	sg_pipeline pip{};

	bool Init()
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

		{
			sg_pipeline_desc pdesc{};
			pdesc.shader = shd;
			pdesc.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT3;
			pip = sg_make_pipeline(&pdesc);
		}

		return true;
	}

	void DrawImage(sg_image img, float fade = 0.0f, hmm_vec2 xy = { 1.0f, -1.0f })
	{
		int cur_width{}, cur_height{};
		glfwGetFramebufferSize(win, &cur_width, &cur_height);

		sg_pass_action pass_action = {0};
		pass_action.colors[0].action = SG_ACTION_LOAD;
		sg_begin_default_pass(&pass_action, cur_width, cur_height);

		sg_apply_pipeline(pip);

		fsq_vs_params_t params;
		params.pcolor = { 1.0f - fade, 1.0f - fade, 1.0f - fade };
		params.xy = xy;
		sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_fsq_vs_params, { &params, sizeof(params) });

		sg_bindings bind{};
		bind.vertex_buffers[0] = vbuf;
		bind.fs_images[SLOT_fs_bitmap] = img;

		sg_apply_bindings(bind);

		sg_draw(0, 6, 1);

		sg_end_pass();
	}
};

SgTexture debugFont{};
SgTexture noise{};
SgTexture white{};
SgTexture skydome{};
SgTexture screen{};
SgTexture renderTargetColor{};
SgTexture renderTargetGDepth{};
SgTexture renderTargetZBuffer{};
sg_pass renderPassScene0{};
sg_pass renderPassScene1{};
sg_pass renderPassScreen{};

FullscreenBitmapData FbdRender;
FullscreenSky FullscreenSky;
FullscreenClouds FullscreenClouds;
ModelRenderData ModelRender;
GroundRenderData GroundRender;

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

ObjectCacheManager<RSEntity, ModelRenderData::Mesh> cacheEntityToModel;
ObjectCacheManager<AreaBlock, GroundRenderData::Mesh> cacheBlockToModel;

// DIRTY...
const uint32_t PASS_VCOLOR = 0x12345678;
const uint32_t PASS_BLEND = 0x12345679;

SCRenderer::SCRenderer()
: initialized(false)
{
}

SCRenderer::~SCRenderer(){
}

template <size_t N, typename S, typename... PARAMS>
std::array<uint8_t, 4> FractalNoiseSkyDome(const RSVector3& v, const std::array<hmm_vec2, N>& seeds, const S& sampler, const PARAMS&... params)
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

void SCRenderer::Init()
{
	int32_t width  = 320;
	int32_t height = 200;

	//Load the default palette
	IffLexer lexer;
	lexer.InitFromFile("PALETTE.IFF");
	//lexer.List(stdout);

	RSPalette palette;
	palette.InitFromIFF(&lexer);
	this->palette = *palette.GetColorPalette();

	camera.SetPersective(50.0f, width / (float)height, 0.1f, 20000.0f);

	lightDir = HMM_NormalizeVec3({ 1, 1, 1 });

	std::vector<uint32_t> pixels = { 0xffffffffu };
	white = MakeImage(1, 1, SG_PIXELFORMAT_RGBA8, SG_USAGE_IMMUTABLE, 0, pixels);
	noise = LoadDDS("../assets/noise.dds").value_or(white);
	debugFont = LoadDDS("../assets/font.dds").value_or(white);

	std::array<hmm_vec2, 7> seeds;
	for (hmm_vec2& v : seeds) {
		const float a = HMM_PI32 * 2.0f * float(rand() % 1024) / 1023.0f;
		v = HMM_NormalizeVec2({ cosf(a), sinf(a) });
	}

	const size_t sz = 32;
	const float cf = 1.0f / sqrtf(2.0f);
	std::array<hmm_vec2, sz * sz> gradients{};
	for (hmm_vec2& g : gradients) {
		const float a = HMM_PI32 * 2.0f * float(rand() % 1024) / 1023.0f;
		g = cf * HMM_NormalizeVec2({ cosf(a), sinf(a) });
	}
	skydome = MakeSkyDome(1024, 1024, [&] (const RSVector3& d) {
		return FractalNoiseSkyDome(d, seeds, PerlinNoise<sz>, gradients);
	});

	ModelRender.Init();
	FullscreenSky.Init();
	FullscreenClouds.Init();
	GroundRender.Init();
	FbdRender.Init();

	initialized = true;
}

void SCRenderer::Release()
{
}

void
SCRenderer::MakeContext()
{
	sg_desc desc{ 0 };
	desc.buffer_pool_size = 1 << 15;
	desc.image_pool_size = 1 << 15;
	desc.context.color_format = RSColorFormat;
	desc.context.depth_format = RSDepthFormat;
	desc.context.sample_count = RSSampleCount;
	sg_setup(desc);
}

void
SCRenderer::Draw3D(const Render3DParams& params, std::function<void()>&& f)
{
	ModelRender.dirtyGlobals = true;

	{
		sg_pass_action pass_action = {0};
		if ((params.flags & Render3DParams::CLEAR_COLORS) == 0) {
			pass_action.colors[0] = { SG_ACTION_DONTCARE };
		} else {
			pass_action.colors[0] = { SG_ACTION_CLEAR, { 1, 0, 0, 0 } };
		}
		pass_action.colors[1] = { SG_ACTION_CLEAR, { 0, 0, 0, 0 } };

		{
			int cur_width{}, cur_height{};
			glfwGetFramebufferSize(win, &cur_width, &cur_height);
			if (renderTargetColor.w != cur_width || renderTargetColor.h != cur_height) {
				if (renderTargetColor.w != 0) {
					sg_destroy_pass(renderPassScene0);
					sg_destroy_pass(renderPassScene1);
					sg_destroy_pass(renderPassScreen);
					sg_destroy_image(renderTargetColor.img);
					sg_destroy_image(renderTargetGDepth.img);
					sg_destroy_image(renderTargetZBuffer.img);
				}
				const int rtWidth = cur_width;
				const int rtHeight = cur_height;
				renderTargetColor = MakeImage(rtWidth, rtHeight, RSColorFormat, _SG_USAGE_DEFAULT, IFRenderTarget);
				renderTargetGDepth = MakeImage(rtWidth, rtHeight, SG_PIXELFORMAT_R32F, _SG_USAGE_DEFAULT, IFRenderTarget);
				renderTargetZBuffer = MakeImage(rtWidth, rtHeight, RSDepthFormat, _SG_USAGE_DEFAULT, IFRenderTarget);
				sg_pass_desc passdesc{};
				passdesc.color_attachments[0].image = renderTargetColor.img;
				renderPassScreen = sg_make_pass(passdesc);
				passdesc.depth_stencil_attachment.image = renderTargetZBuffer.img;
				renderPassScene1 = sg_make_pass(passdesc);
				passdesc.color_attachments[1].image = renderTargetGDepth.img;
				renderPassScene0 = sg_make_pass(passdesc);
			}
			sg_begin_pass(renderPassScene0, &pass_action);
		}
	}

	f();

	sg_end_pass();

	const bool renderSky = (params.flags & Render3DParams::SKY) != 0;
	if (renderSky) {
		sg_pass_action pass_action = {0};
		pass_action.colors[0].action = SG_ACTION_LOAD;
		pass_action.depth.action = SG_ACTION_LOAD;
		sg_begin_pass(renderPassScene1, pass_action);
		RenderSky();
		sg_end_pass();
	}

	const bool renderClouds = (params.flags & Render3DParams::CLOUDS) != 0;
	if (renderClouds) {
		sg_pass_action screenPassAction = {0};
		screenPassAction.colors[0] = { SG_ACTION_LOAD, { 0, 0, 1, 0 } };
		sg_begin_pass(renderPassScreen, screenPassAction);
		RenderClouds();
		sg_end_pass();
	}

	FbdRender.DrawImage(renderTargetColor.img, 0.0f, { 1, 1 });
}

void SCRenderer::UpdateBitmapQuad(Texel* data, uint32_t width, uint32_t height, float fade)
{
	if (screen.w != width || screen.h != height) {
		if (screen.w != 0)
			sg_destroy_image(screen.img);
		screen = MakeImage(width, height, SG_PIXELFORMAT_RGBA8, SG_USAGE_STREAM, 0 /*IFLinear*/);
	}

	sg_image_data idata{};
	idata.subimage[0][0] = { data, width * height * sizeof(Texel) };
	sg_update_image(screen.img, idata);

	FbdRender.DrawImage(screen.img, fade);
}

bool SCRenderer::CreateTextureInGPU(RSTexture* texture)
{
	if (!initialized)
		return false;

	sg_image img = MakeImage(texture->width, texture->height, SG_PIXELFORMAT_RGBA8, SG_USAGE_DYNAMIC, IFLinear).img;
	texture->id = img.id;

	return true;
}

bool SCRenderer::UploadTextureContentToGPU(RSTexture* texture)
{
	if (!initialized)
		return false;

	sg_image_data data;
	std::vector<std::vector<uint32_t>> mipmapData;

	data.subimage[0][0] = { texture->data, texture->width * texture->height * 4 };

	/*
	mipmapData.resize(mips);
	const int mips = ComputeMipCount(texture->width, texture->height);
	for (int i = 1; i < mips; ++i)
	{
		const size_t nw = std::max<size_t>(1, texture->width >> i);
		const size_t nh = std::max<size_t>(1, texture->height >> i);
		mipmapData[i - 1].resize(nw * nh, 0xffffffffu);
		data.subimage[0][i] = { &mipmapData[0], nw * nh * 4 };
	}
	*/

	sg_update_image({ texture->id }, data);

	return true;
}

void SCRenderer::DeleteTextureInGPU(RSTexture* texture)
{
	if (!initialized)
		return;

	sg_destroy_image({ texture->id });
}

void SCRenderer::RenderSky()
{
	sg_apply_pipeline(FullscreenSky.pip);

	auto& userStrings = UserProperties::Get().Strings;

	sky_vs_params_t vsParams;
	vsParams.view = camera.getView();
	vsParams.proj = camera.getProj();
	sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_sky_vs_params, { &vsParams, sizeof(vsParams) });

	sky_fs_params_t fsParams;
	fsParams.lightdir = lightDir;
	fsParams.colUp = DecodeColor(userStrings.Get("SkyColUp", "1a216e"));
	fsParams.colBot = DecodeColor(userStrings.Get("SkyColBot", "1b669b"));
	fsParams.colLight = DecodeColor(userStrings.Get("SkyColLight", "dfedd3"));
	sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_sky_fs_params, { &fsParams, sizeof(fsParams) });

	FullscreenSky.bind.fs_images[SLOT_skydome] = skydome.img;
	sg_apply_bindings(&FullscreenSky.bind);
	sg_draw(0, 6, 1);
}

void SCRenderer::RenderClouds()
{
	const auto fog = GetFogParams<fog_params_t>();

	sg_apply_pipeline(FullscreenClouds.pip);
	sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_fog_params, { &fog, sizeof(fog) });
	FullscreenClouds.bind.fs_images[SLOT_tex_depth] = renderTargetGDepth.img;
	sg_apply_bindings(&FullscreenClouds.bind);
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

void PrepareModel(SCRenderer& r, const RSEntity* object, size_t lodLevel, ModelRenderData::Mesh& mdata)
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
			return { tx.r, tx.g, tx.b, 128 };
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

	mdata.emplace_back(); // always reserve vertex color at index 0

	int propCount0[256]{};

	if (lodLevel == 0){
		std::map<uint32_t, MeshData> textureData;
		for (const uvxyEntry& textInfo : object->uvs) {
			//Seems we have a textureID that we don't have :( !
			if (textInfo.textureID >= object->images.size())
				continue;

			auto&& image = object->images[textInfo.textureID];
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
		const uint16_t triangleID = lod.triangleIDs[i];
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
		mshOpaque.texture = white.img;
		mshOpaque.vbuf = MakeBuffer(SG_BUFFERTYPE_VERTEXBUFFER, SG_USAGE_IMMUTABLE, opaque.vertice);
		mshOpaque.ibuf = MakeBuffer(SG_BUFFERTYPE_INDEXBUFFER, SG_USAGE_IMMUTABLE, opaque.indice);
		mshOpaque.pcount = opaque.indice.size();
	}

	auto& mshBlend = mdata.emplace_back();
	if (!blend.indice.empty()) {
		const bool opt = blend.total != blend.vertice.size();
		//printf("opt: %s...\n", opt ? "yes" : "no");
		computeNormals(blend);
		mshBlend.texture = white.img;
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

void SCRenderer::DrawModel(const RSEntity* object, size_t lodLevel, const RSMatrix& world)
{
	if (!initialized || object == nullptr || lodLevel >= object->lods.size())
		return;

	ModelRenderData::Mesh& meshes = cacheEntityToModel.GetData(object, [&] (const RSEntity* o, ModelRenderData::Mesh& tmp) {
		PrepareModel(*this, o, lodLevel, tmp);
	});

	// update global constants only once per frame...
	if (ModelRender.dirtyGlobals) {
		ModelRender.dirtyGlobals = false;

		model_vs_global_params_t gparams;
		gparams.proj = camera.getProj();
		gparams.view = camera.getView();
		gparams.pcampos = camera.getPosition();
		gparams.lightDir = lightDir;

		sg_apply_pipeline(ModelRender.pip_opaque);
		sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_model_vs_global_params, { &gparams, sizeof(gparams) });

		sg_apply_pipeline(ModelRender.pip_blend);
		sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_model_vs_global_params, { &gparams, sizeof(gparams) });
	}

	const model_vs_instance_params_t iparams{ world };
	bool iparamsOpaqueOk = false;
	bool iparamsBlendOk = false;

	for (const auto& msh : meshes) {
		if (msh.pcount != 0) {
			if (msh.blend) {
				sg_apply_pipeline(ModelRender.pip_blend);
				if (!iparamsBlendOk)
					sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_model_vs_instance_params, { &iparams, sizeof(iparams) });
				iparamsBlendOk = true;
			} else {
				sg_apply_pipeline(ModelRender.pip_opaque);
				if (!iparamsOpaqueOk)
					sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_model_vs_instance_params, { &iparams, sizeof(iparams) });
				iparamsOpaqueOk = true;
			}
			sg_bindings bind{};
			bind.vertex_buffers[0] = msh.vbuf;
			bind.index_buffer = msh.ibuf;
			bind.fs_images[SLOT_model_bitmap] = msh.texture;
			sg_apply_bindings(bind);
			sg_draw(0, msh.pcount, 1);
		}
	}
}

void SCRenderer::SetLight(const RSVector3& l)
{
	this->lightDir = l;
}

void SCRenderer::Prepare(RSEntity* object)
{
	for (auto&& img : object->images)
		img->SyncTexture();
	object->prepared = true;
}

bool SCRenderer::IsTextured(const MapVertex* tri0, const MapVertex* tri1, const MapVertex* tri2)
{
	return
		// tri0->type != tri1->type ||
		//tri0->type != tri2->type ||
		tri0->upperImageID == 0xFF || tri0->lowerImageID == 0xFF ;
}

constexpr int LOWER_TRIANGE = 0;
constexpr int UPPER_TRIANGE = 1;

void SCRenderer::RenderTexturedTriangle(
	const AddVertex& vfunc,
	const RSArea& area,
	const MapVertex& tri0,
	const MapVertex& tri1,
	const MapVertex& tri2,
	int triangleType)
{
	constexpr float white[4] { 1, 1, 1, 1 };

	constexpr float TEX_ZERO = 0.0f;
	constexpr float TEX_ONE = 1.0f;
	// What is this offset ? It is used to get rid of the red delimitations
	// in the 64x64 textures.
	constexpr float OFFSET = (1.1f / 64.0f);
	constexpr float textTrianCoo64[2][3][2] = {
		{{TEX_ZERO,TEX_ZERO+OFFSET},    {TEX_ONE-2*OFFSET,TEX_ONE-OFFSET},    {TEX_ZERO,TEX_ONE-OFFSET} }, // LOWER_TRIANGE
		{{TEX_ZERO+2*OFFSET,TEX_ZERO+OFFSET},    {TEX_ONE,TEX_ZERO+OFFSET},    {TEX_ONE,TEX_ONE-OFFSET} }  //UPPER_TRIANGE
	};
	constexpr float textTrianCoo[2][3][2] = {
		{{TEX_ZERO,TEX_ZERO},    {TEX_ONE,TEX_ONE},    {TEX_ZERO,TEX_ONE} }, // LOWER_TRIANGE
		{{TEX_ZERO,TEX_ZERO},    {TEX_ONE,TEX_ZERO},    {TEX_ONE,TEX_ONE} }  //UPPER_TRIANGE
	};

	RSImage* image = NULL;
	if (triangleType == LOWER_TRIANGE)
		image = area.GetImageByID(tri0.lowerImageID);
	if (triangleType == UPPER_TRIANGE)
		image = area.GetImageByID(tri0.upperImageID);

	if (image == NULL){
		assert(false);
		printf("This should never happen: Put a break point here.\n");
		return;
	}

	// switch tex coord depending on texture size
	const bool is64 = image->width == 64;
	const auto& ttc = is64 ? textTrianCoo64 : textTrianCoo;

	const uint32_t texId = image->GetTexture()->GetTextureID();
	vfunc(texId, tri0.v, tri0.n, white, ttc[triangleType][0]);
	vfunc(texId, tri1.v, tri1.n, white, ttc[triangleType][1]);
	vfunc(texId, tri2.v, tri2.n, white, ttc[triangleType][2]);
}

void SCRenderer::RenderColoredTriangle(
	const AddVertex& vfunc,
	const MapVertex& tri0,
	const MapVertex& tri1,
	const MapVertex& tri2)
{
	const float noUv[2] = { 0.5f, 0.5f };
	if (tri0.type != tri1.type || tri0.type != tri2.type) {
		const MapVertex* tri{};
		if (tri1.type > tri0.type)
			if (tri1.type > tri2.type)
				tri = &tri1;
			else
				tri = &tri2;
		else
			if (tri0.type > tri2.type)
				tri = &tri0;
			else
				tri = &tri2;
		vfunc(PASS_VCOLOR, tri0.v, tri0.n, tri->color, noUv);
		vfunc(PASS_VCOLOR, tri1.v, tri1.n, tri->color, noUv);
		vfunc(PASS_VCOLOR, tri2.v, tri2.n, tri->color, noUv);
	} else{
		vfunc(PASS_VCOLOR, tri0.v, tri0.n, tri0.color, noUv);
		vfunc(PASS_VCOLOR, tri1.v, tri1.n, tri1.color, noUv);
		vfunc(PASS_VCOLOR, tri2.v, tri2.n, tri2.color, noUv);
	}
}

void SCRenderer::RenderQuad(
	const AddVertex& vfunc,
	const RSArea& area,
	const MapVertex& currentVertex,
	const MapVertex& rightVertex,
	const MapVertex& bottomRightVertex,
	const MapVertex& bottomVertex,
	bool renderTexture)
{
	if (!renderTexture){
		//if (currentVertex->lowerImageID == 0xFF )
		RenderColoredTriangle(vfunc, currentVertex, bottomRightVertex, bottomVertex);
		// if (currentVertex->upperImageID == 0xFF )
		RenderColoredTriangle(vfunc, currentVertex, rightVertex, bottomRightVertex);
	} else{
		if (currentVertex.lowerImageID != 0xFF)
			RenderTexturedTriangle(vfunc, area, currentVertex, bottomRightVertex, bottomVertex, LOWER_TRIANGE);
		if (currentVertex.upperImageID != 0xFF)
			RenderTexturedTriangle(vfunc, area, currentVertex, rightVertex, bottomRightVertex, UPPER_TRIANGE);
	}
}

void SCRenderer::RenderBlock(const AddVertex& vfunc, const RSArea& area, int LOD, int i, bool renderTexture)
{
	const AreaBlock& block = area.GetAreaBlockByID(LOD, i);
	const uint32_t sideSize = block.sideSize;

	for (size_t x=0 ; x < sideSize-1 ; x ++){
		for (size_t y=0 ; y < sideSize-1 ; y ++){
			const MapVertex& currentVertex     = block.vertice[x+y*sideSize];
			const MapVertex& rightVertex       = block.vertice[(x+1)+y*sideSize];
			const MapVertex& bottomRightVertex = block.vertice[(x+1)+(y+1)*sideSize];
			const MapVertex& bottomVertex      = block.vertice[x+(y+1)*sideSize];
			RenderQuad(vfunc,area,currentVertex,rightVertex, bottomRightVertex, bottomVertex,renderTexture);
		}
	}

	//Inter-block right side
	if (i % 18 != 17){
		const AreaBlock& currentBlock = block;
		const AreaBlock& rightBlock = area.GetAreaBlockByID(LOD, i+1);
		for (int y=0 ; y < sideSize-1 ; y ++){
			const MapVertex& currentVertex     = *currentBlock.GetVertice(currentBlock.sideSize-1, y);
			const MapVertex& rightVertex       = *rightBlock.GetVertice(0, y);
			const MapVertex& bottomRightVertex = *rightBlock.GetVertice(0, y+1);
			const MapVertex& bottomVertex      = *currentBlock.GetVertice(currentBlock.sideSize-1, y+1);
			RenderQuad(vfunc,area,currentVertex,rightVertex, bottomRightVertex, bottomVertex,renderTexture);
		}
	}

	//Inter-block bottom side
	if (i / 18 != 17) {
		const AreaBlock& currentBlock = block;
		const AreaBlock& bottomBlock = area.GetAreaBlockByID(LOD, i+BLOCK_PER_MAP_SIDE);
		for (int x=0 ; x < sideSize-1 ; x++){
			const MapVertex& currentVertex     = *currentBlock.GetVertice(x,currentBlock.sideSize-1);
			const MapVertex& rightVertex       = *currentBlock.GetVertice(x+1,currentBlock.sideSize-1);
			const MapVertex& bottomRightVertex = *bottomBlock.GetVertice(x+1,0);
			const MapVertex& bottomVertex      = *bottomBlock.GetVertice(x,0);
			RenderQuad(vfunc,area,currentVertex,rightVertex, bottomRightVertex, bottomVertex,renderTexture);
		}
	}

	//Inter bottom-right quad
	if (i % 18 != 17 && i / 18 != 17) {
		const AreaBlock& currentBlock = block;
		const AreaBlock& rightBlock = area.GetAreaBlockByID(LOD, i+1);
		const AreaBlock& rightBottonBlock = area.GetAreaBlockByID(LOD, i+1+BLOCK_PER_MAP_SIDE);
		const AreaBlock& bottomBlock = area.GetAreaBlockByID(LOD, i+BLOCK_PER_MAP_SIDE);
		const MapVertex& currentVertex     = *currentBlock.GetVertice(currentBlock.sideSize-1,currentBlock.sideSize-1);
		const MapVertex& rightVertex       = *rightBlock.GetVertice(0,currentBlock.sideSize-1);
		const MapVertex& bottomRightVertex = *rightBottonBlock.GetVertice(0,0);
		const MapVertex& bottomVertex      = *bottomBlock.GetVertice(currentBlock.sideSize-1,0);
		RenderQuad(vfunc,area,currentVertex,rightVertex, bottomRightVertex, bottomVertex,renderTexture);
	}
}

void SCRenderer::RenderJets(const RSArea& area)
{
	for(RSEntity* entity : area.GetJets()) {
		RSMatrix world = HMM_QuaternionToMat4(entity->orientation) * HMM_Scale({ OBJECT_SCALE, OBJECT_SCALE, OBJECT_SCALE });
		world.Elements[3][0] = entity->position.X;
		world.Elements[3][1] = entity->position.Y;
		world.Elements[3][2] = entity->position.Z;
		DrawModel(entity, LOD_LEVEL_MAX, world);
	}
}

void SCRenderer::RenderWorldSolid(const RSArea& area, int LOD, double gtime)
{
	running = true;
	RenderWorldGround(area, LOD, gtime);
	RenderWorldModels(area, LOD, gtime);
}

void SCRenderer::RenderWorldGround(const RSArea& area, int LOD, double gtime)
{
	static std::vector<GroundRenderData::MeshItem> groundMeshes;
	groundMeshes.resize(0);

	for(int i = 0; i < BLOCKS_PER_MAP; i++) {
		const AreaBlock& block = area.GetAreaBlockByID(LOD, i);
		const GroundRenderData::Mesh& meshes = cacheBlockToModel.GetData(&block, [&] (const AreaBlock* block, GroundRenderData::Mesh& meshes) {
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
				RSVector3 normal;
				hmm_vec2 uv;
				std::array<uint8_t, 4> col;
			};
			using BlockCache = std::map<uint32_t, std::vector<AreaVertex>>;
			BlockCache tmp;
			AddVertex vadd = [&] (uint32_t texId, const RSVector3& pos, const RSVector3& n, const float* col, const float* uv) {
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
					vert.push_back({ pos, n, { uv[0], uv[1] }, { toByte(r), toByte(g), toByte(b), toByte(col[3]) } });
			};
			RenderBlock(vadd, area, LOD, i, false);
			RenderBlock(vadd, area, LOD, i, true);
			const auto& data = tmp[PASS_VCOLOR];
			if (data.size() != 0) {
				auto& msh = meshes.emplace_back();
				msh.texture = white.img;
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

		for (const auto& msh : meshes)
			groundMeshes.push_back(msh);
	}

	if (!groundMeshes.empty())
	{
		std::sort(groundMeshes.begin(), groundMeshes.end(), [] (const GroundRenderData::MeshItem& a, const GroundRenderData::MeshItem& b) {
			if (a.texture.id != b.texture.id)
				return a.texture.id < b.texture.id;
			return false;
		});

		ground_vs_params_t params;
		params.view = camera.getView();
		params.proj = camera.getProj();
		params.world = HMM_Mat4d(1.0f);
		params.pcampos = camera.getPosition();
		params.plightdir = lightDir;
		params.gtime = float(gtime);

		sg_apply_pipeline(GroundRender.pip);
		sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_ground_vs_params, { &params, sizeof(params) });

#if 0
		const auto fog = GetFogParams<ground_fog_params_t>();
		sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_ground_fog_params, { &fog, sizeof(fog) });
#endif

		sg_bindings bind{};
		bind.fs_images[SLOT_water] = noise.img;
		for (const auto& msh : groundMeshes) {
			bind.vertex_buffers[0] = msh.vbuf;
			bind.fs_images[SLOT_ground_bitmap] = msh.texture;
			sg_apply_bindings(bind);
			sg_draw(0, msh.pcount, 1);
		}
	}
}

void SCRenderer::RenderWorldModels(const RSArea& area, int LOD, double gtime)
{
	const auto& userInts = UserProperties::Get().Ints;
	const auto& userFloats = UserProperties::Get().Floats;

	const float ofs0 = userFloats.Get("BlockObjOfsX", 1.0f);
	const float ofs1 = userFloats.Get("BlockObjOfsZ", 1.0f);
	const float factorX = userFloats.Get("BlockObjFactorX", -1.0f);
	const float factorZ = userFloats.Get("BlockObjFactorZ", -1.0f);
	const float objScale = userFloats.Get("BlockObjScale", OBJECT_SCALE);

	const int axisX = userInts.Get("BlockObjX", 1);
	const int axisZ = userInts.Get("BlockObjZ", 0);

	//Render objects on the map
	for(int id = 0; id < BLOCKS_PER_MAP; id++) {
		const std::vector<MapObject>& objects = area.objects[id];

		const float bx = float(id % 18) + ofs0;
		const float by = float(id / 18) + ofs1;

		const float offset[3] = {
			bx * BLOCK_WIDTH,
			//area.elevation[id] / (2 * (float)HEIGHT_DIVIDER),
			2.0f * area.elevation[id] / (float)(1 << 8),
			by * BLOCK_WIDTH,
		};

		for (const MapObject& object : objects)
		{
			//const float factorXZ = BLOCK_WIDTH;
			const float localDelta[3] = {
				object.position[axisX] * factorX,
				0.5f * object.position[2] / (float)HEIGHT_DIVIDER,
				object.position[axisZ] * factorZ,
			};

			const float toDraw[3] = {
				localDelta[0] + offset[0],
				localDelta[1] + offset[1],
				localDelta[2] + offset[2],
			};

			const RSVector3 worldPos{ toDraw[0], toDraw[1], toDraw[2] };

			RSMatrix mworld = HMM_Mat4d(1);
			mworld.Elements[0][0] = objScale * object.transform[0][0];
			mworld.Elements[0][1] = objScale * object.transform[0][1];
			mworld.Elements[0][2] = objScale * object.transform[0][2];
			mworld.Elements[1][0] = objScale * object.transform[1][0];
			mworld.Elements[1][1] = objScale * object.transform[1][1];
			mworld.Elements[1][2] = objScale * object.transform[1][2];
			mworld.Elements[2][0] = objScale * object.transform[2][0];
			mworld.Elements[2][1] = objScale * object.transform[2][1];
			mworld.Elements[2][2] = objScale * object.transform[2][2];
			mworld.Elements[3][0] = worldPos.X;
			mworld.Elements[3][1] = worldPos.Y;
			mworld.Elements[3][2] = worldPos.Z;

			DrawModel(object.entity, LOD_LEVEL_MAX, mworld);
		}
	}

	RenderJets(area);
}

#if USE_SHADER_PIPELINE != 1

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
