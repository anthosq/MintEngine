#type vertex
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 view;
uniform mat4 projection;

out vec3 nearPoint;
out vec3 farPoint;
out mat4 fragView;
out mat4 fragProjection;

vec3 UnprojectPoint(float x, float y, float z,mat4 viewInv, mat4 ProjInv) {
    vec4 unprojectedPoint = viewInv * ProjInv * vec4(x, y, z, 1.0);
    return unprojectedPoint.xyz / unprojectedPoint.w;
}

void main() {
    mat4 viewInverse = inverse(view);
    mat4 projectionInverse = inverse(projection);
    nearPoint = UnprojectPoint(aPos.x, aPos.y, 0.0, viewInverse, projectionInverse);
    farPoint = UnprojectPoint(aPos.x, aPos.y, 1.0, viewInverse, projectionInverse);

    fragView = view;
    fragProjection = projection;

    gl_Position = vec4(aPos, 1.0);
}

#type fragment
#version 330 core
out vec4 FragColor;

in vec3 nearPoint;
in vec3 farPoint;
in mat4 fragView;
in mat4 fragProjection;

uniform float gridSize;
uniform float gridCellSize;
uniform vec3 gridColorThin;
uniform vec3 gridColorThick;
uniform float near;
uniform float far;

vec4 grid(vec3 fragPos, float scale) {
    vec2 coord = fragPos.xz * scale;
    vec2 derivative = fwidth(coord);
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
    float line = min(grid.x, grid.y);
    
    vec4 color = vec4(gridColorThin, 1.0 - min(line, 1.0));
    
    float axisWidth = 0.05;
    
    if (abs(fragPos.x) < axisWidth) {
        color = vec4(0.2, 0.2, 1.0, 1.0);
    }
    
    if (abs(fragPos.z) < axisWidth) {
        color = vec4(1.0, 0.2, 0.2, 1.0);
    }
    
    return color;
}

float computeDepth(vec3 pos) {
    vec4 clip_space_pos = fragProjection * fragView * vec4(pos, 1.0);
    float ndc_depth = clip_space_pos.z / clip_space_pos.w;
    return (ndc_depth + 1.0) * 0.5;
}

float computeLinearDepth(vec3 pos) {
    vec4 clip_space_pos = fragProjection * fragView * vec4(pos, 1.0);
    float clip_space_depth = (clip_space_pos.z / clip_space_pos.w) * 2.0 - 1.0;
    float linear_depth = (2.0 * near * far) / (far + near - clip_space_depth * (far - near));
    return linear_depth / far;
}

void main() {
    float t = -nearPoint.y / (farPoint.y - nearPoint.y);

    // 检查射线是否与平面相交
    if (t < 0.0) {
        discard;
    }
    
    // 计算交点的世界坐标
    vec3 fragPos = nearPoint + t * (farPoint - nearPoint);

    float maxDistance = gridSize * 0.5;
    if (abs(fragPos.x) > maxDistance || abs(fragPos.z) > maxDistance) {
        discard;
    }

    // 设置深度
    gl_FragDepth = computeDepth(fragPos);
    
    float linearDepth = computeLinearDepth(fragPos);
    float fading = max(0.0, 0.8 - linearDepth * 0.5);

    // 绘制网格
    vec4 color = grid(fragPos, gridCellSize);
    color.a *= fading;

    // Alpha 测试
    if (color.a < 0.005) {
        discard;
    }

    FragColor = color;
}
