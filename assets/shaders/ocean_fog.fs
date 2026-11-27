#version 330 core
// ==============================================================================
// SHADER BIEN SAU & HE THONG CHIEU SANG (Do Ban B phu trach)
// Anh xa kien thuc CG.6:
// 1. Chieu sang Blinn-Phong (Ambient, Diffuse, Specular)
// 2. Texture Mapping (Lay mau anh diffuse map da ca, san ho)
// 3. Khuc xa tia nang mat troi (Caustics duoi day bien)
// 4. Suong mu bien sau (Underwater Exponential Fog: fog = e^(-density * d))
// 5. Alpha Blending cho kinh vom va mat nuoc
// ==============================================================================

in vec3 vWorldPosition;
in vec3 vWorldNormal;
in vec2 vTexCoord;
in vec4 vColor;

uniform vec4 uTint;
uniform vec3 uViewPosition;
uniform vec3 uFogColor;
uniform float uFogDensity;
uniform float uEmission;
uniform float uTime;
uniform float uPattern;
uniform float uCaustics;
uniform sampler2D uTexture;
uniform float uUseTexture;

out vec4 FragColor;

void main() {
    vec3 normal = normalize(vWorldNormal);
    vec3 lightDirection = normalize(vec3(-0.45, 0.85, 0.38));
    vec3 viewDirection = normalize(uViewPosition - vWorldPosition);
    vec3 halfDirection = normalize(lightDirection + viewDirection);

    // 1. Blinn-Phong Lighting Components (CG.6)
    float diff = max(dot(normal, lightDirection), 0.0);
    float spec = pow(max(dot(normal, halfDirection), 0.0), 42.0);

    // 2. Texture Mapping & Base Color
    vec3 baseColor = uTint.rgb;
    baseColor *= vColor.rgb;
    if (uUseTexture > 0.5) {
        baseColor *= texture(uTexture, vTexCoord).rgb;
    }
    if (uPattern > 0.5) {
        float pattern = sin(vWorldPosition.x * 27.0 + sin(vWorldPosition.y * 19.0))
                      * sin(vWorldPosition.y * 31.0)
                      * sin(vWorldPosition.z * 25.0);
        float spots = step(0.91, pattern);
        baseColor = mix(baseColor, vec3(0.76, 0.91, 1.0), spots * 0.72);
    }

    // 3. Caustics (Tia nang khuc xa tao vet loang nuoc)
    vec2 waterUV = vWorldPosition.xz * 1.35;
    float rippleA = sin(waterUV.x * 3.1 + uTime * 0.75
                      + sin(waterUV.y * 2.0 - uTime * 0.45));
    float rippleB = cos(waterUV.y * 3.7 - uTime * 0.62
                      + cos(waterUV.x * 2.3 + uTime * 0.50));
    float causticCell = 0.5 + 0.5 * rippleA * rippleB;
    float caustic = 1.0 + uCaustics * 0.65 * pow(max(causticCell, 0.0), 10.0);

    // Blinn-Phong combined: Ambient + Diffuse + Specular
    vec3 ambient = 0.28 * baseColor;
    vec3 diffuseColor = 0.76 * diff * baseColor;
    vec3 specularColor = vec3(0.55, 0.78, 0.92) * spec * 0.38;
    vec3 litColor = (ambient + diffuseColor) * caustic + specularColor;
    litColor += baseColor * uEmission;

    // 4. Underwater Exponential Fog (CG.6: fog = e^(-density * distance))
    float distanceToCamera = length(uViewPosition - vWorldPosition);
    float fogFactor = exp(-uFogDensity * distanceToCamera);
    vec3 finalColor = mix(uFogColor, litColor, clamp(fogFactor, 0.0, 1.0));

    // 5. Alpha Blending (CG.6: glBlendFunc, uTint.a xu ly do trong suot kinh va nuoc)
    FragColor = vec4(finalColor, uTint.a * vColor.a);
}
