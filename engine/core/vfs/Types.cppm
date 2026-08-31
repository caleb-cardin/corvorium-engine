// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "CorvoriumApi.h"
#include "common/FlagTypeMacro.hpp"

#include "EASTL/string.h"
#include "EASTL/vector.h"
#include "EASTL/atomic.h"
export module VFS:Types;
import Common;
import OS;

using namespace Corvorium::FileSystem;

namespace Corvorium::VFS {
export using ByteArray = eastl::vector<u8>;
export using VirtualPath = eastl::string;
export using PhysicalPath = eastl::string;

// Auto will find any prefix-matching mounts and copy the flags, or default to R/W
export enum class MountFlagBits : u8 {
    Auto = 0,
    Read = 0x1 << 0,
    Write = 0x1 << 1
};

// Assets : TODO Move to some asset reg module

export enum class AssetState : u8 {
    None,
    Unloaded,
    Queued,
    Loading,
    Ready,
    Failed
};

export template<typename T>
class AssetHandle {
public:
    static constexpr u64 kInvalidHandle = ~0ull;
    AssetHandle() = default;
    ~AssetHandle() = default;

    static AssetHandle NewHandle()
    {
        return AssetHandle{NextHandle.fetch_add(1, eastl::internal::memory_order_relaxed_s{})};
    }

    AssetHandle(const AssetHandle& Other) = default;
    AssetHandle(AssetHandle&& Other) noexcept = default;
    AssetHandle& operator=(const AssetHandle& Other) = default;
    AssetHandle& operator=(AssetHandle&& Other) noexcept = default;

    operator bool() const
    {
        return Handle != kInvalidHandle;
    }

private:
    static eastl::atomic<u64> NextHandle;
    u64 Handle{kInvalidHandle};

    explicit AssetHandle(const u64 NewHandle)
        : Handle{NewHandle}
    {}
};

template<typename T>
eastl::atomic<u64> AssetHandle<T>::NextHandle = 0;

const char* ToCStr(const FileError Error)
{
    switch (Error)
    {
    case FileError::Success:
        return "Success";
    case FileError::FileNotFound:
        return "FileNotFound";
    case FileError::PermissionDenied:
        return "PermissionDenied";
    case FileError::NotADirectory:
        return "NotADirectory";
    case FileError::IsADirectory:
        return "IsADirectory";
    case FileError::NameLength:
        return "NameLength";
    default:
        return "Unknown";
    }
}
}

namespace Corvorium {
DECLARE_FLAG_TYPE_NS(MountFlags, VFS::MountFlagBits, VFS);

namespace VFS {
class IVFSBackend;

struct VirtualPathInfo {
    IVFSBackend* Backend{nullptr};
    PhysicalPath PhysicalPath{};
    MountFlags Flags{};
};
}
}
