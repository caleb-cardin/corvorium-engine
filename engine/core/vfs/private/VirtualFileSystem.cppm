// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "common/Logging.hpp"

#include "EASTL/unique_ptr.h"
#include "EASTL/string.h"
module VFS:VirtualFileSystem_Impl;
import :VirtualFileSystem;
import :MountTable;
import :LooseFileSyncBackend;

namespace Corvorium::VFS {
VirtualFileSystem::VirtualFileSystem()
    : pMountTable{eastl::make_unique<MountTable>()}
    , pBackend{eastl::make_unique<LooseFileSyncBackend>()}
{}

VirtualFileSystem::~VirtualFileSystem() = default;

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
    return WriteFile(eastl::string{Path}, Bytes);
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
