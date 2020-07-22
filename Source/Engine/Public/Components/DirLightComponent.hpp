#pragma once
#include "Components/SceneComponent.hpp"
#include "Interfaces/Light.hpp"

namespace oeng::engine
{
	class ENGINE_API DirLightComponent : public SceneComponent, public IDirLight
	{
	public:
		DELETE_CPMV(DirLightComponent);
		
		explicit DirLightComponent(AActor& owner, int update_order = 100);
		~DirLightComponent();

		void SetColor(const Vec3& color) noexcept { data_.color = color; }
		const Data& GetData() const noexcept override { return data_; }
		
	private:
		void OnTrsfChanged() override { data_.dir = GetForward(); }
		void OnActivated() override;
		void OnDeactivated() override;
		void OnBeginPlay() override;
		
		Data data_;
	};
}