#pragma once
#include "VulkanBuffer.hpp"
#include "VulkanTexture.hpp"

#include <utility>

class VulkanDescriptorSet
{
private:
	friend class VulkanGpuDevice;
	friend class VulkanCmdBuffer;
	vk::DescriptorSet set; // this goes to Commandlist
  vk::PipelineLayout layout;
	std::vector<std::pair< unsigned, VulkanBufferShaderView >> buffers;
	std::vector<std::pair< unsigned, VulkanTextureShaderView>> textures;
public:
	VulkanDescriptorSet(vk::DescriptorSet set, vk::PipelineLayout layout)
		:set(set), layout(layout)
	{}

	void bind(unsigned slot, VulkanBufferShaderView& buffer);
	void bind(unsigned slot, VulkanTextureShaderView& texture);
	std::vector<vk::WriteDescriptorSet> compile();
};

using DescriptorSetImpl = VulkanDescriptorSet;