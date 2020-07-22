#include "Components/SceneComponent.hpp"
#include "Debug.hpp"

namespace oeng::engine
{
	SceneComponent::SceneComponent(AActor& owner, int update_order)
		:ActorComponent{owner, update_order}, parent_{}
	{
	}

	void SceneComponent::AttachTo(SceneComponent* new_parent, AttachRule rule)
	{
		if (parent_)
		{
			auto& siblings = parent_->childs_;
			const auto me = std::find_if(siblings.begin(), siblings.end(), 
				[this](const SceneComponent& other) { return &other == this; }
			);
		
			assert(me != siblings.end());
			siblings.erase(me);
		}

		parent_ = new_parent;

		if (new_parent)
		{
			new_parent->childs_.emplace_back(*this);
		}

		switch (rule)
		{
		case AttachRule::kKeepRelative: RecalcWorldTrsf(); break;
		case AttachRule::kKeepWorld: RecalcRelTrsf(); break;
		default: EXPECT_NO_ENTRY();
		}
	}

	void SceneComponent::RecalcWorldTrsf() noexcept
	{
		if (parent_)
		{
			world_mat_ = rel_trsf_.ToMatrix() * parent_->GetWorldTrsfMatrix();
			world_trsf_ = Transform{world_mat_};
		}
		else
		{
			world_mat_ = rel_trsf_.ToMatrix();
			world_trsf_ = rel_trsf_;
		}

		for (auto c : childs_)
		{
			c.get().RecalcWorldTrsf();
		}

		OnTrsfChanged();
	}

	void SceneComponent::RecalcRelTrsf() noexcept
	{
		world_mat_ = world_trsf_.ToMatrix();
		
		if (parent_)
		{
			if (const auto pwi = parent_->GetWorldTrsfMatrix().Inv())
			{
				// pw: parent's world transform
				// r: unknown new relative transform to parent
				// w: desired world transform
				// 
				// r * pw = w
				// r * pw * pw(-1) = w * pw(-1)
				// r = w * pw(-1)
				rel_trsf_ = Transform{world_mat_ * *pwi};
			}
			else
			{
				rel_trsf_ = world_trsf_;
			}
		}
		else
		{
			rel_trsf_ = world_trsf_;
		}

		for (auto c : childs_)
		{
			c.get().RecalcWorldTrsf();
		}

		OnTrsfChanged();
	}
}