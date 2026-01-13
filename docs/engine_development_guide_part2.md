# MintEngine 引擎开发指南 (Part 2)

> 本部分继续深入讨论光照系统、后处理管线、渲染优化等高级主题。

---

## 目录

10. [光照系统深入设计](#10-光照系统深入设计)
11. [阴影系统](#11-阴影系统)
12. [后处理管线](#12-后处理管线)
13. [渲染管线架构](#13-渲染管线架构)
14. [性能优化策略](#14-性能优化策略)
15. [调试工具与可视化](#15-调试工具与可视化)
16. [常见问题与解决方案](#16-常见问题与解决方案)

---

## 10. 光照系统深入设计

### 10.1 光照模型选择

```
┌─────────────────────────────────────────────────────────────┐
│                     光照模型演进                             │
├─────────────────────────────────────────────────────────────┤
│  Flat Shading → Gouraud → Phong → Blinn-Phong → PBR        │
│      ↓             ↓        ↓          ↓           ↓        │
│   最简单      顶点光照  像素光照   改进高光    物理正确     │
└─────────────────────────────────────────────────────────────┘
```

**建议路径**：先实现 Blinn-Phong，再迁移到 PBR

### 10.2 Blinn-Phong 光照实现

```glsl
// ============ shaders/common/lighting.glsl ============

#ifndef LIGHTING_GLSL
#define LIGHTING_GLSL

// 光源数据结构
struct DirectionalLight {
    vec4 direction;      // xyz: direction, w: unused
    vec4 colorIntensity; // xyz: color, w: intensity
};

struct PointLight {
    vec4 position;       // xyz: position, w: radius
    vec4 colorIntensity; // xyz: color, w: intensity
};

struct SpotLight {
    vec4 position;       // xyz: position, w: range
    vec4 direction;      // xyz: direction, w: unused
    vec4 colorIntensity; // xyz: color, w: intensity
    vec4 coneParams;     // x: inner angle cos, y: outer angle cos
};

// 光照 UBO (Binding 3)
layout(std140, binding = 3) uniform LightData {
    DirectionalLight u_DirectionalLight;
    vec4 u_AmbientColor;  // xyz: color, w: intensity
    int u_PointLightCount;
    int u_SpotLightCount;
    vec2 _padding;
    PointLight u_PointLights[16];
    SpotLight u_SpotLights[8];
};

// ============ 衰减函数 ============

// 平滑衰减 (UE4 风格)
float DistanceAttenuation(float distance, float radius) {
    float d = distance / radius;
    float d2 = d * d;
    float d4 = d2 * d2;
    float attenuation = saturate(1.0 - d4);
    return attenuation * attenuation / (distance * distance + 1.0);
}

// 聚光灯衰减
float SpotAttenuation(vec3 lightDir, vec3 spotDir, float innerCos, float outerCos) {
    float cosAngle = dot(lightDir, spotDir);
    return smoothstep(outerCos, innerCos, cosAngle);
}

// ============ Blinn-Phong 光照计算 ============

struct SurfaceData {
    vec3 position;
    vec3 normal;
    vec3 viewDir;
    vec3 albedo;
    float shininess;
};

// 方向光计算
vec3 CalculateDirectionalLight(DirectionalLight light, SurfaceData surface) {
    vec3 lightDir = normalize(-light.direction.xyz);
    vec3 halfDir = normalize(lightDir + surface.viewDir);

    // Diffuse
    float NdotL = max(dot(surface.normal, lightDir), 0.0);
    vec3 diffuse = surface.albedo * NdotL;

    // Specular (Blinn-Phong)
    float NdotH = max(dot(surface.normal, halfDir), 0.0);
    vec3 specular = vec3(pow(NdotH, surface.shininess));

    return (diffuse + specular) * light.colorIntensity.xyz * light.colorIntensity.w;
}

// 点光源计算
vec3 CalculatePointLight(PointLight light, SurfaceData surface) {
    vec3 lightVec = light.position.xyz - surface.position;
    float distance = length(lightVec);
    vec3 lightDir = lightVec / distance;
    vec3 halfDir = normalize(lightDir + surface.viewDir);

    // 衰减
    float attenuation = DistanceAttenuation(distance, light.position.w);
    if (attenuation <= 0.0) return vec3(0.0);

    // Diffuse
    float NdotL = max(dot(surface.normal, lightDir), 0.0);
    vec3 diffuse = surface.albedo * NdotL;

    // Specular
    float NdotH = max(dot(surface.normal, halfDir), 0.0);
    vec3 specular = vec3(pow(NdotH, surface.shininess));

    return (diffuse + specular) * light.colorIntensity.xyz * light.colorIntensity.w * attenuation;
}

// 聚光灯计算
vec3 CalculateSpotLight(SpotLight light, SurfaceData surface) {
    vec3 lightVec = light.position.xyz - surface.position;
    float distance = length(lightVec);
    vec3 lightDir = lightVec / distance;
    vec3 halfDir = normalize(lightDir + surface.viewDir);

    // 距离衰减
    float distAtten = DistanceAttenuation(distance, light.position.w);
    if (distAtten <= 0.0) return vec3(0.0);

    // 聚光衰减
    float spotAtten = SpotAttenuation(-lightDir, light.direction.xyz,
                                       light.coneParams.x, light.coneParams.y);
    if (spotAtten <= 0.0) return vec3(0.0);

    float attenuation = distAtten * spotAtten;

    // Diffuse
    float NdotL = max(dot(surface.normal, lightDir), 0.0);
    vec3 diffuse = surface.albedo * NdotL;

    // Specular
    float NdotH = max(dot(surface.normal, halfDir), 0.0);
    vec3 specular = vec3(pow(NdotH, surface.shininess));

    return (diffuse + specular) * light.colorIntensity.xyz * light.colorIntensity.w * attenuation;
}

// ============ 完整光照计算 ============

vec3 CalculateLighting(SurfaceData surface) {
    // 环境光
    vec3 ambient = u_AmbientColor.xyz * u_AmbientColor.w * surface.albedo;

    // 方向光
    vec3 color = CalculateDirectionalLight(u_DirectionalLight, surface);

    // 点光源
    for (int i = 0; i < u_PointLightCount && i < 16; i++) {
        color += CalculatePointLight(u_PointLights[i], surface);
    }

    // 聚光灯
    for (int i = 0; i < u_SpotLightCount && i < 8; i++) {
        color += CalculateSpotLight(u_SpotLights[i], surface);
    }

    return ambient + color;
}

#endif // LIGHTING_GLSL
```

### 10.3 PBR 光照 (Cook-Torrance BRDF)

```glsl
// ============ shaders/common/pbr.glsl ============

#ifndef PBR_GLSL
#define PBR_GLSL

const float PI = 3.14159265359;

// PBR 材质数据
struct PBRMaterial {
    vec3 albedo;
    float metallic;
    float roughness;
    float ao;
    vec3 emissive;
};

// ============ BRDF 函数 ============

// 法线分布函数 (GGX/Trowbridge-Reitz)
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return a2 / denom;
}

// 几何遮蔽函数 (Smith's method with Schlick-GGX)
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, roughness);
    float ggx2 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// 菲涅尔方程 (Schlick 近似)
vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// 带粗糙度的菲涅尔 (用于 IBL)
vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// ============ PBR 光照计算 ============

vec3 CalculatePBRDirectLight(vec3 lightDir, vec3 lightColor, vec3 N, vec3 V, PBRMaterial mat) {
    vec3 L = normalize(lightDir);
    vec3 H = normalize(V + L);

    // 基础反射率
    vec3 F0 = vec3(0.04);  // 非金属的基础反射率
    F0 = mix(F0, mat.albedo, mat.metallic);

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, mat.roughness);
    float G = GeometrySmith(N, V, L, mat.roughness);
    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    // 镜面反射项
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    // 漫反射项
    vec3 kS = F;  // 镜面反射比例
    vec3 kD = vec3(1.0) - kS;  // 漫反射比例
    kD *= 1.0 - mat.metallic;  // 金属没有漫反射

    float NdotL = max(dot(N, L), 0.0);

    return (kD * mat.albedo / PI + specular) * lightColor * NdotL;
}

// 方向光 PBR
vec3 CalculatePBRDirectionalLight(DirectionalLight light, vec3 N, vec3 V, vec3 worldPos, PBRMaterial mat) {
    vec3 L = normalize(-light.direction.xyz);
    vec3 radiance = light.colorIntensity.xyz * light.colorIntensity.w;
    return CalculatePBRDirectLight(L, radiance, N, V, mat);
}

// 点光源 PBR
vec3 CalculatePBRPointLight(PointLight light, vec3 N, vec3 V, vec3 worldPos, PBRMaterial mat) {
    vec3 lightVec = light.position.xyz - worldPos;
    float distance = length(lightVec);
    vec3 L = lightVec / distance;

    float attenuation = DistanceAttenuation(distance, light.position.w);
    vec3 radiance = light.colorIntensity.xyz * light.colorIntensity.w * attenuation;

    return CalculatePBRDirectLight(L, radiance, N, V, mat);
}

#endif // PBR_GLSL
```

### 10.4 C++ 光照管理

```cpp
// ============ src/render/light.h ============

#pragma once
#include <glm/glm.hpp>
#include "core/ref.h"
#include "render/uniform_buffer.h"

namespace Mint {

    // ============ 光源类型 ============

    struct DirectionalLight {
        glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f);
        glm::vec3 color = glm::vec3(1.0f);
        float intensity = 1.0f;
        bool castShadow = true;
    };

    struct PointLight {
        glm::vec3 position = glm::vec3(0.0f);
        glm::vec3 color = glm::vec3(1.0f);
        float intensity = 1.0f;
        float radius = 10.0f;
    };

    struct SpotLight {
        glm::vec3 position = glm::vec3(0.0f);
        glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f);
        glm::vec3 color = glm::vec3(1.0f);
        float intensity = 1.0f;
        float range = 10.0f;
        float innerConeAngle = 30.0f;  // degrees
        float outerConeAngle = 45.0f;
    };

    // ============ GPU 数据结构 (std140 对齐) ============

    struct alignas(16) GPUDirectionalLight {
        glm::vec4 direction;       // xyz: direction, w: unused
        glm::vec4 colorIntensity;  // xyz: color, w: intensity
    };

    struct alignas(16) GPUPointLight {
        glm::vec4 position;        // xyz: position, w: radius
        glm::vec4 colorIntensity;  // xyz: color, w: intensity
    };

    struct alignas(16) GPUSpotLight {
        glm::vec4 position;        // xyz: position, w: range
        glm::vec4 direction;       // xyz: direction, w: unused
        glm::vec4 colorIntensity;  // xyz: color, w: intensity
        glm::vec4 coneParams;      // x: inner cos, y: outer cos
    };

    struct alignas(16) LightingUBOData {
        GPUDirectionalLight directionalLight;
        glm::vec4 ambientColor;    // xyz: color, w: intensity
        int pointLightCount;
        int spotLightCount;
        glm::vec2 _padding;
        GPUPointLight pointLights[16];
        GPUSpotLight spotLights[8];
    };

    // ============ 光照环境 ============

    class LightEnvironment {
    public:
        LightEnvironment();

        // 设置光源
        void SetDirectionalLight(const DirectionalLight& light);
        void ClearDirectionalLight();

        void AddPointLight(const PointLight& light);
        void ClearPointLights();

        void AddSpotLight(const SpotLight& light);
        void ClearSpotLights();

        // 环境光
        void SetAmbientColor(const glm::vec3& color, float intensity = 1.0f);

        // 上传到 GPU
        void UploadToGPU();

        // 获取数据
        const DirectionalLight* GetDirectionalLight() const {
            return m_hasDirectionalLight ? &m_directionalLight : nullptr;
        }

        const std::vector<PointLight>& GetPointLights() const { return m_pointLights; }
        const std::vector<SpotLight>& GetSpotLights() const { return m_spotLights; }

    private:
        // CPU 数据
        DirectionalLight m_directionalLight;
        bool m_hasDirectionalLight = false;

        std::vector<PointLight> m_pointLights;
        std::vector<SpotLight> m_spotLights;

        glm::vec3 m_ambientColor = glm::vec3(0.1f);
        float m_ambientIntensity = 1.0f;

        // GPU 数据
        Ref<UniformBuffer> m_lightUBO;
        LightingUBOData m_uboData;
        bool m_dirty = true;
    };

} // namespace Mint
```

```cpp
// ============ src/render/light.cpp ============

#include "render/light.h"
#include <glm/gtc/constants.hpp>

namespace Mint {

    LightEnvironment::LightEnvironment() {
        m_lightUBO = UniformBuffer::Create(sizeof(LightingUBOData),
                                           UniformBindingPoints::Lighting);
    }

    void LightEnvironment::SetDirectionalLight(const DirectionalLight& light) {
        m_directionalLight = light;
        m_hasDirectionalLight = true;
        m_dirty = true;
    }

    void LightEnvironment::ClearDirectionalLight() {
        m_hasDirectionalLight = false;
        m_dirty = true;
    }

    void LightEnvironment::AddPointLight(const PointLight& light) {
        if (m_pointLights.size() < 16) {
            m_pointLights.push_back(light);
            m_dirty = true;
        }
    }

    void LightEnvironment::ClearPointLights() {
        m_pointLights.clear();
        m_dirty = true;
    }

    void LightEnvironment::AddSpotLight(const SpotLight& light) {
        if (m_spotLights.size() < 8) {
            m_spotLights.push_back(light);
            m_dirty = true;
        }
    }

    void LightEnvironment::ClearSpotLights() {
        m_spotLights.clear();
        m_dirty = true;
    }

    void LightEnvironment::SetAmbientColor(const glm::vec3& color, float intensity) {
        m_ambientColor = color;
        m_ambientIntensity = intensity;
        m_dirty = true;
    }

    void LightEnvironment::UploadToGPU() {
        if (!m_dirty) return;

        // 清空数据
        m_uboData = {};

        // 方向光
        if (m_hasDirectionalLight) {
            m_uboData.directionalLight.direction = glm::vec4(
                glm::normalize(m_directionalLight.direction), 0.0f);
            m_uboData.directionalLight.colorIntensity = glm::vec4(
                m_directionalLight.color, m_directionalLight.intensity);
        }

        // 环境光
        m_uboData.ambientColor = glm::vec4(m_ambientColor, m_ambientIntensity);

        // 点光源
        m_uboData.pointLightCount = (int)m_pointLights.size();
        for (size_t i = 0; i < m_pointLights.size(); i++) {
            const auto& light = m_pointLights[i];
            m_uboData.pointLights[i].position = glm::vec4(light.position, light.radius);
            m_uboData.pointLights[i].colorIntensity = glm::vec4(light.color, light.intensity);
        }

        // 聚光灯
        m_uboData.spotLightCount = (int)m_spotLights.size();
        for (size_t i = 0; i < m_spotLights.size(); i++) {
            const auto& light = m_spotLights[i];
            m_uboData.spotLights[i].position = glm::vec4(light.position, light.range);
            m_uboData.spotLights[i].direction = glm::vec4(
                glm::normalize(light.direction), 0.0f);
            m_uboData.spotLights[i].colorIntensity = glm::vec4(light.color, light.intensity);
            m_uboData.spotLights[i].coneParams = glm::vec4(
                glm::cos(glm::radians(light.innerConeAngle)),
                glm::cos(glm::radians(light.outerConeAngle)),
                0.0f, 0.0f);
        }

        // 上传
        m_lightUBO->SetData(&m_uboData, sizeof(m_uboData));
        m_dirty = false;
    }

} // namespace Mint
```

---

## 11. 阴影系统

### 11.1 阴影映射原理

```
┌─────────────────────────────────────────────────────────────┐
│                    Shadow Mapping 流程                       │
├─────────────────────────────────────────────────────────────┤
│  Pass 1: 从光源视角渲染场景深度到 Shadow Map                  │
│          ┌─────────────┐                                    │
│  Light → │ Shadow Map  │ (只存深度)                          │
│          │ (2048x2048) │                                    │
│          └─────────────┘                                    │
│                                                             │
│  Pass 2: 正常渲染时，比较像素深度与 Shadow Map 中的深度        │
│          if (pixelDepth > shadowMapDepth) → 在阴影中          │
└─────────────────────────────────────────────────────────────┘
```

### 11.2 ShadowPass 实现

```cpp
// ============ src/render/shadow_pass.h ============

#pragma once
#include "render/render_pass.h"
#include "render/light.h"

namespace Mint {

    struct ShadowPassSpecification {
        uint32_t shadowMapSize = 2048;
        float nearPlane = 0.1f;
        float farPlane = 100.0f;
        float orthoSize = 20.0f;  // 正交投影大小 (方向光)
    };

    class ShadowPass : public RenderPass {
    public:
        ShadowPass(const ShadowPassSpecification& spec);

        void SetLight(const DirectionalLight& light, const glm::vec3& sceneCenter);

        void Begin() override;
        void End() override;

        void RenderMesh(const Ref<MeshResource>& mesh, const glm::mat4& transform);

        // 获取阴影数据
        Ref<Texture2D> GetShadowMap() const;
        glm::mat4 GetLightSpaceMatrix() const { return m_lightSpaceMatrix; }

    private:
        ShadowPassSpecification m_spec;

        Ref<Framebuffer> m_shadowFramebuffer;
        Ref<Shader> m_depthShader;

        glm::mat4 m_lightViewMatrix;
        glm::mat4 m_lightProjectionMatrix;
        glm::mat4 m_lightSpaceMatrix;
    };

} // namespace Mint
```

```cpp
// ============ src/render/shadow_pass.cpp ============

#include "render/shadow_pass.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Mint {

    ShadowPass::ShadowPass(const ShadowPassSpecification& spec)
        : m_spec(spec) {

        // 创建阴影 Framebuffer (只有深度附件)
        FramebufferSpecification fbSpec;
        fbSpec.Width = spec.shadowMapSize;
        fbSpec.Height = spec.shadowMapSize;
        fbSpec.Attachments = { FramebufferTextureFormat::DEPTH32F };
        m_shadowFramebuffer = Framebuffer::Create(fbSpec);

        // 加载深度着色器
        m_depthShader = Shader::Create("shaders/shadow_depth.glsl");
    }

    void ShadowPass::SetLight(const DirectionalLight& light, const glm::vec3& sceneCenter) {
        // 计算光源视图矩阵
        glm::vec3 lightPos = sceneCenter - glm::normalize(light.direction) * m_spec.farPlane * 0.5f;
        m_lightViewMatrix = glm::lookAt(lightPos, sceneCenter, glm::vec3(0, 1, 0));

        // 正交投影 (方向光)
        float size = m_spec.orthoSize;
        m_lightProjectionMatrix = glm::ortho(-size, size, -size, size,
                                              m_spec.nearPlane, m_spec.farPlane);

        m_lightSpaceMatrix = m_lightProjectionMatrix * m_lightViewMatrix;
    }

    void ShadowPass::Begin() {
        m_shadowFramebuffer->Bind();

        glViewport(0, 0, m_spec.shadowMapSize, m_spec.shadowMapSize);
        glClear(GL_DEPTH_BUFFER_BIT);

        // 启用深度偏移减少阴影痤疮
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(2.0f, 4.0f);

        // 可选: 只渲染背面以减少自阴影问题
        // glCullFace(GL_FRONT);

        m_depthShader->Bind();
        m_depthShader->SetMat4("u_LightSpaceMatrix", m_lightSpaceMatrix);
    }

    void ShadowPass::End() {
        glDisable(GL_POLYGON_OFFSET_FILL);
        // glCullFace(GL_BACK);

        m_shadowFramebuffer->Unbind();
    }

    void ShadowPass::RenderMesh(const Ref<MeshResource>& mesh, const glm::mat4& transform) {
        m_depthShader->SetMat4("u_Model", transform);
        mesh->Draw();
    }

    Ref<Texture2D> ShadowPass::GetShadowMap() const {
        return m_shadowFramebuffer->GetDepthAttachment();
    }

} // namespace Mint
```

### 11.3 阴影着色器

```glsl
// ============ shaders/shadow_depth.glsl ============

#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;

uniform mat4 u_LightSpaceMatrix;
uniform mat4 u_Model;

void main() {
    gl_Position = u_LightSpaceMatrix * u_Model * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

void main() {
    // 深度自动写入，不需要输出颜色
    // gl_FragDepth 由 gl_Position.z / gl_Position.w 自动计算
}
```

### 11.4 在主Pass中采样阴影

```glsl
// ============ shaders/common/shadow.glsl ============

#ifndef SHADOW_GLSL
#define SHADOW_GLSL

uniform sampler2D u_ShadowMap;
uniform mat4 u_LightSpaceMatrix;

// 基础阴影计算
float CalculateShadow(vec3 worldPos, vec3 normal, vec3 lightDir) {
    // 转换到光源空间
    vec4 lightSpacePos = u_LightSpaceMatrix * vec4(worldPos, 1.0);
    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
    projCoords = projCoords * 0.5 + 0.5;  // [-1,1] → [0,1]

    // 超出阴影图范围
    if (projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0) {
        return 0.0;  // 不在阴影中
    }

    float currentDepth = projCoords.z;

    // 阴影偏移 (减少阴影痤疮)
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

    // PCF 软阴影
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(u_ShadowMap, 0);

    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            float pcfDepth = texture(u_ShadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    return shadow;
}

// 软阴影 (更高质量)
float CalculateSoftShadow(vec3 worldPos, vec3 normal, vec3 lightDir) {
    vec4 lightSpacePos = u_LightSpaceMatrix * vec4(worldPos, 1.0);
    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0) return 0.0;

    float currentDepth = projCoords.z;
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

    // 5x5 PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(u_ShadowMap, 0);

    for (int x = -2; x <= 2; x++) {
        for (int y = -2; y <= 2; y++) {
            float pcfDepth = texture(u_ShadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 25.0;

    return shadow;
}

#endif // SHADOW_GLSL
```

---

## 12. 后处理管线

### 12.1 后处理系统架构

```
┌─────────────────────────────────────────────────────────────┐
│                    Post-Processing Pipeline                  │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  Scene Color → [Bloom] → [Tone Mapping] → [FXAA] → Output  │
│       ↓           ↓           ↓              ↓              │
│   HDR Buffer   Blur Pass   LDR Convert   Anti-alias        │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### 12.2 PostProcessStack 设计

```cpp
// ============ src/render/post_process/post_process_stack.h ============

#pragma once
#include "core/ref.h"
#include "render/framebuffer.h"
#include "render/shader.h"

namespace Mint {

    // 后处理效果基类
    class PostProcessEffect : public RefCounter {
    public:
        virtual ~PostProcessEffect() = default;

        virtual void Init() = 0;
        virtual void Render(Ref<Framebuffer>& input, Ref<Framebuffer>& output) = 0;
        virtual void OnResize(uint32_t width, uint32_t height) {}

        bool IsEnabled() const { return m_enabled; }
        void SetEnabled(bool enabled) { m_enabled = enabled; }

        int GetOrder() const { return m_order; }
        void SetOrder(int order) { m_order = order; }

    protected:
        bool m_enabled = true;
        int m_order = 0;
    };

    // 后处理栈
    class PostProcessStack {
    public:
        PostProcessStack();
        ~PostProcessStack();

        void Init(uint32_t width, uint32_t height);
        void Resize(uint32_t width, uint32_t height);

        // 添加效果
        template<typename T, typename... Args>
        Ref<T> AddEffect(Args&&... args) {
            auto effect = Ref<T>::Create(std::forward<Args>(args)...);
            effect->Init();
            m_effects.push_back(effect);
            SortEffects();
            return effect;
        }

        void RemoveEffect(const Ref<PostProcessEffect>& effect);

        // 执行后处理
        void Execute(Ref<Framebuffer>& sceneBuffer);

        // 获取最终结果
        Ref<Framebuffer> GetOutputBuffer() const { return m_outputBuffer; }

    private:
        void SortEffects();
        void SwapBuffers();

    private:
        std::vector<Ref<PostProcessEffect>> m_effects;

        Ref<Framebuffer> m_pingBuffer;
        Ref<Framebuffer> m_pongBuffer;
        Ref<Framebuffer> m_outputBuffer;

        Ref<VertexArray> m_fullscreenQuad;
        uint32_t m_width = 0;
        uint32_t m_height = 0;
    };

} // namespace Mint
```

### 12.3 常用后处理效果

```cpp
// ============ Bloom 效果 ============

class BloomEffect : public PostProcessEffect {
public:
    BloomEffect();

    void Init() override;
    void Render(Ref<Framebuffer>& input, Ref<Framebuffer>& output) override;
    void OnResize(uint32_t width, uint32_t height) override;

    // 参数
    float threshold = 1.0f;
    float intensity = 1.0f;
    int blurPasses = 5;

private:
    Ref<Shader> m_thresholdShader;
    Ref<Shader> m_blurShader;
    Ref<Shader> m_combineShader;

    std::vector<Ref<Framebuffer>> m_blurBuffers;  // 多级模糊
};

// ============ Tone Mapping 效果 ============

class ToneMappingEffect : public PostProcessEffect {
public:
    enum class Algorithm {
        Reinhard,
        ACES,
        Uncharted2,
        Exposure
    };

    ToneMappingEffect();

    void Init() override;
    void Render(Ref<Framebuffer>& input, Ref<Framebuffer>& output) override;

    Algorithm algorithm = Algorithm::ACES;
    float exposure = 1.0f;
    float gamma = 2.2f;

private:
    Ref<Shader> m_toneMappingShader;
};

// ============ FXAA 效果 ============

class FXAAEffect : public PostProcessEffect {
public:
    FXAAEffect();

    void Init() override;
    void Render(Ref<Framebuffer>& input, Ref<Framebuffer>& output) override;

    float edgeThreshold = 0.166f;
    float edgeThresholdMin = 0.0833f;

private:
    Ref<Shader> m_fxaaShader;
};
```

### 12.4 后处理着色器示例

```glsl
// ============ shaders/post/tone_mapping.glsl ============

#type vertex
#version 450 core

layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec2 a_TexCoord;

out vec2 v_TexCoord;

void main() {
    v_TexCoord = a_TexCoord;
    gl_Position = vec4(a_Position, 0.0, 1.0);
}

#type fragment
#version 450 core

in vec2 v_TexCoord;
out vec4 FragColor;

uniform sampler2D u_SceneColor;
uniform float u_Exposure;
uniform float u_Gamma;
uniform int u_Algorithm;

// Reinhard Tone Mapping
vec3 ReinhardToneMapping(vec3 color) {
    return color / (color + vec3(1.0));
}

// ACES Filmic Tone Mapping
vec3 ACESFilm(vec3 x) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

// Uncharted 2 Tone Mapping
vec3 Uncharted2Helper(vec3 x) {
    float A = 0.15;
    float B = 0.50;
    float C = 0.10;
    float D = 0.20;
    float E = 0.02;
    float F = 0.30;
    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

vec3 Uncharted2ToneMapping(vec3 color) {
    float W = 11.2;
    vec3 curr = Uncharted2Helper(2.0 * color);
    vec3 whiteScale = vec3(1.0) / Uncharted2Helper(vec3(W));
    return curr * whiteScale;
}

void main() {
    vec3 hdrColor = texture(u_SceneColor, v_TexCoord).rgb;

    // 曝光调整
    hdrColor *= u_Exposure;

    // Tone Mapping
    vec3 mapped;
    if (u_Algorithm == 0) {
        mapped = ReinhardToneMapping(hdrColor);
    } else if (u_Algorithm == 1) {
        mapped = ACESFilm(hdrColor);
    } else if (u_Algorithm == 2) {
        mapped = Uncharted2ToneMapping(hdrColor);
    } else {
        mapped = vec3(1.0) - exp(-hdrColor);  // Exposure
    }

    // Gamma 校正
    mapped = pow(mapped, vec3(1.0 / u_Gamma));

    FragColor = vec4(mapped, 1.0);
}
```

```glsl
// ============ shaders/post/bloom_threshold.glsl ============

#type fragment
#version 450 core

in vec2 v_TexCoord;
out vec4 FragColor;

uniform sampler2D u_SceneColor;
uniform float u_Threshold;

void main() {
    vec3 color = texture(u_SceneColor, v_TexCoord).rgb;

    // 计算亮度
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));

    // 提取高亮部分
    if (brightness > u_Threshold) {
        FragColor = vec4(color, 1.0);
    } else {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}
```

---

## 13. 渲染管线架构

### 13.1 完整的前向渲染管线

```cpp
// ============ src/render/forward_renderer.h ============

#pragma once
#include "render/render_pass.h"
#include "render/shadow_pass.h"
#include "render/post_process/post_process_stack.h"
#include "scene/scene.h"

namespace Mint {

    class ForwardRenderer {
    public:
        ForwardRenderer();
        ~ForwardRenderer();

        void Init(uint32_t width, uint32_t height);
        void Resize(uint32_t width, uint32_t height);

        void BeginFrame();
        void EndFrame();

        void RenderScene(Scene& scene, const Camera& camera);

        // 设置
        void SetShadowsEnabled(bool enabled) { m_shadowsEnabled = enabled; }
        void SetPostProcessEnabled(bool enabled) { m_postProcessEnabled = enabled; }

        // 获取
        Ref<PostProcessStack> GetPostProcessStack() { return m_postProcessStack; }
        Ref<Framebuffer> GetFinalFramebuffer() { return m_finalFramebuffer; }

    private:
        void RenderShadowPass(Scene& scene);
        void RenderOpaquePass(Scene& scene, const Camera& camera);
        void RenderSkyboxPass(const Camera& camera);
        void RenderTransparentPass(Scene& scene, const Camera& camera);
        void RenderPostProcess();

    private:
        uint32_t m_width = 0;
        uint32_t m_height = 0;

        // Framebuffers
        Ref<Framebuffer> m_sceneFramebuffer;   // HDR 场景
        Ref<Framebuffer> m_finalFramebuffer;   // 最终输出

        // Passes
        Ref<ShadowPass> m_shadowPass;
        Ref<PostProcessStack> m_postProcessStack;

        // 全局数据
        Ref<UniformBuffer> m_cameraUBO;
        LightEnvironment m_lightEnvironment;

        // 设置
        bool m_shadowsEnabled = true;
        bool m_postProcessEnabled = true;
    };

} // namespace Mint
```

```cpp
// ============ src/render/forward_renderer.cpp ============

#include "render/forward_renderer.h"
#include <algorithm>

namespace Mint {

    ForwardRenderer::ForwardRenderer() {
        m_cameraUBO = UniformBuffer::Create(sizeof(CameraData), UniformBindingPoints::Camera);

        ShadowPassSpecification shadowSpec;
        shadowSpec.shadowMapSize = 2048;
        m_shadowPass = Ref<ShadowPass>::Create(shadowSpec);

        m_postProcessStack = Ref<PostProcessStack>::Create();
    }

    void ForwardRenderer::Init(uint32_t width, uint32_t height) {
        m_width = width;
        m_height = height;

        // HDR 场景缓冲
        FramebufferSpecification sceneSpec;
        sceneSpec.Width = width;
        sceneSpec.Height = height;
        sceneSpec.Attachments = {
            FramebufferTextureFormat::RGBA16F,   // HDR Color
            FramebufferTextureFormat::DEPTH24STENCIL8
        };
        m_sceneFramebuffer = Framebuffer::Create(sceneSpec);

        // 最终输出 (LDR)
        FramebufferSpecification finalSpec;
        finalSpec.Width = width;
        finalSpec.Height = height;
        finalSpec.Attachments = { FramebufferTextureFormat::RGBA8 };
        m_finalFramebuffer = Framebuffer::Create(finalSpec);

        // 初始化后处理
        m_postProcessStack->Init(width, height);

        // 默认添加 Tone Mapping
        auto toneMapping = m_postProcessStack->AddEffect<ToneMappingEffect>();
        toneMapping->SetOrder(100);  // 最后执行
    }

    void ForwardRenderer::Resize(uint32_t width, uint32_t height) {
        m_width = width;
        m_height = height;

        m_sceneFramebuffer->Resize(width, height);
        m_finalFramebuffer->Resize(width, height);
        m_postProcessStack->Resize(width, height);
    }

    void ForwardRenderer::BeginFrame() {
        // 清空光照
        m_lightEnvironment.ClearPointLights();
        m_lightEnvironment.ClearSpotLights();
    }

    void ForwardRenderer::EndFrame() {
        // 提交渲染命令 (如果使用命令队列)
    }

    void ForwardRenderer::RenderScene(Scene& scene, const Camera& camera) {
        // 1. 收集光照信息
        CollectLights(scene);

        // 2. 更新相机 UBO
        CameraData cameraData;
        cameraData.viewProjection = camera.GetViewProjection();
        cameraData.view = camera.GetViewMatrix();
        cameraData.projection = camera.GetProjectionMatrix();
        cameraData.position = camera.GetPosition();
        m_cameraUBO->SetData(&cameraData, sizeof(cameraData));

        // 3. 阴影 Pass
        if (m_shadowsEnabled && m_lightEnvironment.GetDirectionalLight()) {
            RenderShadowPass(scene);
        }

        // 4. 主场景渲染
        m_sceneFramebuffer->Bind();
        glViewport(0, 0, m_width, m_height);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 4a. 不透明物体
        RenderOpaquePass(scene, camera);

        // 4b. 天空盒
        RenderSkyboxPass(camera);

        // 4c. 透明物体
        RenderTransparentPass(scene, camera);

        m_sceneFramebuffer->Unbind();

        // 5. 后处理
        if (m_postProcessEnabled) {
            RenderPostProcess();
        } else {
            // 直接复制到最终缓冲
            BlitFramebuffer(m_sceneFramebuffer, m_finalFramebuffer);
        }
    }

    void ForwardRenderer::RenderShadowPass(Scene& scene) {
        auto* dirLight = m_lightEnvironment.GetDirectionalLight();

        // 计算场景中心和范围
        glm::vec3 sceneCenter = CalculateSceneCenter(scene);
        m_shadowPass->SetLight(*dirLight, sceneCenter);

        m_shadowPass->Begin();

        // 渲染所有投射阴影的物体
        auto view = scene.GetAllEntitiesWith<TransformComponent, MeshRendererComponent>();
        for (auto entity : view) {
            auto [transform, meshRenderer] = view.get<TransformComponent, MeshRendererComponent>(entity);

            if (!meshRenderer.castShadow || !meshRenderer.mesh) continue;

            m_shadowPass->RenderMesh(meshRenderer.mesh->GetResource(), transform.GetTransform());
        }

        m_shadowPass->End();
    }

    void ForwardRenderer::RenderOpaquePass(Scene& scene, const Camera& camera) {
        // 上传光照数据
        m_lightEnvironment.UploadToGPU();

        // 绑定阴影贴图
        if (m_shadowsEnabled) {
            m_shadowPass->GetShadowMap()->Bind(15);  // 固定槽位
        }

        // 收集不透明物体并排序 (前到后减少 overdraw)
        std::vector<RenderItem> opaqueItems;
        CollectRenderItems(scene, opaqueItems, false);

        // 按材质排序以减少状态切换
        std::sort(opaqueItems.begin(), opaqueItems.end(), [](const auto& a, const auto& b) {
            return a.material.get() < b.material.get();
        });

        // 渲染
        for (const auto& item : opaqueItems) {
            item.material->Bind();

            // 设置阴影相关 Uniform
            if (m_shadowsEnabled) {
                item.material->GetShader()->SetMat4("u_LightSpaceMatrix",
                    m_shadowPass->GetLightSpaceMatrix());
                item.material->GetShader()->SetInt("u_ShadowMap", 15);
            }

            item.mesh->Render(item.transform, item.material);
        }
    }

    void ForwardRenderer::RenderTransparentPass(Scene& scene, const Camera& camera) {
        // 收集透明物体
        std::vector<RenderItem> transparentItems;
        CollectRenderItems(scene, transparentItems, true);

        // 按距离排序 (后到前)
        glm::vec3 cameraPos = camera.GetPosition();
        std::sort(transparentItems.begin(), transparentItems.end(),
            [&cameraPos](const auto& a, const auto& b) {
                float distA = glm::length(glm::vec3(a.transform[3]) - cameraPos);
                float distB = glm::length(glm::vec3(b.transform[3]) - cameraPos);
                return distA > distB;  // 远的先渲染
            }
        );

        // 启用混合
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);  // 禁用深度写入

        for (const auto& item : transparentItems) {
            item.material->Bind();
            item.mesh->Render(item.transform, item.material);
        }

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }

    void ForwardRenderer::RenderPostProcess() {
        m_postProcessStack->Execute(m_sceneFramebuffer);

        // 复制到最终缓冲
        BlitFramebuffer(m_postProcessStack->GetOutputBuffer(), m_finalFramebuffer);
    }

} // namespace Mint
```

### 13.2 延迟渲染预览 (未来方向)

```
┌─────────────────────────────────────────────────────────────┐
│                    Deferred Rendering                        │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  Geometry Pass                    Lighting Pass             │
│  ┌───────────────┐               ┌───────────────┐          │
│  │ GBuffer:      │               │               │          │
│  │  - Position   │──────────────▶│ Full-screen   │          │
│  │  - Normal     │               │ Lighting      │          │
│  │  - Albedo     │               │ Calculation   │          │
│  │  - Metallic   │               │               │          │
│  │  - Roughness  │               └───────────────┘          │
│  └───────────────┘                                          │
│                                                             │
│  优点: 光照计算与物体数量解耦                                 │
│  缺点: 带宽消耗大、不支持透明物体、MSAA 复杂                  │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

---

## 14. 性能优化策略

### 14.1 渲染优化清单

| 优化项 | 实现复杂度 | 性能提升 | 建议阶段 |
|--------|------------|----------|----------|
| 视锥裁剪 | 低 | 高 | Phase 3 |
| 材质排序 | 低 | 中 | Phase 2 |
| 实例化渲染 | 中 | 高 | Phase 4 |
| LOD 系统 | 中 | 高 | Phase 5 |
| 遮挡剔除 | 高 | 高 | Phase 6 |
| Indirect Draw | 高 | 高 | Phase 7 |

### 14.2 视锥裁剪

```cpp
// ============ src/render/frustum.h ============

#pragma once
#include <glm/glm.hpp>
#include <array>

namespace Mint {

    struct Plane {
        glm::vec3 normal;
        float distance;

        float DistanceToPoint(const glm::vec3& point) const {
            return glm::dot(normal, point) + distance;
        }
    };

    struct AABB {
        glm::vec3 min;
        glm::vec3 max;

        glm::vec3 GetCenter() const { return (min + max) * 0.5f; }
        glm::vec3 GetExtents() const { return (max - min) * 0.5f; }

        // 获取相对于平面最远的点
        glm::vec3 GetPositiveVertex(const glm::vec3& normal) const {
            glm::vec3 p = min;
            if (normal.x >= 0) p.x = max.x;
            if (normal.y >= 0) p.y = max.y;
            if (normal.z >= 0) p.z = max.z;
            return p;
        }

        // 获取相对于平面最近的点
        glm::vec3 GetNegativeVertex(const glm::vec3& normal) const {
            glm::vec3 p = max;
            if (normal.x >= 0) p.x = min.x;
            if (normal.y >= 0) p.y = min.y;
            if (normal.z >= 0) p.z = min.z;
            return p;
        }
    };

    class Frustum {
    public:
        Frustum() = default;

        // 从 VP 矩阵提取视锥体
        void ExtractPlanes(const glm::mat4& viewProjection);

        // 检测包围盒是否在视锥内
        bool IsBoxVisible(const AABB& box) const;

        // 检测点是否在视锥内
        bool IsPointVisible(const glm::vec3& point) const;

        // 检测球体是否在视锥内
        bool IsSphereVisible(const glm::vec3& center, float radius) const;

    private:
        // 6 个裁剪面: Left, Right, Bottom, Top, Near, Far
        std::array<Plane, 6> m_planes;
    };

} // namespace Mint
```

```cpp
// ============ src/render/frustum.cpp ============

#include "render/frustum.h"

namespace Mint {

    void Frustum::ExtractPlanes(const glm::mat4& vp) {
        // 从 VP 矩阵提取裁剪面
        // 参考: Fast Extraction of Viewing Frustum Planes from the World-View-Projection Matrix

        // Left plane
        m_planes[0].normal.x = vp[0][3] + vp[0][0];
        m_planes[0].normal.y = vp[1][3] + vp[1][0];
        m_planes[0].normal.z = vp[2][3] + vp[2][0];
        m_planes[0].distance = vp[3][3] + vp[3][0];

        // Right plane
        m_planes[1].normal.x = vp[0][3] - vp[0][0];
        m_planes[1].normal.y = vp[1][3] - vp[1][0];
        m_planes[1].normal.z = vp[2][3] - vp[2][0];
        m_planes[1].distance = vp[3][3] - vp[3][0];

        // Bottom plane
        m_planes[2].normal.x = vp[0][3] + vp[0][1];
        m_planes[2].normal.y = vp[1][3] + vp[1][1];
        m_planes[2].normal.z = vp[2][3] + vp[2][1];
        m_planes[2].distance = vp[3][3] + vp[3][1];

        // Top plane
        m_planes[3].normal.x = vp[0][3] - vp[0][1];
        m_planes[3].normal.y = vp[1][3] - vp[1][1];
        m_planes[3].normal.z = vp[2][3] - vp[2][1];
        m_planes[3].distance = vp[3][3] - vp[3][1];

        // Near plane
        m_planes[4].normal.x = vp[0][3] + vp[0][2];
        m_planes[4].normal.y = vp[1][3] + vp[1][2];
        m_planes[4].normal.z = vp[2][3] + vp[2][2];
        m_planes[4].distance = vp[3][3] + vp[3][2];

        // Far plane
        m_planes[5].normal.x = vp[0][3] - vp[0][2];
        m_planes[5].normal.y = vp[1][3] - vp[1][2];
        m_planes[5].normal.z = vp[2][3] - vp[2][2];
        m_planes[5].distance = vp[3][3] - vp[3][2];

        // 归一化
        for (auto& plane : m_planes) {
            float length = glm::length(plane.normal);
            plane.normal /= length;
            plane.distance /= length;
        }
    }

    bool Frustum::IsBoxVisible(const AABB& box) const {
        for (const auto& plane : m_planes) {
            // 获取相对于平面最远的顶点
            glm::vec3 pVertex = box.GetPositiveVertex(plane.normal);

            // 如果最远的顶点都在平面外，则整个盒子不可见
            if (plane.DistanceToPoint(pVertex) < 0) {
                return false;
            }
        }
        return true;
    }

    bool Frustum::IsSphereVisible(const glm::vec3& center, float radius) const {
        for (const auto& plane : m_planes) {
            if (plane.DistanceToPoint(center) < -radius) {
                return false;
            }
        }
        return true;
    }

} // namespace Mint
```

### 14.3 实例化渲染

```cpp
// ============ 实例化渲染示例 ============

class InstancedRenderer {
public:
    struct InstanceData {
        glm::mat4 transform;
        glm::vec4 color;  // 可选的实例参数
    };

    void Init(const Ref<MeshResource>& mesh, uint32_t maxInstances) {
        m_mesh = mesh;
        m_maxInstances = maxInstances;

        // 创建实例缓冲
        m_instanceBuffer = VertexBuffer::Create(nullptr,
            maxInstances * sizeof(InstanceData), BufferUsage::Dynamic);

        // 设置实例属性布局
        BufferLayout instanceLayout = {
            { ShaderDataType::Mat4, "a_InstanceTransform", true },  // divisor = 1
            { ShaderDataType::Float4, "a_InstanceColor", true },
        };
        m_instanceBuffer->SetLayout(instanceLayout);

        // 添加到 VAO
        m_mesh->GetVertexArray()->AddVertexBuffer(m_instanceBuffer);
    }

    void UpdateInstances(const std::vector<InstanceData>& instances) {
        m_instanceCount = (uint32_t)instances.size();
        if (m_instanceCount > 0) {
            m_instanceBuffer->SetData(instances.data(),
                m_instanceCount * sizeof(InstanceData));
        }
    }

    void Render() {
        if (m_instanceCount == 0) return;

        m_mesh->GetVertexArray()->Bind();
        glDrawElementsInstanced(
            GL_TRIANGLES,
            m_mesh->GetIndexCount(),
            GL_UNSIGNED_INT,
            nullptr,
            m_instanceCount
        );
    }

private:
    Ref<MeshResource> m_mesh;
    Ref<VertexBuffer> m_instanceBuffer;
    uint32_t m_maxInstances = 0;
    uint32_t m_instanceCount = 0;
};
```

---

## 15. 调试工具与可视化

### 15.1 调试渲染器

```cpp
// ============ src/render/debug_renderer.h ============

#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "core/ref.h"

namespace Mint {

    class DebugRenderer {
    public:
        static void Init();
        static void Shutdown();

        // 绘制基本图元
        static void DrawLine(const glm::vec3& start, const glm::vec3& end,
                            const glm::vec4& color = glm::vec4(1.0f));

        static void DrawBox(const glm::vec3& center, const glm::vec3& size,
                           const glm::vec4& color = glm::vec4(1.0f));

        static void DrawWireBox(const glm::vec3& center, const glm::vec3& size,
                               const glm::vec4& color = glm::vec4(1.0f));

        static void DrawSphere(const glm::vec3& center, float radius,
                              const glm::vec4& color = glm::vec4(1.0f));

        static void DrawFrustum(const glm::mat4& viewProjection,
                               const glm::vec4& color = glm::vec4(1.0f));

        static void DrawGrid(const glm::vec3& center, float size, int divisions,
                            const glm::vec4& color = glm::vec4(0.5f));

        // 绘制坐标轴
        static void DrawAxis(const glm::mat4& transform, float size = 1.0f);

        // 绘制骨骼
        static void DrawSkeleton(const std::vector<glm::mat4>& boneTransforms,
                                const std::vector<int>& parentIndices);

        // 绘制包围盒
        static void DrawAABB(const AABB& aabb, const glm::vec4& color = glm::vec4(0, 1, 0, 1));

        // 执行渲染
        static void Flush(const glm::mat4& viewProjection);

    private:
        struct LineVertex {
            glm::vec3 position;
            glm::vec4 color;
        };

        static std::vector<LineVertex> s_lineVertices;
        static Ref<VertexArray> s_lineVAO;
        static Ref<VertexBuffer> s_lineVBO;
        static Ref<Shader> s_lineShader;
    };

} // namespace Mint
```

### 15.2 性能统计

```cpp
// ============ src/render/render_stats.h ============

#pragma once
#include <cstdint>

namespace Mint {

    struct RenderStats {
        uint32_t drawCalls = 0;
        uint32_t triangles = 0;
        uint32_t vertices = 0;
        uint32_t meshesRendered = 0;
        uint32_t meshesCulled = 0;
        uint32_t textureBinds = 0;
        uint32_t shaderBinds = 0;

        float frameTime = 0.0f;  // ms
        float gpuTime = 0.0f;    // ms (需要 GPU 计时器查询)

        void Reset() {
            drawCalls = 0;
            triangles = 0;
            vertices = 0;
            meshesRendered = 0;
            meshesCulled = 0;
            textureBinds = 0;
            shaderBinds = 0;
        }
    };

    class RenderStatsCollector {
    public:
        static RenderStats& Get() { return s_stats; }

        static void BeginFrame() {
            s_stats.Reset();
            s_frameStartTime = GetTime();
        }

        static void EndFrame() {
            s_stats.frameTime = (GetTime() - s_frameStartTime) * 1000.0f;
        }

        static void RecordDrawCall(uint32_t triangleCount, uint32_t vertexCount) {
            s_stats.drawCalls++;
            s_stats.triangles += triangleCount;
            s_stats.vertices += vertexCount;
        }

    private:
        static RenderStats s_stats;
        static double s_frameStartTime;
    };

} // namespace Mint
```

### 15.3 ImGui 调试面板

```cpp
// ============ 渲染统计面板 ============

void DrawRenderStatsPanel() {
    ImGui::Begin("Render Statistics");

    const auto& stats = RenderStatsCollector::Get();

    ImGui::Text("Frame Time: %.3f ms (%.1f FPS)",
                stats.frameTime, 1000.0f / stats.frameTime);

    ImGui::Separator();

    ImGui::Text("Draw Calls: %u", stats.drawCalls);
    ImGui::Text("Triangles: %u", stats.triangles);
    ImGui::Text("Vertices: %u", stats.vertices);

    ImGui::Separator();

    ImGui::Text("Meshes Rendered: %u", stats.meshesRendered);
    ImGui::Text("Meshes Culled: %u", stats.meshesCulled);
    ImGui::Text("Cull Efficiency: %.1f%%",
                100.0f * stats.meshesCulled /
                (stats.meshesRendered + stats.meshesCulled + 0.001f));

    ImGui::Separator();

    ImGui::Text("Texture Binds: %u", stats.textureBinds);
    ImGui::Text("Shader Binds: %u", stats.shaderBinds);

    ImGui::End();
}

// ============ GBuffer 可视化 (延迟渲染调试) ============

void DrawGBufferDebugPanel(Ref<Framebuffer>& gbuffer) {
    ImGui::Begin("GBuffer Debug");

    const char* items[] = { "Final", "Position", "Normal", "Albedo", "Depth" };
    static int currentItem = 0;
    ImGui::Combo("View", &currentItem, items, IM_ARRAYSIZE(items));

    ImVec2 size = ImGui::GetContentRegionAvail();

    uint64_t texId;
    switch (currentItem) {
        case 0: texId = m_finalBuffer->GetColorAttachmentRendererID(); break;
        case 1: texId = gbuffer->GetColorAttachmentRendererID(0); break;
        case 2: texId = gbuffer->GetColorAttachmentRendererID(1); break;
        case 3: texId = gbuffer->GetColorAttachmentRendererID(2); break;
        case 4: texId = gbuffer->GetDepthAttachmentRendererID(); break;
    }

    ImGui::Image((void*)texId, size, ImVec2(0, 1), ImVec2(1, 0));

    ImGui::End();
}
```

---

## 16. 常见问题与解决方案

### 16.1 渲染问题诊断

| 现象 | 可能原因 | 解决方案 |
|------|----------|----------|
| 黑屏 | Shader 编译失败 | 检查日志，验证 Shader 语法 |
| 黑屏 | Framebuffer 未解绑 | 确保 Unbind() 被调用 |
| 模型不显示 | 背面剔除 | 检查顶点顺序或禁用剔除 |
| 模型不显示 | 深度测试 | 检查近/远裁剪面 |
| Z-Fighting | 深度精度不足 | 调整近裁剪面，使用对数深度 |
| 阴影痤疮 | 偏移不足 | 增加 PolygonOffset 或 bias |
| 纹理闪烁 | Mipmap 缺失 | 生成 Mipmap |
| 颜色偏暗 | Gamma 未校正 | 应用 Gamma 校正 |
| 颜色过曝 | HDR 未处理 | 添加 Tone Mapping |

### 16.2 性能问题诊断

| 现象 | 可能原因 | 解决方案 |
|------|----------|----------|
| 低帧率 | Draw Call 过多 | 合批、实例化 |
| 低帧率 | 状态切换频繁 | 材质排序 |
| 低帧率 | 带宽瓶颈 | 压缩纹理、降低分辨率 |
| 卡顿 | 资源加载 | 异步加载 |
| 内存泄漏 | 资源未释放 | 检查引用计数 |

### 16.3 调试技巧

```cpp
// 1. OpenGL 错误检查
#define GL_CHECK(x) do { \
    x; \
    GLenum err = glGetError(); \
    if (err != GL_NO_ERROR) { \
        MINT_ERROR("OpenGL Error {}: {} at {}:{}", err, #x, __FILE__, __LINE__); \
    } \
} while(0)

// 2. 使用 RenderDoc
// - 在创建窗口前加载 RenderDoc
// - F12 捕获帧
// - 分析 Draw Call、纹理、Shader

// 3. GPU 计时查询
class GPUTimer {
public:
    void Begin() {
        glBeginQuery(GL_TIME_ELAPSED, m_queryId);
    }

    void End() {
        glEndQuery(GL_TIME_ELAPSED);
    }

    float GetTimeMs() {
        GLuint64 result;
        glGetQueryObjectui64v(m_queryId, GL_QUERY_RESULT, &result);
        return result / 1000000.0f;  // ns -> ms
    }

private:
    GLuint m_queryId;
};
```

---

*本文档为 MintEngine 开发指南的第二部分，涵盖了高级渲染主题。请结合 Part 1 一起阅读。*
