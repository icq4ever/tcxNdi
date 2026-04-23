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
    void keyPressed(int key) override;

private:
    NdiFinder              finder_;
    NdiReceiver            receiver_;
    std::vector<NdiSource> sources_;
    int                    selected_ = -1;
    Texture                texture_;
};
