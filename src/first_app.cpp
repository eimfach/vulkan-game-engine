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
		Engine::AABBRenderSystem aabb_render{ device, renderer.getSwapChainRenderPass(),
		main_render.getGobalSetLayout() };

		Engine::Camera camera{};
		camera.setViewTarget({ -1.f, -2.f, -5.f }, { .5f, .5f, 0.f });

		// TODO: refactor viewer to be a entity with transform component
		auto viewer = ecsManager.createEntity();
		ECS::Transform t{};
		t.translation.z = -2.5f;
		ecsManager.addComponent(viewer, t);
		ecsManager.commit(viewer);
		ECS::Transform& viewer_transfrom = ecsManager.getEntityComponent<ECS::Transform>(viewer.id);
		Engine::MovementControl camera_control{};

		auto current_time = std::chrono::high_resolution_clock::now();

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
				//line_render.render(frame);
				aabb_render.render(frame, ecsManager.getComponents<ECS::AABB>());
				gui_render_sys.render(frame);

				renderer.endSwapChainRenderPass(cmd_buffer);
				renderer.endFrame();
			}
		}

		renderer.deviceWaitIdle();
	}

	ECS::Entity FirstApp::createMeshEntity(std::string name, std::string modelpath, ECS::Transform transform) {
		auto& model = Engine::VertexModel::createModelFromFile(device, modelpath);
		auto e = ecsManager.createEntity();
		ECS::Identification id{ name };
		ecsManager.addComponent(e, id);
		ECS::Mesh mesh{ model.first };
		ECS::AABB aabb{ device, model.second.verticies, transform.mat4() };
		ecsManager.addComponent(e, mesh);
		ecsManager.addComponent(e, aabb);
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

		ecsManager.reserve_size_entities(50004);
		ecsManager.reserve_size_components<ECS::Voxel>(Settings::VOXEL_GRID_EXTENT * Settings::VOXEL_GRID_EXTENT * Settings::VOXEL_GRID_EXTENT);
		ecsManager.reserve_size_components<ECS::AABB>(4);
		ecsManager.reserve_size_components<ECS::Transform>(50004);
		ecsManager.reserve_size_components<ECS::Mesh>(50004);
		ecsManager.reserve_size_components<ECS::Visibility>(50004);

		ecsManager.reserve_size_components<ECS::Color>(50000);
		ecsManager.reserve_size_components<ECS::RenderLines>(50000);

		ecsManager.commit(createMeshEntity("flat_vase", "models/flat_vase.obj", ECS::Transform{ { -.1f, .5f, 0.f } , { 3.f, 1.5f, 3.f } }));
		ecsManager.commit(createMeshEntity("smooth_vase", "models/smooth_vase.obj", ECS::Transform{ { .1f, .5f, 0.f } , { 3.f, 1.5f, 3.f } }));
		ecsManager.commit(createMeshEntity("smooth_vase2", "models/smooth_vase.obj", ECS::Transform{ { -1.f, -.5f, 0.f } , { 1.f, 1.1f, 1.f } }));
		ecsManager.commit(createMeshEntity("floor", "models/quad.obj", ECS::Transform{ { .5f, .7f, 0.f } , { 3.f, 1.5f, 3.f } }));

		const int ext = Settings::VOXEL_GRID_EXTENT;
		for (int x = -ext; x < ext; x++) {
			for (int y = -ext; y < ext; y++) {
				for (int z = -ext; z < ext; z++) {
					ECS::Transform transform{ {float(x), float(y), float(z)}, glm::vec3{1.f} };
					ECS::Voxel voxel{ transform.mat4() };
					auto e = ecsManager.createEntity();
					ecsManager.addComponent(e, voxel);
					ecsManager.commit(e);
				}
			}
		}

		//auto line_model = renderer.createLine(glm::vec3{ 0.f });
		//for (int i = 0; i < 50000; i++)
		//{
		//	auto e = ecsManager.createEntity();
		//	ecsManager.addComponent(e, ECS::RenderLines{});
		//	ECS::Transform transform{};
		//	const float z{ float(i) * .025f };
		//	transform.translation = { -.5f, -1.5f, z };
		//	ECS::Mesh mesh{ line_model };
		//	ecsManager.addComponent(e, mesh);
		//	ecsManager.addComponent(e, transform);
		//	ecsManager.addComponent(e, ECS::Visibility{});
		//	ECS::Color color{ { .3f, .1f, .6f } };
		//	ecsManager.addComponent(e, color);
		//	ecsManager.commit(e);
		//}
	}
}