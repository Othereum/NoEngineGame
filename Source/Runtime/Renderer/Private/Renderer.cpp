#include "Renderer.hpp"

#include <stdexcept>
#include <SDL2/SDL.h>

#include "EngineBase.hpp"
#include "Material.hpp"
#include "Mesh.hpp"
#include "OpenGL.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "VertexArray.hpp"
#include "Interfaces/Drawable.hpp"
#include "Interfaces/Light.hpp"

namespace oeng::renderer
{
	static constexpr auto kMaxPointLights = 4;
	static constexpr auto kMaxSpotLights = 4;
	
	static void SetGlAttribute(SDL_GLattr attr, int value)
	{
		if (0 != SDL_GL_SetAttribute(attr, value))
			throw std::runtime_error{SDL_GetError()};
	}

	class WindowCreator
	{
	public:
		DELETE_CPMV(WindowCreator);
		
		WindowCreator()
		{
			WriteDisplayModes();
			LoadDisplayIdx();
			WriteWindowSize();
			LoadDisplayMode();
			SetupAttributes();
		}
		
		[[nodiscard]] WindowPtr operator()() const
		{
			auto* const window = SDL_CreateWindow(
				AsString(EngineBase::Get().GetGameName().data()),
				SDL_WINDOWPOS_CENTERED_DISPLAY(dp_idx_), SDL_WINDOWPOS_CENTERED_DISPLAY(dp_idx_),
				dp_mode_.w, dp_mode_.h, flags_
			);

			if (!window) throw std::runtime_error{SDL_GetError()};
			SDL_SetWindowDisplayMode(window, &dp_mode_);
			
			return {window, &SDL_DestroyWindow};
		}

		~WindowCreator()
		{
			(void)ConfigSystem::Get().Save(cfg_name_);
		}

	private:
		void WriteDisplayModes() const
		{
			const auto num_dp = SDL_GetNumVideoDisplays();
			if (num_dp < 1) throw std::runtime_error{SDL_GetError()};
			
			auto& displays = cfg_["Display modes (read only)"] = Json::object();
			for (auto dp = 0; dp < num_dp; ++dp)
			{
				const auto num_dm = SDL_GetNumDisplayModes(dp);
				if (num_dm < 1) throw std::runtime_error{SDL_GetError()};

				auto& modes = displays[fmt::format("Display {}", dp)] = Json::array();
				for (auto dm = 0; dm < num_dm; ++dm)
				{
					SDL_DisplayMode mode;
					SDL_GetDisplayMode(dp, dm, &mode);
					auto str = Format(u8"[{}] {}x{} {}Hz"sv, dm, mode.w, mode.h, mode.refresh_rate);
					modes.emplace_back(AsString(std::move(str)));
				}
			}
		}

		void WriteWindowSize() const
		{
			if (!cfg_.contains("WindowedSize"))
			{
				SDL_DisplayMode dp_mode;
				SDL_GetDesktopDisplayMode(dp_idx_, &dp_mode);
				Vector size{dp_mode.w, dp_mode.h};
				cfg_["WindowedSize"] = (size *= 5, size /= 6);
			}
		}
		
		void LoadDisplayIdx()
		{
			const auto num_dp = SDL_GetNumVideoDisplays();
			if (dp_idx_ >= num_dp)
			{
				log::Warn(u8"Attempted to use a non-existent display (tried: {}, max: {})"sv, dp_idx_, num_dp-1);
				log::Warn(u8"Using display 0..."sv);
				dp_idx_ = 0;
			}
		}

		void LoadDisplayMode()
		{
			if (fullscreen_) LoadFullscreenDpMode();
			else LoadWindowedDpMode();
		}

		void LoadFullscreenDpMode()
		{
			auto dm_idx = cfg_.at("FullscreenDisplayMode").get<int>();
			const auto num_dm = SDL_GetNumDisplayModes(dp_idx_);
			
			if (dm_idx >= num_dm)
			{
				log::Warn(u8"Attempted to use a non-existent display mode (tried: {}, max: {})"sv, dm_idx, num_dm-1);
				log::Warn(u8"Using display mode 0..."sv);
				dm_idx = 0;
			}

			if (0 != SDL_GetDisplayMode(dp_idx_, dm_idx, &dp_mode_))
				throw std::runtime_error{SDL_GetError()};

			log::Info(u8"Fullscreen mode: {}x{} {}Hz"sv, dp_mode_.w, dp_mode_.h, dp_mode_.refresh_rate);
		}

		void LoadWindowedDpMode()
		{
			const auto size = cfg_.at("WindowedSize").get<Vec2u16>();
			dp_mode_.w = size[0];
			dp_mode_.h = size[1];
			log::Info(u8"Windowed mode: {}x{}"sv, dp_mode_.w, dp_mode_.h);
		}

		static void SetupAttributes()
		{
			SetGlAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
			SetGlAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
			SetGlAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
			SetGlAttribute(SDL_GL_RED_SIZE, 8);
			SetGlAttribute(SDL_GL_GREEN_SIZE, 8);
			SetGlAttribute(SDL_GL_BLUE_SIZE, 8);
			SetGlAttribute(SDL_GL_ALPHA_SIZE, 8);
			SetGlAttribute(SDL_GL_DEPTH_SIZE, 24);
			SetGlAttribute(SDL_GL_DOUBLEBUFFER, true);
			SetGlAttribute(SDL_GL_ACCELERATED_VISUAL, true);
		}

		const Name cfg_name_{u8"Display"sv};
		Json& cfg_{ConfigSystem::Get()(cfg_name_)};

		const bool fullscreen_ = cfg_.at("Fullscreen").get<bool>();
		const uint32_t flags_ = SDL_WINDOW_OPENGL | (fullscreen_ ? SDL_WINDOW_FULLSCREEN : 0);
		
		int dp_idx_ = cfg_.at("Display").get<int>();
		SDL_DisplayMode dp_mode_{};
	};

	static GlContextPtr CreateGlContext(SDL_Window& window)
	{
		GlContextPtr context{
			SDL_GL_CreateContext(&window),
			&SDL_GL_DeleteContext
		};
		if (!context) throw std::runtime_error{SDL_GetError()};
		
		glewExperimental = true;

		if (const auto err = glewInit(); err != GLEW_OK)
			throw std::runtime_error{reinterpret_cast<const char*>(glewGetErrorString(err))};
		
		// On some platforms, GLEW will emit a benign error code, so clear it
		glGetError();

		// TODO: vSync config
		SDL_GL_SetSwapInterval(0);

		gl(glClearColor, 0.f, 0.f, 0.f, 1.f);
		
		return context;
	}

	static VertexArray CreateSpriteVerts()
	{
		constexpr Vertex vertex_buffer[]
		{
			{{-0.5_f, 0.5_f, 0_f}, {}, {0_f, 0_f}},
			{{0.5_f, 0.5_f, 0_f}, {}, {1_f, 0_f}},
			{{0.5_f, -0.5_f, 0_f}, {}, {1_f, 1_f}},
			{{-0.5_f, -0.5_f, 0_f}, {}, {0_f, 1_f}}
		};

		constexpr Vec3u16 index_buffer[]
		{
			{0, 1, 2},
			{2, 3, 0}
		};

		return {vertex_buffer, index_buffer};
	}

	const Vec3& DefaultCamera::GetPos() const noexcept
	{
		return Vec3::zero;
	}

	const Mat4& DefaultCamera::GetViewProj() const noexcept
	{
		return view_proj_;
	}

	void DefaultCamera::OnScreenSizeChanged(Vec2u16 scr)
	{
		static const auto view = *MakeLookAt(Vec3::zero, UVec3::forward, UVec3::up);
		const auto& data = GetData();
		const auto proj = MakePerspective(Vec2{scr}, data.near, data.far, data.vfov);
		view_proj_ = view * proj;
	}

	const ICamera::Data& DefaultCamera::GetData() const noexcept
	{
		static const Data data{10, 10000, 60_deg};
		return data;
	}

	Renderer::Renderer()
		:window_{WindowCreator{}()},
		gl_context_{CreateGlContext(*window_)},
		sprite_shader_{u8"../Engine/Shaders/Sprite"sv},
		sprite_verts_{CreateSpriteVerts()},
		materials_{shaders_.default_obj, textures_.default_obj},
		meshes_{materials_.default_obj}
	{
		UnregisterCamera();
		UnregisterDirLight();
		UnregisterSkyLight();
		sprite_shader_.SetUniform(u8"uViewProj"sv, MakeSimpleViewProj<4>(GetWindowSize()));
	}

	Renderer::~Renderer() = default;

	void Renderer::PreDrawScene() const
	{
		SCOPE_STACK_COUNTER(PreDrawScene);
		gl(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void Renderer::DrawScene()
	{
		SCOPE_STACK_COUNTER(DrawScene);
		PreDrawScene();
		TRY(Draw3D());
		TRY(Draw2D());
		PostDrawScene();
	}

	void Renderer::PostDrawScene() const
	{
		SCOPE_STACK_COUNTER(PostDrawScene);
		SDL_GL_SwapWindow(window_.get());
	}

	void Renderer::Draw3D()
	{
		gl(glEnable, GL_DEPTH_TEST);
		gl(glDisable, GL_BLEND);

		prev_ = {};
		
		for (auto mesh_comp : mesh_comps_) try
		{
			DrawMesh(mesh_comp);
		}
		catch (const std::exception& e)
		{
			const auto stem = mesh_comp.get().GetMesh().GetStem();
			OE_ELOG(u8"Failed to draw mesh '{}': {}"sv, *stem, What(e));
		}
	}

	void Renderer::Draw2D()
	{
		gl(glEnable, GL_BLEND);
		gl(glDisable, GL_DEPTH_TEST);
		gl(glBlendFunc, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		sprite_shader_.Activate();
		for (auto sprite_ref : sprites_) try
		{
			const auto& sprite = sprite_ref.get();
			if (!sprite.ShouldDraw()) continue;

			SCOPE_STACK_COUNTER(DrawSprite);
			sprite_shader_.SetUniform(u8"uWorldTransform"sv, sprite.GetDrawTrsf());
			gl(glDrawElements, GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
		}
		catch (const std::exception& e)
		{
			const auto stem = sprite_ref.get().GetTexture().GetStem();
			OE_ELOG(u8"Failed to draw sprite '{}': {}"sv, *stem, What(e));
		}
	}

	void Renderer::DrawMesh(const IMeshComponent& mesh_comp)
	{
		if (!ShouldDraw(mesh_comp)) return;
		SCOPE_STACK_COUNTER(DrawMesh);
		
		auto& material = mesh_comp.GetMaterial();
		auto& shader = material.GetShader();
		if (&shader != prev_.shader)
		{
			shader.Activate();
			shader.TryUniform(u8"uViewProj"sv, camera_->GetViewProj());
			shader.TryUniform(u8"uCamPos"sv, camera_->GetPos());
			
			const auto& dir_light = dir_light_->GetData();
			shader.TryUniform(u8"uDirLight.dir"sv, dir_light.dir);
			shader.TryUniform(u8"uDirLight.color"sv, dir_light.color);
			shader.TryUniform(u8"uSkyLight"sv, sky_light_->GetColor());

			prev_.shader = &shader;
		}

		if (&material != prev_.material)
		{
			material.TryUniforms();
			prev_.material = &material;
		}

		auto& texture = material.GetTexture();
		if (&texture != prev_.texture)
		{
			texture.Activate();
			prev_.texture = &texture;
		}

		auto& mesh = mesh_comp.GetMesh();
		auto& verts = mesh.GetVertexArray();
		if (&mesh != prev_.mesh)
		{
			verts.Activate();
			prev_.mesh = &mesh;
		}

		DrawPointLights(mesh_comp);
		DrawSpotLights(mesh_comp);

		shader.TryUniform(u8"uWorldTransform"sv, mesh_comp.GetDrawTrsfMatrix());
		gl(glDrawElements, GL_TRIANGLES, GLsizei(verts.GetNumIndices() * 3), GL_UNSIGNED_SHORT, nullptr);
	}

	template <class Light, class Fn>
	static void DrawLights(std::u8string_view name, const Renderer::CompArr<Light>& lights,
		const int max_lights, const IMeshComponent& mesh_comp, Fn&& try_extra_uniforms)
	{
		auto& shader = mesh_comp.GetMaterial().GetShader();
		const auto loc_num = shader.GetUniformLocation(Format(u8"uNum{}Lights"sv, name));
		if (loc_num == Shader::invalid_uniform_) return;
		
		const auto& mesh_trsf = mesh_comp.GetDrawTrsf();
		const auto mesh_radius = mesh_comp.GetScaledRadius();
		
		auto idx = 0;
		for (auto ref : lights)
		{
			if (idx >= max_lights) break;
			
			const auto& l = ref.get();
			if (!l.ShouldAffect()) continue;
			
			const auto& data = l.GetData();
			if (!IsOverlapped({data.pos, data.radius}, {mesh_trsf.pos, mesh_radius})) continue;

			SCOPE_STACK_COUNTER(DrawLight);

			auto try_uniform = [&]<class T>(std::u8string_view uniform, T&& value)
			{
				return shader.TryUniform(
					Format(u8"u{}Lights[{}].{}"sv, name, idx, uniform),
					std::forward<T>(value)
				);
			};

			try_uniform(u8"color"sv, data.color);
			try_uniform(u8"pos"sv, data.pos);
			try_uniform(u8"radius"sv, data.radius);
			try_extra_uniforms(try_uniform, data);
			
			++idx;
		}
		
		shader.TryUniform(loc_num, idx);
	}

	void Renderer::DrawPointLights(const IMeshComponent& mesh_comp) const
	{
		DrawLights(u8"Point"sv, point_lights_, kMaxPointLights, mesh_comp, [](auto&&...){});
	}

	void Renderer::DrawSpotLights(const IMeshComponent& mesh_comp) const
	{
		auto try_uniforms = [](auto&& try_uniform, const ISpotLight::Data& data)
		{
			try_uniform(u8"dir"sv, data.dir);
			try_uniform(u8"inner"sv, data.angle_cos.inner);
			try_uniform(u8"outer"sv, data.angle_cos.outer);
		};
		DrawLights(u8"Spot"sv, spot_lights_, kMaxSpotLights, mesh_comp, try_uniforms);
	}

	bool Renderer::ShouldDraw(const IMeshComponent& mesh_comp) const noexcept
	{
		if (!mesh_comp.ShouldDraw()) return false;
		
		const Sphere mesh_sphere{mesh_comp.GetDrawTrsf().pos, mesh_comp.GetScaledRadius()};
		const Sphere camera_sphere{camera_->GetPos(), mesh_comp.GetMaxDrawDist()};
		if (!IsOverlapped(mesh_sphere, camera_sphere)) return false;

		return true;
	}

	
	class DefaultDirLight : public IDirLight
	{
	public:
		[[nodiscard]] const Data& GetData() const noexcept override
		{
			return data_;
		}

	private:
		const Data data_{UVec3::down, Vec3::zero};
	};

	class DefaultSkyLight : public ISkyLight
	{
	public:
		[[nodiscard]] const Vec3& GetColor() const noexcept override
		{
			return Vec3::zero;
		}
	};

	static const DefaultDirLight kDefaultDirLight;
	static const DefaultSkyLight kDefaultSkyLight;

	void Renderer::UnregisterDirLight() noexcept
	{
		dir_light_ = &kDefaultDirLight;
	}

	void Renderer::UnregisterSkyLight() noexcept
	{
		sky_light_ = &kDefaultSkyLight;
	}

	bool Renderer::IsDirLightRegistered() const noexcept
	{
		return dir_light_ != &kDefaultDirLight;
	}

	bool Renderer::IsSkyLightRegistered() const noexcept
	{
		return sky_light_ != &kDefaultSkyLight;
	}

	
	template <class T, class... Args>
	SharedRef<T> Get(AssetCache<T>& cache, Path path, Args&&... args)
	{
		auto& map = cache.map;
		const auto found = map.find(path);
		if (found != map.end()) return SharedRef<T>{found->second};

		try
		{
			SharedRef<T> loaded(New<T>(path, std::forward<Args>(args)...),
				[&map, path](T* p) { map.erase(path); Delete(p); });
			
			map.emplace(path, loaded);
			return loaded;
		}
		catch (const std::exception& e)
		{
			log::Error(u8"Failed to load '{}': {}"sv, path.Str(), What(e));
			return cache.default_obj;
		}
	}

	SharedRef<Texture> Renderer::GetTexture(Path path)
	{
		return Get(textures_, path);
	}

	SharedRef<Mesh> Renderer::GetMesh(Path path)
	{
		return Get(meshes_, path, *this);
	}

	SharedRef<Shader> Renderer::GetShader(Path path)
	{
		return Get(shaders_, path);
	}

	SharedRef<Material> Renderer::GetMaterial(Path path)
	{
		return Get(materials_, path, *this);
	}

	Vec2u16 Renderer::GetWindowSize() const noexcept
	{
		int w, h;
		SDL_GetWindowSize(window_.get(), &w, &h);
		return {uint16_t(w), uint16_t(h)};
	}

	template <class T, class Compare>
	void Register(Renderer::CompArr<T>& arr, const T& obj, Compare cmp)
	{
		const auto pos = std::upper_bound(arr.begin(), arr.end(), obj, cmp);
		arr.emplace(pos, obj);
	}

	template <class T>
	void Unregister(Renderer::CompArr<T>& arr, const T& obj)
	{
		const auto cmp = [&obj](const T& x) { return &x == &obj; };
		const auto found = std::find_if(arr.rbegin(), arr.rend(), cmp);
		if (found != arr.rend()) arr.erase(found.base() - 1);
	}

	void Renderer::RegisterSprite(const ISpriteComponent& sprite)
	{
		Register(sprites_, sprite, [](const ISpriteComponent& a, const ISpriteComponent& b)
		{
			return a.GetDrawOrder() < b.GetDrawOrder();
		});
	}

	void Renderer::RegisterMesh(const IMeshComponent& mesh)
	{
		// Group mesh components in order of importance for [Shader -> Materials -> Texture -> Mesh]
		Register(mesh_comps_, mesh, [](const IMeshComponent& a, const IMeshComponent& b)
		{
			auto &mat1 = a.GetMaterial(), &mat2 = b.GetMaterial();
			auto &s1 = mat1.GetShader(), &s2 = mat2.GetShader();
			if (&s1 != &s2) return &s1 < &s2;
			if (&mat1 != &mat2)
			{
				auto &t1 = mat1.GetTexture(), &t2 = mat2.GetTexture();
				if (&t1 != &t2) return &t1 < &t2;
				return &mat1 < &mat2;
			}
			auto &mesh1 = a.GetMesh(), &mesh2 = b.GetMesh();
			return &mesh1 < &mesh2;
		});
	}

	void Renderer::RegisterCamera(ICamera& camera) noexcept
	{
		camera_ = &camera;
		camera.OnScreenSizeChanged(GetWindowSize());
	}

	void Renderer::UnregisterSprite(const ISpriteComponent& sprite)
	{
		Unregister(sprites_, sprite);
	}

	void Renderer::UnregisterMesh(const IMeshComponent& mesh)
	{
		Unregister(mesh_comps_, mesh);
	}

	void Renderer::UnregisterPointLight(const IPointLight& light)
	{
		Unregister(point_lights_, light);
	}

	void Renderer::UnregisterSpotLight(const ISpotLight& light)
	{
		Unregister(spot_lights_, light);
	}
}
