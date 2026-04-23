# tcxNdi

NDI (Network Device Interface) send/receive addon for [TrussC](http://github.com/trussc-org/trussc).

NDI is a low-latency video-over-IP protocol from Vizrt (formerly NewTek), widely used in broadcast, AV installations, and live streaming.

## Features

- **Send** — broadcast a TrussC app's pixels onto the NDI network
- **Receive** — pull frames from any NDI source on the network into a `tc::Pixels`
- **Find** — discover NDI sources currently advertising on the network

## Installing the NDI SDK

The NDI SDK is **not redistributable**, so you must install it yourself. Download from https://ndi.video/sdk/ (free, registration required).

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

NDI on Linux relies on the **avahi-daemon** (mDNS service) for network discovery — without it other machines cannot find your sender. Make sure it is installed and running:

```
sudo apt install avahi-daemon libnss-mdns       # Debian / Raspbian
sudo pacman -S avahi nss-mdns                    # Arch
sudo systemctl enable --now avahi-daemon
```

#### Arch Linux (and EndeavourOS / Manjaro)

Install the SDK from the AUR:

```
yay -S ndi-sdk
```

If receivers on other machines can't find the sender or see the name but never get frames, see [Troubleshooting](#troubleshooting).


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

See `example-ndi-sender/` and `example-ndi-receiver/` for runnable samples.

## Troubleshooting

### Linux: receivers see the source name but no frames arrive

Discovery is fine but the TCP video stream is being blocked or advertised on the wrong interface.

**1. Open NDI's ports in `firewalld`.** EndeavourOS ships it enabled by default; skip if `systemctl is-active firewalld` says `inactive`.

```
sudo firewall-cmd --zone=public --add-port=5353/udp --permanent
sudo firewall-cmd --zone=public --add-port=5960-5990/tcp --permanent
sudo firewall-cmd --zone=public --add-port=5960-5990/udp --permanent
sudo firewall-cmd --reload
```

**2. Multi-NIC machines (Tailscale, Docker, VPNs)** — pin NDI to the LAN IP in `~/.ndi/ndi-config.v1.json`. Restart the sender after editing; the NDI runtime reads this config at init.

```
{
  "ndi": {
    "adapters":  { "allowed": ["192.168.x.x"] },
    "multicast": { "send": { "enable": false } }
  }
}
```

### Linux: sender only visible on the local machine (plain Arch)

Arch ships `systemd-resolved` bound to UDP 5353, the same port `avahi` needs for mDNS. When both run, avahi ends up publishing only on `lo` and other machines can't see the source. Hand 5353 to avahi:

```
sudo mkdir -p /etc/systemd/resolved.conf.d
sudo tee /etc/systemd/resolved.conf.d/no-mdns.conf > /dev/null <<'EOF'
[Resolve]
MulticastDNS=no
EOF
sudo systemctl restart systemd-resolved avahi-daemon
```

Confirm the source now advertises on your LAN NIC (not just `lo`):

```
avahi-browse _ndi._tcp -r -t
```

## Tested

- [x] Linux (EndeavourOS / Arch)
- [ ] Linux (Ubuntu / Debian)
- [ ] Linux (Raspbian)
- [ ] Windows
- [ ] macOS

## Credits

API surface inspired by [ofxNDI](https://github.com/leadedge/ofxNDI).
