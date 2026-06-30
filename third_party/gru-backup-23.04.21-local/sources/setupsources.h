/****************************************************************************
** Form interface generated from reading ui file 'setupsources.ui'
**
** Created: Thu Jun 1 09:23:25 2006
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.4   edited Nov 24 2003 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef SETUPSOURCES_H
#define SETUPSOURCES_H

#include <qvariant.h>
#include <qdialog.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QPushButton;
class QLabel;
class QButtonGroup;
class QSpinBox;
class QRadioButton;
class QLineEdit;

class SetupSources : public QDialog
{
    Q_OBJECT

public:
    SetupSources( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~SetupSources();

    QPushButton* buttonOk;
    QPushButton* buttonCancel;
    QLabel* textLabel4;
    QButtonGroup* File_2;
    QPushButton* FileBroswer_2;
    QSpinBox* LowRangeSpinBox;
    QSpinBox* HighRangeSpinBox;
    QButtonGroup* File;
    QLabel* textLabelHistogramFile;
    QRadioButton* ganilFile;
    QRadioButton* gruFile;
    QPushButton* FileBroswer;
    QLineEdit* fileEdit;
    QButtonGroup* Network;
    QRadioButton* gruNet;
    QRadioButton* ganilNet;
    QRadioButton* noNet;
    QLineEdit* netEdit;

public slots:
    virtual void Ok_clicked();
    virtual void FileBroswer_clicked();
    virtual void ganilNet_clicked();
    virtual void gruNet_clicked();
    virtual void ganilFile_clicked();
    virtual void gruFile_clicked();
    virtual void fileEdit_lostFocus();

signals:
    void confSource(const QString &, const QString &,const QString &, const QString &,int,int);

protected:
    QGridLayout* SetupSourcesLayout;
    QHBoxLayout* Layout1;
    QSpacerItem* Horizontal_Spacing2;

protected slots:
    virtual void languageChange();

private:
    void init();

};

#endif // SETUPSOURCES_H
