
// common functions...

float hash(vec2 p) {
	float h = dot(p, vec2(127.1, 311.7));
	return fract(sin(h) * 43758.5453123);
}

float noise( in vec2 p ) {
	vec2 i = floor( p );
	vec2 f = fract( p );
	vec2 u = f * f * (3.0 - 2.0 * f);
	return -1.0 + 2.0 * mix(
		mix( hash(i + vec2(0.0, 0.0)), hash(i + vec2(1.0, 0.0)), u.x),
		mix( hash(i + vec2(0.0, 1.0)), hash(i + vec2(1.0, 1.0)), u.x),
		u.y
	);
}

#if CLOUD

uniform sampler2D cloudDensity;
uniform sampler2D depthMap;

vec3 rgb(float r, float g, float b)
{
	return vec3(r, g, b);
}

vec3 unitToColor(vec3 v) {
	return normalize(v) * 0.5 + 0.5;
}

vec3 colSun = rgb(1.0, 1.0, 1.0);
vec3 colTop = rgb(0.5, 0.7, 1.0);
vec3 colFar = rgb(0.3, 0.5, 0.8);

float cloudH = 250.0;
float dH = 30.0;
float coeffUV = 7251.0;

float sampleCloud1(vec2 uv)
{
	vec2 p = uv;
	vec4 t0 = texture(cloudDensity, p);
	float n1 = (noise(220 * uv) + 1) / 2;
	return (1 - 0.8 * n1) * t0.r;
}

float sampleCloud2(vec3 uv)
{
	vec3 p = uv;
	vec4 t0 = texture(cloudDensity, p.xz);
	//float c = 10.0;
	//vec4 t1 = texture(cloudDensity, 4 * c * p.xy) * texture(cloudDensity, 3 * c * p.yz);
	//vec4 t2 = texture(cloudDensity, 3 * c * p.yx) * texture(cloudDensity, 4 * c * p.zy);
	//return (t0.r * (1 - 0.2 * t1.g)) * (1 - 0.2 * t2.g);
	float n1 = (noise(20 * uv.xz) + 1) / 2;
	float n2 = (noise(220 * uv.xz) + 1) / 2;
	return (1 - 0.8 * n1) * (1 - 0.8 * n2) * t0.r;
}

float cloudTest1(vec2 uv)
{
	float r = sampleCloud1(uv / coeffUV);
	return r;
}

float cloudTest2(vec3 uv)
{
	float r = sampleCloud2(uv / coeffUV);
	return r;
}

vec3 baseSkyColor(vec3 dir, vec3 light) {
	float ty = 0.5 * (dir.y + 1.0);
	float tl = pow(0.5 * (dot(dir, -light) + 1.0), 4);
	vec3 cy = vec3(mix(colFar, colTop, ty));
	vec3 cl = vec3(mix(cy, colSun, tl));
	return cl;
}

vec3 traceClouds(vec3 sky, vec3 base, float hb, float ht, vec3 cam, vec3 from, vec3 to, vec3 dir)
{
	float stride = 0.09;
	float c = 1 - exp(-0.011 * length(to - from));
	vec3 col0 = sky;
	vec3 col1 = mix(sky, colSun, exp(-0.0001 * length(from - cam)));

	int steps = 22;
	for (int i = 0; i < steps; i++) {
		float r0 = float(i) / float(steps - 1);
		vec3 pos = mix(to, from, r0);
		float cloud = cloudTest2(pos);
		float r1 = (pos.y - hb) / (ht - hb);
		float th = 0.5 + 0.8 * (pow(r1 * 2 - 1, 2) - 0.5);
		float d = c * smoothstep(th - stride, th + stride, cloud);
		base = mix(base, mix(col0, col1, r1), d);
	}

	return base;
}

vec3 traceClouds3(vec3 sky, vec3 base, float hb, float ht, vec3 cam, vec3 from, vec3 to, vec3 dir)
{
	vec3 pos = from;
	vec3 delta = to - from;
	vec3 dp = delta / abs(delta.y);
	for (int i = 0; i < 16; i++) {
		float c = 1 - cloudTest1(pos.xz);
		pos = pos + c * dp / 16;
	}
	return traceClouds(sky, base, hb, ht, cam, pos, mix(from, to, 0.7), dir);
}

vec3 makeSkyColor(vec3 sky, vec3 base, vec3 campos, vec3 dir, vec3 wpos) {
	float ht = cloudH + dH;
	float hb = cloudH - dH;
	if (dir.y < 0.0 && (campos.y < hb || wpos.y > ht))
		return base;
	if (dir.y > 0.0 && (campos.y > ht || wpos.y < hb))
		return base;
	vec3 from, to;
	if (dir.y < 0) {
		from = campos - (dir * ((campos.y - ht) / dir.y));
		to = campos - (dir * ((campos.y - hb) / dir.y));
	} else {
		from = campos - (dir * ((campos.y - hb) / dir.y));
		to = campos - (dir * ((campos.y - ht) / dir.y));
	}
	//if ((campos.y - to.y) * (campos.y - from.y) < 0.0)
	//	from = campos;
	if ((to.y - wpos.y) * (wpos.y - from.y) > 0.0)
		to = wpos;
	return traceClouds3(sky, base, hb, ht, campos, from, to, dir);
}

vec3 sceneSkyColor(vec3 light, vec3 base, float z, vec3 campos, vec3 dir, vec3 wpos) {
	vec3 sky = baseSkyColor(dir, light);
	vec4 dr = vec4(sky, z);
	base = mix(base, dr.xyz, z);
	return makeSkyColor(sky, base, campos, dir, wpos);
}

vec3 domeSkyColor(vec3 light, float z, vec3 campos, vec3 dir) {
	vec3 sky = baseSkyColor(dir, light);
	float ht = cloudH + dH;
	float hb = cloudH - dH;
	if (dir.y < 0.0 && campos.y < hb)
		return sky;
	if (dir.y > 0.0 && campos.y > ht)
		return sky;
	vec3 from, to;
	if (dir.y < 0) {
		from = campos - (dir * ((campos.y - ht) / dir.y));
		to = campos - (dir * ((campos.y - hb) / dir.y));
	} else {
		from = campos - (dir * ((campos.y - hb) / dir.y));
		to = campos - (dir * ((campos.y - ht) / dir.y));
	}
	return traceClouds3(sky, sky, hb, ht, campos, from, to, dir);
}

#endif

#if WATER

/*
 * "Seascape" by Alexander Alekseev aka TDM - 2014
 * License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
 * Contact: tdmaav@gmail.com
 */

const int iTime = 0;
const ivec2 iResolution = ivec2(1280, 720);

const int NUM_STEPS = 32;
const float PI = 3.141592;
const float EPSILON = 1e-3;
#define EPSILON_NRM (0.1 / iResolution.x)

// sea
const int ITER_GEOMETRY = 3;
const int ITER_FRAGMENT = 5;
const float SEA_HEIGHT = 0.6;
const float SEA_CHOPPY = 4.0;
const float SEA_SPEED = 0.8;
const float SEA_FREQ = 0.16;
const vec3 SEA_BASE = vec3(0.0, 0.09, 0.18);
//const vec3 SEA_WATER_COLOR = vec3(0.8, 0.9, 0.6)*0.6;
const vec3 SEA_WATER_COLOR = vec3(1, 1, 1);
const mat2 octave_m = mat2(1.6, 1.2, -1.2, 1.6);

#define SEA_TIME (1.0 + iTime * SEA_SPEED)

// water height

float sea_octave(vec2 uv, float choppy) {
	uv += noise(uv);
	vec2 wv = 1.0-abs(sin(uv));
	vec2 swv = abs(cos(uv));
	wv = mix(wv, swv, wv);
	return pow(1.0-pow(wv.x * wv.y, 0.65), choppy);
}

float map(vec3 p) {
	float freq = SEA_FREQ;
	float amp = SEA_HEIGHT;
	float choppy = SEA_CHOPPY;
	vec2 uv = p.xz; uv.x *= 0.75;

	float d, h = 0.0;
	for(int i = 0; i < ITER_GEOMETRY; i++) {
		d = sea_octave((uv+SEA_TIME)*freq, choppy);
		d += sea_octave((uv-SEA_TIME)*freq, choppy);
		h += d * amp;
		uv *= octave_m; freq *= 1.9; amp *= 0.22;
		choppy = mix(choppy, 1.0, 0.2);
	}
	return p.y - h;
}

float map_detailed(vec3 p) {
	float freq = SEA_FREQ;
	float amp = SEA_HEIGHT;
	float choppy = SEA_CHOPPY;
	vec2 uv = p.xz; uv.x *= 0.75;

	float d, h = 0.0;
	for(int i = 0; i < ITER_FRAGMENT; i++) {
		d = sea_octave((uv+SEA_TIME)*freq, choppy);
		d += sea_octave((uv-SEA_TIME)*freq, choppy);
		h += d * amp;
		uv *= octave_m; freq *= 1.9; amp *= 0.22;
		choppy = mix(choppy, 1.0, 0.2);
	}
	return p.y - h;
}

// lighting

float diffuse(vec3 n, vec3 l, float p) {
	return pow(dot(n, l) * 0.4 + 0.6, p);
}

float specular(vec3 n, vec3 l, vec3 e, float s) {
	float nrm = (s + 8.0) / (PI * 8.0);
	return pow(max(dot(reflect(e, n), l), 0.0), s) * nrm;
}

// sky

vec3 getSkyColor(vec3 e) {
	e.y = (max(e.y, 0.0)*0.8+0.2)*0.8;
	return vec3(pow(1.0-e.y, 2.0), 1.0-e.y, 0.6+(1.0-e.y)*0.4) * 1.1;
}

// sea

vec3 getSeaColor(vec3 p, vec3 n, vec3 l, vec3 eye, vec3 dist) {
	float fresnel = clamp(1.0 - dot(n, -eye), 0.0, 1.0);
	fresnel = min(fresnel * fresnel * fresnel, 0.5);

	vec3 reflected = getSkyColor(reflect(eye, n));
	vec3 refracted = SEA_BASE + diffuse(n, l, 80.0) * SEA_WATER_COLOR * 0.12;

	vec3 color = mix(refracted, reflected, fresnel);

	float atten = max(1.0 - dot(dist, dist) * 0.001, 0.0);
	color += SEA_WATER_COLOR * (p.y - SEA_HEIGHT) * 0.18 * atten;

	color += specular(n, l, eye, 600.0 * inversesqrt(dot(dist, dist)));

	return color;
}

// tracing

vec3 getNormal(vec3 p, float eps) {
	vec3 n;
	n.y = map_detailed(p);
	n.x = map_detailed(vec3(p.x+eps, p.y, p.z)) - n.y;
	n.z = map_detailed(vec3(p.x, p.y, p.z+eps)) - n.y;
	n.y = eps;
	return normalize(n);
}

float heightMapTracing(vec3 ori, vec3 dir, out vec3 p) {
	float tm = 0.0;
	float tx = 1000.0;
	float hx = map(ori + dir * tx);
	if(hx > 0.0) {
		p = ori + dir * tx;
		return tx;
	}
	float hm = map(ori);
	for(int i = 0; i < NUM_STEPS; i++) {
		float tmid = mix(tm, tx, hm / (hm - hx));
		p = ori + dir * tmid;
		float hmid = map(p);
		if(hmid < 0.0) {
			tx = tmid;
			hx = hmid;
		} else {
			tm = tmid;
			hm = hmid;
		}
		if(abs(hmid) < EPSILON) break;
	}
	return mix(tm, tx, hm / (hm - hx));
}

vec3 getWorldSea(vec3 ori, vec3 dir, vec3 light)
{
	vec3 p;
	heightMapTracing(ori, dir, p);
	vec3 dist = p - ori;
	vec3 n = getNormal(p, dot(dist, dist) * EPSILON_NRM);
	return getSeaColor(p, n, light, dir, dist);
}

#endif

