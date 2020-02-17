#include "components/sprite_component.h"
#include <SDL.h>
#include "actors/actor.h"
#include "application.h"
#include "world.h"

namespace game
{
	sprite_component::sprite_component(actor& owner, const int draw_order, const int update_order)
		:component{owner, update_order}, draw_order_{draw_order}
	{
		owner.get_world().get_app().add_sprite(*this);
	}

	sprite_component::~sprite_component()
	{
		get_owner().get_world().get_app().remove_sprite(*this);
	}

	void sprite_component::draw(SDL_Renderer& renderer) const
	{
		if (!texture_) return;

		const vector2<int> size{tex_size_ * get_owner().get_scale()};
		const vector2<int> pos{get_owner().get_pos() - size/2};

		const SDL_Rect rect{pos.x, pos.y, size.x, size.y};
		SDL_RenderCopyEx(&renderer, texture_.get(), nullptr, &rect, get_owner().get_rot().get(), nullptr, SDL_FLIP_NONE);
	}

	void sprite_component::set_texture(std::shared_ptr<SDL_Texture>&& texture)
	{
		texture_ = std::move(texture);

		int w, h;
		SDL_QueryTexture(texture_.get(), nullptr, nullptr, &w, &h);
		tex_size_ = vector2{w, h};
	}
}
