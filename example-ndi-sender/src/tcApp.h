#pragma once

#include "tcBaseApp.h"
#include "tcxNdi.h"

using namespace tc;
using namespace tcx;

class tcApp : public App {
public:
    void setup() override;
    void update() override;
    void draw() override;

private:
    NdiSender sender_;
    Pixels    pixels_;
};
