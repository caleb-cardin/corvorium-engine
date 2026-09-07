// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "common/Logging.hpp"

module VFS:LooseFileSyncBackend;
import :BackendInterface;
import :Types;
import OS;
import Logging;
import std;

using namespace Corvorium::FileSystem;

namespace Corvorium::VFS {
extern Log::Logger VfsLogger;

class LooseFileSyncBackend : public IVFSBackend {
public:
    [[nodiscard]] ResultValue<ByteArray> ReadFile(const PhysicalPath& Path) noexcept override;
    [[nodiscard]] FileError WriteFile(const PhysicalPath& Path, const ByteArray& Bytes) noexcept override;
    [[nodiscard]] bool Exists(const PhysicalPath& Path) override;
};

ResultValue<ByteArray> LooseFileSyncBackend::ReadFile(const PhysicalPath& Path) noexcept
{
    CRVM_LOG_RETURN_IF(!Exists(Path),
                       ResultError(FileError::FileNotFound),
                       VfsLogger,
                       kError,
                       "Read file failed: {}",
                       ToCStr(FileError::FileNotFound));

    const std::filesystem::path path{Path.c_str()};

    std::ifstream file;
    file.open(path, std::ios::in | std::ios::binary);

    CRVM_LOG_RETURN_IF(!file.is_open(),
                       ResultError(FileError::InternalError),
                       VfsLogger,
                       kError,
                       "Failed to open file for read: {}",
                       path.string());

    ByteArray bytes;
    const u64 size = file.tellg();
    file.seekg(0, std::ios::beg);
    bytes.resize(size);
    CRVM_LOG_RETURN_IF(!file.read(reinterpret_cast<char*>(bytes.data()), size),
                       ResultError(FileError::InternalError),
                       VfsLogger,
                       kError,
                       "Failed to read file : {}",
                       path.string());
    return std::move(bytes);
}

FileError LooseFileSyncBackend::WriteFile(const PhysicalPath& Path, const ByteArray& Bytes) noexcept
{
    CRVM_LOG_RETURN_IF(!Exists(Path),
                       FileError::FileNotFound,
                       VfsLogger,
                       kError,
                       "Write file failed: {}",
                       ToCStr(FileError::FileNotFound));

    const std::filesystem::path path{Path.c_str()};

    std::ofstream file;
    file.open(path, std::ios::out | std::ios::binary);

    CRVM_LOG_RETURN_IF(!file.is_open(),
                       FileError::InternalError,
                       VfsLogger,
                       kError,
                       "Failed to open file for write: {}",
                       path.string());


    CRVM_LOG_RETURN_IF(!file.write(reinterpret_cast<const char*>(Bytes.data()), Bytes.size()),
                       FileError::InternalError,
                       VfsLogger,
                       kError,
                       "Failed to write file : {}",
                       path.string());

    return FileError::Success;
}

bool LooseFileSyncBackend::Exists(const PhysicalPath& Path)
{
    return std::filesystem::exists({Path.c_str()});
}
}
