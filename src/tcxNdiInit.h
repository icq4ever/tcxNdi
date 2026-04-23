#pragma once

// Internal helper: ensures NDIlib_initialize() is called exactly once
// for the lifetime of the process (and NDIlib_destroy() at exit). Used
// internally by Sender / Receiver / Finder — applications should not
// need to touch this directly.

namespace tcx {
namespace ndi_internal {

// Returns true if NDI runtime is available and was initialized successfully.
bool ensureInitialized();

} // namespace ndi_internal
} // namespace tcx
