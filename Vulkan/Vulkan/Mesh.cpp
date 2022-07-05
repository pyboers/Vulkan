#include "Mesh.h"
#include <iostream>
#include <numeric>

void Mesh::draw(vk::CommandBuffer commandBuffer) const
{
	if (m_moved)
		throw "Mesh has been moved. This mesh is now invalid.";
	commandBuffer.bindVertexBuffers(0, { m_buffer }, { 0 });
	commandBuffer.bindIndexBuffer(m_indexBuffer, { 0 }, vk::IndexType::eUint32);

	commandBuffer.drawIndexed(static_cast<uint32_t>(m_indices.size()), 1
		, 0, 0, 0);
}

Mesh::~Mesh() {
	if (m_moved)
		return; //No need to delete. Burden is on the reciever of the move now.
	m_device->m_device.destroyBuffer(m_buffer);
	m_device->m_device.freeMemory(m_bufferMemory);

	m_device->m_device.destroyBuffer(m_indexBuffer);
	m_device->m_device.freeMemory(m_indexBufferMemory);
}


Mesh Mesh::loadModel(const char* filename, const Device *device) {
	FILE* f = fopen(filename, "r"); // open file
	if (!f) {
		throw "Could not open file!";
	}
	char buffer[0xFF];
	std::vector<glm::vec3> positions;
	std::vector<float> vertices;
	std::vector<float> normals;
	std::vector<float> tex;
	std::vector<float> vertexdata;
	while (fgets(buffer, 0xFF, f)) {
		char type[3];
		float x;
		float y;
		float z;
		sscanf(buffer, "%2s %f %f %f", type, &x, &y, &z);
		if (!strcmp(type, "v")) {
			vertices.push_back(x);
			vertices.push_back(y);
			vertices.push_back(z);
		}
		else if (!strcmp(type, "vt")) {
			tex.push_back(x);
			tex.push_back(y);
		}
		else if (!strcmp(type, "vn")) {
			normals.push_back(x);
			normals.push_back(y);
			normals.push_back(z);
		}
		else if (!strcmp(type, "f")) {
			int faces[9];
			sscanf(buffer, "%2s %d/%d/%d %d/%d/%d %d/%d/%d", type, &(faces[0]), &(faces[1]), &(faces[2]),
				&(faces[3]), &(faces[4]), &(faces[5]),
				&(faces[6]), &(faces[7]), &(faces[8]));
			int i;
			for (i = 6; i >= 0; i -= 3) {
				///vertex coords
				vertexdata.push_back(vertices[(faces[i] - 1) * 3]);
				vertexdata.push_back(vertices[((faces[i] - 1) * 3) + 1]);
				vertexdata.push_back(vertices[((faces[i] - 1) * 3) + 2]);
				positions.push_back(glm::vec3(vertices[(faces[i] - 1) * 3], vertices[((faces[i] - 1) * 3) + 1]
					, vertices[((faces[i] - 1) * 3) + 2]));

				//uv coords
				vertexdata.push_back(tex[((faces[i + 1] - 1) * 2)]);
				vertexdata.push_back(tex[((faces[i + 1] - 1) * 2) + 1]);
				//normals
				vertexdata.push_back(normals[(faces[i + 2] - 1) * 3]);
				vertexdata.push_back(normals[((faces[i + 2] - 1) * 3) + 1]);
				vertexdata.push_back(normals[((faces[i + 2] - 1) * 3) + 2]);
				//Todo. Push string to vector. comparestr and dont add if not needed, construct indices as you do that
			}
		}
	}

	std::vector<uint32_t> indices;
	indices.resize(positions.size());

	std::iota(indices.begin(), indices.end(), 0);
	return Mesh(positions, indices, device);
}

void Mesh::setupBuffers()
{
	throw "Mesh has been moved. This mesh is now invalid.";
	vk::BufferCreateInfo bufferInfo({}, m_vertices.size() * sizeof(glm::vec3), vk::BufferUsageFlagBits::eVertexBuffer
		, vk::SharingMode::eExclusive);

	m_buffer = m_device->m_device.createBuffer(bufferInfo, nullptr);

	vk::BufferCreateInfo indexBufferInfo({}, m_indices.size() * sizeof(uint32_t), vk::BufferUsageFlagBits::eIndexBuffer
		, vk::SharingMode::eExclusive);

	m_indexBuffer = m_device->m_device.createBuffer(indexBufferInfo, nullptr);

}

void Mesh::setupMemory()
{
	throw "Mesh has been moved. This mesh is now invalid.";
	vk::MemoryRequirements requirements = m_device->getBufferMemoryRequirements(m_buffer);
	vk::MemoryRequirements indexRequirements = m_device->getBufferMemoryRequirements(m_indexBuffer);

	vk::PhysicalDeviceMemoryProperties memProps = m_device->getMemoryProperties();

	uint32_t memTypeI = 1;
	vk::DeviceSize memHeapSize = 0;

	for (uint32_t i = 0; i < memProps.memoryTypeCount; i++)
	{
		vk::MemoryType memType = memProps.memoryTypes[i];
		if ((vk::MemoryPropertyFlagBits::eHostVisible & memType.propertyFlags) &&
			(vk::MemoryPropertyFlagBits::eHostCoherent & memType.propertyFlags))
		{
			memHeapSize = memProps.memoryHeaps[memType.heapIndex].size;
			memTypeI = i;
			break;
		}
	}

	vk::MemoryAllocateInfo memAllocInfo(requirements.size, memTypeI);
	vk::MemoryAllocateInfo indexMemAllocInfo(indexRequirements.size, memTypeI);
	m_bufferMemory = m_device->allocateMemory(memAllocInfo);
	m_bufferMemory = m_device->allocateMemory(indexMemAllocInfo);

	glm::vec3* bufferData = static_cast<glm::vec3*>(m_device->mapMemory(m_bufferMemory, 0, requirements.size));
	uint32_t* indexBufferData = static_cast<uint32_t*>(m_device->mapMemory(m_indexBufferMemory, 0, indexRequirements.size));
	std::copy(m_vertices.begin(), m_vertices.end(), bufferData);
	std::copy(m_indices.begin(), m_indices.end(), indexBufferData);
	
	m_device->unMapMemory(m_bufferMemory);
	m_device->unMapMemory(m_indexBufferMemory);

	

	m_device->bindBufferMemory(m_buffer, m_bufferMemory, 0);
	m_device->bindBufferMemory(m_indexBuffer, m_indexBufferMemory, 0);
}

Mesh::Mesh(std::vector<glm::vec3> vertices, std::vector<uint32_t> indices, const Device *device) : m_vertices(m_vertices), m_indices(indices), m_device(device) {

}

Mesh::Mesh(Mesh&& mesh)
{
	m_vertices = mesh.m_vertices;
	m_indices = mesh.m_indices;
	m_buffer = mesh.m_buffer;
	m_indexBuffer = mesh.m_indexBuffer;
	m_bufferMemory = mesh.m_bufferMemory;
	m_indexBufferMemory = mesh.m_indexBufferMemory;
	m_device = mesh.m_device;

	mesh.m_moved = true;

}
