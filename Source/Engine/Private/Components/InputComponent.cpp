#include "Components/InputComponent.hpp"
#include "InputSystem.hpp"

namespace oeng
{
	InputComponent::InputComponent(AActor& owner, int update_order)
		:ActorComponent{owner, update_order}
	{
	}

	void InputComponent::OnUpdate(Float)
	{
		if (IsActive())
		{
			ProcessActions();
			ProcessAxises();
		}
	}

	void InputComponent::BindAction(Name action, bool pressed, std::function<void()>&& callback)
	{
		actions_[pressed].emplace(action, std::move(callback));
	}

	void InputComponent::BindAxis(Name axis, std::function<void(Float)>&& callback)
	{
		axises_.emplace(axis, std::move(callback));
	}

	void InputComponent::ProcessActions() const
	{
		const auto& input_system = InputSystem::Get();
		for (const auto& event : input_system.GetEvents())
		{
			auto [it, end] = actions_[event.pressed].equal_range(event.name);
			for (; it != end; ++it) it->second();
		}
	}

	void InputComponent::ProcessAxises() const
	{
		const auto& input_system = InputSystem::Get();
		for (const auto& axis_map : axises_)
		{
			axis_map.second(input_system.GetAxisValue(axis_map.first));
		}
	}
}