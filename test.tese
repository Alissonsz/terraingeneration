#version 430
 
// triangles, quads, or isolines
layout (triangles, equal_spacing, ccw) in;
in vec3 evaluationpoint_wor[];
in vec2 tcTexCoord[];

uniform sampler2D texture1;

in TC_OUT
{
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} te_in[];

out TE_OUT
{
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
    float incUV;
} te_out;

out float incUV;
 
// could use a displacement map here

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 lightPos;
uniform vec3 viewPos;

uniform bool useHeights = false;

float hash(float n) { return fract(sin(n) * 1e4); }
float hash(vec2 p) { return fract(1e4 * sin(17.0 * p.x + p.y * 0.1) * (0.1 + abs(sin(p.y * 13.0 + p.x)))); }

float noise(vec3 x) {
	const vec3 step = vec3(110, 241, 171);

	vec3 i = floor(x);
	vec3 f = fract(x);

	// For performance, compute the base input to a 1D hash from the integer part of the argument and the
	// incremental change to the 1D based on the 3D -> 1D wrapping
    float n = dot(i, step);

	vec3 u = f * f * (3.0 - 2.0 * f);
	return mix(mix(mix( hash(n + dot(step, vec3(0, 0, 0))), hash(n + dot(step, vec3(1, 0, 0))), u.x),
                   mix( hash(n + dot(step, vec3(0, 1, 0))), hash(n + dot(step, vec3(1, 1, 0))), u.x), u.y),
               mix(mix( hash(n + dot(step, vec3(0, 0, 1))), hash(n + dot(step, vec3(1, 0, 1))), u.x),
                   mix( hash(n + dot(step, vec3(0, 1, 1))), hash(n + dot(step, vec3(1, 1, 1))), u.x), u.y), u.z);
}

const mat3 m3  = mat3( 0.00,  0.80,  0.60,
                      -0.80,  0.36, -0.48,
                      -0.60, -0.48,  0.64 );

float fbm( vec3 p ){
    float f = 0.0;
    f += 0.5000*noise( p ); p = m3*p*2.02;
    f += 0.2500*noise( p ); p = m3*p*2.03;
    f += 0.1250*noise( p ); p = m3*p*2.01;
    f += 0.0625*noise( p );

    return f/0.9375;
}

void main () {
  te_out.FragPos   = gl_TessCoord[0] * te_in[0].FragPos
                   + gl_TessCoord[1] * te_in[1].FragPos
	           	     + gl_TessCoord[2] * te_in[2].FragPos;

	te_out.TexCoords = gl_TessCoord[0] * te_in[0].TexCoords
	           	     + gl_TessCoord[1] * te_in[1].TexCoords
	                 + gl_TessCoord[2] * te_in[2].TexCoords;

  vec3 p0 = gl_TessCoord.x * evaluationpoint_wor[0]; // x is one corner
  vec3 p1 = gl_TessCoord.y * evaluationpoint_wor[1]; // y is the 2nd corner
  vec3 p2 = gl_TessCoord.z * evaluationpoint_wor[2]; // z is the 3rd corner (ignore when using quads)
  vec3 pos = (p0 + p1 + p2);
  
  te_out.incUV = 1 / (gl_TessLevelInner[0] * (16 + 1));

  if (useHeights) {
    gl_Position = vec4(pos.x, texture(texture1, te_out.TexCoords).r * 10, pos.z, 1.0);
  } else {
    gl_Position = vec4(pos.x, 0.0, pos.z, 1.0);
  }
}