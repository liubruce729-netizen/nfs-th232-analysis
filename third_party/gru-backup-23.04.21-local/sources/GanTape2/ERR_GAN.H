  /*     CODES ERREURS DETECTES ........Appel en C	26 nov 97
        du type $XXYY ou XX est la classe et YY le code   
     ATTENTION: Toute ligne de definition de classe doit imperativement
                commencer par la sequence: "/" suivi de "* CLASSE $".
       de plus: Toute ligne de definition d'erreur doit commencer par
                "#define" et le numero sera defini par la classe + le
                numero de code (ex: "#define ERR CLASSE + COD_ERR")
 
     En consequence de quoi,le message situe sur la ligne de definition
     d'erreur sera imprime par la fonction g_exit(pour abort),ainsi
     que par la fonction g_err,et sa commande associee G_ERR

     NB: les codes PDOS (< 256) doivent etre donnes en decimal
  */

#ifndef __ERR_GAN_DEF
#define __ERR_GAN_DEF 1
 
/* CLASSE $0000  ===> OK et codes PDOS */
#ifdef PDOS
#define R_OK 0
#endif

#ifdef __vms
#include ssdef
#define GR_OK SS$_NORMAL
#else
#define GR_OK 0
#endif

#define R_PDOS	GR_OK
#define R_BOOT	GR_OK + 1 /* to store the boot time */
#define R_START GR_OK + 2 /* to store time of beginning of an application */
#define R_PARAM R_PDOS + 67 /* PDOS parameter error */
#define ER_ABORT R_PDOS + 85 /* PDOS Aborted Task */
#define R_FILNF R_PDOS + 53 /* FILE NOT FOUND */ 

/* CLASSE $0100  ===> TASK MANAGEMENT */
#define R_TSK 0x0100
#define R_TSK_1 R_TSK + 1 /* double task name */
#define R_TSK_2 R_TSK + 2 /* not enough memory */
#define R_TSK_3 R_TSK + 3 /* error in XTCB call */	
#define R_TSK_4 R_TSK + 4 /* This task does not exist */
#define R_TSK_5 R_TSK + 5 /* Your task isn't task "SYST" */
#define R_TSK_6 R_TSK + 6 /* Son task not created */

/* CLASSE $0200  ===> SEMAPHORE */
#define R_SEM 0x0200 
#define R_SEM_1 R_SEM + 1   /* This semaphore was taken after time_out */
#define R_SEMBUSY R_SEM + 2 /* This semaphore is yet busy after timeout */
#define R_SEMTASK R_SEM + 3 /* This semaphore has been taken by another task */


/* CLASSE $0300  ===> FACTEUR ET MESSAGERIE */
#define R_FAC 0x0300
#define R_FAC_1 R_FAC + 1 /* the "FACT" task is inactive */
#define R_FAC_2 R_FAC + 2 /* transmission error */
#define R_FAC_3 R_FAC + 3 /* destination task is inactive */
#define R_FAC_4 R_FAC + 4 /* time-out (probably "FACT" inactive on dest cpu) */
#define R_FAC_5 R_FAC + 5 /* unexpected status found (???) */
#define R_FAC_6 R_FAC + 6 /* time-out on acknowledge of answer */
#define R_FAC_7 R_FAC + 7 /* unexpected local message (???) */
#define R_FAC_8 R_FAC + 8 /* unexpected destination CPU */
#define R_FAC_9 R_FAC + 9 /* unexpected GANIL protocol code found */

/* CLASSE $400  ===> CONFIGURATION SYSTEME */
#define R_CONF 0x0400
#define R_UNKCPU R_CONF + 1 /* Unknown CPU in configuration table */
#define R_UNKPFG R_CONF + 2 /* Unkn. global memory for PF's in equip. table */
#define R_UNKDET R_CONF + 3 /* Unknown detector in configuration table */
#define R_GMAIL  R_CONF + 4 /* GMAIL already initialized */
#define R_MAILSZ R_CONF + 5 /* GMAIL area too small at initialization */
#define R_INTERF R_CONF + 6 /* Unknown acquisition interface */
#define R_EQUFIL R_CONF + 7 /* Error when reading equipment file */
#define R_LENCPU R_CONF + 8 /* CPU name too long */
#define R_MXEXT  R_CONF + 9 /* error when loading MXEXT */
#define R_UNKEQ	 R_CONF + 10 /* unknown equipmemt in equipment table */
#define R_BADCPU R_CONF + 11 /* Not expected prompt */
#define R_NOPRPT R_CONF + 12 /* No prompt */
#define R_BTESC1 R_CONF + 13 /* CPU doesn't stop to send char. after ESC */
#define R_BTESC2 R_CONF + 14 /* CPU doesn't answer after ESC */
#define R_BTCTRK R_CONF + 15 /* CPU doesn't stop to send char. after CTRL_K */
#define R_BTFAIL R_CONF + 16 /* Boot failed */
#define R_BTID   R_CONF + 17 /* No PDOS ID at boot time */
#define R_BDPRPT R_CONF + 18 /* Bad prompt  */
#define R_REBOOT R_CONF + 19 /* CPU is under PDOS, no reboot */
#define R_PORT	 R_CONF + 20 /* port already used by another task */
#define R_MAILGET R_CONF + 21 /* not enough memory to get asked mail */
#define R_NOCBD	 R_CONF + 22  /* no CBD for this CPU, use CAMAC window (F7) */
#define R_NOCAM  R_CONF + 23  /* application type of this CPU is not CAM */
#define R_SYSCBD R_CONF + 24  /* no CAMAC branch driver on this system */
#define R_CAMOFF R_CONF + 25  /* CAMAC crate offline */
#define R_OFFCBD R_CONF + 26  /* CBD offline or bad crate number */
#define R_UNKSERV R_CONF + 27 /* Unknown server */
#define R_FILSERV R_CONF + 28 /* Servor file SERVCC.DAT open error */
#define R_SRVTBL R_CONF + 29 /* Servor table not initialized in GMAIL */
#define R_CAMALLOFF R_CONF + 30 /* All CAMAC crates offline */
#define R_UNKSTG R_CONF + 31 /* Global Status Memory not in equipment table */
#define R_CAMPURGE R_CONF + 32 /* CAMAC event FIFO has been purged */

/* CLASSE $500  ===> ACQUISITION */
#define R_ACQ 0x500
#define R_ACQOK	R_ACQ + 0  /* Acquisition initialized */
#define R_PFGEVT R_ACQ + 1 /* PF global mem too small for asked event length */
#define R_BUFSIZ R_ACQ + 2 /* memory size too small for asked buffers */
#define R_PFGINT R_ACQ + 3 /* PF global mem too small for interface table */
#define R_ACQFAM R_ACQ + 4 /* equipment family not treated by acquisition */
#define R_ACQCMD R_ACQ + 5 /* acquisition error command */
#define R_XBUFCPU R_ACQ + 6 /* init croisee de buffers interdit sur ce CPU */
#define R_EVLEN	 R_ACQ + 7 /* event length too large */
#define R_EVNB	 R_ACQ + 8 /* evt nb decorrelation between PF and interfaces*/
#define R_TBUFSIZ R_ACQ + 9 /* incoherent buffers lengths in MEV boards */
#define R_DMPTO	 R_ACQ + 10 /* event dump timeout, no event has been dumped */
#define R_DMPSIZ R_ACQ + 11 /* dump buffer size < evt length, dump troncated */
#define R_DMPSYS R_ACQ + 12 /* system dump buf size < evt length */
#define R_PFOFF	 R_ACQ + 13 /* all PF's processors inactive */
#define R_NOPF	 R_ACQ + 14 /* no PF processor in equipment table */
#define R_ASCUPF R_ACQ + 15 /* no ASCU found for PF chassis in equip. table */
#define R_PFINTF R_ACQ + 16 /* not enough mem. for interf. table in acq. tsk */
#define R_PFNOEVT R_ACQ + 17 /* no evt found after evt interrupt on PF */
#define R_IFVVIDE R_ACQ + 18 /* error on empty status of IFV */
#define R_MAXARG R_ACQ + 19 /* two many arguments */
#define R_MISARG R_ACQ + 20 /* missing argument */
#define R_NOCMD  R_ACQ + 21 /* command not yet implemented */
#define R_PARFIL R_ACQ + 22 /* acq parameter file cannot be openned */
#define R_STRFIL R_ACQ + 23 /* structure file cannot be openned */
#define R_FILTFIL R_ACQ + 24 /* filter file cannot be openned */
#define R_ACQINIT R_ACQ + 25 /* acquisition not initialized */
#define R_ACQOPT R_ACQ + 26 /* bad acquisition option */
#define R_ACQPAR R_ACQ + 27 /* cmd not in acq paramater file */
#define R_ACQEXE R_ACQ + 28 /* error during acquisition command */
#define R_CMDILLG R_ACQ + 29 /* acquisition initiated from another task */
#define R_CPUACQ R_ACQ + 30 /* cpu not in acquisition in PFGLOB */
#define R_CPLACQ R_ACQ + 31 /* interface not in acquisition in PFGLOB */
#define R_BUFEVT R_ACQ + 32 /* incoherent buffer */
#define R_EVCTFORM R_ACQ + 33  /* unknown control event format */
#define R_ACQFILE R_ACQ + 34 /* Acquisition file not found */
#define R_ACQSTOP R_ACQ + 35 /* Acquisition is stopping (FIFO's not empty) */
#define R_INSTALL R_ACQ + 36 /* type INS_ERR on VME for details */
#define R_ENDCHIM R_ACQ + 37 /* CHIMERA : bad end of block */

/* CLASSE $600  ===> MISCELLANEOUS  */
#define R_MISC 0x600
#define R_FIFOFULL R_MISC + 1  /* FIFO is full */
#define R_FIFOEMPT R_MISC + 2  /* FIFO is empty */
#define R_VICPAGE R_MISC + 3   /* Mauvais numero de page pour VIC8250 */
#define R_DTACK R_MISC + 4     /* Pas de DTACK, mauvaise adresse VME */
#define R_MALLOC R_MISC + 5    /* Probleme d'allocation memoire */
#define R_REPONSE R_MISC + 6   /* Buffer reponse trop petit */
#define R_FERCREATE R_MISC + 7 /* erreur de creation fichier d'erreur */
#define R_FEROPEN R_MISC + 8   /* erreur d'ouverture fichier d'erreur*/ 
#define R_FERPOS R_MISC + 9    /* erreur de positionnement fichier d'erreur */
#define R_FERLEN R_MISC + 10   /* Fichier d'erreur trop petit pour message */
#define R_FERDEFINE R_MISC + 11 /* Fichier d'erreur deja defini */
#define R_BADOPT R_MISC + 12   /* Bad option */
#define R_CMDVIDE R_MISC + 13  /* Commande vide */
#define R_REPFULL R_MISC + 14  /* Buffer reponse plein */
#define R_UNKCMD R_MISC + 15   /* Unknown command */
#define R_UNIMPCMD R_MISC + 16 /* Unimplemented command */
#define R_AMBXFREE R_MISC + 17 /* AmbxSend : Mailbox deja occupee */
#define R_AMBXACK R_MISC + 18  /* AmbxSend : pas d'acknowledge */
#define R_AMBXTO  R_MISC + 19  /* AmbxWait : timeout sur execution commande */
#define R_AMBX_RUN R_MISC + 20 /* AmbxWait: Acq already running on start cmd */
#define R_AMBX_STOP R_MISC + 21 /* AmbxWait: Acq already stopped on stop cmd */
#define R_AMBX_INIT R_MISC + 22 /* AmbxWait: Acquisition not initialized */
#define R_AMBX_CMD R_MISC + 23 /* AmbxWait: Unknown command */
#define R_AMBXRET R_MISC + 24 /* AmbxWait: Unknown return code */
#define R_AMBX_NOCMD R_MISC + 25 /* AmbxWait: No command */
#define R_AMBX_UNK R_MISC + 26 /* Acquisition mailbox unknown in equip table */

/* CLASSE $700  ===> SERVEUR ACQUISITION */
#define R_SERVACQ 0x700
#define R_NOACQTSK R_SERVACQ + 1 /* pas de tache d'acq. (g_mail->m_acqvar=0) */
#define R_ACQEXIT  R_SERVACQ + 2 /* tache acq. disparue (g_mail->m_acqvar=0) */
#define R_ACQNSTOP R_SERVACQ + 3 /* STOP non pris en compte par tache d'acq. */
#define R_ACQRUN   R_SERVACQ + 4 /* tache d'acquisition en run */
#define R_ACQNOGO  R_SERVACQ + 5 /* GO non pris en compte par tache d'acq. */
#define R_ACQNEXI  R_SERVACQ + 6 /* EXIT non pris en compte par tache d'acq */
#define R_ACQSST   R_SERVACQ + 7 /* erreur commande set bits options acq */
#define R_ACQRST   R_SERVACQ + 8 /* erreur commande reset bits options acq */
#define R_ACQNORUN R_SERVACQ + 9 /* l'acquisition n'est pas en run */
#define R_BUFTYP   R_SERVACQ + 10 /* Type de buffer inconnu */
#define R_ACQABORT R_SERVACQ + 11 /* tache d'acquisition disparue */
#define R_MEXTDRIVER R_SERVACQ + 12 /* MEMEXT : Type de driver inconnu */
#define R_UNKMEXT  R_SERVACQ + 13 /* MEMEXT : inconnu dans table equipements */
#define R_MEXTSIZ  R_SERVACQ +14  /* MEMEXT : memoire trop petite */

/* CLASSE $800  ===> ERREURS LIEES AUX CPU'S EQUIPES DE FGA_002 */
#define R_FGACPU 0x800
#define R_BADSLOT  R_FGACPU + 1	/* mauvais numero de slot */
#define R_NOFGA	   R_FGACPU + 2 /* CPU non equipe de FGA_002 */

/* CLASSE $900  ===> ERREURS DE COMMUNICATION AVEC MACHINE HOTE */
#define R_TRANSM 0x900
#define R_FICALL   R_TRANSM + 1 /* pas de reponse du FIC */
#define R_FIC_LEN  R_TRANSM + 2 /* mauvaise longueur transmise par FIC */
#define R_FIC_ES   R_TRANSM + 3 /* err. d'E/S pendant envoi de data par FIC */
#define R_SBFIFO   R_TRANSM + 4 /* FIFO serv. Eth. pleine */
#define R_ETH_LEN  R_TRANSM + 5 /* mauvaise long. transmise par serv. Eth. */
#define R_ETH_ES   R_TRANSM + 6 /* err. d'E/S pendant envoi de data par Eth. */
#define R_NODLFIC  R_TRANSM + 7 /* No ODL_FIC in this VME crate */
#define R_BS_SIZE  R_TRANSM + 8 /* BufStk: buf size incoherent with acq. */
#define R_BSMAXSIZ  R_TRANSM + 9 /* BufStk: buffer size too large */
#define R_BS_WAY   R_TRANSM + 10 /* BufStk: bad way for buffers transmission */
#define R_BS_FLAG  R_TRANSM + 11 /* BufStk: bad flag for buffer transmission */
#define R_BS_MEM   R_TRANSM + 12 /* BufStk: not enough memory for buffers */
#define R_BS_OPT   R_TRANSM + 13 /* BufStk: bad option value */
#define R_BS_BUF   R_TRANSM + 14 /* BufStk: no more buffer available */
#define R_BS_UNKN  R_TRANSM + 15 /* BufStk: Buffer not present in this stack */
#define R_BS_FIC   R_TRANSM + 16 /* BufStk: Bad answer from FIC_ODL */
#define R_BS_SEND  R_TRANSM + 17 /* BufStk: Sending stack not empty when stop*/

/* CLASSE $1000  ===>   ACQUISITION CAMAC       */
#define R_ACCA 0x1000
#define R_CAMFILE R_ACCA + 1 /* CAMAC file not found */
#define R_ALLOC   R_ACCA + 2 /* Pb allocation memoire           */
			     /* 0 : tete de liste LAM           */
			     /* 1 : noeud LAM		        */
			     /* 2 : membre LAM			*/
 	                     /*					*/
#define R_CAMAC   R_ACCA + 3 /* erreur CAMAC			*/
			     /* en sortie status + adresse      */
#define R_ERRLAM  R_ACCA + 4 /* erreur de recuperation de LAM   */
#define R_TABCAM  R_ACCA + 5 /* table fcts CAMAC trop petite dans ini_cam */
#define R_ALARM   R_ACCA + 6 /* execution d'un LAM ALARM (cf fenetre CAMAC) */
#define R_ALARMST R_ACCA + 7 /* arret sur LAM ALARM (cf fenetre CAMAC) */
#define R_CAMFORM R_ACCA + 8 /* mauvais format de fichier CAMAC */
#define R_CAMREV  R_ACCA + 9 /* rev. prog et fichier CAMAC incompatibles */
#define R_CAMPHAS R_ACCA + 10 /* manque phase dans fichier CAMAC */
#define R_ASKCAM  R_ACCA + 11 /* il y a deja une demande de ressource CAMAC */
#define R_RUNCAM  R_ACCA + 12 /* la ressource CAMAC ne se libere pas */
#define R_CAMCRATE R_ACCA + 13 /* chassis CAMAC hors tension */
#define R_LAMPHASE R_ACCA + 14 /* Phase LAM non implementee hors acquisition */
#define R_CAMLIST R_ACCA + 15  /* Liste CAMAC non trouve */
#define R_CAMNONUM R_ACCA + 16 /* CAMAC : argument non numerique */
#define R_CAMCMDARG R_ACCA + 17 /* CAMAC : mauvais argument */
#define R_CAMOPT  R_ACCA + 18  /* CAMAC : erreur d'option */
#define R_CAMDATA R_ACCA + 19 /* CAMAC : Fonction d'ecriture, manque data */
#define R_CAMNBRANCH R_ACCA + 20 /* CAMAC : Mauvais numero de branche */
#define R_CAMNCRATE R_ACCA + 21 /* CAMAC : Mauvais numero de chassis */
#define R_CAMNSLOT R_ACCA + 22 /* CAMAC : Mauvais numero de station */
#define R_CAMNADDR R_ACCA + 23 /* CAMAC : Mauvais numero de sous_adresse */
#define R_CAMNFUNC R_ACCA + 24 /* CAMAC : Mauvais numero de fonction */
#define R_CAMLCMD R_ACCA + 25  /* CAMAC : Commande trop longue (80 car. max) */
#define R_CAMNOX R_ACCA + 26   /* CAMAC : pas de reponse X */
#define R_CAMNOQ R_ACCA + 27   /* CAMAC : pas de reponse Q */
#define R_CAMSRV R_ACCA + 28   /* CAMSERV : Au moins une erreur */

/* CLASSE $1100 ===>   CONTROLE EVENEMENTS   */
#define R_CTREVT 0x1100
#define R_SEVFIL R_CTREVT + 1  /* probleme d'acces au fichier structure evt */
#define R_SEVMEM R_CTREVT + 2  /* memoire structure evenement trop petite */
#define R_SEVCODE R_CTREVT + 3 /* code inconnu dans structure evenement */
#define R_SEVBAD R_CTREVT + 4  /* struct evt incompatible avec sa longueur */
#define R_SEVID R_CTREVT + 5   /* id coupleur different ds struct et ds evt */
#define R_CTEVSIZ R_CTREVT + 6 /* control event buffer too small */
#define R_SEVEOF R_CTREVT + 7  /* unwaited end of file in evt structure file */
#define R_VSNSA R_CTREVT + 8   /* vsn_sa not defined in event structure */
#define R_NOVSN R_CTREVT + 9   /* no VSN where it was attempted */
#define R_NOSA  R_CTREVT + 10  /* no SA where it was attempted */
#define R_NOEVT R_CTREVT + 11  /* this is not an event */
#define R_VSNLEN R_CTREVT + 12  /* event length and FERA WC incompatible */
#define R_MEXTLAB R_CTREVT + 13 /* MEMEXT: label not defined in event struct */
#define R_SEVSTRUCT R_CTREVT + 14 /* bad format of event structure file */

/* CLASSE $1200 ===>   ERREUR ROUTINES TCP_****   */
#define R_TCP 0X1200 
#define R_WB R_TCP + 1 /* TCP_VME: Cette operation suspend la fonction mode b*/
#define R_INPROG R_TCP + 2 /* TCP_VME: Entree sortie en attente mode nb */
#define R_ALREADY R_TCP	+ 3 /* TCP_VME: Une operation est deja en cours */
#define R_DAREQ R_TCP + 4 /* TCP_VME: Une adresse de destination manque */
#define R_MSIZE R_TCP + 5 /* TCP_VME: Le message a envoyer est trop long */
#define R_BPROTO R_TCP + 6 /* TCP_VME: Ce protocole interdit pour ce socket */
#define R_NOTSOCKET R_TCP + 7 /* TCP_VME: Cet argument n'est pas un socket */
#define R_PNAV R_TCP + 8 /* TCP_VME: Ce protocole n'est pas supporte */
#define R_STNS R_TCP + 9 /* TCP_VME: Ce type de socket n'est pas supporte */
#define R_OPNS R_TCP + 10 /* TCP_VME: Operation interdite sur ce socket */
#define R_PFNS R_TCP + 11 /* TCP_VME: Famille de protocole non implantee */
#define R_AFNS R_TCP + 12 /* TCP_VME: Famille d'adresse non acceptee */
#define R_ADDRINU R_TCP + 13 /* TCP_VME: Cette adresse est deja utilisee */
#define R_ADDRNAV R_TCP	+ 14 /* TCP_VME: Adresse inconnue sur cette machine */
#define R_NETDOWN R_TCP	+ 15 /* TCP_VME: L'interface est inaccessible */
#define R_NETUNRCH R_TCP + 16 /* TCP_VME: L'interface n'est pas connectee */
#define R_RESET R_TCP + 17 /* TCP_VME: Deconnection par le service distant */
#define R_SWABORT R_TCP + 18 /* TCP_VME: Deconnection par le service local */
#define R_PRES R_TCP +19 /* TCP_VME: Reinitialisation par le service distant */
#define R_NOBUFF R_TCP + 20 /* TCP_VME: Pas de buffer libre pour l'operation */
#define R_ISCONN R_TCP + 21 /* TCP_VME: Ce socket est deja connecte */
#define R_NOTCONN R_TCP + 22 /* TCP_VME: Ce socket n'est pas connecte */
#define R_SHUTDOWN R_TCP + 23 /* TCP_VME: Socket ferme :message non transmis */
#define R_TOMANY R_TCP + 24 /* TCP_VME: Trop de references */
#define R_TO R_TCP + 25 /* TCP_VME: Le service distant ne repond pas */
#define R_CONNREF R_TCP + 26 /* TCP_VME: Refus de connection,service distant */
#define R_FORMAT R_TCP + 27 /* TCP_VME: Nom ou adresse,du service,inconnu */
#define R_SERVN	 R_TCP + 28 /* TCP_VME: Le service est ferme */
#define R_TIMOUT R_TCP + 29 /* TCP_VME: Attente trop longue (timeout)*/
#define R_LBUFR	 R_TCP + 30 /* TCP_VME: Buffer de reception de longueur = 0 */
#define R_CANAL R_TCP + 31 /* TCP_VME: Pas de nom de canal disponible */

//#define R_TCP_HOST R_TCP + 40 /* TCP_Unix: Erreur host inconnu */
//#define R_TCP_SOCK R_TCP + 41 /* TCP_Unix: Erreur TcpOpen - socket() */
//#define R_TCP_BIND R_TCP + 42 /* TCP_Unix: Erreur TcpOpen - bind() */
//#define R_TCP_LISTEN R_TCP + 43 /* TCP_Unix: Erreur TcpOpen - listen() */
//#define R_TCP_CLONE R_TCP + 44 /* TCP_Unix: Erreur TcpOpen - accept() */
//#define R_TCP_CONNECT R_TCP + 45 /* TCP_Unix: Erreur TcpOpen - connect()  */
//#ifndef PDOS
//#define R_TCP_CLOSE R_TCP + 46 /* TCP_Unix: Erreur close */
//#endif
//#define R_TCP_READ R_TCP + 47 /* TCP_Unix: Erreur TcpRead - read() */
//#define R_TCP_WRITE R_TCP + 48 /* TCP_Unix: Erreur TcpWrite - write() */
//#define R_TCP_LGBUF R_TCP + 49 /* TCP_Unix: Erreur longueur buffer < 0  */
//#define R_TCP_TIMEOUT R_TCP + 50 /* TCP_Unix: Time out  */
//#define R_TCP_BROKEN R_TCP + 51 /* TCP_Unix: Broken pipe  */
//#define R_TCP_FATAL    R_TCP + 52 /* le service distant ne repond plus */

/* CLASSE $1300 ===>   SERVER VME POUR LIAISON  TCP   */
#define R_SVTCP 0X1300 
#define R_LECMEM R_SVTCP + 1 /* SERV_TCP: Erreur lecture memoire VME */
#define R_TYPCOM R_SVTCP + 2 /* SERV_TCP: Type de commande inconnue */
#define R_TAILLE R_SVTCP + 3 /* SERV_TCP: Taille du message attendu incorrect*/
#define R_ENTETE R_SVTCP + 4 /* SERV_TCP: Entete de message incorrecte */
#define R_GCODE	 R_SVTCP + 5 /* SERV_TCP_GCODE: Un Buffer perdu par GCODE */
#define R_STCMDE R_SVTCP + 6 /* SERV_TCP_GCODE: Commande inconnue */
#define R_STBTDG R_SVTCP + 7 /* SERV_TCP_GCODE: Adr. Bt. de dialogue erronee */
#define R_MSESS  R_SVTCP + 8 /* SERV_TCP: Too many TCP connexions */
#define R_STNEMEM R_SVTCP + 9 /* SERV_TCP : Not Enough Memory for Data   */

/* CLASSE $1400 ===>   FILTRE   */
#define R_FILTRE 0X1400
#define R_FILOPEN R_FILTRE + 1 /* FILTRE: Cannot open command file */
#define R_FILTDC  R_FILTRE + 2 /* FILTRE: Error in user command file */
#define R_FILSIZE R_FILTRE + 3 /* FILTRE: User command file too big */
#define R_FILVAR  R_FILTRE + 4 /* FILTRE: Not implemented with EVCT_VAR */

/* CLASSE $1500 ===>   VXI   */
#define R_VXI 0X1500
#define R_VXMEMSIZE R_VXI + 1  /* VXI : Memoire globale trop petite */
#define R_VXNOMEM R_VXI + 2    /* VXI : Memoire non declaree dans le systeme */
#define R_VXUNKMOD R_VXI + 3   /* VXI : type de module inconnu */
#define R_VXBADCRATE R_VXI + 4 /* VXI : mauvais numero de chassis */
#define R_VXBADSLOT R_VXI + 5  /* VXI : mauvais numero de slot */
#define R_VXNOCRATE R_VXI + 6  /* VXI : chassis non initialise */
#define R_VXNODEV R_VXI + 7    /* VXI : slot vide */
#define R_VXNUMDEV R_VXI + 8   /* VXI : numero de device non initialise */
#define R_VXRNAME R_VXI + 9    /* VXI : acces reg par nom non disponible */
#define R_VXNOREG R_VXI + 10   /* VXI : registre inexistant sur ce module */
#define R_VXREGVV R_VXI + 11   /* VXI : mauvaise routine pour init reg VV */
#define R_VXCMD R_VXI + 12     /* VXI : erreur de commande */
#define R_VXROVF R_VXI + 13    /* VXI : adr registre hors du champ du module */
#define R_VXBADVAL R_VXI + 14  /* VXI : donnee non numerique */
#define R_VXCAFCT R_VXI + 15   /* VXI : liste camac , nb code > 1 */
#define R_VXTTMOD R_VXI + 16   /* VXI : acq de test, un seul module autorise */
#define R_VXIREG R_VXI + 17    /* VXI : mauvaise initialisation de registre */
#define R_VXI_MP R_VXI + 18    /* VXI : module en marche (bit MP de VXINTER) */
#define R_VXIACT R_VXI + 19    /* VXI : action impossible sur ce module */
#define R_VXDATMOD R_VXI + 20  /* VXI : zone data non initialisee */
#define R_VXISPEC R_VXI + 21   /* VXI : routine specifique inconnue */
#define R_VXRGITEM R_VXI + 22  /* VXI : mauvais num d'element de registre */
#define R_VXREAD R_VXI + 23    /* VXI : registre non accessible en lecture */
#define R_VXWRITE R_VXI + 24   /* VXI : registre non accessible en ecriture */
#define R_VXWRDATA R_VXI + 25  /* VXI : erreur d'ecriture data */
#define R_VXRGADDR R_VXI + 26  /* VXI : mauvaise adresse de registre */
#define R_VXRGNAME R_VXI + 27  /* VXI : mauvais nom de registre */
#define R_VXNOADDR R_VXI + 28  /* VXI : acces par adresse impossible */
#define R_VXUNKCMD R_VXI + 29  /* VXI : commande inconnue */
#define R_VXNOLABEL R_VXI + 30 /* VXI : manque un label sur 2 */
#define R_VXLECAM R_VXI + 31   /* VXI : timeout sur leccam correlateur */
#define R_VXFDCTO R_VXI + 32   /* VXI : timeout sur FDC correlateur */
#define R_VXINVOIE R_VXI + 33  /* VXI : au moins une voie non initialisee */
#define R_VXCPTVOI R_VXI + 34  /* VXI : VXACQ != ffffffff sur CPTVOI */
#define R_VXCVXTBL R_VXI + 35  /* VXI : table vxtbl trop petite dans vxcamin */
#define R_VXLABEL R_VXI + 36   /* VXI : label deja initialise */
#define R_VXFDLCAM R_VXI + 37  /* VXI : timeout fin lecture CAMAC correlateur*/

/* CLASSE $1600 ===>   VXI_RM   */
#define R_VXIRM 0X1600
#define R_RMBERR R_VXIRM + 1	/* RM : Bus ERROR sur acces VXI*/
#define R_RMABSZERO R_VXIRM + 2	/* RM : le slot zero est absent */
#define R_RMATRIB R_VXIRM + 3	/* RM : non concordance d'attribution */
#define R_RMMULTI R_VXIRM + 4	/* RM : plus de 1 device dans un slot */
#define R_RMMISMAT R_VXIRM + 5	/* RM : module incompatible avec dyn. config */
#define R_RMLOGADR R_VXIRM + 6	/* RM : 1ere @ logique non trouvee */

/* CLASSE $1700 ===>   VXLDBIB  */
#define R_VXIBI 0X1700
#define R_BNOFILE R_VXIBI + 1	/* BIB : Fichier bibliotheque absent*/
#define R_BREADF R_VXIBI + 2	/* BIB : Mauvaise lecture fichier bib.*/
#define R_BNOPARA R_VXIBI + 3	/* BIB : Pas de parametre apres le signe egal*/
#define R_BNOEGAL R_VXIBI + 4	/* BIB : Pas de signe egal*/
#define R_BTOEMPTY R_VXIBI + 5	/* BIB : Trop de lignes vides*/
#define R_BSPACE R_VXIBI + 6	/* BIB : Pas de place en memoire*/
#define R_BNOREG R_VXIBI + 7	/* BIB : Pas de registres*/
#define R_BCHAIN R_VXIBI + 8	/* BIB : Fichier chaine absent*/
#define R_BNOCEGAL R_VXIBI + 9	/* BIB : Pas de egal apres FILE*/
#define R_BNAME R_VXIBI + 10	/* BIB : Mauvais nom*/
#define R_BSTAND R_VXIBI + 11	/* BIB : Mauvais standard de module*/
#define R_BMOD R_VXIBI + 12	/* BIB : Mauvais model*/
#define R_BNREG R_VXIBI + 13	/* BIB : Mauvais nombre de registres*/
#define R_BNBIT R_VXIBI + 14	/* BIB : Mauvais nombre de champs de bits*/
#define R_BRINEG R_VXIBI + 15	/* BIB : Nombre de registres inegal*/
#define R_BBINEG R_VXIBI + 16	/* BIB : Nombre de champs de bits inegal*/
#define R_BMODE R_VXIBI + 17	/* BIB : Mauvais mode*/
#define R_BADRS R_VXIBI + 18	/* BIB : Mauvaise adresse*/
#define R_BLONG R_VXIBI + 19	/* BIB : Mauvaise longueur*/
#define R_BBIT R_VXIBI + 20	/* BIB : Mauvais champ de bits*/
#define R_BUSE R_VXIBI + 21	/* BIB : Mauvais usage*/
#define R_BITEM R_VXIBI + 22	/* BIB : Mauvais item*/
#define R_BSTEP R_VXIBI + 23	/* BIB : Mauvais step*/
#define R_BINIT R_VXIBI + 24	/* BIB : Mauvais init*/
#define R_BFACT R_VXIBI + 25	/* BIB : Mauvais facteur*/
#define R_BUNIT R_VXIBI + 26	/* BIB : Mauvaise unite*/
#define R_BREG R_VXIBI + 27	/* BIB : Mauvais nom de registre (bits)*/
#define R_BIDENT R_VXIBI + 28	/* BIB : Mauvais identificateur*/


/* CLASSE $1800 ===>   Erreurs VXI Acquisition  */
#define R_VXIACQ 0X1800
#define R_VXCALIB R_VXIACQ + 1	/* SELECTEUR INDRA : Erreur de calibration */
#define R_VXTRIG R_VXIACQ + 2   /* VXCAMRD:SR -> Flag trigger VXI false */


/* CLASSE $2000 ===> SERVEUR CAENET */
#define R_SVCAENET 0X2000
#define R_NOCNTL   R_SVCAENET + 1 /* SVCAENET : No CAENET controller in syst */
#define R_TYPCNTL  R_SVCAENET + 2 /* SVCAENET : Bad CAENET controller type   */
#define R_NOCPU    R_SVCAENET + 3 /* SVCAENET : No cpu for this controller   */
#define R_NOCRATE  R_SVCAENET + 4 /* SVCAENET : This Crate Is NOT In System */
#define R_NOMOD    R_SVCAENET + 5 /* SVCAENET : This Module Is Not In System */
#define R_NOCHAN   R_SVCAENET + 6 /* SVCAENET : This Channel Is Not Present */
#define R_RNGDATA  R_SVCAENET + 7 /* SVCAENET : Data Out Of Range */
#define R_UNKNCOD  R_SVCAENET + 8 /* SVCAENET : This Is Not A Legal Code */
#define R_BADCHAN  R_SVCAENET + 9 /* SVCAENET : Bad Chan Number in Command */
#define R_BADCRAT  R_SVCAENET +10 /* SVCAENET : Bad Crate Number in Command */
#define R_BADMOD   R_SVCAENET +11 /* SVCAENET : Bad Module Number in Command */
#define R_NEMEM    R_SVCAENET +12 /* SVCAENET : Not Enough Memory for Data   */
#define R_BADCMD   R_SVCAENET +13 /* SVCAENET : BAD Svcaenet cmde	*/
#define R_NIMPL    R_SVCAENET +14 /* SVCAENET : Non Implemented command */
#define R_BADPARM  R_SVCAENET +15 /* SVCAENET : Bad Parameters in command */
#define R_TYPCRAT  R_SVCAENET +16 /* SVCAENET : Bad Crate Type in command */
#define R_NOLABEL  R_SVCAENET +17 /* SVCAENET : No label free for new command*/
#define R_NLSTCMD  R_SVCAENET +18 /* SVCAENET : No Such Command in HT list */
#define R_NLSTSTA  R_SVCAENET +19 /* SVCAENET : No Such Status in HT list */
#define R_WAIT     R_SVCAENET +20 /* SVCAENET : Wait For Command Execution */
#define R_LIBMEM   R_SVCAENET +21 /* SVCAENET : Liberation Mem apres Timeout */

/* CLASSE $2100 ===> ERREURS CAENET V288 <-> SY403 */
#define R_SY403  0X2100
#define R_Q_IDENT  R_SY403 + 1  /* SY403 : Write Error Ident in V288 */
#define R_Q_CRATE  R_SY403 + 2  /* SY403 : Write Error Crate in V288 */
#define R_Q_CODE   R_SY403 + 3  /* SY403 : Write Error Code in V288 */
#define R_Q_DATA   R_SY403 + 4  /* SY403 : Write Error Data in V288 */
#define R_Q_TX	   R_SY403 + 5  /* SY403 : Transmission Packet Error */
#define R_Q_RX	   R_SY403 + 6  /* SY403 : Reception Packet Error */
#define R_LESSDAT  R_SY403 + 7  /* SY403 : Read Error Data From Sy403 */
#define R_BUFVIDE  R_SY403 + 9  /* SY403 : Transmit Buffer Empty */
#define R_MODBUSY  R_SY403 + 10 /* SY403 : Module Sy403 Busy */
#define R_MODPRES  R_SY403 + 11 /* SY403 : Module Not Installed */
#define R_CHPRES   R_SY403 + 12 /* SY403 : Channel Not Present  */ 
#define R_OUTVAL   R_SY403 + 13 /* SY403 : Value out of range   */
#define R_TIMEOUT  R_SY403 + 14 /* SY403 : Timeout sur le CAENET */
#define R_IDCNTRL  R_SY403 + 15 /* SY403 : Bad Id controller on CAENET   */
#define R_BADMSG   R_SY403 + 16 /* SY403 : Bad Code or Message Incorrect */
#define R_MISSINT  R_SY403 + 17 /* SY403 : Missing Interrupt on V288 */



/* CLASSE $2200 ===> ERREURS ECHELLE V260 <-> V260 */
#define R_V260  0X2200
#define R_NOECH      R_V260 + 1  /* ECHELLE : Echelle absente */
#define R_ECHINC     R_V260 + 2  /* ECHELLE : Echelle inconnue */
#define R_MEM        R_V260 + 3  /* ECHELLE : Pas assez de memoire */
#define R_CODCMD     R_V260 + 4  /* ECHELLE : Erreur de code de commande */
#define R_PROTOC     R_V260 + 5  /* ECHELLE : Protocole inconnu */
#define R_TRANS      R_V260 + 6  /* ECHELLE : Erreur transmission message */
#define R_RAFF       R_V260 + 8  /* FECHEL  : Erreur de code */
#define R_CODECH     R_V260 + 10 /* ECHCMD  : Erreur de code de commande */
#define R_ECHSENDBUF R_V260 + 11 /* ECHELLE : erreur de transfert */
#define R_ECHTRANS   R_V260 + 12 /* ECHELLE : buffer en cours de transfert */
#define R_ECHBUFSIZE R_V260 + 13 /* ECHELLE : buffer trop petit */
#define R_ECHFIC     R_V260 + 14 /* ECHELLE : pas de reponse du FIC */
#define R_ECH_SET    R_V260 + 15 /* ECHELLE : Pas chgt SETUP acquis en cours */

/* CLASSE $2300 ===> Erreurs du Serveur de Buffers (SBUF). */
#define R_SBUF 0x2300
#define R_SNTFIFO  R_SBUF +  1 /* SBUF: Mem. non  declaree dans le systeme */
#define R_SNTKWN   R_SBUF +  2 /* SBUF: Commande inconnue */
#define R_ONTSES   R_SBUF +  3 /* SBUF: Plus de session libre avec OPEN */
#define R_CNTNUM   R_SBUF +  4 /* SBUF: Pas de Num. de Sess. avec CLOSE */
#define R_CNUMFAUX R_SBUF +  5 /* SBUF: Numero de session faux avec CLOSE */
#define R_CDEJASTP R_SBUF +  6 /* SBUF: Session deja stoppe avec CLOSE */
#define R_CNTALLC  R_SBUF +  7 /* SBUF: Session non allouee avec CLOSE */
#define R_SNTNUM   R_SBUF +  8 /* SUBF: Pas de Num. de Sess. avec SETUP */
#define R_SNUMFAUX R_SBUF +  9 /* SUBF: Numero de session faux avec SETUP */
#define R_STYPEINE R_SBUF + 11 /* SBUF: Type de Buf. inexistant avec SETUP */
#define R_SMODEINE R_SBUF + 12 /* SBUF: Mode inexistant avec SETUP */
#define R_SNTSIZE  R_SBUF + 13 /* SBUF: Pas de taille de Buf. avec SETUP */
#define R_SNTRATE  R_SBUF + 14 /* SBUF: Pas de taux specifie avec SETUP */
#define R_SNTSTR   R_SBUF + 16 /* SBUF: La Sess. n'est pas START avec SETUP */
#define R_DSTOP    R_SBUF + 17 /* SBUF: Le mode DEBUG est stoppe */
#define R_DSTART   R_SBUF + 18 /* SBUF: Le mode DEBUG est lance */
#define R_SBADLINK R_SBUF + 19 /* SBUF: Type de lien inconnu */
#define R_SNOLINK  R_SBUF + 20 /* SBUF: Type de lien non specifie dans OPEN */
#define R_FICOPEN  R_SBUF + 21 /* SBUF: Session FIC deja allouee */
#define R_SBUFIFO  R_SBUF + 22 /* SBUF: Fifo d'emission plein */

/* CLASSE $2400 ===> ERREURS COMMUNICATIONS SERVEUR CC */
#define R_SVCC 0X2400
#define R_SVCC_1   R_SVCC + 1   /* SVCC  : Not Enough Memory */
#define R_SVCC_2   R_SVCC + 2   /* SVCC  : Missing Argument In command */
#define R_SVCC_3   R_SVCC + 3   /* SVCC  : Protocol Non Implemented  */
#define R_SVCC_4   R_SVCC + 4   /* SVCC  : Liberation Apres Timeout  */



/* CLASSE $2500 ===> ERREURS SERVEUR JBUS */
#define R_JBS 0X2500
#define R_JBS_1    R_JBS + 1	/* JBUS : Fonction inconnue */
#define R_JBS_2    R_JBS + 2	/* JBUS : Adresse incorecte */
#define R_JBS_3    R_JBS + 3	/* JBUS : Donnee incorecte  */
#define R_JBS_4    R_JBS + 4	/* JBUS : Automate non pret */
#define R_JBS_5    R_JBS + 5	/* JBUS : Acquittement      */
#define R_JBS_6    R_JBS + 6	/* JBUS : Erreur inconnue   */
#define R_JBS_7    R_JBS + 7	/* JBUS : Non acquittement  */
#define R_JBS_8    R_JBS + 8	/* JBUS : Defaut d'ecriture */
#define R_JBS_9    R_JBS + 9	/* JBUS : Zone chevauche    */
#define R_JBS_10   R_JBS + 10	/* JBUS : Err liaison emiss" */
#define R_JBS_11   R_JBS + 11	/* JBUS : Err liaison recep" */
#define R_JBS_12   R_JBS + 12	/* JBUS : Erreur de CRC    " */


/* CLASSE $2600 ===> ERREURS GESTION LISTES CHAINEES  */
#define R_LIST 0X2600
#define R_LIST_1   R_LIST + 1   /* LIST  : Not enough memory            */
#define R_LIST_2   R_LIST + 2   /* LIST  : List is empty                */
#define R_LIST_3   R_LIST + 3   /* LIST  : Inexisting Element           */
 
/* CLASSE $2700 ===> ERREURS DE CHARGEMENT LISTE ACQUISITION */
#define R_ACQLOAD 0x2700
#define R_PHASEARG R_ACQLOAD + 1 /* AcqLoad : PHASE bad or missing arguments */
#define R_TRIGARG  R_ACQLOAD + 2 /* AcqLoad : TRIG bad or missing arguments  */
#define R_LAMARG   R_ACQLOAD + 3 /* AcqLoad : LAM bad or missing arguments   */
#define R_UNKTRIG  R_ACQLOAD + 4 /* AcqLoad : Unknown trigger type           */
#define R_BADVERBE R_ACQLOAD + 5 /* AcqLoad : Bad Verbe */
#define R_NOTRIG   R_ACQLOAD + 6 /* AcqLoad : No trigger at given position   */
#define R_ACTSIZE  R_ACQLOAD + 7 /* AcqLoad : Action table too small         */

#endif
