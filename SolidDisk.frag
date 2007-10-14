// Basic single-light shader: performance-optimized.
// Does not perform any z-writes, allowing z-cull to accelerate the rendering.

varying vec3 light_dir;
//varying vec3 diffuse_color;
varying float this_point_size;
//varying float backdepth;
//varying float radius;

void main (void)
{
	vec2 xy;
	xy.x = (gl_TexCoord[0].s - 0.5) * 2.0;
	xy.y = (gl_TexCoord[0].t - 0.5) * -2.0;
	
	float r = length(xy);
	if (r > 1.00)
		discard;
	gl_FragColor = gl_Color;
}