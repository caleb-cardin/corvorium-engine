// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "../../../lib/mimalloc/include/mimalloc/atomic.h"
#include "common/Logging.hpp"

#include "EASTL/unique_ptr.h"
export module VFS;
export import :Types;

import Common;
import :BackendInterface;
import :MountTable;
import :LooseFileSyncBackend;

namespace Corvorium::VFS {
Log::Logger VfsLogger{"VFSLog"};

export class VirtualFileSystem {
public:
    VirtualFileSystem();

    template<AssetType T>
    void Mount(const eastl::string& PhysicalPath);

    /**
     *
     * @tparam T AssetType of the file. Resolves AssetPathTypeTraits::Name for virtual path.
     * @param Path Relative path of file.
     */
    [[nodiscard]] ResultType<ByteArray> ReadFile(const char* Path) const;
    [[nodiscard]] ResultType<ByteArray> ReadFile(const eastl::string& Path) const;

    [[nodiscard]] VfsError WriteFile(const char* Path, const ByteArray& Bytes) const;
    [[nodiscard]] VfsError WriteFile(const eastl::string& Path, const ByteArray& Bytes) const;

private:
    eastl::unique_ptr<MountTable> pMountTable{nullptr};
    eastl::unique_ptr<IVFSBackend> pBackend{nullptr};

    VfsError Resolve(const VirtualPath& Path, VirtualPathInfo& PathInfo) const;
};

template<AssetType T>
void VirtualFileSystem::Mount(const eastl::string& PhysicalPath)
{
    CRVM_LOG_RETURN_IF(pMountTable->IsMounted(AssetPathTypeTraits<T>::Name),
                       /*void*/
                       ;,
                       VfsLogger,
                       kError,
                       "Attempted to mount and existing asset.");

    pMountTable->Mount(AssetPathTypeTraits<T>::Name, pBackend.get(), PhysicalPath);
}

VirtualFileSystem::VirtualFileSystem()
    : pMountTable{eastl::make_unique<MountTable>()}
    , pBackend{eastl::make_unique<LooseFileSyncBackend>()}
{}

ResultType<ByteArray> VirtualFileSystem::ReadFile(const char* Path) const
{
    VirtualPathInfo info;
    if (const auto err = Resolve(Path, info); err != VfsError::Success)
    {
        return ResultError(err);
    }
    if (!(info.Flags & MountFlagBits::Read))
    {
        return ResultError(VfsError::PermissionDenied);
    }

    return info.Backend->ReadFile(info.PhysicalPath);
}

ResultType<ByteArray> VirtualFileSystem::ReadFile(const eastl::string& Path) const
{
    return ReadFile(Path.c_str());
}

VfsError VirtualFileSystem::WriteFile(const char* Path, const ByteArray& Bytes) const
{
    return WriteFile({Path}, Bytes);
}

VfsError VirtualFileSystem::WriteFile(const eastl::string& Path, const ByteArray& Bytes) const
{
    VirtualPathInfo info;
    if (const auto err = Resolve(Path, info); err != VfsError::Success)
    {
        return err;
    }
    if (!(info.Flags & MountFlagBits::Write))
    {
        return VfsError::PermissionDenied;
    }

    return info.Backend->WriteFile(Path, Bytes);
}

VfsError VirtualFileSystem::Resolve(const VirtualPath& Path, VirtualPathInfo& PathInfo) const
{
    auto result = pMountTable->Resolve(Path);
    if (!result.has_value()) return result.error();

    PathInfo = std::move(result.value());
    return VfsError::Success;
}
}
