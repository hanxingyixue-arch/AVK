#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform ShaderArgs
{
	uint array_size;
} args;

layout(location = 0) in vec4 frag_color;
layout(location = 0) out vec4 out_color;

void main()
{
	out_color = frag_color;
}