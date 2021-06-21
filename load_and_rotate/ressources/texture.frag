#version 440

layout(location = 0) in vec2 v_texcoord;

layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 mvp;
    int flip;
} ubuf;

layout(binding = 1) uniform sampler2D tex;

layout(std140, binding = 2) uniform s_buf {
    float scale;
}scale_buf;

void main()
{
    vec2 v_newtexcoord = v_texcoord / scale_buf.scale;
    vec4 c = texture(tex, v_newtexcoord);
    fragColor = vec4(c.rgb * c.a, c.a);
}
