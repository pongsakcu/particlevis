varying vec3 light_dir;
//varying vec3 light_dir_2;
//varying vec3 light_half;
varying float this_point_size;
//varying float backdepth;
varying float radius;

varying vec2 window_pos;
varying vec3 world_pos;
varying vec2 viewport;
varying float world_radius;

void main (void)
{
	//vec4 t = texture2D(tex, gl_TexCoord[0].st); // texture lookup is fairly slow
	vec2 xy;
	xy.x = (gl_TexCoord[0].s - 0.5) * 2.0 * this_point_size;
	xy.y = (gl_TexCoord[0].t - 0.5) * -2.0 * this_point_size;	
		
	vec2 window_loc = window_pos.xy + xy;
	vec4 clip_loc = vec4(window_loc.x /  viewport.x, window_loc.y / viewport.y, 0, 1);
	vec4 world_loc = (gl_ModelViewProjectionMatrixInverse * clip_loc);
	vec4 world_ray_loc = (gl_ModelViewProjectionMatrixInverse * (clip_loc + vec4(0,0,1,0)) );
	
	float wradius_inv = 1.0 / world_radius;		
	vec3 ray_origin = wradius_inv*((world_loc.xyz / world_loc.w) - (world_pos));
	vec3 ray_direction = normalize( (world_ray_loc.xyz/world_ray_loc.w) - (world_loc.xyz / world_loc.w) );				
	float b = dot(ray_origin, ray_direction);
	float discr = 4.0 * (b*b - dot(ray_origin, ray_origin)) + 4.0;
	if (discr < 0)
		discard;				
	float z = -b - sqrt(discr)/2.0; // front sphere location: ray_origin + z * ray_direction
	vec3 normal = gl_NormalMatrix * (ray_origin + (z * ray_direction));		
	//gl_FragDepth = (backdepth-(radius*z)+1.0)/2.0;	
	//gl_FragDepth = ((z * world_radius) - gl_DepthRange.near) / gl_DepthRange.diff;
	float color = max(0.0, dot(normal, light_dir));
	//float color_2 = max(0.0, dot(normal, light_dir_2));
	//float spec = pow(max(0.0, dot(normal, normalize(light_half))), 40.0);
	//vec3 final_color = (in_color.xyz * (color+color_2*0.2+normal.z*0.05 - backdepth*0.5)) + spec;
		
	gl_FragColor = vec4( color * gl_Color.rgb, gl_Color.a);
	//gl_FragColor = vec4(1, 1, 1, 1);
}