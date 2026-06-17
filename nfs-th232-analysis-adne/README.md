# NFS Th-232 ADNE Branch Notice

This directory is based on the public ADNE Data Analysis Framework:
<https://gitlab.in2p3.fr/emmanuel.clement/adne-data-analysis-framework>.

The upstream ADNE project is licensed under the GNU Affero General Public
License v3.0. The verbatim license text from the copied ADNE source is kept in
[`LICENSE`](LICENSE). Additional source and local-modification notes are kept in
[`LICENSE_NOTICE.md`](LICENSE_NOTICE.md).

Local changes in this repository are maintained for the NFS Th-232 / EXOGAM2
analysis workflow, including NFS-specific tree/spectra outputs and ROOT helper
macros under [`lsy_nfs/`](lsy_nfs/).

The original upstream README follows.

---

# ADNE Data Analysis Framework

Data analysis framework based on GRU for GANIL Data in MFM and EBYEDAT. 

The package reads mfm files and spy NARVAL 1.14 GANIL Data flow.

The T**** are the Detector classes. They only depend on the MFMLibs.
For each TDetector, there is first class unpacking and treating the data and a second Class ("Data") used to define the
structure of the output TTree.

The GUser.C is the main Class managing the data process

These 2 structures are IDENTICAL for online spy or replay to root tree

- AnalysisADNEe is the offline program and produce spectra and root tree

./AnalysisADNE help  :: for instructions

- SpyAnalysisADNE online program taking as input the online Data flow

In the directory Utils/, To read the output TTree
- cd Replay
- mkdir build && cd build
- cmake ../
- make -j2
- ./replay_executable


The detectors class have only MFM dependences as the main executables have also GRU dependences, therefore :
The MFM package must be installed from https://gitlab.in2p3.fr/Ganil-acq/Analysis/MFMlib
The GRU package must be installed from https://gitlab.in2p3.fr/Ganil-acq/Analysis/gru

ROOT framework also installed. yaml-cpp is mandatory.

Presently, the ADNE package has been valided up to ROOT 6.30 and Ubuntu22

_Installation_

_The GRUSYS and MFMSYS variables must be known


1. Download git clone git@gitlab.in2p3.fr:emmanuel.clement/adne-data-analysis-framework.git
2. cd adne-data-analysis-framework
3. Input run's directory uses a symbolic link "data" ; output uses a symbolic link "out"
4. All configurations are read from  ~/Yaml_config_files/config.yaml
5. make clean ; rm CMakeCache.txt
6. cmake -DGRUSYS=/home/acqexp/Analysis/GRU/ubu22.04.x86_64/R6_v23.04.21 -DMFMSYS=/home/acqexp/Analysis/MFMlib//ubu22.04.x86_64/v23.04.21/
7. make -j4
8. ./AnalysisADNE to analyse data or ./SpyAnalysisADNE to spy the Narval flow


In the Conf/ directory, a topology manager Apps for exogam2 is proposed; run as
: cd Conf/ ; python3 TopologyManagerExogam.py

Please note that the old compilation method is available in old/ 
Copy the 4 Makefile files to the main directory
Copy the scriptInstall.sh to the main directory
run as ./scriptInstall.sh
Cite as

Clement. (2022). ADNE Data Analysis Framework (1.0). Zenodo. https://doi.org/10.5281/zenodo.7398495

