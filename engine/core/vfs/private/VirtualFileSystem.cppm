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

void VirtualFileSystem::Mount(const eastl::string& PhysicalPath, const eastl::string& Name)
{
    CRVM_LOG_RETURN_IF(pMountTable->IsMounted(Name),
                       /*void*/
                       ;,
                       VfsLogger,
                       kError,
                       "Attempted to mount and existing asset.");

    pMountTable->Mount(Name, pBackend.get(), PhysicalPath);
}

ResultValue<eastl::vector<u8>> VirtualFileSystem::ReadFile(const eastl::string& Path) const
{
    VirtualPathInfo info;
    if (const auto err = Resolve(Path, info); err != FileError::Success)
    {
        return ResultError(err);
    }
    if (!(info.Flags & MountFlagBits::Read))
    {
        return ResultError(FileError::PermissionDenied);
    }

    return info.Backend->ReadFile(info.PhysicalPath);
}

FileError VirtualFileSystem::WriteFile(const char* Path, const ByteArray& Bytes) const
{
    return WriteFile(eastl::string{Path}, Bytes);
}

FileError VirtualFileSystem::WriteFile(const eastl::string& Path, const ByteArray& Bytes) const
{
    VirtualPathInfo info;
    if (const auto err = Resolve(Path, info); err != FileError::Success)
    {
        return err;
    }
    if (!(info.Flags & MountFlagBits::Write))
    {
        return FileError::PermissionDenied;
    }

    return info.Backend->WriteFile(Path, Bytes);
}

ResultValue<eastl::string> VirtualFileSystem::ResolveVirtualPath(const VirtualPath& Path) const
{
    VirtualPathInfo info;
    FileError err = Resolve(Path, info);
    CRVM_LOG_RETURN_IF(err != FileError::Success, ResultError(err), VfsLogger, kError, "Failed to resolve path {}", Path.c_str());
    return eastl::move(info.PhysicalPath);
}

FileError VirtualFileSystem::Resolve(const VirtualPath& Path, VirtualPathInfo& PathInfo) const
{
    ResultValue<VirtualPathInfo> result = pMountTable->Resolve(Path);
    if (!result.has_value()) return result.error();

    PathInfo = std::move(result.value());
    return FileError::Success;
}
}
