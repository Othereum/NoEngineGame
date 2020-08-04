#pragma once
#include "Actor.hpp"
#include "Path.hpp"

namespace oeng::renderer
{
	class Mesh;
	class Material;
}

namespace oeng::engine
{
	class MeshComponent;
	
	class ENGINE_API AMeshActor : public AActor
	{
	public:
		explicit AMeshActor(World& world);
		void SetMesh(Path path) const;
		void SetMesh(SharedRef<Mesh> mesh) const;
		void SetMaterial(Path path) const;
		void SetMaterial(SharedRef<Material> material) const;
		[[nodiscard]] Mesh& GetMesh() const noexcept;
		[[nodiscard]] Material& GetMaterial() const noexcept;
		[[nodiscard]] MeshComponent& GetMeshComp() const noexcept { return mesh_; }

	private:
		MeshComponent& mesh_;
	};
}
