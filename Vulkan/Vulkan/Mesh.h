#pragma once
#ifndef __Mesh_H
#define __Mesh_H
#define _CRT_SECURE_NO_WARNINGS
#include "vulkan//vulkan.hpp"
#include <vector>;
#include "glm/vec3.hpp"
#include "Device.h"

class Mesh {
private: 
	bool m_moved;
	Device *m_device;
	void setupBuffers();
	void setupMemory();
public:
	std::vector<glm::vec3> m_vertices;
	std::vector<uint32_t> m_indices;

	vk::Buffer m_buffer;
	vk::DeviceMemory m_bufferMemory;

	vk::Buffer m_indexBuffer;
	vk::DeviceMemory m_indexBufferMemory;

	Mesh(Mesh &mesh) = delete;
	Mesh& operator=(Mesh other) = delete;

	Mesh(const std::vector<glm::vec3> vertices, const std::vector<uint32_t> indices, const Device *device);
	Mesh(Mesh&& mesh);

	void draw(vk::CommandBuffer commandBuffer) const;

	~Mesh();

	static Mesh loadModel(const char *filename, const Device *device);
};
#endif