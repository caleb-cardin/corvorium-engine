// Copyright © 2026 Caleb Cardin. All rights reserved.
#pragma once

#define VFS_DECLARE_ASSET_HANDLE(_assetType, _handlePrefix)     \
template<> struct AssetPathTypeTraits<_assetType> {             \
    static constexpr const char* Name = _handlePrefix;          \
}
