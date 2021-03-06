#include "Engine/Mesh.hpp"
#include "DynamicRHI.hpp"
#include "Engine/AssetManager.hpp"
#include "Materials/IMaterial.hpp"
#include "RHIMesh.hpp"
#include "Vertex.hpp"

namespace oeng
{
inline namespace engine
{
SharedRef<Mesh> Mesh::GetDefault()
{
    return AssetManager::Get().Load<Mesh>(u8"../Engine/Assets/SM_Default.json"sv);
}

Mesh::Mesh() : material_{IMaterial::GetDefault()}
{
}

Mesh::~Mesh() = default;

void Mesh::from_json(const Json& json)
{
    const auto vertices = json.at("Vertices").get<std::vector<Vertex>>();
    const auto indices = json.at("Indices").get<std::vector<Vec3u16>>();

    material_ = AssetManager::Get().Load<IMaterial>(json.at("Material").get<Path>());
    rhi_.reset(DynamicRHI::Get().CreateMesh(vertices, indices));

    auto max = 0_f;
    for (const auto& v : vertices)
        max = Max(max, v.pos.DistSqr(Vec3::zero));
    radius_ = std::sqrt(max);
}
} // namespace engine
} // namespace oeng
