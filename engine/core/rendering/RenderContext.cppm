// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "CorvoriumApi.h"
export module Rendering:Context;
export import :Device;
export namespace Corvorium::Rendering {
class RenderDevice;

class CRVM_API RenderContext {
public:
    virtual ~RenderContext() = default;

    virtual bool Initialize() = 0;
    virtual void Shutdown() = 0;
};
}
