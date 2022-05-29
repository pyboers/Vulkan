#pragma once
#include <vector>
#include <string>
#include <fstream>


static std::vector<char> readFile(const std::string& fileName) {
    std::ifstream file(fileName, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

static void vkCheck(VkResult result) {
    if (result != VK_SUCCESS) {
        throw std::exception("Vulkan call failed.");
    }
}

static double randNorm() {
    return ((double)rand() / (RAND_MAX));
}

static double randNorm(const float min, const float max) {
    return min + (((double)rand() / (RAND_MAX)) * (max - min));
}