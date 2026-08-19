// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "EASTL/vector.h"
#include <cstddef>
module VFS:BackendInterface;
import :Types;

namespace Corvorium::VFS {
class IVFSBackend {
public:
    virtual ~IVFSBackend() = default;

    [[nodiscard]] virtual ResultType<ByteArray> ReadFile(const PhysicalPath& Path) noexcept = 0;
    [[nodiscard]] virtual VfsError WriteFile(const PhysicalPath& Path, const ByteArray& Bytes) noexcept = 0;
    [[nodiscard]] virtual bool Exists(const PhysicalPath& Path) = 0;
};
}
