// Vertex shader for blurred spheres, rendered via point sprites.
// Transform the vertex as well as a point on the surface of the sphere to size the point sprite.
// Additionally pass the eyespace velocity vector for use in motion blur.

varying vec3 light_dir;
//varying vec3 diffuse_color;
varying float this_point_size;
//varying float backdepth;
//varying float radius;
varying vec2 velocity;
varying float vel_length;

attribute float psize;
uniform float viewport_mapping;

void main (void)
{	   	
    // Eye-coordinate position of vertex, needed in various calculations       
    //gl_Position = ftransform();
    vec4 ecPosition = gl_ModelViewMatrix * gl_Vertex;
    gl_Position = gl_ProjectionMatrix*ecPosition;
    float dist = length(vec3(ecPosition)/ecPosition.w);        
    
    vec2 eye_velocity = (gl_NormalMatrix*gl_MultiTexCoord0.xyz).xy;
    velocity = normalize(eye_velocity);
    vel_length = min(length(eye_velocity), 1.0);
    
    vec4 clipDir = normalize(gl_ModelViewProjectionMatrixInverse * vec4(0.0, 1.0, 0, 0)) * psize + gl_Vertex;
    vec4 clipPosition = gl_ModelViewProjectionMatrix * clipDir;     
    
    vec3 p1 = gl_Position.xyz / gl_Position.w;
    vec3 p2 = clipPosition.xyz / clipPosition.w;
    float ps = (viewport_mapping * length(p1-p2));
    gl_PointSize = ps * (1.0 + vel_length); //2.0;
    this_point_size = ps * (1.0 + vel_length); //2.0;
    
    //vec4 z_Dir = normalize(gl_ModelViewMatrixInverse * vec4(0.0, 0.0, 1.0, 0)) * psize + gl_Vertex;
    //vec4 z_Pos = gl_ModelViewProjectionMatrix * z_Dir;
    //vec3 pz = z_Pos.xyz / z_Pos.w;    
    //backdepth = p1.z;
    //radius = (p1.z - pz.z);
    
    // Do fixed functionality vertex transform
    //diffuse_color = gl_Color.rgb;
    gl_FrontColor = gl_Color;
	//light_dir = normalize(gl_LightSource[0].position.xyz);
    light_dir = normalize(gl_LightSource[0].position.xyz - gl_LightSource[0].position.w*ecPosition.xyz);
}
