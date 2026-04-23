#include "tcApp.h"

void tcApp::setup() {
    sender_.setup("TrussC NDI Sender Example");
}

void tcApp::update() {
    // Capture this frame's window pixels and ship them to the NDI network.
    if (sender_.isSetup() && grabScreen(pixels_)) {
        sender_.send(pixels_);
    }
}

void tcApp::draw() {
    clear(0.05f, 0.05f, 0.08f);

    // Animated palette so receivers have something to look at.
    const float t = getElapsedTime();
    const int   cols = 8, rows = 5;
    const float cw = (float)getWindowWidth()  / cols;
    const float ch = (float)getWindowHeight() / rows;

    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            float r = 0.5f + 0.5f * std::sin(t + x * 0.4f);
            float g = 0.5f + 0.5f * std::sin(t + y * 0.5f + 1.5f);
            float b = 0.5f + 0.5f * std::sin(t + (x + y) * 0.3f + 3.0f);
            setColor(r, g, b);
            drawRect(x * cw, y * ch, cw - 2, ch - 2);
        }
    }

    // HUD
    setColor(1.0f);
    drawBitmapString(std::string("NDI: ") + sender_.getName(), 12, 20);
    drawBitmapString(std::string("Connections: ") +
                         std::to_string(sender_.getNumConnections()),
                     12, 40);
}
