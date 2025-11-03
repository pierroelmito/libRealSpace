#pragma once

#include <map>
#include <memory>
#include <optional>
#include <random>
#include <span>

#include <algorithm>
#include <cstdint>
#include <initializer_list>
#include <string>
#include <variant>
#include <vector>

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

namespace rlt {

struct Layout {
	const int dx {};
	const int dy {};
	const int strideX {};
	const int strideY {};
	inline Rectangle get(int x, int y) const
	{
		return { float(dx + x * strideX), float(dy + y * strideY), 16, 16 };
	}
	inline Rectangle uv(int x, int y, int w, int h) const
	{
		const float sx = float(dx + x * strideX);
		const float sy = float(dy + y * strideY);
		return { sx / w, sy / h, 16.0f / w, 16.0f / h };
	}
};

inline Rectangle Expand(Rectangle r, int o = 1)
{
	return { r.x - o, r.y - o, r.width + 2 * o, r.height + 2 * o };
}

inline Rectangle Mirror(bool bb, Rectangle r)
{
	return { bb ? r.x + r.width : r.x, r.y, -r.width, r.height };
}

template <typename TO, typename FROM>
TO ToVec3(const FROM& from)
{
	return { from.X, from.Y, from.Z };
}

inline Matrix MakeMat4(const float (&v)[4][4])
{
	Matrix r {
		v[0][0],
		v[0][1],
		v[0][2],
		v[0][3],
		v[1][0],
		v[1][1],
		v[1][2],
		v[1][3],
		v[2][0],
		v[2][1],
		v[2][2],
		v[2][3],
		v[3][0],
		v[3][1],
		v[3][2],
		v[3][3],
	};
	return r;
}

inline Texture CreateColorTexture(int w, int h, Color c)
{
	Image img = GenImageColor(w, h, c);
	Texture tex = LoadTextureFromImage(img);
	UnloadImage(img);
	return tex;
}

template <typename R, typename T>
inline R* AllocCopy(const std::vector<T>& v)
{
	if (v.empty())
		return nullptr;
	void* mem = MemAlloc(v.size() * sizeof(T));
	T* r = (T*)mem;
	std::copy(v.begin(), v.end(), r);
	return (R*)mem;
}

inline Mesh MakeMesh(const std::vector<Vector3>& vertice,
	const std::vector<Vector3>& normals,
	const std::vector<Vector2>& uvs,
	const std::vector<Color>& colors,
	const std::vector<uint16_t>& indice)
{
	Mesh mesh {};
	mesh.vertexCount = vertice.size();
	mesh.triangleCount = indice.size() / 3;
	mesh.vertices = AllocCopy<float>(vertice);
	mesh.normals = AllocCopy<float>(normals);
	mesh.texcoords = AllocCopy<float>(uvs);
	mesh.colors = AllocCopy<uint8_t>(colors);
	mesh.indices = AllocCopy<uint16_t>(indice);
	UploadMesh(&mesh, false);
	return mesh;
}

inline Mesh FSQ()
{
	const std::vector<Vector3> vertice {
		{ -1, -1, 1 },
		{ 1, -1, 1 },
		{ 1, 1, 1 },
		{ -1, 1, 1 },
	};
	return MakeMesh(vertice, {}, {}, {}, { 0, 1, 3, 1, 2, 3 });
}

template <class T0 = std::initializer_list<std::pair<size_t, const char*>>,
	class T1 = std::initializer_list<std::pair<size_t, const char*>>>
inline Shader MakeShader(std::string_view vs, std::string_view fs,
	T0 uniforms = {}, T1 attribs = {})
{
	const std::string path = "shaders/";
	const std::string fvs = path + std::string(vs) + ".vs.glsl";
	const std::string ffs = path + std::string(fs) + ".fs.glsl";
	TraceLog(LOG_INFO, "shader: %s / %s", fvs.c_str(), ffs.c_str());
	Shader r = LoadShader(vs.empty() ? nullptr : fvs.c_str(),
		fs.empty() ? nullptr : ffs.c_str());
	for (const auto& uniform : uniforms) {
		auto loc = GetShaderLocation(r, uniform.second);
		TraceLog(LOG_INFO, "loc %s: [%d] = %d", uniform.second, uniform.first, loc);
		r.locs[uniform.first] = loc;
	}
	for (const auto& attrib : attribs) {
		auto loc = GetShaderLocationAttrib(r, attrib.second);
		TraceLog(LOG_INFO, "loc %s: [%d] = %d", attrib.second, attrib.first, loc);
		r.locs[attrib.first] = loc;
	}
	return r;
}

template <class T>
inline void SetUniform(Shader& shd, int loc, T* t, ShaderUniformDataType type)
{
	if (shd.locs[loc] != -1)
		SetShaderValue(shd, shd.locs[loc], t, type);
}

inline Matrix GetCameraTransform(const Camera& cam, float width, float height)
{
	const float dbg = 1.0f;
	const float near = RL_CULL_DISTANCE_NEAR;
	const float fovy = cam.fovy * PI / 180.0f;
	const float ay = tanf(0.5f * fovy);
	const float ratio = width / height;
	const float cf = near;
	const float cx = dbg * cf * ay * ratio;
	const float cy = dbg * cf * ay;
	const float cw = near;
	const auto& dir = Vector3Normalize(cam.target - cam.position);
	const auto& strafe = Vector3Normalize(Vector3CrossProduct(dir, cam.up));
	const auto& up = Vector3Normalize(Vector3CrossProduct(strafe, dir));
	const auto cp = cam.position;
	const auto matFsq = rlt::MakeMat4({
		{ cx * strafe.x, cy * up.x, cw * dir.x, cp.x },
		{ cx * strafe.y, cy * up.y, cw * dir.y, cp.y },
		{ cx * strafe.z, cy * up.z, cw * dir.z, cp.z },
		{ 0, 0, 0, 1 },
	});
	return matFsq;
}

inline void UpdateRtSize(RenderTexture& rt, int w, int h)
{
	if (rt.texture.width == w && rt.texture.height == h)
		return;
	TraceLog(LOG_INFO, "rebuild RT %dx%d", w, h);
	if (rt.texture.width != 0)
		UnloadRenderTexture(rt);
	rt = LoadRenderTexture(w, h);
}

inline void DrawRt(RenderTexture rt, int scrW, int scrH, Color c = WHITE)
{
	DrawTexturePro(rt.texture,
		{ 0, 0, float(rt.texture.width), -float(rt.texture.height) },
		{ 0, 0, float(scrW), float(scrH) }, {}, 0, c);
}

template <typename... T>
inline int MyDrawText(int x, int y, Color col, int sz, const char* fmt, T&&... args)
{
	char txt[512] {};
	snprintf(txt, sizeof(txt), fmt, args...);
	DrawText(txt, x, y, sz, col);
	return y + sz;
}

template <typename... T>
inline int MyDrawTextOutline(int x, int y, Color col0, Color col1, int sz, const char* fmt, T&&... args)
{
	char txt[512] {};
	snprintf(txt, sizeof(txt), fmt, args...);
	DrawText(txt, x - 1, y, sz, col0);
	DrawText(txt, x + 1, y, sz, col0);
	DrawText(txt, x, y - 1, sz, col0);
	DrawText(txt, x, y + 1, sz, col0);
	DrawText(txt, x, y, sz, col1);
	return y + sz;
}

template <typename T>
class Dict {
public:
	struct ID {
		using Data = T;
		size_t v {};
	};
	virtual ~Dict() { }
	T& Add(ID& id, bool owned = true)
	{
		id = { vec.size() };
		return vec.emplace_back(T(), owned).first;
	}
	inline const T& Get(ID id) const { return vec[id.v].first; }
	inline T& Get(ID id) { return vec[id.v].first; }
	template <typename F>
	void ForEach(const F& f)
	{
		for (auto& i : vec) {
			f(i.second, i.first);
		}
	}
	template <typename F>
	void Release(const F& f)
	{
		for (auto& i : vec) {
			if (i.second)
				f(i.first);
		}
		vec = {};
	}

protected:
	std::vector<std::pair<T, bool>> vec {};
};

template <typename... R>
struct Resources : public Dict<R>... {
	template <typename T>
	inline typename T::Data& Add(T& id, bool o = true)
	{
		return Dict<typename T::Data>::Add(id, o);
	}
	template <typename T>
	inline const typename T::Data& Get(const T& id) const
	{
		return Dict<typename T::Data>::Get(id);
	}
	template <typename T>
	inline typename T::Data& Get(const T& id)
	{
		return Dict<typename T::Data>::Get(id);
	}
	template <typename T, typename F>
	inline void Release(const F& f)
	{
		return Dict<T>::Release(f);
	}
};

struct BaseControls {
	struct Button {
		GamepadButton button {};
	};
	struct Key {
		KeyboardKey key {};
	};
	struct DirKey {
		KeyboardKey k0 {};
		KeyboardKey k1 {};
	};
	struct Axis {
		GamepadAxis axis {};
	};
	struct MouseButton {
		int button {};
	};
	struct MouseMove {
		int axis {};
	};

	using DigitalCtrl = std::variant<Button, Key, MouseButton>;
	using AnalogCtrl = std::variant<Axis, DirKey, MouseMove>;

	static inline float GetAxis(const AnalogCtrl& ctrl)
	{
		if (auto* mm = std::get_if<MouseMove>(&ctrl)) {
			Vector2 delta = GetMouseDelta();
			const std::array<float, 2> a = { delta.x, delta.y };
			return a[mm->axis] / 4.0f;
		} else if (auto* dirkey = std::get_if<DirKey>(&ctrl)) {
			float v0 = IsKeyDown(dirkey->k0) ? 0.0f : 1.0f;
			float v1 = IsKeyDown(dirkey->k1) ? 0.0f : 1.0f;
			return v0 - v1;
		} else if (auto* axis = std::get_if<Axis>(&ctrl)) {
			int gamepad = 0;
			if (IsGamepadAvailable(gamepad)) {
				return GetGamepadAxisMovement(gamepad, axis->axis);
			}
		}
		return 0.0f;
	}

	static inline bool IsPressed(const DigitalCtrl& ctrl)
	{
		if (auto* mb = std::get_if<MouseButton>(&ctrl)) {
			return IsMouseButtonPressed(mb->button);
		} else if (auto* key = std::get_if<Key>(&ctrl)) {
			return IsKeyPressed(key->key);
		} else if (auto* button = std::get_if<Button>(&ctrl)) {
			int gamepad = 0;
			if (IsGamepadAvailable(gamepad)) {
				return IsGamepadButtonPressed(gamepad, button->button);
			}
		}
		return false;
	}

	static inline bool IsDown(const DigitalCtrl& ctrl)
	{
		if (auto* mb = std::get_if<MouseButton>(&ctrl)) {
			return IsMouseButtonDown(mb->button);
		} else if (auto* key = std::get_if<Key>(&ctrl)) {
			return IsKeyDown(key->key);
		} else if (auto* button = std::get_if<Button>(&ctrl)) {
			int gamepad = 0;
			if (IsGamepadAvailable(gamepad)) {
				return IsGamepadButtonDown(gamepad, button->button);
			}
		}
		return false;
	}
};

} // namespace rlt
