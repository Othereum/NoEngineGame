#include "Materials/MaterialInstance.hpp"
#include "Engine/AssetManager.hpp"
#include "Engine/Texture.hpp"
#include "Materials/Material.hpp"
#include "RHIShader.hpp"

namespace oeng
{
inline namespace engine
{
MaterialInstance::MaterialInstance() : root_{Material::GetDefault()}, parent_{root_}
{
}

[[nodiscard]] static SharedRef<Material> FindRoot(SharedRef<IMaterial> parent) noexcept
{
    if (auto mat = Cast<Material>(SharedPtr(parent)))
        return SharedRef(std::move(mat));

    return FindRoot(CastChecked<MaterialInstance>(std::move(parent))->GetParent());
}

void MaterialInstance::from_json(const Json& json)
{
    parent_ = AssetManager::Get().Load<IMaterial>(json.at("Parent").get<Path>());
    root_ = FindRoot(parent_);
    LoadParams(json.at("ParameterOverrides"));
}

void MaterialInstance::ApplyParams() const
{
    for (auto& [name, value] : root_->GetScalarParams())
        GetRHI().ApplyParam(name, GetScalarParam(name));

    for (auto& [name, value] : root_->GetVectorParams())
        GetRHI().ApplyParam(name, GetVectorParam(name));

    for (auto& [name, value] : root_->GetTextureParams())
        GetRHI().ApplyParam(name, GetTextureParam(name)->GetRHI());
}

ScalarParam MaterialInstance::GetScalarParam(Name name) const
{
    auto it = scalars_.find(name);
    return it != scalars_.end() ? it->second : parent_->GetScalarParam(name);
}

VectorParam MaterialInstance::GetVectorParam(Name name) const
{
    auto it = vectors_.find(name);
    return it != vectors_.end() ? it->second : parent_->GetVectorParam(name);
}

SharedRef<Texture> MaterialInstance::GetTextureParam(Name name) const
{
    auto it = textures_.find(name);
    return it != textures_.end() ? it->second : parent_->GetTextureParam(name);
}

RHIShader& MaterialInstance::GetRHI() const noexcept
{
    return root_->GetRHI();
}

bool MaterialInstance::IsScalarParam(Name name) const
{
    return root_->GetScalarParams().contains(name);
}

bool MaterialInstance::IsVectorParam(Name name) const
{
    return root_->GetVectorParams().contains(name);
}

bool MaterialInstance::IsTextureParam(Name name) const
{
    return root_->GetTextureParams().contains(name);
}
} // namespace engine
} // namespace oeng
