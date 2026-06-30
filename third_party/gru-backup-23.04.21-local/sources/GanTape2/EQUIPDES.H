 /*	EQUIPDES:H -- last update : 28 MAR 88	creation date : 19 NOV 87
	Copyright Acquisition group, GANIL Caen, France
	author	: Bruno RAINE

 	contents : structure de description des equipements du systeme VME
		d'acquisition.
		   La table des equipements est initialisee dans GMAIL au
		demarrage du systeme par la commande INI$SYS qui initialise
		la totalite de GMAIL ou par la commande INIEQUIP lorsqu'on
		veut remettre a jour la table des equipements sans rebooter.
		Le fichier des equipements SY$EQUIP utilise est celui se
		trouvant sur le disque courant.
		   La table des equipements est compose de 4 mots longs (dont
		le premier contient la longueur de la table en octets) puis des
		descripteurs d'equipements chaines entre eux. Chaque
		descripteur debute par une structure de type EQUIPDES.
*/

#define EQUIPFIL "SY$EQUIP"	/* nom du fichier des equipements */

/* definition des familles d'equipements */
#define CPUFAMI 1		/* famille CPU */
#define ASCUFAMI 2		/* famille ASCU */
#define IFVFAMI 3		/* famille IFV */
#define MEMFAMI 4		/* famille zone memoire */
#define TFAMI 5			/* famille T trransfert vers hote */
#define CAMFAMI CPUFAMI		/* famille CAMAC */

/* definition des types d'applications */
#define DEV 0			/* developpement */
#define PF 1			/* prefiltrage */
#define TRA 2 			/* traitement et control */
#define CAM 3			/* camac */
#define TRANS 4			/* transfert vers hote */

/* definition des types de CPU's */
#define F20 0X20			/* FORCE CPU 20 */
#define F21 0X21			/* FORCE CPU 21 */
#define F5  0X05			/* FORCE CPU 5  */
#define F22 0X22			/* FORCE CPU 22 */
#define F23 0X23			/* FORCE CPU 23 */
#define F29 0X29			/* FORCE CPU 29 */
#define F30 0X30			/* FORCE CPU 30 */
#define F31 0X31			/* FORCE CPU 31 */


typedef struct {	/* entete de tous les descripteurs d'equipement */
	unsigned long chfwd;	/* chainage en deplacement sur descr. suivant*/
	short int famille;	/* famille d'equipement */
	char name[8];		/* nom de l'equipement */
	short int num;		/* numero d'equipement */ 
	} EQUIPDES;
	
typedef struct {	/* entete de la table des equipements */
	unsigned long equipsiz;	/* taille de la table en octets */
	long free[3];
	EQUIPDES equipement;	/* 1er equipement de la table */
	} EQUIPHD;

typedef struct {	/* descripteur de CPU (attention, le numero
			 d'equipement est compose du numero de chassis VME
			 sur l'octet de poids fort et du numero de CPU sur
			 l'octet de poids faible). */
	EQUIPDES equip;	/* entete de descripteur */
	short int port;		/* numero du port de connexion sur DEV */
	char cputype;		/* type de cpu */
	char appltyp;		/* type d'application */
	char *cpuload;		/* adresse du systeme en PROM vue de DEV */
	char *cpugo;		/* adresse de boot vue du cible */
	char *xmaildev;		/* adresse du xmail vue du DEV */
	char *xmailadr;		/* adresse du xmail vue du cible */
	char *shmemvme;		/* adresse memoire partagee vue du VME */
	char *shmemcpu;		/* adresse memoire partagee vue du cpu */
	unsigned long int shmemsiz;	/* taille memoire partagee */
	char *rddevad;		/* adresse ramdisk partagee vue du DEV */
	char *rdadr;		/* adresse ramdisk partagee vue du cible */
	short int rdsize;	/* taille de la ramdisk en secteurs */
	short int rddevnb;	/* numero ramdisk vu du DEV */
	short int rdnb;		/* numero ramdisk vu du cible */
	char free1[2];
	char strtfil[10];	/* nom du fichier start au boot */
	char *cpusync;		/* adresse de la structure de synchro inter
				   cpu en acquisition (initialise uniquement
				   pour les PF's, 0 sinon)	*/
	char free[2];
	} CPUDES;

typedef struct {	/* Descripteur d'ASCU; si un ascu est present dans un
			 chassis, son nom est ASCUc (c etant le numero du
			 chassis) et son numero d'equipement est egal au numero
			 du chassis */
	EQUIPDES equip;
	char *ascubase;		/* adresse de base de l'ascu */
	long free[3];
	} ASCUDES;

typedef struct {	/* Descripteur de coupleurs sur le VME avec les 
			   bus d'acquisition des differents multidetecteurs
			   (IFV pour FERA) */
	EQUIPDES equip;
	char *md_adr;	/* adresse VME du coupleur */
	int md_bufsz;	/* taille des buffers */
	int md_bufnb;	/* nombre de buffers */
	long free;
	} MDETDES;

typedef struct {	/* Descripteur de la carte transfert vers modcomp */
	EQUIPDES equip;
	char *t_adr;	/* adresse VME de la carte transfert */
	char free[14];
	} TDES;

typedef struct {	/* Descripteur de zone memoire VME */
	EQUIPDES equip;
	char *mem_adr;	/* adresse VME de la carte transfert */
	long unsigned mem_siz;	/* taille en octets */
	char free[10];
	} MEMDES;


/* Description d'une carte IFV */
/* Offsets des adresses de la carte */
#define IFVRFR	0x800	/* Lecture evenement en fifo, parametre random */
#define IFVRRR	0x0	/* Lecture evenement random , parametre random */
#define IFVRFF	0x1800	/* Lecture evenement en fifo, parametre en fifo */
#define IFVRRF	0x1000	/* Lecture evenement random , parametre en fifo */
#define IFVWBAS	0x2000	/* ecriture reg. de base pour lecture evt en random */
#define IFVCTRL	0x2800	/* ecriture mot de controle / lecture status */
/* Description  codes de commande (bits 12-8) du mot de controle */
/* Attention, lorsqu'on ecrit ces bits, on modifie aussi les bits 0 et 1 */
#define IFV_SO	0x300	/* shift-out evt */
#define IFV_RDIS 0x500	/* interdit mise en fifo des evts rejetes */
#define IFV_RENB 0x700	/* autorise mise en fifo des evts rejetes */
#define IFV_ZL1	 0x1100	/* mettre a zero sortie LEMO no 1 */
#define IFV_SL1	0x1900	/* mettre a +5V sortie LEMO no 1 */
#define IFV_ZL2 0x1300	/* mettre a zero sortie LEMO no 2 */
#define IFV_SL2 0x1B00	/* mettre a +5V sortie LEMO no 2 */
#define IFV_SL3 0x1500	/* mettre a +5V (80 ns) sortie LEMO no 3 */
/* Description  codes de commande (bits 1-0) du mot de controle */
#define IFV_RES	0x1	/* reset carte IFV */
#define IFV_IENB 0x2	/* autoriser interruptions face avant */
/* Description bits de status (en lecture) du mot de controle */
/* bits 23-16 contiennent nombre de places (nb evts) occupees dans la fifo */
#define IFV_INT	0x4	/* interruptions face avant autorisees */
#define IFV_VIDE 0x8	/* fifo vide */
#define IFV_PLEIN 0x10	/* fifo pleine */
#define IFV_L1	0x20	/* etat entree LEMO 1 */
#define IFV_L2	0x40	/* etat entree LEMO 2 */
#define IFV_RDY	0x80	/* ready */
/* Description des bits de status de l'evenement sur 1er mot de 16 bits de
   l'evenement contenant status + longueur */
#define IFVS_ACC 0x0400	/* evenement accepte */
#define IFVS_REJ 0x0800	/* evenement rejete */
#define IFVS_OVF 0x1000	/* evenement en overflow */
#define MSKSTIFV 0x1c00	/* masque pour extraire bits de status sur 16 bits */
#define MSKLGIFV 0x3ff   /* masque pour extraire longueur sur 16 bits */
