#include "gui_render_system.hpp"

#include "swap_chain.hpp"
#include <iostream>


namespace nEngine::Engine {
	GuiRenderSystem::GuiRenderSystem(Device& device, GLFWwindow* window, VkRenderPass render_pass) : device{device} {
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

		// Setup Platform/Renderer backends
		ImGui_ImplGlfw_InitForVulkan(window, true);
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.ApiVersion = VK_API_VERSION_1_0;              // Pass in your value of VkApplicationInfo::apiVersion, otherwise will default to header version.
		init_info.Instance = device.getInstance();
		init_info.PhysicalDevice = device.getPhysicalDevice();
		init_info.Device = device.device();
		init_info.QueueFamily = device.findPhysicalQueueFamilies().graphicsFamily;
		init_info.Queue = device.graphicsQueue();
		init_info.PipelineCache = VK_NULL_HANDLE;
		init_info.DescriptorPoolSize = 16;
		// customizing needs own descriptor pool ?
		init_info.DescriptorPool = nullptr;
		init_info.RenderPass = render_pass;
		init_info.Subpass = 0;
		init_info.MinImageCount = SwapChain::MAX_FRAMES_IN_FLIGHT;
		init_info.ImageCount = SwapChain::MAX_FRAMES_IN_FLIGHT;
		init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		init_info.Allocator = VK_NULL_HANDLE;
		init_info.CheckVkResultFn = VK_NULL_HANDLE;
		ImGui_ImplVulkan_Init(&init_info);

	}

	GuiRenderSystem::~GuiRenderSystem() {
		auto result = vkDeviceWaitIdle(device.device());
		assert(result == VK_SUCCESS && "ERROR: wait idle failed");

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void GuiRenderSystem::render(const Frame& frame) {
		ImGuiIO& io = ImGui::GetIO();
		// Start the Dear ImGui frame
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		if (showDemoWindow)
			ImGui::ShowDemoWindow(&showDemoWindow);
		//
		if (showMetrics)
			ImGui::ShowMetricsWindow(&showMetrics);
		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.

		if (showSettings)
			showDebugWindow(io, &showSettings, frame);

		// Rendering
		ImGui::Render();
		ImDrawData* draw_data = ImGui::GetDrawData();
		ImGui_ImplVulkan_RenderDrawData(draw_data, frame.cmdBuffer, nullptr);

	}

	void GuiRenderSystem::showDebugWindow(ImGuiIO& io, bool* open, const Frame& frame) {
		ImGui::Begin("Debug");                          // Create a window called "Hello, world!" and append into it.

		static float f = 0.0f;
		static int counter = 0;

		ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
		ImGui::Text("%d vertices, %d indices (%d triangles)", io.MetricsRenderVertices, io.MetricsRenderIndices, io.MetricsRenderIndices / 3);

		if (ImGui::CollapsingHeader("Frame")) {
			if (ImGui::TreeNode("Camera")) {
				
				ImGui::Text("Projection Matrix");
				if (ImGui::BeginTable("Projection Matrix", 4))
				{
					//ImGui::TableSetupColumn("One");
					//ImGui::TableSetupColumn("Two");
					//ImGui::TableSetupColumn("Three");
					//ImGui::TableSetupColumn("Four");
					//ImGui::TableHeadersRow();

					glm::mat4 pm = frame.camera.getProjection();
					for (int row = 0; row < 4; row++)
					{
						ImGui::TableNextRow();
						for (int column = 0; column < 4; column++)
						{
							ImGui::TableSetColumnIndex(column);
							auto& val = pm[column][row];
							ImGui::Text("%.1f", val);
						}
					}
					ImGui::EndTable();
				}

				ImGui::Text("View Matrix");
				if (ImGui::BeginTable("View Matrix", 4))
				{
					//ImGui::TableSetupColumn("One");
					//ImGui::TableSetupColumn("Two");
					//ImGui::TableSetupColumn("Three");
					//ImGui::TableSetupColumn("Four");
					//ImGui::TableHeadersRow();

					glm::mat4 pm = frame.camera.getView();
					for (int row = 0; row < 4; row++)
					{
						ImGui::TableNextRow();
						for (int column = 0; column < 4; column++)
						{
							ImGui::TableSetColumnIndex(column);
							auto& val = pm[column][row];
							ImGui::Text("%.1f", val);
						}
					}
					ImGui::EndTable();
				}

				glm::vec3 position = frame.camera.getPosition();
				ImGui::Text("Position");
				ImGui::Text("X %.1f Y %.1f Z %.1f", position[0], position[1], position[2]);

				ImGui::TreePop();
				ImGui::Spacing();
			}
			ImGui::Separator();
		}

		if (ImGui::CollapsingHeader("Settings")) {
			//ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
			ImGui::Checkbox("Show Metrics", &showMetrics);		// Edit bools storing our window open/close state
			ImGui::Checkbox("VSync", &vsync);
			ImGui::Checkbox("Gamma Correction", &gammaCorrection);
		}

		//ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		//ImGui::ColorEdit3("clear color", (float*)&clearColor);	// Edit 3 floats representing a color
		//ImGui::DragFloat3("Translate", )

		//if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
		//	counter++;
		//ImGui::SameLine();
		//ImGui::Text("counter = %d", counter);

		//ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
		ImGui::End();
	}



}