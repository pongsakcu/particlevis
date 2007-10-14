// Basic single-light shader: performance-optimized.
// Does not perform any z-writes, allowing z-cull to accelerate the rendering.

varying vec3 light_dir;
//varying vec3 diffuse_color;
varying float this_point_size;
//varying float backdepth;
//varying float radius;

void main (void)
{
	//vec4 in_color = gl_Color;
	vec2 xy;
	xy.x = (gl_TexCoord[0].s - 0.5) * 2.0;
	xy.y = (gl_TexCoord[0].t - 0.5) * -2.0;
	
	float r = length(xy);
	if (r > 1.00)
		discard;
		
	float z = sqrt(1.0 - r*r);
	vec3 normal = vec3(xy, z);

	float color = max(0.0, dot(normal, light_dir));
	vec3 final_color = gl_Color.xyz * color;
	gl_FragColor = vec4( final_color, gl_Color.a);
	//gl_FragDepth = (backdepth-(radius*z)+1.0)/2.0;
}