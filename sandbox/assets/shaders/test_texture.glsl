#type vertex
#version 450 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_texCoord;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

out vec2 v_texCoord;

void main()
{
    v_texCoord = a_texCoord;
    gl_Position = u_ViewProjection * u_Transform * vec4(a_position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 color;

in vec2 v_texCoord;

uniform sampler2D u_Texture;

layout(std140, binding = 0) uniform MaterialData {
    uniform vec4 u_Color;
};

void main()
{
    color = texture(u_Texture, v_texCoord) * u_Color;
}