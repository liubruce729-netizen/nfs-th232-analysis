#ifndef _MFM_ALL_TYPES_
#define _MFM_ALL_TYPES_
#include "MFMFewDefines.h"


//value of shift, Event Number, TimeStamps , location(bord number and channel)

#define NUMEXO_EN_SHI   MFM_BLOB_HEADER_SIZE /// SHI = Shift
#define NUMEXO_TS_SHI   MFM_BLOB_HEADER_SIZE + 4
#define NUMEXO_LO_SHI   MFM_BLOB_HEADER_SIZE + 4 + 6 // LO = location
#define NUMEXO_LO_SHI2  MFM_BLOB_HEADER_SIZE
#define NUMEXO_EN_SHI_INV   MFM_BLOB_HEADER_SIZE + 6 // INV = inverse
#define NUMEXO_TS_SHI_INV   MFM_BLOB_HEADER_SIZE

#define NO_TS           0
#define NO_EN           0 
#define NO_LO           0 
#define BAS_EN_SHI      MFM_BASIC_DEFAULT_HEADERSIZE// Basic shift of event number.
#define BAS_TS_SHI      MFM_BASIC_DEFAULT_HEADERSIZE + 4
#define BAS_LO_SHI      MFM_BASIC_DEFAULT_HEADERSIZE + 4 + 6// Basic shift location
#define BAS_EN_SHI_INV  MFM_BASIC_DEFAULT_HEADERSIZE + 6
#define BAS_TS_SHI_INV  MFM_BASIC_DEFAULT_HEADERSIZE

#define BAS_EN_SHI2     BAS_EN_SHI + 2
#define BAS_TS_SHI2     BAS_TS_SHI + 2 
#define BAS_LO_SHI2     MFM_BASIC_DEFAULT_HEADERSIZE 
#define FAS_LO_SHI      8  // Faster location shift

#define MFM_MIN_TYPE    0x01

//       Type Name             Type code  Information             Shift :   Event Number      |TimeStamps       |location       |Inherits from NumExoFrame
#define MFM_COMMON_FRAME_TYPE     0x00 /// Common frame                     NO_EN             NO_TS             NO_LO            0
#define MFM_COBO_FRAME_TYPE       0x01 /// Cobo card frame                  BAS_EN_SHI_INV    BAS_TS_SHI_INV    BAS_LO_SHI       0
#define MFM_COBOF_FRAME_TYPE      0x02 /// Cobo card frame full signals     BAS_EN_SHI_INV    BAS_TS_SHI_INV    BAS_LO_SHI       0
#define MFM_COBOT_FRAME_TYPE      0x07 /// Cobo topology frame              NO_EN             NO_TS             NO_LO            0
#define MFM_MUTANT_FRAME_TYPE     0x08 /// Mutant Data Frame                NUMEXO_EN_SHI_INV NUMEXO_TS_SHI_INV NO_LO            0
#define MFM_MUTANT1_FRAME_TYPE    0x09 /// Mutant Data Frame Reserved       NUMEXO_EN_SHI_INV NUMEXO_TS_SHI_INV NO_LO 	         0
#define MFM_MUTANT2_FRAME_TYPE    0x0A /// Mutant Data Frame Reserved       NUMEXO_EN_SHI_INV NUMEXO_TS_SHI_INV NO_LO 	         0
#define MFM_MUTANT3_FRAME_TYPE    0x0B /// Mutant Data Frame Reserved       NUMEXO_EN_SHI_INV NUMEXO_TS_SHI_INV NO_LO 	         0

#define MFM_EXO2_FRAME_TYPE       0x10 /// numexo2 card frame               NUMEXO_EN_SHI     NUMEXO_TS_SHI      NUMEXO_LO_SHI   0
#define MFM_OSCI_FRAME_TYPE       0x11 /// Oscillo data frame use in Numo2  NO_EN             NO_TS    	         BAS_LO_SHI2     0
#define MFM_NEDA_FRAME_TYPE       0x12 /// Raw data frame use in NEDA       BAS_EN_SHI2       BAS_TS_SHI2        BAS_LO_SHI2     0 
#define MFM_NEDACOMP_FRAME_TYPE   0x13 /// Compressed  frame used inNEDA    NUMEXO_EN_SHI     NUMEXO_TS_SHI      NUMEXO_LO_SHI   0 
#define MFM_VAMOSIC_FRAME_TYPE    0x14 /// Vamos Ionization Chamber Frame   NUMEXO_EN_SHI     NUMEXO_TS_SHI      NUMEXO_LO_SHI   1
#define MFM_VAMOSPD_FRAME_TYPE    0x15 /// Vamos Position Detector Frame    NUMEXO_EN_SHI     NUMEXO_TS_SHI      NUMEXO_LO_SHI   0
#define MFM_DIAMANT_FRAME_TYPE    0x16 /// Diamant Frame                    NUMEXO_EN_SHI     NUMEXO_TS_SHI      NUMEXO_LO_SHI   1
#define MFM_S3_BAF2_FRAME_TYPE    0x17 /// for detector S3 Baf2 Frame       NUMEXO_EN_SHI     NUMEXO_TS_SHI      NUMEXO_LO_SHI   1
#define MFM_S3_ALPHA_FRAME_TYPE   0x18 /// for detector S3 Alpha Frame      NUMEXO_EN_SHI     NUMEXO_TS_SHI      NUMEXO_LO_SHI   1
#define MFM_S3_RUTH_FRAME_TYPE    0x19 /// for detector S3 Rutherford Frame NUMEXO_EN_SHI     NUMEXO_TS_SHI      NUMEXO_LO_SHI   1
#define MFM_S3_EGUN_FRAME_TYPE    0x1A /// for detector S3 eGUN             NUMEXO_EN_SHI     NUMEXO_TS_SHI      NUMEXO_LO_SHI   1
#define MFM_S3_SYNC_FRAME_TYPE    0x1B /// for detector S3 Synchro          NUMEXO_EN_SHI     NUMEXO_TS_SHI      NUMEXO_LO_SHI   1
#define MFM_REA_GENE_FRAME_TYPE   0x1C /// Generic Rea Frame                NUMEXO_EN_SHI     NUMEXO_TS_SHI      NUMEXO_LO_SHI   1
#define MFM_VAMOSTAC_FRAME_TYPE   0x1D /// Vamos Time  Frame                NUMEXO_EN_SHI     NUMEXO_TS_SHI      NUMEXO_LO_SHI   1
#define MFM_BOX_DIAG_FRAME_TYPE   0x1E /// Box Diagnostic                   NUMEXO_EN_SHI     NUMEXO_TS_SHI      NUMEXO_LO_SHI   1

#define MFM_EBY_EN_FRAME_TYPE     0x20 /// Ganil data frame with EN         BAS_EN_SHI	      NO_TS	         NO_LO           0
#define MFM_EBY_TS_FRAME_TYPE     0x21 /// Ganil data frame with TS         NO_EN	      BAS_TS_SHI_INV     NO_LO	         0
#define MFM_EBY_EN_TS_FRAME_TYPE  0x22 /// Ganil data frame with TS & EN    BAS_EN_SHI	      BAS_TS_SHI	 NO_LO           0
#define MFM_MATACQ_FRAME_TYPE     0x23 /// Mataq card frame                 no used 
#define MFM_SCALER_DATA_FRAME_TYPE 0x24 /// Ganil Scaler data frame         BAS_EN_SHI_INV    BAS_TS_SHI_INV     NO_LO           0    

#define MFM_RIBF_DATA_FRAME_TYPE  0x30 /// RIBF transport frame             NUMEXO_EN_SHI_INV NUMEXO_TS_SHI_INV  NO_LO	         0
#define MFM_FAZIA_DATA_FRAME_TYPE 0x40 /// FAZIA transport frame            NUMEXO_EN_SHI_INV NUMEXO_TS_SHI_INV  NO_LO	         0        
#define MFM_FASTER_FRAME_TYPE     0xA0 /// FASTER transport frame           NUMEXO_EN_SHI_INV NUMEXO_TS_SHI_INV  FAS_LO_SHI      0   
#define MFM_FASTERDTS_FRAME_TYPE  0xA1 /// FASTER Delta TS transport frame  NUMEXO_EN_SHI_INV NUMEXO_TS_SHI_INV  NO_LO  	 0  

#define MFM_CHIMERA_DATA_FRAME_TYPE 0x60 /// CHIMERA transport frame        NUMEXO_TS_SHI_INV  NUMEXO_TS_SHI     NO_LO	         0
#define MFM_SIRIUS_FRAME_TYPE       0x70 /// Sirius data frame              BAS_EN_SHI2        BAS_TS_SHI2       NUMEXO_LO_SHI2  0
#define MFM_REA_TRACE_FRAME_TYPE    0x71 /// Trace  Rea Frame               BAS_EN_SHI2        BAS_TS_SHI2       BAS_LO_SHI      0
#define MFM_S3_DEFLECTOR_FRAME_TYPE 0x80 /// Deflector frame                NUMEXO_EN_SHI      NUMEXO_TS_SHI     NUMEXO_LO_SHI   1
#define MFM_PARIS_FRAME_TYPE        0x90 /// PARIS     frame                NUMEXO_EN_SHI      NUMEXO_TS_SHI     NUMEXO_LO_SHI   1

#define MFM_MESYTEC_FRAME_TYPE   0x4ADF ///  MESYTEC transport frame        NUMEXO_EN_SHI_INV  NUMEXO_TS_SHI_INV NO_LO	         0

#define MFM_HELLO_FRAME_TYPE     0xFF00  /// Hello Frame                    NUMEXO_EN_SHI_INV  NUMEXO_TS_SHI_INV NO_LO	         0
#define MFM_MERGE_EN_FRAME_TYPE  0xFF01  /// Merge frame in envent number   BAS_EN_SHI	       NO_TS	         NO_LO           0
#define MFM_MERGE_TS_FRAME_TYPE  0xFF02  /// Merge frame in time stamp      NO_EN	       BAS_TS_SHI_INV	 NO_LO           0

#define MFM_XML_DATA_DESCRIPTION_FRAME_TYPE 0xFF10  /// Description data    NO_EN              NO_TS             NO_LO           0
#define MFM_XML_FILE_HEADER_FRAME_TYPE      0xFF11  /// Run description     NO_EN              NO_TS             NO_LO           0
#define MFM_MAX_TYPE  			 0xFF11

#endif
    
