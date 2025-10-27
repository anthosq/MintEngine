#type vertex
#version 330 core

layout(location = 0) in vec3 a_position;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

out vec3 v_position;

void main()
{
    v_position = a_position;
    gl_Position = u_ViewProjection * u_Transform * vec4(a_position, 1.0);
}

#type fragment
#version 330 core

layout(location = 0) out vec4 color;

in vec3 v_position;

uniform vec4 u_Color;

void main()
{
    color = u_Color;
}