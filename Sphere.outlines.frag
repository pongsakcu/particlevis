// SPHERE FRAGMENT SHADER: BLINN-PHONG LIGHTING + OUTLINES
// Uses the radius parameter to add a one-pixel black outline to the sphere.

varying vec3 light_dir;
varying vec3 light_dir_2;
varying vec3 light_half;

varying float this_point_size;
varying float backdepth;
varying float radius;

void main (void)
{
	vec2 xy;
	xy.x = (gl_TexCoord[0].s - 0.5) * 2.0;
	xy.y = (gl_TexCoord[0].t - 0.5) * -2.0;
	
	float r = length(xy);
	if (r > 1.00)
		discard;
		
	float z = sqrt(1.0 - r*r);
	vec3 normal = vec3(xy, z);

	float color = max(0.0, dot(normal, light_dir));
	float color_2 = max(0.0, dot(normal, light_dir_2));
	float spec = pow(max(0.0, dot(normal, light_half)), 40.0);
	vec3 final_color = (gl_Color.rgb * (color+color_2*0.2+normal.z*0.2)) + spec;

	gl_FragColor = vec4( final_color * step(2.0/this_point_size, 1.0 - r), gl_Color.a);	
	gl_FragDepth = (backdepth-(radius*z)+1.0)/2.0;
}