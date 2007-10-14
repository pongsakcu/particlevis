varying vec3 light_dir;
//varying vec3 light_dir_2;
//varying vec3 light_half;
varying float this_point_size;
//varying float backdepth;
varying float radius;

varying vec2 window_pos;
varying vec3 world_pos;
varying float world_radius;

attribute float psize;
uniform float viewport_mapping_w;
uniform float viewport_mapping_h;
varying vec2 viewport;

void main (void)
{	   	
    // Eye-coordinate position of vertex, needed in various calculations   
    
    //gl_Position = ftransform();
    vec4 ecPosition = gl_ModelViewMatrix * gl_Vertex;
    gl_Position = gl_ProjectionMatrix * ecPosition;
    //float dist = length(vec3(ecPosition)/ecPosition.w);
	
    vec4 clipDir = normalize(gl_ModelViewProjectionMatrixInverse * vec4(1.0, 0.0, 0, 0)) * psize + gl_Vertex;
    vec4 clipPosition = gl_ModelViewProjectionMatrix * clipDir;
     
    vec3 p1 = gl_Position.xyz / gl_Position.w;
    vec3 p2 = clipPosition.xyz / clipPosition.w;
                 
	radius = length(p1-p2);
    float ps = (viewport_mapping_w * radius);
    gl_PointSize = ps*1.5;
    this_point_size = ps*1.5;
    
    vec4 z_Dir = normalize(gl_ModelViewMatrixInverse * vec4(0.0, 0.0, 1.0, 0)) * psize + gl_Vertex;
    vec4 z_Pos = gl_ModelViewProjectionMatrix * z_Dir;
    vec3 pz = z_Pos.xyz / z_Pos.w;
    
    //backdepth = p1.z;
    //radius = (p1.z - pz.z);
    window_pos = vec2(p1.x * viewport_mapping_w, p1.y * viewport_mapping_h);
    world_pos = gl_Vertex.xyz;
    world_radius = psize;
    viewport = vec2(viewport_mapping_w, viewport_mapping_h);

    // Do fixed functionality vertex transform          
            
    gl_FrontColor = gl_Color;
    
    light_dir = normalize(gl_LightSource[0].position.xyz - gl_LightSource[0].position.w*ecPosition.xyz);
    //light_dir_2 = normalize(gl_LightSource[1].position.xyz);
    //light_dir_2 =  vec3(gl_ModelViewMatrix * vec4(0, -1, 0, 0));    
    //light_half = (light_dir + vec3(0, 0, 1.0));  
}

//float ps = radius * (gl_DepthRange.near / dist) * (viewport_mapping / gl_DepthRange.diff);