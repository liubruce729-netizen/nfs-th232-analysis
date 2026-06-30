/**
 * \file BControl.h
 * \author Paul Goux
 * \date 20 avril 2018
 *
 */

#ifndef GTAPE_GTCONTROLCONSOLE_H_
#define GTAPE_GTCONTROLCONSOLE_H_

#include <string>
#include <iostream>
#include "GTape.h"
#include "GAcq.h"
#include <unistd.h>
#include <map>
#include "BProgressBar.h"
#include "CError.h"
#include "Blocktest.h"
#include "STR_EVT.H"

/**
 * Niveau de verbose nécessaire pour afficher les statistiques des blocs..
 */
#define VERBO_STAT 1
/**
 * Niveau de verbose nécessaire pour afficher les statistiques des event.
 */
#define VERBO_STAT_EVENT 2
/**
 * Niveau de verbose nécessaire pour afficher les informations du fichier.
 */
#define VERBO_INFO_FILE 3
/**
 * Niveau de verbose nécessaire pour afficher les headers des blocs.
 */
#define VERBO_HEADER_BLOCK 5
/**
 * Niveau de verbose nécessaire pour afficher les données des blocs.
 */
#define VERBO_BLOCK 8
/**
 * Niveau de verbose nécessaire pour afficher les events d'un block.
 */
#define VERBO_EVENT 10

/**
 * Taille du buffer par défaut.
 */
#define DEF_BUF_SIZE 16384
/**
 * Nom de l'action file par défaut.
 */
#define DEF_ACTION_FILE "local"
/**
 * Niveau de verbose par défaut.
 */
#define DEF_VERBOSE 1
/**
 * Premier bloc à lire par défaut.
 */
#define DEF_START_BLOCK 1
/**
 * Nombre de bloc à lire par défaut (0 = tous).
 */
#define DEF_NB_BLOCK 0
/**
 * Nombre d'octet affiché par défaut pour tous les blocs affichés.
 */
#define DEF_NB_DUMP 255
/**
 * Premier event du bloc à lire par défaut.
 */
#define DEF_START_EVENT 1
/**
 * Nombre d'event à lire par bloc par défaut (0 = tous).
 */
#define DEF_NB_EVENT 0
/**
 * Si les event s'affiche en données brutes ou pas par défaut.
 */
#define DEF_RAW_EVENT false
/**
 * Nombre d'octet affiché par défaut pour tous évènements à afficher des blocs affichés.
 */
#define DEF_NB_EVENT_DUMP 256

/**
 * Commande courte pour saisir le chemin du fichier.
 */
#define COMC_FILE "-f"
/**
 * Commande courte pour afficher l'aide.
 */
#define COMC_HELP "-h"
/**
 * Commande courte pour saisir la taille du buffer.
 */
#define COMC_BUF_SIZE "-ibs"
/**
 * Commande courte pour saisir le niveau de verbose.
 */
#define COMC_VERBOSE "-v"
/**
 * Commande courte pour saisir le nombre de bloc à afficher.
 */
#define COMC_NB_BLOCK "-nd"
/**
 * Commande courte pour saisir le premier bloc à afficher.
 */
#define COMC_START_BLOCK "-nt"
/**
 * Commande courte pour saisir le chemin du action file.
 */
#define COMC_ACTION_FILE "-iea"
/**
 * Commande courte pour saisir le nombre d'octet à afficher par bloc.
 */
#define COMC_NB_DUMP "-d"
/**
 * Commande courte pour saisir le nombre d'event à afficher par bloc.
 */
#define COMC_NB_EVENT "-ned"
/**
 * Commande courte pour saisir le premier event du bloc à afficher.
 */
#define COMC_START_EVENT "-net"
/**
 * Commande courte pour choisir un dump des events brut.
 */
#define COMC_RAW_EVENT "-red"
/**
 * Commande courte pour choisir le nombre d'octet à afficher par évènement.
 */
#define COMC_NB_DUMP_EVENT "-ed"

/**
 * Commande longue pour saisir le chemin du fichier.
 */
#define COML_FILE "--file"
/**
 * Commande longue pour afficher l'aide.
 */
#define COML_HELP "--help"
/**
 * Commande longue pour saisir la taille du buffer.
 */
#define COML_BUF_SIZE "--initbufsize"
/**
 * Commande longue pour saisir le niveau de verbose.
 */
#define COML_VERBOSE "--verbose"
/**
 * Commande longue pour saisir le nombre de bloc à afficher.
 */
#define COML_NB_BLOCK "--numberdump"
/**
 * Commande longue pour saisir le premier bloc à afficher.
 */
#define COML_START_BLOCK "--numberstart"
/**
 * Commande longue pour saisir le chemin du action file.
 */
#define COML_ACTION_FILE "--initeventaction"
/**
 * Commande longue pour saisir le nombre d'octet à afficher par bloc.
 */
#define COML_NB_DUMP "--dump"
/**
 * Commande longue pour saisir le nombre d'event à afficher par bloc.
 */
#define COML_NB_EVENT "--numbereventdump"
/**
 * Commande longue pour saisir le premier event du bloc à afficher.
 */
#define COML_START_EVENT "--numbereventstart"
/**
 * Commande longue pour choisir un dump des events brut.
 */
#define COML_RAW_EVENT "--raweventdump"
/**
 * Commande longue pour choisir le nombre d'octet à afficher par évènement.
 */
#define COML_NB_DUMP_EVENT "--eventdump"

/**
 * \class BControl
 * \brief Controleur ligne de commande.
 * \details Cette classe permet la gestion d'une ligne de commande pour lire des blocs. Remplace Ganil_Tape.
 */
class BControl {

private:
	char **commandLine;/**< Ligne de commande (argv). */
	int tailleCommande;/**< Taille de la ligne de commande (argc). */

	/** \brief Path du fichier à lire.
	 * \details Initialisé par \link COMC_FILE \endlink ou \link DEF_BUF_SIZE \endlink ou directement dans la commande.
	 * Doit être renseigné.
	 */
	std::string filename;


	/** \brief GAcq que l'on lit.
	 * \details Initialisé avec un GTape
	 */
	GAcq *fAcq;

	/**
	 * \brief Ce charge de l'affichage des lignes d'info, de warning et d'erreur
	 */
	CError *mError;

	/** \brief Vrai si la ligne de commande a une erreur, faux sinon. */
	bool mErreur;

	/** \brief Path du fichier action file.
	 * \details La valeur par défaut est définie par \link DEF_ACTION_FILE \endlink\.
	 * Initialisé par \link COMC_ACTION_FILE \endlink ou \link COML_ACTION_FILE \endlink\.
	 */
	std::string actionFile;

	/** \brief Taille du buffer.
	 * \details La valeur par défaut est définie par \link DEF_BUF_SIZE \endlink\.
	 * Initialisé par \link COMC_BUF_SIZE \endlink ou \link COML_BUF_SIZE \endlink\.
	 */
	int bufSize;

	/** \brief Niveau de verbose.
	 * \details Niveau de verbose. Plus il est élevé, plus d'information seront affiché à l'écran.
	 * La valeur par défault est définie par \link DEF_VERBOSE \endlink\.
	 * Initialisé par \link COMC_VERBOSE \endlink ou \link COML_VERBOSE \endlink\.
	 */
	int verbose;

	/**
	 * \brief Rang du premier block à être affiché.
	 * \details La valeur par défault est définie par \link DEF_START_BLOCK \endlink\.
	 * Initialisé par \link COMC_START_BLOCK \endlink ou \link COML_START_BLOCK \endlink\.
	 */
	int startBlock;

	/**
	 * \brief Nombre de block à être affiché.
	 * \details La valeur par défault est définie par \link DEF_NB_BLOCK \endlink\.
	 * Initialisé par \link COMC_NB_BLOCK \endlink ou \link COML_NB_BLOCK \endlink\.
	 */
	int nbBlock;

	/**
	 * \brief Rang du premier event du bloc à être affiché.
	 * \details La valeur par défault est définie par \link DEF_START_BLOCK \endlink\.
	 * Initialisé par \link COMC_START_BLOCK \endlink ou \link COML_START_BLOCK \endlink\.
	 */
	int startEvent;

	/**
	 * \brief Nombre d'event par block à être affiché.
	 * \details La valeur par défault est définie par \link DEF_NB_BLOCK \endlink\.
	 * Initialisé par \link COMC_NB_BLOCK \endlink ou \link COML_NB_BLOCK \endlink\.
	 */
	int nbEvent;

	/**
	 * \brief Nombre d'octet affiché par block.
	 * \details La valeur par défault est définie par \link DEF_NB_DUMP \endlink\.
	 * Initialisé par \link COMC_NB_DUMP \endlink ou \link COML_NB_DUMP \endlink\.
	 */
	int nbDump;

	/**
	 * \brief Vrai si les l'affichage des events doit est brut, faux sinon.
	 * \details La valeur par défault est définie par \link DEF_RAW_EVENT \endlink\.
	 * Initialisé par \link COMC_RAW_EVENT \endlink ou \link COMC_RAW_EVENT \endlink\.
	 */
	bool rawEvent;

	/**
	 * \brief Nombre d'octet affiché par évènement.
	 * \details La valeur par défault est définie par \link DEF_NB_EVENT_DUMP \endlink\.
	 * Initialisé par \link COMC_NB_DUMP_EVENT \endlink ou \link COML_NB_DUMP_EVENT \endlink\.
	 */
	int nbEventDump;

	/**
	 * \brief Vrai si \link filename \endlink est spécifié dans la ligne de commande, faux sinon.
	 */
	bool doFile;

	/**
	 * \brief Vrai si \link bufSize \endlink est spécifié dans la ligne de commande, faux sinon.
	 */
	bool doInitBufSize;

	/**
	 * \brief Vrai si \link actionFile \endlink est spécifié dans la ligne de commande, faux sinon.
	 */
	bool doInitEventAction;

	/**
	 * \brief Vrai si \link verbose \endlink est spécifié dans la ligne de commande, faux sinon.
	 */
	bool doVerbose;

	/**
	 * \brief Vrai si \link nbBlock \endlink est spécifié dans la ligne de commande, faux sinon.
	 */
	bool doNbBlock;

	/**
	 * \brief Vrai si \link startBlock \endlink est spécifié dans la ligne de commande, faux sinon.
	 */
	bool doStartBlock;

	/**
	 * \brief Vrai si \link nbDump \endlink est spécifié dans la ligne de commande, faux sinon.
	 */
	bool doDump;

	/**
	 * \brief Vrai si \link nbEvent \endlink est spécifié dans la ligne de commande, faux sinon.
	 */
	bool doNbEvent;

	/**
	 * \brief Vrai si \link startEvent \endlink est spécifié dans la ligne de commande, faux sinon.
	 */
	bool doStartEvent;

	/**
	 * \brief Vrai si \link rawEvent \endlink est spécifié dans la ligne de commande, faux sinon.
	 */
	bool doRawEvent;

	/**
	 * \brief Vrai si \link nbEventDump \endlink est spécifié dans la ligne de commande, faux sinon.
	 */
	bool doEventDump;

	/**
	 * \brief Map contant le nombre de chaque type de bloc\.
	 */
	map<int, int> mapBlock;

	/**
	 * \brief Map pour compter le nombre d'occurence de chaque voie.
	 */
	map<int, int> mapEvent;

	/**
	 * \brief Map pour compter le total des occurences multiples d'une voie dans un évènement.
	 */
	map<int, int> mapMultiEvent;

	/**
	 * \brief Change la ligne de commande.
	 * \details Permet de changer la ligne de commande. La ligne de commande est corriger par \link CorrigerCommandLine \endlink\.
	 * \param commandLine Ligne de commande.
	 */
	void SetCommandLine(char** commandLine);

public:
	/**
	 * \brief Constructeur
	 * \param command lLigne de commande tapée dans la console (argv).
	 * \param taille Nombre d'éléments dans la ligne de commande (argc).
	 */
	BControl(char **command, int taille);

	/**
	 * \brief Destructeur
	 */
	virtual ~BControl();

	/**
	 * \brief Corrige la ligne de commande.
	 * \details Permet de corriger les éventuelles fautes dans la ligne de commande (espaces en trop).
	 */
	void CorrigerCommandLine();

	/**
	 * \brief Exécute la ligne de commande.
	 * \details La ligne de commande est lut pour initialiser les différentes variables.
	 * Si la syntaxe n'est pas correct l'exécution s'arrête et affiche les erreurs.
	 * Sinon la lecture du fichier est faite comme demandé par la ligne de commande.
	 */
	void Executer();

	/**
	 * \brief Initialise le fichier à partir du nom de fichier.
	 * \details Initialise \link fAcq \endlink avec le nom du fichier.
	 */
	void SetNameDevice();

	/**
	 * \brief Affiche l'aide.
	 */
	void Help() const;

	/**
	 * \brief Affiche un message d'erreur.
	 * \details Affiche un message d'erreur. \link mErreur \endlink passe à vrai, bloquant l'exécution de la ligne de commande.
	 * \param message Message à afficher.
	 */
	void ErrorMessage(std::string message);

	/**
	 * \brief Affiche un message d'erreur.
	 * \details Affiche un message d'erreur à partir d'un stringstream.
	 * \link mErreur \endlink passe à vrai, bloquant l'exécution de la ligne de commande.
	 * Le stringstream \p stream est vidé.
	 * \param stream Stringstream contenant le message à afficher.
	 */
	void ErrorMessage(std::stringstream* stream);

	/**
	 * \brief Affiche un message d'information.
	 * \param message Message à afficher.
	 */
	void InfoMessage(std::string message) const;

	/**
	 * \brief Affiche un message d'information.
	 * \details Affiche un message d'information. Le stringstream \p stream est vidé.
	 * \param stream Stringstream contenant le message à afficher.
	 */
	void InfoMessage(std::stringstream* stream) const;

	/**
	 * \brief Affiche un mesage de warning.
	 * \details Affiche un message de warning. \link mErreur \endlink ne change pas.
	 * \param message Message à afficher.
	 */
	void WarningMessage(std::string message) const;

	/**
	 * \brief Affiche un mesage de warning.
	 * \details Affiche un message de warning. mErreur ne change pas. Le stringstream \p stream est vidé.
	 * \param stream Stringstream contenant le message à afficher.
	 */
	void WarningMessage(std::stringstream* stream) const;

	/**
	 * \brief Affiche les attributs.
	 * \details Affiche les attributs pour débugage.
	 */
	void AfficherAttributs() const;

	/**
	 * \brief Lit une commmande qui a une valeur.
	 * \details Si la commande \p commandeLine commence par \p commandeCourte ou \p commandeLongue, que \p doSmthg est faux et que un '=' sépare la commande de la valeur,
	 * alors la méthode renvoit vrai et \p doSmthg passe à vrai.
	 * \param[in] commandeLine Commande à traiter
	 * \param[in] commandeCourte Texte court de l'option cherchée.
	 * \param[in] commandeLongue Texte long de l'option cherchée.
	 * \param[out] doSmthg Si la commande est valide, devient vrai. Si une même commande est entrée 2 fois, doSmthg est déjà vrai et une erreur est alors affichée.
	 * \param[in] avecValeur Vrai (valeur par défaut) si une valeur suit la commande (séparée par '='). Faux sinon.
	 * \return Retourne vrai si la commande est juste et prise en compte, faux sinon.
	 */
	bool LireCommande(const char *commandeLine, const char *commandeCourte,
			const char* commandeLongue, bool *doSmthg, bool avecValeur = true);

	/**
	 * \brief Interprète la ligne de commande.
	 * \details Lit toutes les commandes de la ligne en appelant \link LireCommande \endlink\.
	 */
	void LireLigneCommande();

	/**
	 * \brief Affiche un block.
	 * \details Affiche le header et le nombre d'octet demandé (si le \link verbose \endlink est suffisant).
	 */
	void AfficherBlock() const;

	/**
	 * \brief Vérifie les valeurs.
	 * \details Vérifie que les valeurs de la ligne de commande soient cohérentes.
	 *  Affiche un warning ou une erreur (et bloque la suite) s'il y a une erreur.
	 * \return Retourne vrai s'il n'y pas de valeurs incohérents, faux sinon
	 */
	bool VerifierCommande();

	/**
	 * \brief Compte le bloc en fonction de son type.
	 */
	void CompterBlock();

	/**
	 * \brief Affiche la quantité de chaque type de bloc.
	 */
	void AfficherCompteBlock() const;

	/**
	 * \brief Affiche une barre.
	 */
	void Barre() const;

	/**
	 * \brief Change la taille du buffer.
	 * \details Change la taille du buffer si celle-ci est valide. La taille utilisée est toujours celle définie dans le block FILEH.
	 * Si l'utilisateur a saisie une valeur, celle-ci est ignorée et un warning apparait.
	 * Si la taille n'est pas définie dans le block FILEH, la saisie est obligatoire de la part de l'utilisateur (message d'erreur + fin du programme).
	 * \return True si le fichier contient un bloc FILEH au début, faux sinon. Ce booléen permet de savoir si il faut demander le fichier action PAR.
	 */
	bool InitBufferSize();

	/**
	 * \brief Lit tous les blocs.
	 * \details Lit tous les blocs et affiche les informations demandées.
	 */
	void LireTousBlocks();

	/**\brief Affiche le total des events d'un block.
	 * \details Le verbose doit etre >= à \link VERBO_EVENT \endlink\.
	 */
	void AfficherEvent() const;

	/**\brief Compte l'event en fontion de son numero.
	 */
	void CompterEvent();

	/**
	 * \brief Affiche le header du block courant.
	 */
	void AfficherHeaderBlock() const;

	/**
	 * \brief Affiche le total de chaque event.
	 */
	void AfficherCompteEvent();

	/**
	 * \brief Lit tous les events d'un bloc.
	 * \param afficherEvent Vrai pour dumperEvent, faux sinon (vrai par défaut).
	 */
	void LireTousEvents(bool afficherEvent = true);

	/**
	 * \brief Affiche un évènement au format brut.
	 * \details Le verbose doit etre >= à \link VERBO_EVENT \endlink\.
	 * Affiche les données en hexadécimal et leurs correspondances en ASCII si pertinante (entre 32 et 126).
	 */
	void AfficherEventBrut() const;

	/**
	 * \brief Remplit la map \link mapEvent \endlink qui compte les évènements.
	 * \details Initialise a 1 ou 0 tous les indices de la map. 1, l'évènement est attendu, 0 il ne l'est pas.
	 */
	void GenererMapEvent();

	/**
	 * \brief Ouvre le fichier
	 * \details Si le fichier ne veux pas s'ouvrir (n'existe pas par exemple) le programme s'arrête
	 */
	void OuvrirDeviceIn();

	/**
	 * \brief Initialise la lecture des évènements
	 * \details Si le fichier d'action paramètre n'est pas spécifié sur la ligne de commande, la lecture des voies se fait depuis le bloc PARAM
	 * Si le fichier est spécifié, on lit les voies depuis ce fichier.
	 */
	void InitialiserActionFile();
};

/**
 * \brief Affiche l'aide d'une option.
 * \details Affiche en premier \p commandeCourte puis \p commandeLongue puis \p explication \.
 * Le stringstream est vidé.
 * \param commandeCourte Commande courte de l'option.
 * \param commandeLongue Commande longue de l'option.
 * \param explication Explication de l'option.
 * \param avecEgal Vrai si l'on doit afficher un '=' devant les commandes, faux sinon (Vrai par défaut).
 */
void afficherOptionHelp(const char *commandeCourte, const char *commandeLongue,
		stringstream *explication, bool avecEgal = true);

/**
 * \brief Affiche l'aide d'une option.
 * \details Affiche en premier \p commandeCourte, puis \p commandeLongue puis \p explication \.
 * \param commandeCourte Commande courte de l'option.
 * \param commandeLongue Commande longue de l'option.
 * \param explication Explication de l'option.
 * \param avecEgal Vrai si l'on doit afficher un '=' devant les commandes, faux sinon (Vrai par défaut).
 */
void afficherOptionHelp(const char *commandeCourte, const char *commandeLongue,
		string explication, bool avecEgal = true);

/**
 * \brief Affiche l'aide d'une option.
 * \details Affiche en premier \p commandeCourte, puis \p commandeLongue puis \p explication \.
 * \param commandeCourte Commande courte de l'option.
 * \param commandeLongue Commande longue de l'option.
 * \param explication Explication de l'option.
 * \param avecEgal Vrai si l'on doit afficher un '=' devant les commandes, faux sinon (Vrai par défaut).
 */
void afficherOptionHelp(const char *commandeCourte, const char *commandeLongue,
		const char *explication, bool avecEgal = true);

/**
 * \details Permet de séparer la commande de sa valeur, la séparateur étant '='.
 * \param commande Commande à séparer.
 * \return Valeur de la commande.
 */
char* valeurCommande(char* commande);

#endif /* GTAPE_GTCONTROLCONSOLE_H_ */
