// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "CorvoriumApi.h"
#include "common/FlagTypeMacro.hpp"

#include <atomic>
#include <EASTL/string.h>
#include <EASTL/vector.h>
export module VFS:Types;
import Common;

namespace Corvorium::VFS {
export using ByteArray = eastl::vector<std::byte>;
export using VirtualPath = eastl::string;
export using PhysicalPath = eastl::string;

// Auto will find any prefix-matching mounts and copy the flags, or default to R/W
export enum class MountFlagBits : u8 {
    Auto = 0,
    Read = 0x1 << 0,
    Write = 0x1 << 1
};

export enum class VfsError : u32 {
    Success = 0,
    FileNotFound,
    PermissionDenied,
    NotADirectory,
    IsADirectory,
    NameLength,

    Internal,
    Unknown,
};

template<typename V>
using ResultType = ResultValueType<V, VfsError>;
using ResultError = ResultError<VfsError>;


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
        return AssetHandle{NextHandle.fetch_add(1, std::memory_order::relaxed)};
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
    static std::atomic<u64> NextHandle;
    u64 Handle{kInvalidHandle};

    explicit AssetHandle(const u64 NewHandle)
        : Handle{NewHandle}
    {}
};

template<typename T>
std::atomic<u64> AssetHandle<T>::NextHandle = 0;

export template<typename T>
struct AssetPathTypeTraits {
    using Type = T;
    using Handle = AssetHandle<T>;
    static constexpr const char* Name = nullptr;
};

export template<typename T>
concept AssetType = requires {
    { AssetPathTypeTraits<T>::Name != nullptr };
};

const char* ToCStr(const VfsError Error)
{
    switch (Error)
    {
    case VfsError::Success:
        return "Success";
    case VfsError::FileNotFound:
        return "FileNotFound";
    case VfsError::PermissionDenied:
        return "PermissionDenied";
    case VfsError::NotADirectory:
        return "NotADirectory";
    case VfsError::IsADirectory:
        return "IsADirectory";
    case VfsError::NameLength:
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
