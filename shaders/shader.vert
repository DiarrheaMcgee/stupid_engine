#version 450

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : enable

struct Index {
	uint vertex;
	uint normal;
	uint texture;
};

layout(buffer_reference, std430) readonly buffer Positions {
	float v[][3];
};

layout(buffer_reference, std430) readonly buffer IndexBuffer {
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

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec3 pos;

vec3 getVertex(Index index)
{
    return vec3(pc.positions.v[index.vertex][0],
	            pc.positions.v[index.vertex][1],
	            pc.positions.v[index.vertex][2]);
}

vec3 getNormal(Index index)
{
	return vec3(pc.positions.v[index.normal][0],
	            pc.positions.v[index.normal][1],
	            pc.positions.v[index.normal][2]);
}

vec4 colors[] = {
	vec4(1.0, 1.0, 1.0, 1.0),
	vec4(1.0, 1.0, 1.0, 1.0),
	vec4(1.0, 1.0, 1.0, 1.0),
};

void main(void)
{
	Index index = pc.indices.data[gl_VertexIndex];

	vec4 vertex = vec4(getVertex(index), 1.0);
	vertex = pc.models.data[pc.id * 2] * vertex;
	outNormal = normalize((pc.models.data[pc.id * 2 + 1] * vec4(getNormal(index), 1.0)).xyz);

	pos = vertex.xyz;
	gl_Position = pc.view_projection * vertex;

	outColor = colors[gl_VertexIndex % 3];
}

