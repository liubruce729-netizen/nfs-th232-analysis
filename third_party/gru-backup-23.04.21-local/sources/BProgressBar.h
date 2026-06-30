/**
 * \file BProgressBar.h
 * \date 24 mai 2018
 * \author goux
 */

#ifndef BLOCKTEST_BPROGRESSBAR_H_
#define BLOCKTEST_BPROGRESSBAR_H_
#include <iostream>

/**
 * \brief Cette classe affiche une barre de chargement dans le terminal.
 * \class BProgressBar
 */
class BProgressBar {
private:
	//const int mSize = 51;
	//const char mSymbol = '*';
	/**
	 * Nombre total de valeur.
	 */
	int mTotal;

	/**
	 * Nombre de valeurs comptées.
	 */
	int mValeur;

	/**
	 * Pourcentage actuellement affiché.
	 */
	int mCurrentPercent;




public:
	/**
	 * Construteur.
	 * @param total Nombre total de valeur.
	 */
	BProgressBar(int total);

	/**
	 * Affiche un pourcent en plus si la valeur est dans un nouveau pourcentage.
	 */
	void displayPercent();

	/**
	 * Affiche l'en-tête de la bar de chargement.
	 */
	void displayProgressBarHeader() const;

	/**
	 * Destructeur.
	 */
	virtual ~BProgressBar();

	/**
	 * Ajoute un à la valeur courante et met à jour l'affichage.
	 * @return adresse de l'instance.
	 */
	BProgressBar& operator++();

	/**
	 * Ajoute un à la valeur courante et met à jour l'affichage.
	 * @return adresse d'une copie de cette instance comme avant l'incrémentation.
	 */
	BProgressBar operator++(int);
};

#endif /* BLOCKTEST_BPROGRESSBAR_H_ */
