#version 450

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : enable

struct Index {
	uint vertex;
	uint normal;
};

layout(buffer_reference, scalar, std430) readonly buffer Positions {
	vec3 v[];
};

layout(buffer_reference, scalar, std430) readonly buffer IndexBuffer {
	Index data[];
};

layout(buffer_reference, scalar, std430) readonly buffer MatrixBuffer {
	layout(row_major) mat4 data[];
};

layout(push_constant) uniform PushConstants {
	layout(row_major) mat4 view_projection;
	vec4 pos;
	vec4 target;
	Positions positions;
	IndexBuffer indices;
	MatrixBuffer models;
	uint id;
} pc;

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 pos;
layout(location = 0) out vec4 outColor;

void main(void)
{
	vec3 light_pos = pc.pos.xyz;
	outColor = vec4(0.0, 0.0, 0.0, 1.0);
	vec3 light_direction = normalize(light_pos - pos);
	float diff = max(dot(inNormal, light_direction), 0.0);
	vec3 diffuse = diff * inColor.xyz;
	float ambient = 0.0;
	outColor = vec4(ambient + diffuse, 1.0);
}
