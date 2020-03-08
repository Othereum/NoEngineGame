#pragma once

#include <memory>
#include <vector>
#include "vector.h"
#include "rotation.h"

namespace game
{
	class component;
	class actor
	{
	public:
		enum class state
		{
			active, paused, dead
		};

		explicit actor(class world& outer);
		virtual ~actor() = default;

		void set_pos(const fvector2& new_pos) noexcept { pos_ = new_pos; }
		[[nodiscard]] const fvector2& get_pos() const noexcept { return pos_; }

		void set_rot(const degrees& new_rot) noexcept { rot_ = new_rot; }
		[[nodiscard]] const degrees& get_rot() const noexcept { return rot_; }
		[[nodiscard]] fvector2 get_forward() const noexcept;

		[[nodiscard]] state get_state() const noexcept { return state_; }
		
		void set_scale(float scale) noexcept { scale_ = scale; }
		[[nodiscard]] float get_scale() const noexcept { return scale_; }

		[[nodiscard]] world& get_world() const noexcept { return world_; }

		void update(float delta_seconds);
		void destroy();

		template <class T, class... Args>
		T& add_component(Args&&... args)
		{
			static_assert(std::is_base_of_v<component, T>);
			auto ptr = std::make_unique<T>(*this, std::forward<Args>(args)...);
			auto& ref = *ptr;
			register_component(std::move(ptr));
			return ref;
		}

	private:
		void register_component(std::unique_ptr<component>&& comp);
		void update_components(float delta_seconds);
		virtual void update_actor(float delta_seconds) {}

		state state_{};
		
		fvector2 pos_;
		degrees rot_;
		float scale_{1};
		
		world& world_;
		std::vector<std::unique_ptr<component>> comps_;
	};
}