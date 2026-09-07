// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "EASTL/vector.h"

module VFS:BackendInterface;
import :Types;
import OS;
import std;

using namespace Corvorium::FileSystem;

namespace Corvorium::VFS {
class IVFSBackend {
public:
    virtual ~IVFSBackend() = default;

    [[nodiscard]] virtual ResultValue<ByteArray> ReadFile(const PhysicalPath& Path) noexcept = 0;
    [[nodiscard]] virtual FileError WriteFile(const PhysicalPath& Path, const ByteArray& Bytes) noexcept = 0;
    [[nodiscard]] virtual bool Exists(const PhysicalPath& Path) = 0;
};
}
