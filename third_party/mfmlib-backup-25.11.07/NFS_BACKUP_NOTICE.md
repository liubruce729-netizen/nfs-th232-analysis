# MFMlib Backup Notice

This directory is a source backup of MFMlib for reproducing the NFS Th-232
analysis environment. It is not original code from this repository.

Upstream project:

- Name: MFMlib
- Author listed by upstream README: Luc Legeard, GANIL
- Upstream GitLab: `https://gitlab.in2p3.fr/Ganil-acq/Analysis/MFMlib`

Local provenance:

- Local source path copied from:
  `/home/user0/work/IJCLAB/NFS/pkg/MFMlib`
- Local upstream remote:
  `https://gitlab.in2p3.fr/Ganil-acq/Analysis/MFMlib`
- Local commit:
  `d47063dee86eb96fd6ac1fed7bd2d97339e787c2`
- Local git description:
  `v25.10.23-10-gd47063d`
- Project version string in `sources/CMakeLists.txt`:
  `25.11.07`
- Installed local generated version observed in
  `/home/user0/work/IJCLAB/NFS/install/MFMlib/include/Version.h`:
  `#define MFM_VERSION "25.11.07"`

This backup intentionally excludes local `.git` metadata and the local `build/`
directory. Compiled artifacts such as `libMFM.so`, `libMFM.a`, object files, and
`MFMtest.exe` are not stored here. Rebuild them on the target machine.

## Install On A Server

Example install location:

```bash
export NFS_REPO=$HOME/Analysis/nfs-th232-analysis
export NFS_SW=$HOME/Analysis/nfs_sw
export MFM_SRC=$NFS_REPO/third_party/mfmlib-backup-25.11.07

mkdir -p "$NFS_SW/build/MFMlib" "$NFS_SW/install"
cd "$NFS_SW/build/MFMlib"

cmake -DCMAKE_INSTALL_PREFIX="$NFS_SW/install/MFMlib" "$MFM_SRC/sources"
make -j4
make install
```

If the server does not have `tinyxml`, build without the MFM XML part:

```bash
cmake \
  -DCMAKE_INSTALL_PREFIX="$NFS_SW/install/MFMlib" \
  -DNO_MFMNXML=YES \
  "$MFM_SRC/sources"
make -j4
make install
```

If the server has a custom `tinyxml` location:

```bash
cmake \
  -DCMAKE_INSTALL_PREFIX="$NFS_SW/install/MFMlib" \
  -DMYTINYXMLDIR=/path/to/tinyxml \
  "$MFM_SRC/sources"
make -j4
make install
```

Verify the installed MFMlib version:

```bash
cat "$NFS_SW/install/MFMlib/include/Version.h"
```

Expected output contains:

```c
#define MFM_VERSION  "25.11.07"
```

## Rebuild ADNE Against This MFMlib

If an existing GRU install is kept:

```bash
cd "$NFS_REPO/nfs-th232-analysis-adne"
rm -rf CMakeCache.txt CMakeFiles

cmake \
  -DGRUSYS=/path/to/GRU \
  -DMFMSYS="$NFS_SW/install/MFMlib" \
  .

make -j4
```

Check runtime libraries:

```bash
ldd ./AnalysisADNE | grep -E 'GRU|MFM|not found'
```

If the existing GRU was compiled against an older MFMlib and MFM timestamp
decoding is still inconsistent, rebuild GRU against this MFMlib too, then
rebuild ADNE with the new `GRUSYS` and `MFMSYS` paths.

