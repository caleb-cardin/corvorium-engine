// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

export module RenderingCommon:Primitives;
import CommonTypes;
namespace Corvorium::Rendering {

export struct Color {
    u8 R{0};
    u8 G{0};
    u8 B{0};
    u8 A{0};

    Color(u8 Red, u8 Green, u8 Blue, u8 Alpha = 255)
        : R{Red}
        , G{Green}
        , B{Blue}
        , A{Alpha}
    {}
};

}

