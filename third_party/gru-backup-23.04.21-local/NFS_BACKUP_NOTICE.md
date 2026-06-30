# GRU Backup Notice

This directory is a source backup of GRU for reproducing the NFS Th-232
analysis environment. It is not original code from this repository.

Upstream project:

- Name: GRU
- Upstream GitLab: `https://gitlab.in2p3.fr/Ganil-acq/Analysis/gru`

Local provenance:

- Local source path copied from:
  `/home/user0/work/IJCLAB/NFS/pkg/gru`
- Local upstream remote:
  `https://gitlab.in2p3.fr/Ganil-acq/Analysis/gru`
- Local commit:
  `64551f358ca7c2a1a08bda14beb02f54ee83dcad`
- Local git description:
  `v23.04.21-2-g64551f3-dirty`
- GRU version macro in `sources/GVersion.h`:
  `#define GRU_VERSION "23.04.21"`

The local source tree had one build-system modification at backup time:

```diff
- Html Spectrum Gui FFTW
+ GuiHtml Spectrum Gui FFTW
```

This change keeps the GRU ROOT component list compatible with the ROOT version
used on the working NFS analysis machine.

This backup intentionally excludes local `.git` metadata and generated build or
install directories, including `build/`, `gsoap/build/`, `gsoap/bin/`,
`gsoap/include/`, `gsoap/lib/`, and the matching `gsoapSC` generated
directories, including `gsoapSC/sources/bin/`. Compiled artifacts such as `libGRU.so`, `libGRU.a`, ROOT PCM files,
and GRU executables are not stored here. Rebuild them on the target machine.

## Install On A Server

Example layout used at the acquisition server:

```bash
export PKG_ROOT=/home/e877_ana/Analysis/pkg_install
export MFM_DIR=$PKG_ROOT/install/MFMlib
export GETMFM_DIR=/path/to/GetMFM
export FDT_LIB=$GETMFM_DIR/lib/libfdt.a
export GRU_SRC=$PKG_ROOT/pkg/gru-backup-23.04.21-local
export GRU_INSTALL=$PKG_ROOT/install/GRU
```

`GETMFM_DIR` must contain `lib/libGetMFMDeviceStatic.a`. The linked GetMFM
static library may also need `lib/libfdt.a`. Find an existing GetMFM install
with:

```bash
find /home/e877_ana/Analysis /home/acqexp -name libGetMFMDeviceStatic.a 2>/dev/null
```

Build and install GRU against the selected MFMlib:

```bash
mkdir -p "$PKG_ROOT/build/GRU"
cd "$PKG_ROOT/build/GRU"

export MFMDIR="$MFM_DIR"
export GETMFMDIR="$GETMFM_DIR"

cmake \
  -DCMAKE_INSTALL_PREFIX="$GRU_INSTALL" \
  -DNET_LIB=NO \
  -DMFMDIR="$MFMDIR" \
  -DGETMFMDIR="$GETMFMDIR" \
  -DFDT_LIBRARIES="$FDT_LIB" \
  "$GRU_SRC/sources"

make -j4
make install
```

If `libfdt.a` is not present or the executable link step reports undefined
references to `fdtTransmitter` or `fdtReceiver`, rebuild or locate the matching
GetMFM/FDT installation and set `FDT_LIB` to its `libfdt.a`.

For ROOT 6, copy generated PCM files if they are not installed automatically:

```bash
cp "$PKG_ROOT/build/GRU"/*.pcm "$GRU_INSTALL/lib/" 2>/dev/null || true
cp "$PKG_ROOT/build/GRU"/*.pcm "$GRU_INSTALL/bin/" 2>/dev/null || true
```

Verify that GRU was built against the new MFMlib rather than an older MFMlib
whose symbols were statically embedded in `libGRU.so`:

```bash
cat "$MFM_DIR/include/Version.h"
nm -C "$GRU_INSTALL/lib/libGRU.so" | grep 'MFMCommonFrame::GetTimeStampFromCommonFrameData' | head
nm -C "$GRU_INSTALL/lib/libGRU.so" | grep 'MFMCommonFrame::GetTimeStamp()' | head
```

For the intended NFS environment, `Version.h` should contain MFMlib `25.11.07`.
The old `GetTimeStampFromCommonFrameData` symbol should disappear when GRU is
linked against the new MFMlib.
