/*
 * BProgressBar.cpp
 *
 *  Created on: 24 mai 2018
 *      Author: goux
 */

#include "BProgressBar.h"

BProgressBar::BProgressBar(int total) :
		mTotal(total), mValeur(0), mCurrentPercent(0) {
	if (mTotal == 0)
		mTotal = 1;
	//displayProgressBar();
}

BProgressBar::~BProgressBar() {
}

BProgressBar& BProgressBar::operator++() {
	mValeur++;
	displayPercent();
	return *this;
}

BProgressBar BProgressBar::operator++(int) {
	BProgressBar tmp(*this);
	operator++();
	return tmp;
}

void BProgressBar::displayProgressBarHeader() const {
	std::cout << "0%   10   20   30   40   50   60   70   80   90   100%"
			<< std::endl
			<< "|----|----|----|----|----|----|----|----|----|----|"
			<< std::endl;
}

void BProgressBar::displayPercent() {
	if (mValeur * 51 / mTotal > mCurrentPercent) {
		int i;
		for (i = 0; i < mValeur * 51 / mTotal - mCurrentPercent; i++)
			std::cout << '*';
		std::cout << std::flush;
		mCurrentPercent += i;
	}
}
