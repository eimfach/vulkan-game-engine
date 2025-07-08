#include "window.hpp"

#include <stdexcept>

namespace SJFGame::Engine {
	Window::Window(int w, int h, std::string name) : width{ w }, height{ h }, windowName{ name } {
		initWindow();
	}

	Window::~Window() {
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void Window::initWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizedCallback);
	}

	void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
		if (glfwCreateWindowSurface(instance, window, nullptr, surface)) {
			throw std::runtime_error("Failed to create runtime surface");
		}
	}

	void Window::framebufferResizedCallback(GLFWwindow* window, int width, int height) {
		auto next_window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		next_window->framebufferResized = true;
		next_window->width = width;
		next_window->height = height;
	}
}