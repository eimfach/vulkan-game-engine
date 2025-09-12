#include "first_app.hpp"

#include "buffer.hpp"
#include "camera.hpp"
#include "movement.hpp"

#include "simple_render_system.hpp"
#include "main_render_system.hpp"
#include "gui_render_system.hpp"
#include "line_render_system.hpp"
#include "aabb_render_system.hpp"

#include "pointlight_render_system.hpp"
#include "texture.hpp"
#include "vertex_model.hpp"
#include "assets.hpp"
#include "entity_manager.hpp"
#include "settings.hpp"
#include "utils.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

//std
#include <chrono>
#include <ratio>
#include <memory>

namespace SJFGame {


	FirstApp::FirstApp() {
		loadGameEntities();
	}

	FirstApp::~FirstApp() {}

	void FirstApp::run() {
		std::cout << "[Running Game]";
		Engine::MainRenderSystem main_render{ device };
		Engine::SimpleRenderSystem simple_render{ device, renderer.getSwapChainRenderPass(),
			main_render.getGobalSetLayout() };
		Engine::PointLightRenderSystem point_light_render{ device, renderer.getSwapChainRenderPass(),
			main_render.getGobalSetLayout() };
		Engine::GuiRenderSystem gui_render_sys{ device, window.getGLFWwindow(), renderer.getSwapChainRenderPass() };
		Engine::LineRenderSystem line_render{ device, renderer.getSwapChainRenderPass(),
			main_render.getGobalSetLayout() };
		Engine::AABBRenderSystem aabb_render{ device, renderer.getSwapChainRenderPass(),
		main_render.getGobalSetLayout() };

		Engine::Camera camera{};
		camera.setViewTarget({ 0.f, -7.1f, -20.1f }, { 5.f, -10.f, 0.f });

		auto viewer = ecsManager.createEntity();
		auto viewer_entity = viewer.first;
		auto viewer_id = viewer.second;
		ECS::Transform t{ {0.f, -3.1f, -20.1f} };
		//t.translation.z = -2.5f;
		//t.rotation.x = glm::radians(-45.0f);
		ecsManager.addComponent(viewer_entity, t);
		ecsManager.commit(viewer_entity);
		ecsManager.lock();

		ECS::Transform& viewer_transfrom = ecsManager.getEntityComponent<ECS::Transform>(viewer_id);
		Engine::MovementControl camera_control{};

		auto current_time = std::chrono::high_resolution_clock::now();

		int frame_index{};
		Engine::Frame frame{ 0, .0, camera, nullptr, nullptr, gameObjects, ecsManager };

		while (!renderer.windowShouldClose()) {
			glfwPollEvents();

			auto new_time = std::chrono::high_resolution_clock::now();
			float frame_delta_time = std::chrono::duration<float, std::ratio<1>>(new_time - current_time).count();
			current_time = new_time;
			//frame_delta_time = glm::min(frame_delta_time, MAX_FRAME_TIME);

			camera_control.moveInPlaneXZ(window.getGLFWwindow(), frame_delta_time, viewer_transfrom);
			camera.setViewYXZ(viewer_transfrom.translation, viewer_transfrom.rotation);
			float aspect = renderer.getAspectRatio();
			camera.setPerspectiveProjection(glm::radians(Settings::FOV_DEGREES), aspect, Settings::NEAR_PLANE, Settings::FAR_PLANE);
			camera.produceFrustum();

			if (auto cmd_buffer = renderer.beginFrame()) {
				// frame informations
				frame_index = renderer.getFrameIndex();
				frame.frameIndex = frame_index;
				frame.delta = frame_delta_time;
				frame.cmdBuffer = cmd_buffer;
				frame.globalDescriptorSet = main_render.getGlobalDiscriptorSet(frame_index);

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
				point_light_render.render(frame);
				line_render.render(frame);
				//aabb_render.render(frame, ecsManager.getComponents<ECS::AABB>());
				gui_render_sys.render(frame);

				renderer.endSwapChainRenderPass(cmd_buffer);
				renderer.endFrame();
			}
		}

		renderer.deviceWaitIdle();
	}

	ECS::Entity FirstApp::createStaticMeshEntity(std::string name, std::string modelpath, ECS::Transform transform) {
		auto& model = Engine::VertexModel::createModelFromFile(device, modelpath);
		auto e = ecsManager.createEntity();
		auto entity = e.first;
		ECS::Identification id{ name };
		ecsManager.addComponent(entity, id);
		ECS::Mesh mesh{ model.first };
		ECS::AABB aabb{ device, model.second.verticies, transform.modelMatrix()};
		ecsManager.addComponent(entity, mesh);
		ecsManager.addComponent(entity, aabb);
		ecsManager.addComponent(entity, transform);
		ecsManager.addComponent(entity, ECS::Visibility{});
		return entity;
	}

	void FirstApp::loadGameEntities() {

		///////////////////////////////////////////
		// Archetype ECS System                  //
		///////////////////////////////////////////
		constexpr int RANDOMLY_PLACED_STATIC_OBJECTS_COUNT = 1000;
		constexpr int STATIC_OBJECTS_COUNT = 3;
		constexpr int OBJECTS_COUNT = RANDOMLY_PLACED_STATIC_OBJECTS_COUNT + STATIC_OBJECTS_COUNT;
		constexpr int LINE_OBJECTS_COUNT = 4;
		constexpr int POINT_LIGHT_OBJECTS_COUNT = 10;

		ecsManager.reserve_size_entities(OBJECTS_COUNT + LINE_OBJECTS_COUNT + POINT_LIGHT_OBJECTS_COUNT);
		ecsManager.reserve_size_components<ECS::AABB>(LINE_OBJECTS_COUNT + OBJECTS_COUNT);
		ecsManager.reserve_size_components<ECS::Transform>(OBJECTS_COUNT + LINE_OBJECTS_COUNT + POINT_LIGHT_OBJECTS_COUNT);
		ecsManager.reserve_size_components<ECS::Mesh>(LINE_OBJECTS_COUNT + OBJECTS_COUNT);
		ecsManager.reserve_size_components<ECS::Visibility>(LINE_OBJECTS_COUNT);

		ecsManager.reserve_size_components<ECS::Color>(LINE_OBJECTS_COUNT + POINT_LIGHT_OBJECTS_COUNT);
		ecsManager.reserve_size_components<ECS::RenderLines>(LINE_OBJECTS_COUNT);
		ecsManager.reserve_size_components<ECS::PointLight>(POINT_LIGHT_OBJECTS_COUNT);

		// Lines
		auto& line_model = Engine::VertexModel::createModelFromFile(device, "models/quad.obj");

		for (int i = 0; i < LINE_OBJECTS_COUNT; i++) {
			auto e = ecsManager.createEntity();
			auto entity = e.first;
			ecsManager.addComponent(entity, ECS::RenderLines{});
			ECS::Transform transform{};
			const float z{ float(i) * .15f };
			transform.translation = { 0.f, -1.f, -0.15f };
			transform.rotation = { glm::radians(90.f), 0.f, glm::radians(90.f) };
			ECS::Mesh mesh{ line_model.first };
			ECS::AABB aabb{ line_model.second.verticies, transform.modelMatrix() };
			ecsManager.addComponent(entity, mesh);
			ecsManager.addComponent(entity, aabb);
			ecsManager.addComponent(entity, transform);
			ecsManager.addComponent(entity, ECS::Visibility{});
			ECS::Color color{ { .3f, .1f, .6f } };
			ecsManager.addComponent(entity, color);
			ecsManager.commit(entity);
		}

		// Pointlights
		std::vector<glm::vec3> light_colors{
		 {1.f, .1f, .1f},
		 {.1f, .1f, 1.f},
		 {.1f, 1.f, .1f},
		 {1.f, 1.f, .1f},
		 {.1f, 1.f, 1.f},
		 {1.f, 1.f, 1.f}
		};

		for (size_t i{}; i < light_colors.size(); i++) {
			auto e = ecsManager.createEntity();
			auto entity = e.first;
			ECS::Transform t{};
			auto rotate_transform_matrix = glm::rotate(
				glm::mat4{ 1.f },
				(i * glm::two_pi<float>()) / light_colors.size(),
				{ 0.f, -1.f, 0.f });
			t.translation = glm::vec3{ rotate_transform_matrix * glm::vec4{-1.f, -1.f, -1.f, 1.f} };
			t.scale.x = .1f;
			ecsManager.addComponent(entity, t);
			ecsManager.addComponent(entity, ECS::PointLight{3.5f});
			ecsManager.addComponent(entity, ECS::Color{ light_colors[i] });
			ecsManager.commit(entity);
		}

		ecsManager.commit(createStaticMeshEntity("flat_vase", "models/flat_vase.obj", ECS::Transform{ { -.1f, .5f, 0.f } , { 3.f, 1.5f, 3.f } }));
		ecsManager.commit(createStaticMeshEntity("smooth_vase", "models/smooth_vase.obj", ECS::Transform{ { .1f, .5f, 0.f } , { 3.f, 1.5f, 3.f } }));
		ecsManager.commit(createStaticMeshEntity("smooth_vase2", "models/smooth_vase.obj", ECS::Transform{ { -1.f, -.5f, 0.f } , { 1.f, 1.1f, 1.f } }));
		ecsManager.commit(createStaticMeshEntity("floor", "models/quad.obj", ECS::Transform{ { .5f, .7f, 0.f } , { 3.f, 1.5f, 3.f } }));

		// Objects
		for (size_t i = 0; i < RANDOMLY_PLACED_STATIC_OBJECTS_COUNT; i++) {
			ecsManager.commit(createStaticMeshEntity("flat_vase", "models/flat_vase.obj", Utils::randTransform()));
		}


	}
}