// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "CorvoriumApi.h"

#include "EASTL/string.h"
#include "EASTL/vector.h"
export module OS:FileProvider;
import TypeDefs;
import ResultValue;

namespace Corvorium::FileSystem {
export enum class FileError : u32 {
    Success = 0,
    FileNotFound,
    PermissionDenied,
    NotADirectory,
    IsADirectory,
    NameLength,

    InternalError,
    Unknown,
};

export template<typename V>
using ResultValue = Error::ResultValueType<V, FileError>;
export using ResultError = Error::ResultErrorType<FileError>;

export class CRVM_API IFileProvider {
public:
    virtual ~IFileProvider() = default;
    [[nodiscard]] virtual ResultValue<eastl::vector<u8>> ReadFile(const eastl::string& Path) const = 0;
    [[nodiscard]] virtual ResultValue<eastl::string> ResolveVirtualPath(const eastl::string& Path) const = 0;
};
}
