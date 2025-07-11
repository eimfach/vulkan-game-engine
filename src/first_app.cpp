#include "first_app.hpp"

#include "buffer.hpp"
#include "camera.hpp"
#include "movement.hpp"
#include "simple_render_system.hpp"
#include "pointlight_render_system.hpp"
#include "texture.hpp"
#include "main_render_system.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

//std
#include <chrono>
#include <ratio>

namespace SJFGame {


	FirstApp::FirstApp() {
		loadGameObjects();
	}

	FirstApp::~FirstApp() {
		globalPool = nullptr;
	}

	void FirstApp::run() {

		Engine::MainRenderSystem main_render_sys{ device };
		Engine::SimpleRenderSystem render_system{ device, renderer.getSwapChainRenderPass(), 
			main_render_sys.getGobalSetLayout()};
		Engine::PointLightRenderSystem point_light_render_system{ device, renderer.getSwapChainRenderPass(), 
			main_render_sys.getGobalSetLayout() };

		Engine::Camera camera{};
		//camera.setViewTarget(glm::vec3{ -1.f, -2.f, -5.f }, glm::vec3{ .5f, .5f, 0.f });
		auto viewer = GameObject::createGameObject();
		viewer.transform.translation.z = -2.5f;
		Engine::MovementControl camera_control{};

		auto current_time = std::chrono::high_resolution_clock::now();

		while (!renderer.windowShouldClose()) {
			glfwPollEvents();

			auto new_time = std::chrono::high_resolution_clock::now();
			float frame_delta_time = std::chrono::duration<float, std::ratio<1>>(new_time - current_time).count();
			current_time = new_time;
			//frame_delta_time = glm::min(frame_delta_time, MAX_FRAME_TIME);

			camera_control.moveInPlaneXZ(window.getGLFWwindow(), frame_delta_time, viewer);
			camera.setViewYXZ(viewer.transform.translation, viewer.transform.rotation);
			float aspect = renderer.getAspectRatio();
			camera.setPerspectiveProjection(glm::radians(50.f), aspect, .1f, 100.f);

			if (auto cmd_buffer = renderer.beginFrame()) {
				int frame_index = renderer.getFrameIndex();
				Engine::Frame frame{frame_index, frame_delta_time, camera, 
					cmd_buffer, main_render_sys.getGlobalDiscriptorSet(frame_index), gameObjects};

				// update 
				Engine::GlobalUniformBufferOutput ubo{};
				ubo.projectionMatrix = camera.getProjection();
				ubo.viewMatrix = camera.getView();
				ubo.inverseViewMatrix = camera.getInverseView();
				point_light_render_system.update(frame, ubo);
				main_render_sys.getUboBuffer(frame_index)->writeToBuffer(&ubo);
				main_render_sys.getUboBuffer(frame_index)->flush();

				// render
				renderer.beginSwapChainRenderPass(cmd_buffer);

				// order here matters
				render_system.renderObjects(frame);
				point_light_render_system.render(frame);

				renderer.endSwapChainRenderPass(cmd_buffer);
				renderer.endFrame();
			}
		}

		renderer.deviceWaitIdle();
	}

	void FirstApp::loadGameObjects() {
		//auto cube_model = renderer.createCubeModel({.0f, .0f, .0f});
		auto model = Engine::VertexModel::createModelFromFile(device, "models/flat_vase.obj");
		auto obj = GameObject::createGameObject();
		obj.model = model;
		obj.transform.translation = { -.5f, .5f, 0.f };
		obj.transform.scale = glm::vec3{ 3.f, 1.5f, 3.f };

		auto model1 = Engine::VertexModel::createModelFromFile(device, "models/smooth_vase.obj");
		auto obj1 = GameObject::createGameObject();
		obj1.model = model1;
		obj1.transform.translation = { .5f, .5f, 0.f };
		obj1.transform.scale = glm::vec3{ 3.f, 1.5f, 3.f };

		auto plane_model = Engine::VertexModel::createModelFromFile(device, "models/quad.obj");
		auto plane_obj = GameObject::createGameObject();
		plane_obj.model = plane_model;
		plane_obj.transform.translation = { .0f, .5f, 0.f };
		plane_obj.transform.scale = glm::vec3{ 3.f, 1.f, 3.f };

		std::vector<glm::vec3> light_colors{
		 {1.f, .1f, .1f},
		 {.1f, .1f, 1.f},
		 {.1f, 1.f, .1f},
		 {1.f, 1.f, .1f},
		 {.1f, 1.f, 1.f},
		 {1.f, 1.f, 1.f}  //
		};

		for (size_t i{}; i < light_colors.size(); i++) {
			auto point_light = GameObject::createPointLight(.5f, .1f, light_colors[i]);
			auto rotate_transform_matrix = glm::rotate(
				glm::mat4{ 1.f },
				(i * glm::two_pi<float>()) / light_colors.size(),
				{ 0.f, -1.f, 0.f });
			point_light.transform.translation = glm::vec3{ rotate_transform_matrix * glm::vec4{-1.f, -1.f, -1.f, 1.f} };
			gameObjects.emplace(point_light.getId(), std::move(point_light));
		}

		gameObjects.emplace(obj.getId(), std::move(obj));
		gameObjects.emplace(obj1.getId(), std::move(obj1));
		gameObjects.emplace(plane_obj.getId(), std::move(plane_obj));
	}
}