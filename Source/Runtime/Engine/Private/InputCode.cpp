#include "InputCode.hpp"
#include <unordered_map>
#include <SDL_keyboard.h>
#include <SDL_gamecontroller.h>

namespace oeng
{
	String8 GetName(Keycode btn) noexcept
	{
		return AsString8(SDL_GetKeyName(SDL_Keycode(btn)));
	}

	std::u8string_view GetName(CtrlAxis axis) noexcept
	{
		const auto s = SDL_GameControllerGetStringForAxis(SDL_GameControllerAxis(axis));
		if (s) return AsString8(s);
		return {};
	}

	std::u8string_view GetName(CtrlBtn btn) noexcept
	{
		const auto s = SDL_GameControllerGetStringForButton(SDL_GameControllerButton(btn));
		if (s) return AsString8(s);
		return {};
	}

	DyArr<std::u8string_view> GetNames(KeyMod mod) noexcept
	{
		static constexpr KeyMod mods[]
		{
			KeyMod::L_SHIFT, KeyMod::R_SHIFT,
			KeyMod::L_CTRL, KeyMod::R_CTRL,
			KeyMod::L_ALT, KeyMod::R_ALT,
			KeyMod::L_GUI, KeyMod::R_GUI,
			KeyMod::NUM, KeyMod::CAPS, KeyMod::MODE
		};
		
		DyArr<std::u8string_view> names;
		
		for (auto m : mods) if ((mod & m) != KeyMod::NONE)
			names.push_back(GetName(m));
		
		return names;
	}


	std::optional<Keycode> ToKeycode(std::u8string_view name) noexcept
	{
		const auto str = AsString(name.data());
		const auto key = SDL_GetKeyFromName(str);
		if (key != SDLK_UNKNOWN) return Keycode(key);
		return std::nullopt;
	}

	std::optional<CtrlAxis> ToCtrlAxis(std::u8string_view name) noexcept
	{
		const auto str = AsString(name.data());
		const auto axis = SDL_GameControllerGetAxisFromString(str);
		if (axis != SDL_CONTROLLER_AXIS_INVALID) return CtrlAxis(axis);
		return std::nullopt;
	}

	std::optional<CtrlBtn> ToCtrlBtn(std::u8string_view name) noexcept
	{
		const auto str = AsString(name.data());
		const auto btn = SDL_GameControllerGetButtonFromString(str);
		if (btn != SDL_CONTROLLER_BUTTON_INVALID) return CtrlBtn(btn);
		return std::nullopt;
	}

	template <class T>
	static constexpr std::pair<std::u8string_view, T> ToPair(T code) noexcept
	{
		return {GetName(code), code};
	}

	std::optional<MouseAxis> ToMouseAxis(std::u8string_view name) noexcept
	{
		static const std::unordered_map<std::u8string_view, MouseAxis> map
		{
			ToPair(MouseAxis::X),
			ToPair(MouseAxis::Y)
		};
		if (const auto it = map.find(name); it != map.end()) return it->second;
		return std::nullopt;
	}

	std::optional<MouseBtn> ToMouseBtn(std::u8string_view name) noexcept
	{
		static const std::unordered_map<std::u8string_view, MouseBtn> map
		{
			ToPair(MouseBtn::L),
			ToPair(MouseBtn::M),
			ToPair(MouseBtn::R),
			ToPair(MouseBtn::X1),
			ToPair(MouseBtn::X2)
		};
		if (const auto it = map.find(name); it != map.end()) return it->second;
		return std::nullopt;
	}

	std::optional<KeyMod> ToKeyMod(std::u8string_view name) noexcept
	{
		static const std::unordered_map<std::u8string_view, KeyMod> map
		{
			ToPair(KeyMod::L_SHIFT),
			ToPair(KeyMod::R_SHIFT),
			ToPair(KeyMod::L_CTRL),
			ToPair(KeyMod::R_CTRL),
			ToPair(KeyMod::L_ALT),
			ToPair(KeyMod::R_ALT),
			ToPair(KeyMod::L_GUI),
			ToPair(KeyMod::R_GUI),
			ToPair(KeyMod::NUM),
			ToPair(KeyMod::CAPS),
			ToPair(KeyMod::MODE)
		};
		if (const auto it = map.find(name); it != map.end()) return it->second;
		return std::nullopt;
	}
}
