#include "Component/AIComponent.h"
#include "Component/AIState.h"

namespace game
{
	namespace ai_state
	{
		class Default : public Base
		{
		public:
			Default(): Base{nullptr} {}
			[[nodiscard]] FName GetName() const override { return "Default"; }
		};

		static Default default_state;
	}

	AIComponent::AIComponent(AActor& owner, int update_order):
		CActorComponent{owner, update_order},
		cur_{ai_state::default_state}
	{
	}

	void AIComponent::Update(float delta_seconds)
	{
		cur_.get().Update(delta_seconds);
	}

	void AIComponent::ChangeState(FName name)
	{
		auto& prev = cur_.get();
		auto& next = *states_.at(name);
		prev.OnExit(next);
		cur_ = next;
		next.OnEnter(prev);
	}

	void AIComponent::AddState(std::unique_ptr<ai_state::Base>&& state)
	{
		states_.emplace(state->GetName(), std::move(state));
	}
}