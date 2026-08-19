// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "CorvoriumApi.h"
export module Rendering;

namespace Corvorium::Rendering {
export class CRVM_API RenderingModule {
public:
    RenderingModule();
    ~RenderingModule();

    RenderingModule(const RenderingModule&) = delete;
    RenderingModule(RenderingModule&&) = delete;

    RenderingModule& operator=(const RenderingModule&) = delete;
    RenderingModule& operator=(RenderingModule&&) = delete;

private:
};
}
