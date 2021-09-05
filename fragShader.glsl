#version 430 core
out vec4 FragColor;
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform sampler2D normalTexture;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform int parFlag = 0;
uniform int binaryIter = 5;
uniform mat4 model;

/*in vec2 texCord;
in vec3 Normal;
in vec3 FragPos;*/

in GS_OUT {
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
} te_out;
/*in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} te_out;*/

vec2 parallaxBinarySearch(vec2 texCoords, vec3 viewDir){
    vec2 P = viewDir.xy * 0.3;
    vec3 front;
    vec3 back;
    vec3 mid;

    vec2 prevCoords = texCoords;
    front = vec3(texCoords + (0.1 * viewDir.xz), te_out.hBase + 0.3);
    back  = vec3(texCoords - (0.1 * viewDir.xz), te_out.hBase - 0.3);

    //vec2 uvin = front;
    //vec2 uvout = back;

    vec2 currentCoords;
    float Hmax = 1.0f;
    float Hmin = 0.0f;

    for (int i = 0; i < binaryIter; i++) {
        vec3 mid = (front + back) / 2; // middle

        float hTex = texture(texture1, mid.xy).r;

        if (mid.z < hTex) {
            back = mid;
        } else {
            front = mid;
        }
        currentCoords = front.xy * mid.z + back.xy * (1 - mid.z);
    }

    return currentCoords;
}

vec2 parallaxMapping(vec2 texCoords, vec3 viewDir){
    // number of depth layers
    const float numLayers = 60;
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = te_out.hBase;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xy * 0.3;

    vec2 deltaTexCoords = P / numLayers;
    texCoords = texCoords + P;

    // get initial values
    vec2  currentTexCoords     = texCoords;
    float currentDepthMapValue = (1 - (texture(texture1, currentTexCoords).r));
    vec2 finalCoords;

    while(currentLayerDepth < currentDepthMapValue) {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;

        // get depthmap value at current texture coordinates
        currentDepthMapValue = 1 - (texture(texture1, currentTexCoords).r);

        // get depth of next layer
        currentLayerDepth += layerDepth;

    }

    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // get depth after and before collision for linear interpolation
    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = (1 - (texture(texture1, prevTexCoords).r)) - currentLayerDepth + layerDepth;

    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;
}


void main() {
    vec3 newPos;
    vec3 newNormal;

    vec2 texCoords = te_out.TexCoords;

    vec3 viewDir = normalize(te_out.TangentViewPos - te_out.TangentFragPos);
    if(texCoords.x < 0.0 || texCoords.y < 0.0) discard;
    else if(parFlag == 1)
        texCoords = parallaxMapping(te_out.TexCoords, viewDir);
    else if(parFlag == 2)
        texCoords = parallaxBinarySearch(te_out.TexCoords, viewDir);
    if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
       discard;


    // newNormal = 2 * texture(normalTexture, vec2(texCoords.x, 0 + (1 - texCoords.y))).rgb -1.0;
    // newNormal = normalize(newNormal);

    newNormal = texture(normalTexture, vec2(texCoords.x, 0 + (1 - texCoords.y))).rgb;
    newNormal = normalize(newNormal * 2.0 - 1.0);

    //vec3 norm = normalize(te_out.normal);
    vec3 norm = newNormal;
    vec3 lightColor = texture(texture2, vec2(texCoords)).rgb;

    vec3 lightDir = normalize(lightPos - te_out.FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    float specularStrength = 0.5;

    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;


    float ambientStrength = 0.05;
    vec3 ambient = ambientStrength * lightColor;

    vec4 result = vec4(specular + diffuse + ambient, 1.0);
    //vec4 result = vec4(((te_out.TangentFragPos + 50) / 100).xyz, 1.0);
    //vec4 result = vec4(vec3(viewDir), 1.0);
    //vec4 result = vec4(0, 0, te_out.hBase/10, 1.0);
    FragColor = result;
}
