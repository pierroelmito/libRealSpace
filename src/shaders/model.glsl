
@vs vs
layout(location=0) in vec4 position;
out vec2 uv;
void main() {
	uv = 0.5 * (vec2(1, -1) * position.xy + 1);
	gl_Position = position;
}
@end

@fs fs
uniform sampler2D bitmap;
in vec2 uv;
out vec4 frag_color;
void main() {
	vec4 tc = texture(bitmap, uv);
	frag_color = vec4(tc.xyz, 1);
}
@end

@program model vs fs
