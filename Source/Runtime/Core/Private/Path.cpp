#include "Path.hpp"

namespace oeng::core
{
PathSet& Path::Set() noexcept
{
    if constexpr (!kPathThreadSafe)
    {
        assert(IsGameThread());
    }
    return EngineBase::Get().paths_;
}

void to_json(Json& json, const Path& path)
{
    json = AsString(path.Str());
}

void from_json(const Json& json, Path& path)
{
    path = AsString8(json.get<std::string>());
}
}
