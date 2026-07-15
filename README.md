# NFS Th-232 Analysis

This repository collects analysis utilities for the NFS Th-232 / Exogam2 workflow.

## Contents

- [`two-step-nfs-th232-python-scripts/`](two-step-nfs-th232-python-scripts/)  
  Python/PyROOT scripts for the two-step workflow:
  1. run ADNE to produce ROOT files;
  2. post-process ADNE ROOT outputs with Python scripts.


- [`nfs-th232-analysis-adne/`](nfs-th232-analysis-adne/)
  NFS Th-232 ADNE branch based on the public ADNE Data Analysis Framework:
  <https://gitlab.in2p3.fr/emmanuel.clement/adne-data-analysis-framework>.
  The original AGPL-3.0 license text is kept in
  [`nfs-th232-analysis-adne/LICENSE`](nfs-th232-analysis-adne/LICENSE), and the
  inheritance/local-modification notice is in
  [`nfs-th232-analysis-adne/LICENSE_NOTICE.md`](nfs-th232-analysis-adne/LICENSE_NOTICE.md).

- [`standalone-nfs-histo/`](standalone-nfs-histo/)
  High-performance ROOT-only reconstruction of every current ADNE
  `nfs_histoExogam2_1.root` histogram from the primitive `MfmFrameTree`. It
  does not link ADNE, GRU, or MFMlib and includes internal count validation.

More analysis steps can be added as separate folders later.
