#pragma once

#include <string>
#include <vector>

namespace tcx {

// ---------------------------------------------------------------------------
// NdiSource — handle for a discovered NDI source on the network.
// ---------------------------------------------------------------------------
struct NdiSource {
    /// Display name as advertised, e.g. `MACHINE-NAME (My TrussC App)`.
    std::string name;
    /// Network address (`host:port`) — useful for logging or manual connect.
    std::string url;
};

// ---------------------------------------------------------------------------
// NdiFinder — discovers NDI sources currently advertising on the network.
// ---------------------------------------------------------------------------
//
// Usage:
//   tcx::NdiFinder finder;
//   finder.refresh();                // optional: wait briefly for sources
//   for (auto& s : finder.getSources()) cout << s.name;
// ---------------------------------------------------------------------------
class NdiFinder {
public:
    NdiFinder();
    ~NdiFinder();

    NdiFinder(const NdiFinder&)            = delete;
    NdiFinder& operator=(const NdiFinder&) = delete;

    /// Block up to `timeoutMs` for the source list to change, then return.
    /// Pass 0 for a non-blocking poll. Returns true if the list changed.
    bool refresh(int timeoutMs = 1000);

    /// Snapshot of currently-known sources.
    std::vector<NdiSource> getSources() const;

private:
    void* impl_ = nullptr;   // NDIlib_find_instance_t
};

} // namespace tcx
