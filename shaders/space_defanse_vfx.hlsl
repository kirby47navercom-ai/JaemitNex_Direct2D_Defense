// Space Defanse post-process shader reference.
// The current game keeps a stable Direct2D renderer, while SpaceDefanseGameImplRender.cpp
// mirrors these formulas with procedural Direct2D passes for scanlines, bloom,
// chromatic edge offsets, vignette, hit shock rings, halftone dots, cartoon ink
// framing, and combat action lines.

cbuffer VfxConstants : register(b0)
{
    float2 resolution;
    float time;
    float intensity;
    float2 impactUv;
    float impactRadius;
    float impactStrength;
    float3 stageTint;
    float2 unitLightUv;
    float unitLightRadius;
    float unitLightStrength;
};

Texture2D sceneTexture : register(t0);
SamplerState sceneSampler : register(s0);

float hash21(float2 p)
{
    float n = dot(p, float2(12.9898, 78.233));
    return frac(sin(n) * 43758.5453);
}

float ringMask(float2 uv, float2 center, float radius, float width)
{
    float d = distance(uv, center);
    return 1.0 - smoothstep(width, width * 2.0, abs(d - radius));
}

float vignetteMask(float2 uv)
{
    float2 q = uv * (1.0 - uv.yx);
    return saturate(pow(q.x * q.y * 18.0, 0.34));
}

float scanlineMask(float2 uv)
{
    float wave = sin((uv.y * resolution.y + time * 42.0) * 3.14159265);
    return 0.92 + wave * 0.035;
}

float localLightMask(float2 uv, float2 center, float radius)
{
    float d = distance(uv, center);
    return saturate(1.0 - smoothstep(radius * 0.18, radius, d));
}

float directionalShadow(float2 uv, float2 caster, float2 lightDir, float radius)
{
    float2 shadowCenter = caster - normalize(lightDir) * radius * 0.72;
    float2 q = (uv - shadowCenter) / float2(radius * 1.35, radius * 0.42);
    return saturate(1.0 - dot(q, q));
}

float4 main(float4 position : SV_POSITION, float2 uv : TEXCOORD) : SV_TARGET
{
    float2 centered = uv - 0.5;
    float aberration = 0.0025 * intensity * (0.35 + length(centered));

    float3 color;
    color.r = sceneTexture.Sample(sceneSampler, uv + float2(aberration, 0.0)).r;
    color.g = sceneTexture.Sample(sceneSampler, uv).g;
    color.b = sceneTexture.Sample(sceneSampler, uv - float2(aberration, 0.0)).b;

    float ring = ringMask(uv, impactUv, impactRadius, 0.018) * impactStrength;
    float starNoise = step(0.985, hash21(floor(uv * resolution.xy * 0.08) + floor(time * 4.0)));
    float edge = 1.0 - vignetteMask(uv);
    float localLight = localLightMask(uv, unitLightUv, unitLightRadius) * unitLightStrength;
    float shadow = directionalShadow(uv, unitLightUv, normalize(float2(-0.52, -0.86)), unitLightRadius) * unitLightStrength;

    color *= scanlineMask(uv);
    color += stageTint * ring * 0.45;
    color += stageTint * starNoise * 0.16;
    color += stageTint * localLight * 0.22;
    color *= lerp(1.0, 0.68, shadow * 0.42);
    color *= lerp(1.0, 0.55, edge * 0.72);
    color += stageTint * intensity * 0.045;

    return float4(saturate(color), 1.0);
}
