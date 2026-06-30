/****************************************************************************
** Form interface generated from reading ui file 'SetupPrint.ui'
**
** Created: Thu Jun 1 09:23:25 2006
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.4   edited Nov 24 2003 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef SETUPPRINT_H
#define SETUPPRINT_H

#include <qvariant.h>
#include <qdialog.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QPushButton;
class QGroupBox;
class QLineEdit;
class QLabel;
class QButtonGroup;
class QRadioButton;

class SetupPrint : public QDialog
{
    Q_OBJECT

public:
    SetupPrint( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~SetupPrint();

    QPushButton* buttonOk;
    QPushButton* buttonCancel;
    QGroupBox* groupBox19;
    QLineEdit* printerEdit;
    QLabel* textLabel1_2;
    QButtonGroup* buttonGroup3_2;
    QRadioButton* printRadio_pad;
    QRadioButton* printRadio_page;
    QButtonGroup* buttonGroup3;
    QRadioButton* printRadio_file;
    QRadioButton* printRadio_printer;

public slots:
    virtual void okClicked();

signals:
    void confPrint(const QString &, const QString &, const QString &) ;

protected:
    QHBoxLayout* Layout1;
    QSpacerItem* Horizontal_Spacing2;

protected slots:
    virtual void languageChange();

private:
    void init();

};

#endif // SETUPPRINT_H
