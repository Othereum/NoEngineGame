#include <SDL.h>
#include "Component/BgSpriteComponent.h"
#include "Engine.h"
#include "Actor/Actor.h"

namespace Game
{
	CBgSpriteComponent::CBgSpriteComponent(AActor& owner, int drawOrder, int updateOrder)
		:CSpriteComponent{owner, drawOrder, updateOrder}
	{
	}

	void CBgSpriteComponent::Update(const float deltaSeconds)
	{
		CSpriteComponent::Update(deltaSeconds);

		for (auto& bg : textures_)
		{
			bg.offset.x += scrollSpeed_ * deltaSeconds;
			if (bg.offset.x < -kScrsz.x)
			{
				bg.offset.x = static_cast<float>((textures_.size() - 1) * kScrsz.x - 1);
			}
		}
	}

	void CBgSpriteComponent::Draw() const
	{
		for (const auto& bg : textures_)
		{
			GetEngine().Draw(*bg.texture, {GetOwner().GetPos() + bg.offset, TFVector2{kScrsz}}, 0_deg);
		}
	}

	void CBgSpriteComponent::SetBgTextures(std::vector<std::shared_ptr<SDL_Texture>>&& textures)
	{
		textures_.clear();
		AddBgTextures(std::move(textures));
	}

	void CBgSpriteComponent::AddBgTextures(std::vector<std::shared_ptr<SDL_Texture>>&& textures)
	{
		for (size_t i = 0; i < textures.size(); ++i)
		{
			FBgTexture texture;
			texture.texture = std::move(textures[i]);
			texture.offset.x = static_cast<float>(i * kScrsz.x);
			textures_.push_back(std::move(texture));
		}
		textures.clear();
	}
}
