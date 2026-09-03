// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "CorvoriumApi.h"

#include "EASTL/unique_ptr.h"
#include "EASTL/string.h"
export module VFS:VirtualFileSystem;
import Logging;
import OS;
import :Types;

using namespace Corvorium::FileSystem;

namespace Corvorium::VFS {
class MountTable;
class IVFSBackend;

Log::Logger VfsLogger{"VFSLog"};

export class CRVM_API VirtualFileSystem : public IFileProvider {
public:
    VirtualFileSystem();
    ~VirtualFileSystem();

    void Mount(const PhysicalPath& PhysicalPath, const eastl::string& Name);

    [[nodiscard]] ResultValue<ByteArray> ReadFile(const VirtualPath& Path) const override;

    [[nodiscard]] FileError WriteFile(const char* Path, const ByteArray& Bytes) const;
    [[nodiscard]] FileError WriteFile(const eastl::string& Path, const ByteArray& Bytes) const;

    [[nodiscard]] ResultValue<PhysicalPath> ResolveVirtualPath(const VirtualPath& Path) const override;

private:
    eastl::unique_ptr<MountTable> pMountTable{nullptr};
    eastl::unique_ptr<IVFSBackend> pBackend{nullptr};

    FileError Resolve(const VirtualPath& Path, VirtualPathInfo& PathInfo) const;
};
}
