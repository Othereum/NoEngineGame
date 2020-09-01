#pragma once

namespace logcat
{
extern ENGINE_API const LogCategory kAsset;
}

namespace oeng
{
inline namespace engine
{
class ENGINE_API AssetManager
{
public:
    DELETE_CPMV(AssetManager);
    ~AssetManager();

    [[nodiscard]] static AssetManager& Get() noexcept;

    /**
     * Returns the asset if it is already loaded, otherwise load it.
     * @param path Asset path.
     * @return Loaded asset.
     */
    [[nodiscard]] std::shared_ptr<Object> Load(Path path);

    /**
     * Returns the asset if it is already loaded, otherwise load it.
     * @tparam T Asset type.
     * @param path Asset path.
     * @return Loaded asset.
     * @throw std::bad_cast If failed to cast from actual type of asset to type `T`.
     */
    template <class T>
    [[nodiscard]] std::shared_ptr<T> Load(Path path)
    {
        auto ptr = Load(path);
        return {std::move(ptr), *dynamic_cast<T&>(*ptr)};
    }

private:
    std::unordered_map<Path, std::weak_ptr<Object>> assets_;
#ifndef NDEBUG
    std::unordered_map<Path, uint32_t> reload_count_;
#endif
};
}
}
