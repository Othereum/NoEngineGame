#pragma once
#include "EngineFwd.hpp"
#include "TimerManager.hpp"
#include "Templates/DyArr.hpp"
#include "Templates/Pointer.hpp"
#include "Templates/Time.hpp"

namespace oeng::engine
{
	class ENGINE_API World
	{
	public:
		DELETE_CPMV(World);
		
		World();
		~World();

		void BeginTick();
		void Tick();
		
		template <class T>
		T& SpawnActor()
		{
			static_assert(std::is_base_of_v<AActor, T>);
			auto ptr = MakeShared<T>(*this);
			auto& ref = *ptr;
			pending_actors_.emplace_back(std::move(ptr));
			return ref;
		}

		void RegisterCollision(SphereComponent& comp);
		void UnregisterCollision(SphereComponent& comp);
		
		[[nodiscard]] TimerManager& GetTimerManager() noexcept { return timer_; }
		[[nodiscard]] TimePoint GetTime() const noexcept { return time_; }
		[[nodiscard]] Float GetDeltaSeconds() const noexcept { return delta_seconds_; }

	private:
		void UpdateGame();
		void UpdateTime();

		TimerManager timer_;

		DyArr<std::reference_wrapper<SphereComponent>> collisions_;
		
		DyArr<SharedRef<AActor>> actors_;
		DyArr<SharedRef<AActor>> pending_actors_;
		
		TimePoint time_;
		Float delta_seconds_;
	};
}
