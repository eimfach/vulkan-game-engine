#include "first_app.hpp"

#include "buffer.hpp"
#include "camera.hpp"
#include "movement.hpp"

#include "simple_render_system.hpp"
#include "main_render_system.hpp"
#include "gui_render_system.hpp"
#include "line_render_system.hpp"

#include "pointlight_render_system.hpp"
#include "texture.hpp"
#include "vertex_model.hpp"
#include "assets.hpp"
#include "entity_manager.hpp"
#include "settings.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

//std
#include <chrono>
#include <ratio>

namespace SJFGame {


	FirstApp::FirstApp() {
		loadGameEntities();
	}

	FirstApp::~FirstApp() {
		
	}

	void FirstApp::run() {

		Engine::MainRenderSystem main_render{ device };
		Engine::SimpleRenderSystem simple_render{ device, renderer.getSwapChainRenderPass(),
			main_render.getGobalSetLayout() };
		Engine::PointLightRenderSystem point_light_render{ device, renderer.getSwapChainRenderPass(),
			main_render.getGobalSetLayout() };
		Engine::GuiRenderSystem gui_render_sys{ device, window.getGLFWwindow(), renderer.getSwapChainRenderPass() };
		Engine::LineRenderSystem line_render{ device, renderer.getSwapChainRenderPass(),
			main_render.getGobalSetLayout() };

		Engine::Camera camera{};
		camera.setViewTarget(glm::vec3{ -1.f, -2.f, -5.f }, glm::vec3{ .5f, .5f, 0.f });
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
			camera.setPerspectiveProjection(glm::radians(Settings::FOV_DEGREES), aspect, Settings::NEAR_PLANE, Settings::FAR_PLANE);

			if (auto cmd_buffer = renderer.beginFrame()) {
				int frame_index = renderer.getFrameIndex();
				Engine::Frame frame{frame_index, frame_delta_time, camera, 
					cmd_buffer, main_render.getGlobalDiscriptorSet(frame_index), gameObjects, ecsManager };

				// update 
				Engine::GlobalUniformBufferOutput ubo{};
				ubo.projectionMatrix = camera.getProjection();
				ubo.viewMatrix = camera.getView();
				ubo.inverseViewMatrix = camera.getInverseView();
				point_light_render.update(frame, ubo);
				main_render.getUboBuffer(frame_index)->writeToBuffer(&ubo);
				main_render.getUboBuffer(frame_index)->flush();

				// render
				renderer.beginSwapChainRenderPass(cmd_buffer);

				// order here matters
				simple_render.render(frame);
				//point_light_render.render(frame);
				line_render.render(frame);
				gui_render_sys.render(frame);

				renderer.endSwapChainRenderPass(cmd_buffer);
				renderer.endFrame();
			}
		}

		renderer.deviceWaitIdle();
	}

	ECS::Entity FirstApp::createMeshEntity(std::string modelpath, ECS::Transform transform) {
		auto model = Engine::VertexModel::createModelFromFile(device, modelpath);
		auto e = ecsManager.createEntity();
		ECS::Mesh mesh{ model };
		ecsManager.addComponent(e, mesh);
		ecsManager.addComponent(e, transform);
		ecsManager.addComponent(e, ECS::Visibility{});
		return e;
	}

	void FirstApp::loadGameEntities() {

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

		///////////////////////////////////////////
		// new ECS System                        //
		///////////////////////////////////////////

		ecsManager.commit(createMeshEntity("models/flat_vase.obj", ECS::Transform{ { -.5f, .5f, 0.f } , { 3.f, 1.5f, 3.f } }));
		ecsManager.commit(createMeshEntity("models/smooth_vase.obj", ECS::Transform{ { .5f, .5f, 0.f } , { 3.f, 1.5f, 3.f } }));
		ecsManager.commit(createMeshEntity("models/quad.obj", ECS::Transform{ { .5f, .5f, 0.f } , { 3.f, 1.5f, 3.f } }));
		ecsManager.commit(createMeshEntity("models/flat_vase.obj", ECS::Transform{ { -.5f, .5f, 0.f } , { 3.f, 1.5f, 3.f } }));

		auto line_model = renderer.createLine(glm::vec3{ 0.f });

		for (int i = 0; i < 5; i++)
		{
			auto e = ecsManager.createEntity();
			ecsManager.addComponent(e, ECS::RenderLines{});
			ECS::Transform transform{};
			const float z{ float(i) * .025f };
			transform.translation = { -.5f, -1.5f, z };
			ECS::Mesh mesh{ line_model };
			ecsManager.addComponent(e, mesh);

			ecsManager.addComponent(e, transform);
			ecsManager.addComponent(e, ECS::Visibility{});
			ECS::Color color{ { .3f, .1f, .6f } };
			ecsManager.addComponent(e, color);
			ecsManager.commit(e);
		}

		void;
	}
}