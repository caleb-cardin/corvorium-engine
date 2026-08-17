// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

export module Rendering:Device;

export namespace Corvorium::Rendering {

class RenderContext;

class RenderDevice {
public:
    virtual ~RenderDevice() = default;

    virtual bool Initialize(const RenderContext& Context) = 0;
    virtual void Shutdown() = 0;
};

}
