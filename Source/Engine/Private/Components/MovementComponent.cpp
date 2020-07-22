#include "Components/MovementComponent.hpp"
#include "Debug.hpp"
#include "Actors/Actor.hpp"
#include "Components/SceneComponent.hpp"

namespace oeng::engine
{
	MovementComponent::MovementComponent(AActor& owner, int update_order)
		:ActorComponent{owner, update_order}
	{
	}

	void MovementComponent::OnUpdate(Float delta_seconds)
	{
		auto* root = GetOwner().GetRootComponent();
		if (!ENSURE(root)) return;

		auto trsf = root->GetRelTrsf();
		
		const auto moved = Move(trsf, delta_seconds);
		const auto rotated = Rotate(trsf);

		if (moved || rotated) root->SetRelTrsf(trsf);
	}

	bool MovementComponent::Move(Transform& trsf, Float delta_seconds) noexcept
	{
		const auto lensqr = mov_input_.LenSqr();
		if (lensqr <= kSmallNum) return false;
		
		if (lensqr > 1) mov_input_ /= sqrt(lensqr);
		trsf.pos += mov_input_ * (max_speed_ * delta_seconds);
		
		mov_input_ = {};
		return true;
	}

	bool MovementComponent::Rotate(Transform& trsf) noexcept
	{
		if (IsNearlyEqual(rot_input_, Quat::identity)) return false;
		
		trsf.rot = rot_input_ * trsf.rot;
		rot_input_ = {};
		return true;
	}
}