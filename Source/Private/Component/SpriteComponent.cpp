#include "Components/SpriteComponent.hpp"
#include <GL/glew.h>
#include "Actor.hpp"
#include "Engine.hpp"
#include "Graphics/Renderer.hpp"
#include "Graphics/Shader.hpp"
#include "Graphics/Texture.hpp"

namespace oeng
{
	SpriteComponent::SpriteComponent(AActor& owner, const int draw_order, const int update_order)
		:ActorComponent{owner, update_order}, draw_order_{draw_order}
	{
	}

	SpriteComponent::~SpriteComponent()
	{
		GetRenderer().UnregisterSprite(*this);
	}

	void SpriteComponent::SetTexture(std::shared_ptr<Texture>&& texture)
	{
		texture_ = std::move(texture);
	}

	void SpriteComponent::SetTexture(const std::shared_ptr<Texture>& texture)
	{
		texture_ = texture;
	}

	void SpriteComponent::SetTexture(Path file)
	{
		SetTexture(GetEngine().GetTexture(file));
	}

	Renderer& SpriteComponent::GetRenderer() const noexcept
	{
		return GetEngine().GetRenderer();
	}

	void SpriteComponent::Draw(Shader& shader) const
	{
		static const Name name = "uWorldTransform";
		shader.SetMatrixUniform(name, MakeScale<4>({texture_->Size(), 1}) * GetOwner().GetTransformMatrix());
		texture_->Activate();
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
	}
	
	void SpriteComponent::OnBeginPlay()
	{
		GetRenderer().RegisterSprite(*this);
	}
}
