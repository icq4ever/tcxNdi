# tcxNdi

NDI (Network Device Interface) send/receive addon for TrussC.

NDI is a low-latency video-over-IP protocol from Vizrt (formerly NewTek),
widely used in broadcast, AV installations, and live streaming.

## Features

- **Send** — broadcast a TrussC app's pixels onto the NDI network
- **Receive** — pull frames from any NDI source on the network into a `tc::Pixels`
- **Find** — discover NDI sources currently advertising on the network

## Installing the NDI SDK

The NDI SDK is **not redistributable**, so you must install it yourself.
Download from https://ndi.video/sdk/ (free, registration required).

### macOS

Install the `.pkg` from the SDK download. Headers and library land at:

```
/Library/NDI SDK for Apple/include/
/Library/NDI SDK for Apple/lib/macOS/
```

### Windows

Run the installer. Default install location:

```
C:\Program Files\NDI\NDI 6 SDK\Include\
C:\Program Files\NDI\NDI 6 SDK\Lib\x64\
```

(The exact `NDI 6 SDK` part follows the SDK version you downloaded.)

### Linux

Run the SDK installer script. Default install location:

```
/usr/include/ndi/
/usr/lib/x86_64-linux-gnu/libndi.so.*       # x86_64
/usr/lib/aarch64-linux-gnu/libndi.so.*      # arm64 (Raspberry Pi etc.)
```

NDI on Linux relies on the **avahi-daemon** (mDNS service) for network
discovery — without it other machines cannot find your sender. Make
sure it is installed and running:

```
sudo apt install avahi-daemon libnss-mdns       # Debian / Raspbian
sudo pacman -S avahi nss-mdns                    # Arch
sudo systemctl enable --now avahi-daemon
```

#### Arch Linux

Install the SDK from the AUR:

```
yay -S ndi-sdk
```

**Arch-specific gotcha — avahi vs systemd-resolved conflict.** Arch ships with `systemd-resolved` enabled, and by default it also binds UDP 5353 for its own mDNS stack. When you additionally install `avahi`, both daemons try to share the port and `avahi` ends up publishing only on `lo` — so other machines on your network can't find the sender even
though it looks fine locally.

Fix by telling `systemd-resolved` to leave mDNS to `avahi`:

```
sudo mkdir -p /etc/systemd/resolved.conf.d
sudo tee /etc/systemd/resolved.conf.d/no-mdns.conf > /dev/null <<'EOF'
[Resolve]
MulticastDNS=no
EOF

sudo systemctl restart systemd-resolved
sudo systemctl restart avahi-daemon
```

Verify only `avahi-daemon` is on UDP 5353 afterwards:

```
sudo ss -tulnp | grep 5353
```

Then restart the sender and confirm it advertises on your LAN interface
(not just `lo`):

```
avahi-browse _ndi._tcp -r -t
```


## Usage

Add the addon to your project's `addons.make`:

```
tcxNdi
```

### Send

```cpp
#include "tcxNdi.h"

tcx::NdiSender sender;

void tcApp::setup() {
    sender.setup("My TrussC App");   // name visible to NDI receivers
}

void tcApp::draw() {
    // ... draw your scene ...
    sender.send(getPixels());        // ship the frame to the NDI network
}
```

### Receive

```cpp
#include "tcxNdi.h"

tcx::NdiFinder   finder;
tcx::NdiReceiver receiver;

void tcApp::setup() {
    finder.refresh();
    auto sources = finder.getSources();
    if (!sources.empty()) {
        receiver.connect(sources[0]);
    }
}

void tcApp::update() {
    if (receiver.update()) {
        // new frame arrived; receiver.getPixels() now valid
    }
}

void tcApp::draw() {
    if (receiver.hasFrame()) {
        receiver.getPixels().draw(0, 0);
    }
}
```

See `examples/ndiSenderExample/` and `examples/ndiReceiverExample/` for
runnable samples.

## Credits

API surface inspired by [ofxNDI](https://github.com/leadedge/ofxNDI).
