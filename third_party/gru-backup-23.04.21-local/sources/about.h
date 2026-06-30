/****************************************************************************
** Form interface generated from reading ui file 'about.ui'
**
** Created: Thu Jun 1 09:23:25 2006
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.4   edited Nov 24 2003 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef ABOUT_H
#define ABOUT_H

#include <qvariant.h>
#include <qdialog.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QLabel;
class QPushButton;

class About : public QDialog
{
    Q_OBJECT

public:
    About( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~About();

    QLabel* textLabel3;
    QLabel* textLabel1;
    QPushButton* buttonOk;
    QLabel* versionLabel;

public slots:
    virtual void okClicked();

protected:

protected slots:
    virtual void languageChange();

private:
    void init();

};

#endif // ABOUT_H
