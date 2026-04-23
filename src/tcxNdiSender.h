#pragma once

#include "tc/graphics/tcPixels.h"

#include <string>

namespace tcx {

// ---------------------------------------------------------------------------
// NdiSender — broadcast a TrussC pixel buffer onto the NDI network.
// ---------------------------------------------------------------------------
//
// Usage:
//   tcx::NdiSender sender;
//   sender.setup("My TrussC App");
//   ...
//   sender.send(pixels);   // tc::Pixels (RGBA, BGRA, or RGB/U8)
//
// Multiple senders may coexist as long as each has a unique name.
// ---------------------------------------------------------------------------
class NdiSender {
public:
    NdiSender();
    ~NdiSender();

    NdiSender(const NdiSender&)            = delete;
    NdiSender& operator=(const NdiSender&) = delete;

    /// Create the NDI source. `name` is what receivers see in their picker.
    /// Returns false if NDI initialization failed or the source could not
    /// be created.
    bool setup(const std::string& name);

    /// Tear down the NDI source explicitly. Also called from the destructor.
    void close();

    /// Push a frame to the network. Pixels must be U8; channel count
    /// determines the format (4 → BGRA/RGBA, 3 → RGB packed, others rejected).
    /// Returns false on invalid input or when the sender is not setup.
    bool send(const trussc::Pixels& pixels);

    /// Number of receivers currently subscribed to this source.
    int  getNumConnections() const;

    bool        isSetup() const { return impl_ != nullptr; }
    const std::string& getName() const { return name_; }

private:
    void*       impl_ = nullptr;   // NDIlib_send_instance_t
    std::string name_;
};

} // namespace tcx
