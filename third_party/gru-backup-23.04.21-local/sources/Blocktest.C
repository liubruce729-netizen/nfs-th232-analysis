/*
 * GTConsole.C
 *
 *  Created on: 9 avr. 2018
 *      Author: goux
 */

#include "Blocktest.h"
#include "General.h"
#include "GRUCore.h"
#include "GBase.h"
using namespace std;

int main(int argc, char **argv) {
	BControl controleur(argv, argc);
	controleur.Executer();
}

