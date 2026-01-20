#type vertex
#version 330 core
layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_texCoord;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;
out vec2 v_TexCoord;

void main() {
    gl_Position = u_ViewProjection * u_Transform * vec4(a_position, 1.0);
    v_TexCoord = a_texCoord;
}

#type fragment
#version 330 core
in vec2 v_TexCoord;

out vec4 fragColor;

uniform sampler2D u_Texture;

void main() {
    fragColor = texture(u_Texture, v_TexCoord);
}
