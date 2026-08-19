// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "CorvoriumApi.h"
#include "common/Logging.hpp"

#include "EASTL/unique_ptr.h"
#include "EASTL/string.h"
export module VFS:VirtualFileSystem;
import Common;
import :Types;

namespace Corvorium::VFS {
class MountTable;
class IVFSBackend;

Log::Logger VfsLogger{"VFSLog"};

export class CRVM_API VirtualFileSystem {
public:
    VirtualFileSystem();
    ~VirtualFileSystem();

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
#include "SilenceDllMemberWarningBegin.inc"
    eastl::unique_ptr<MountTable> pMountTable{nullptr};
    eastl::unique_ptr<IVFSBackend> pBackend{nullptr};
#include "SilenceDllMemberWarningEnd.inc"

    VfsError Resolve(const VirtualPath& Path, VirtualPathInfo& PathInfo) const;
};
}
