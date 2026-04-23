#include "tcApp.h"

void tcApp::setup() {
    finder_.refresh(1500);
    sources_ = finder_.getSources();
    if (!sources_.empty()) {
        selected_ = 0;
        receiver_.connect(sources_[0]);
    }
}

void tcApp::update() {
    // Refresh the source list periodically; doesn't disturb the active connection.
    static float lastRefresh = 0.0f;
    if (getElapsedTime() - lastRefresh > 2.0f) {
        finder_.refresh(0);
        sources_    = finder_.getSources();
        lastRefresh = getElapsedTime();
    }

    if (receiver_.update(0) && receiver_.hasFrame()) {
        // Upload the freshly received CPU pixels to a GPU texture for drawing.
        const auto& p = receiver_.getPixels();
        if (!texture_.isAllocated() ||
             texture_.getWidth()  != p.getWidth() ||
             texture_.getHeight() != p.getHeight())
        {
            texture_.allocate(p.getWidth(), p.getHeight(), 4, TextureUsage::Stream);
        }
        texture_.loadData(p.getData(), p.getWidth(), p.getHeight(), 4);
    }
}

void tcApp::draw() {
    clear(0.05f, 0.05f, 0.08f);

    if (texture_.isAllocated()) {
        // Fit-to-window with letterboxing.
        float scale = std::min((float)getWindowWidth()  / texture_.getWidth(),
                               (float)getWindowHeight() / texture_.getHeight());
        float w = texture_.getWidth()  * scale;
        float h = texture_.getHeight() * scale;
        texture_.draw((getWindowWidth()  - w) * 0.5f,
                      (getWindowHeight() - h) * 0.5f, w, h);
    } else {
        setColor(1.0f);
        setTextAlign(Center, Baseline);
        drawBitmapString("Waiting for NDI source...",
                         getWindowWidth() / 2,
                         getWindowHeight() / 2);
    }

    // Source picker HUD
    setTextAlign(Left, Baseline);
    setColor(1.0f);
    drawBitmapString("Sources (number key to switch):", 12, 20);
    for (size_t i = 0; i < sources_.size(); ++i) {
        bool sel = ((int)i == selected_);
        setColor(sel ? 0.4f : 0.7f, sel ? 0.85f : 0.7f, sel ? 0.4f : 0.7f);
        drawBitmapString(std::to_string(i + 1) + ". " + sources_[i].name,
                         12, 40 + (int)i * 16);
    }
}

void tcApp::keyPressed(int key) {
    if (key >= '1' && key <= '9') {
        size_t idx = (size_t)(key - '1');
        if (idx < sources_.size() && (int)idx != selected_) {
            selected_ = (int)idx;
            receiver_.connect(sources_[idx]);
        }
    }
}
