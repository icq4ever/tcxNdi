#pragma once

// tcPixels.h uses Color and memcpy without including their headers itself
// (it expects to be pulled in after TrussC.h). Bring those in first so the
// addon can include tcPixels.h directly.
#include <cstring>
#include "tcColor.h"
#include "tcxNdiFinder.h"
#include "tc/graphics/tcPixels.h"

#include <string>

namespace tcx {

// ---------------------------------------------------------------------------
// NdiReceiver — pull frames from an NDI source into a tc::Pixels buffer.
// ---------------------------------------------------------------------------
//
// Usage:
//   tcx::NdiFinder   finder;
//   tcx::NdiReceiver receiver;
//
//   finder.refresh();
//   auto sources = finder.getSources();
//   if (!sources.empty()) receiver.connect(sources[0]);
//
//   void update() {
//       if (receiver.update()) {
//           // new frame; receiver.getPixels() valid
//       }
//   }
// ---------------------------------------------------------------------------
class NdiReceiver {
public:
    NdiReceiver();
    ~NdiReceiver();

    NdiReceiver(const NdiReceiver&)            = delete;
    NdiReceiver& operator=(const NdiReceiver&) = delete;

    /// Connect to an NDI source previously discovered by NdiFinder.
    bool connect(const NdiSource& source);

    /// Disconnect; safe to call when not connected.
    void disconnect();

    /// Poll for a new video frame. `timeoutMs` 0 = non-blocking.
    /// Returns true when a new frame has been written into getPixels().
    /// Audio frames are silently discarded for now.
    bool update(int timeoutMs = 0);

    /// True if at least one frame has been received since connect().
    bool hasFrame() const { return hasFrame_; }

    /// CPU pixels of the latest received frame (RGBA, U8).
    trussc::Pixels&       getPixels()       { return pixels_; }
    const trussc::Pixels& getPixels() const { return pixels_; }

    bool isConnected() const { return impl_ != nullptr; }

private:
    void*           impl_ = nullptr;   // NDIlib_recv_instance_t
    trussc::Pixels  pixels_;
    bool            hasFrame_ = false;
};

} // namespace tcx
