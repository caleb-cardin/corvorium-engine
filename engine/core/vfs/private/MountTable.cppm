// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "common/Logging.hpp"

#include "EASTL/vector_multimap.h"
#include "EASTL/string.h"

#include <filesystem>
module VFS:MountTable;
import :BackendInterface;
import :Types;
import OS;
import Logging;

using namespace Corvorium::FileSystem;

namespace Corvorium::VFS {
extern Log::Logger VfsLogger;

static constexpr auto kMountPathDelimiter = "://";
static constexpr u64 kMountPathDelimiterSize = sizeof("://") - 1;


class MountTable {
    using PathPrefix = eastl::string;

public:
    MountTable() = default;

    [[nodiscard]] bool IsMounted(const eastl::string& Prefix) const
    {
        return MountTableMappings.contains(Prefix);
    }

    [[nodiscard]] MountFlags GetFlags(const eastl::string& Prefix) const
    {
        if (IsMounted(Prefix))
        {
            return MountTableMappings.find(Prefix)->second.Flags;
        }
        return MountFlagBits::Auto;
    }

    VirtualPath Mount(eastl::string Prefix,
                      IVFSBackend* Backend,
                      const PhysicalPath& PhysicalPath,
                      MountFlags Flags = MountFlagBits::Auto)
    {
        CRVM_LOG_RETURN_IF(Prefix.find(kMountPathDelimiter) != eastl::string::npos,
                           {},
                           VfsLogger,
                           kError,
                           "Failed to mount virtual path: Illegal prefix contains mount delimiter (\":\\\\\"): {}",
                           Prefix.c_str());

        if (Flags == MountFlagBits::Auto)
        {
            if (MountTableMappings.contains(Prefix))
            {
                const auto it = MountTableMappings.find(Prefix);
                Flags = it->second.Flags;
            }
            else
            {
                Flags = MountFlagBits::Read | MountFlagBits::Write;
            }
        }

        Prefix.append(kMountPathDelimiter);
        MountTableMappings.emplace(Prefix, VirtualPathInfo{.Backend = Backend, .PhysicalPath = PhysicalPath, .Flags = Flags});
        return Prefix;
    }

    VirtualPath Mount(const char* Prefix, IVFSBackend* Backend, const PhysicalPath& PhysicalPath, const MountFlags Flags = MountFlagBits::Auto)
    {
        return Mount(eastl::string{Prefix}, Backend, PhysicalPath, Flags);
    }

    ResultValue<VirtualPathInfo> Resolve(const VirtualPath& Path)
    {
        const auto index = Path.find_first_of(kMountPathDelimiter) + kMountPathDelimiterSize;
        const eastl::string prefix = Path.substr(0, index);
        const PhysicalPath physicalSubpath = Path.substr(index, Path.size() - index);

        const auto range = MountTableMappings.equal_range(prefix);

        for (auto it = range.first; it != range.second; ++it)
        {
            IVFSBackend* pBackend = it->second.Backend;

            std::filesystem::path fsPath = it->second.PhysicalPath.c_str();

            fsPath /= physicalSubpath.c_str();

            if (std::filesystem::exists(fsPath))
            {
                return VirtualPathInfo{
                    .Backend = pBackend,
                    .PhysicalPath = PhysicalPath{reinterpret_cast<const char*>(fsPath.u8string().c_str())},
                    .Flags = it->second.Flags
                };
            }
        }

        CRVM_LOG(VfsLogger, kWarning, "Resource \"{}\" could not be resolved.", Path.c_str());

        return ResultError(FileError::FileNotFound);
    }

private:
    eastl::vector_multimap<PathPrefix, VirtualPathInfo> MountTableMappings;
};
}
