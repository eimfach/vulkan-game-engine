#include "descriptors.hpp"

// std
#include <cassert>
#include <stdexcept>

namespace Biosim::Engine {

    // *************** Descriptor Set Layout Builder *********************

    DescriptorSetLayout::Builder& DescriptorSetLayout::Builder::addBinding(
        uint32_t binding,
        VkDescriptorType descriptor_type,
        VkShaderStageFlags stage_flags,
        uint32_t count) {
            assert(bindings.count(binding) == 0 && "Binding already in use");
            VkDescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.binding = binding;
            layoutBinding.descriptorType = descriptor_type;
            layoutBinding.descriptorCount = count;
            layoutBinding.stageFlags = stage_flags;
            bindings[binding] = layoutBinding;
            return *this;
    }

    std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::Builder::build() const {
        return std::make_unique<DescriptorSetLayout>(device, bindings);
    }

    // *************** Descriptor Set Layout *********************

    DescriptorSetLayout::DescriptorSetLayout(Device& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
        : device{ device }, bindings{ bindings } {
            std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
            for (auto kv : bindings) {
                setLayoutBindings.push_back(kv.second);
            }

            VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
            descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
            descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

            if (vkCreateDescriptorSetLayout(
                device.device(),
                &descriptorSetLayoutInfo,
                nullptr,
                &descriptorSetLayout) != VK_SUCCESS) {
                throw std::runtime_error("failed to create descriptor set layout!");
            }
    }

    DescriptorSetLayout::~DescriptorSetLayout() {
        vkDestroyDescriptorSetLayout(device.device(), descriptorSetLayout, nullptr);
    }

    // *************** Descriptor Pool Builder *********************

    DescriptorPool::Builder& DescriptorPool::Builder::addPoolSize(
        VkDescriptorType descriptor_type, uint32_t count) {
        poolSizes.push_back({ descriptor_type, count });
        return *this;
    }

    DescriptorPool::Builder& DescriptorPool::Builder::setPoolFlags(
        VkDescriptorPoolCreateFlags flags) {
        poolFlags = flags;
        return *this;
    }
    DescriptorPool::Builder& DescriptorPool::Builder::setMaxSets(uint32_t count) {
        maxSets = count;
        return *this;
    }

    std::unique_ptr<DescriptorPool> DescriptorPool::Builder::build() const {
        return std::make_unique<DescriptorPool>(device, maxSets, poolFlags, poolSizes);
    }

    // *************** Descriptor Pool *********************

    DescriptorPool::DescriptorPool(
        Device& device,
        uint32_t max_sets,
        VkDescriptorPoolCreateFlags pool_flags,
        const std::vector<VkDescriptorPoolSize>& pool_sizes)
        : device{ device } {
            VkDescriptorPoolCreateInfo descriptorPoolInfo{};
            descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
            descriptorPoolInfo.pPoolSizes = pool_sizes.data();
            descriptorPoolInfo.maxSets = max_sets;
            descriptorPoolInfo.flags = pool_flags;

            if (vkCreateDescriptorPool(device.device(), &descriptorPoolInfo, nullptr, &descriptorPool) !=
                VK_SUCCESS) {
                throw std::runtime_error("failed to create descriptor pool!");
            }
    }

    DescriptorPool::~DescriptorPool() {
        vkDestroyDescriptorPool(device.device(), descriptorPool, nullptr);
    }

    bool DescriptorPool::allocateDescriptor(
        const VkDescriptorSetLayout descriptor_set_layout, VkDescriptorSet& descriptor) const {
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = descriptorPool;
            allocInfo.pSetLayouts = &descriptor_set_layout;
            allocInfo.descriptorSetCount = 1;

            // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
            // a new pool whenever an old pool fills up. But this is beyond our current scope
            if (vkAllocateDescriptorSets(device.device(), &allocInfo, &descriptor) != VK_SUCCESS) {
                return false;
            }
            return true;
    }

    void DescriptorPool::freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const {
        vkFreeDescriptorSets(
            device.device(),
            descriptorPool,
            static_cast<uint32_t>(descriptors.size()),
            descriptors.data());
    }

    void DescriptorPool::resetPool() {
        vkResetDescriptorPool(device.device(), descriptorPool, 0);
    }

    // *************** Descriptor Writer *********************

    DescriptorWriter::DescriptorWriter(DescriptorSetLayout& set_layout, DescriptorPool& pool)
        : setLayout{ set_layout }, pool{ pool } {
    }

    DescriptorWriter& DescriptorWriter::writeBuffer(
        uint32_t binding, VkDescriptorBufferInfo* buffer_info) {
            assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

            auto& bindingDescription = setLayout.bindings[binding];

            assert(
                bindingDescription.descriptorCount == 1 &&
                "Binding single descriptor info, but binding expects multiple");

            VkWriteDescriptorSet write{};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.descriptorType = bindingDescription.descriptorType;
            write.dstBinding = binding;
            write.pBufferInfo = buffer_info;
            write.descriptorCount = 1;

            writes.push_back(write);
            return *this;
    }

    DescriptorWriter& DescriptorWriter::writeImage(
        uint32_t binding, VkDescriptorImageInfo* image_info) {
            assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

            auto& bindingDescription = setLayout.bindings[binding];

            assert(
                bindingDescription.descriptorCount == 1 &&
                "Binding single descriptor info, but binding expects multiple");

            VkWriteDescriptorSet write{};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.descriptorType = bindingDescription.descriptorType;
            write.dstBinding = binding;
            write.pImageInfo = image_info;
            write.descriptorCount = 1;

            writes.push_back(write);
            return *this;
    }

    bool DescriptorWriter::build(VkDescriptorSet& set) {
        bool success = pool.allocateDescriptor(setLayout.getDescriptorSetLayout(), set);
        if (!success) {
            return false;
        }
        overwrite(set);
        return true;
    }

    void DescriptorWriter::overwrite(VkDescriptorSet& set) {
        for (auto& write : writes) {
            write.dstSet = set;
        }
        vkUpdateDescriptorSets(pool.device.device(), writes.size(), writes.data(), 0, nullptr);
    }

}  // namespace lve
