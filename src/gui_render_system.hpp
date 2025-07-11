#pragma once

#include "device.hpp"
#include "frame.hpp"

// libs
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

namespace SJFGame::Engine {
	class GuiRenderSystem {
	public:
		GuiRenderSystem(Device& device, GLFWwindow* window, VkRenderPass render_pass);
		~GuiRenderSystem();

		// delete copy constructor and copy operator
		GuiRenderSystem(const GuiRenderSystem&) = delete;
		GuiRenderSystem& operator= (const GuiRenderSystem&) = delete;

		void render(const Frame& frame);
	private:
		Device& device;
		bool showDemoWindow{true};
		bool showAnotherWindow{false};
		ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	};
}