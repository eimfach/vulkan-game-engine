#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace bm {
	class Window {
	
	public:
		Window(int w, int h, std::string name);
		~Window();

		// delete copy constructor and copy operator
		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;

		bool shouldClose() {
			return glfwWindowShouldClose(window);
		}

		bool wasWindowResized() {
			return framebufferResized;
		}

		void resetWindowResizedFlag() {
			framebufferResized = false;
		}

		VkExtent2D getExtent() {
			return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
		}

		void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

	private:
		GLFWwindow* window;
		std::string windowName;
		int width;
		int height;
		bool framebufferResized;

		void initWindow();
		static void framebufferResizedCallback(GLFWwindow* window, int width, int height);
	};
}