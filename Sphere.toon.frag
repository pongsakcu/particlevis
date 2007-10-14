// SPHERE FRAGMENT SHADER: CEL SHADING + OUTLINES
// Take a "cartoon" approach to lighting, fix to 3 values.
// Uses the radius parameter to add a two-pixel black outline to the sphere.

varying vec3 light_dir;
//varying vec3 light_dir_2;
varying vec3 light_half;

varying vec3 diffuse_color;
//varying float alpha;
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
	//float color_2 = max(0.0, dot(normal, light_dir_2));
	float spec = pow(max(0.0, dot(normal, light_half)), 40.0);
	if (spec > 0.5)
		spec = 1.0;
	else
		spec = 0.0;			
	
	//float intensity = (color * gl_LightSource[0].diffuse.x) + (color_2 * gl_LightSource[1].diffuse.x);
	float intensity = color;
	
	if (intensity < 0.1)
		intensity = 0.1;
	else if (intensity < 0.3)
		intensity = 0.4;
	else
		intensity = 1.0;
		
	vec3 final_color = (gl_Color.xyz * intensity) + spec;

	gl_FragColor = vec4( final_color * step(4.0/this_point_size, 1.0 - r), gl_Color.a);	
	gl_FragDepth = (backdepth-(radius*z)+1.0)/2.0;	
}