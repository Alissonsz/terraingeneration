#version 430 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform sampler2D texture1;
uniform sampler2D normalTexture;

out vec3 tangentLightPos;
out vec3 tangentViewPos;
out vec3 tangentFragPos;

in TE_OUT {
  vec3 FragPos;
  vec2 TexCoords;
  vec3 TangentLightPos;
  vec3 TangentViewPos;
  vec3 TangentFragPos;
  float incUV;
} gs_in[];

out GS_OUT {
  vec3 FragPos;
  vec2 TexCoords;
  vec3 TangentLightPos;
  vec3 TangentViewPos;
  vec3 TangentFragPos;
  vec3 normal;
  float hBase;
  vec3 T;
  vec3 B;
  vec3 N;
  float incUV;
} gs_out;

void main( void ) {
    vec3 edge1 = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
    vec3 edge2 = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;

    vec2 deltaUV1  = gs_in[1].TexCoords - gs_in[0].TexCoords;
    vec2 deltaUV2  = gs_in[2].TexCoords - gs_in[0].TexCoords;

    float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y); 

    vec3 tang;
    vec3 bitang;

    tang.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
    tang.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
    tang.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

    bitang.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
    bitang.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
    bitang.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
    //vec3 normal = texture(normalTexture, gs_in[0].TexCoords).rgb;
    vec3 normal = cross(bitang, tang);

    vec3 T = normalize(mat3(model) * tang);
    vec3 B = normalize(mat3(model) * bitang);
    vec3 N = normalize(mat3(model) * normal);
    mat3 TBN = transpose(mat3(T, B, N));
    
    for( int i=0; i < gl_in.length( ); i++ )
    {
        gs_out.TexCoords = gs_in[i].TexCoords;
        gs_out.hBase = gl_in[i].gl_Position.y/10;
        gs_out.normal = normalize(normal);
        gs_out.T = tang;
        gs_out.B = bitang;
        gs_out.N = normalize(normal);
        
        gl_Position = projection * view * model * vec4(gl_in[i].gl_Position.xyz, 1.f);
        //fNormal = normal;

        vec3 fragPos = vec3(model * vec4(gl_in[i].gl_Position.xyz, 1.0));
        gs_out.TangentFragPos  = TBN * fragPos;
        gs_out.TangentLightPos = TBN * lightPos;
        gs_out.TangentViewPos  = TBN * viewPos;
        gs_out.incUV = gs_in[i].incUV;
        gs_out.FragPos = fragPos;

        EmitVertex();
    }

    EndPrimitive( );
}