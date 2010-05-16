#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QWidget>
#include <QSettings>
#include <QTreeWidgetItem>
#include "cugstbinds.h"

namespace Ui {
    class Preferences;
}

class Preferences : public QWidget {
    Q_OBJECT
public:
    Preferences(QWidget *parent = 0);
	void setTrPref();
    ~Preferences();

protected:
    void changeEvent(QEvent *e);
	void resizeEvent(QResizeEvent  *e);

private:
    Ui::Preferences *ui;
	CuGstBinds *cugstbinds;

private slots:
	void saveSettings();
	void readSettings();
	void prefDeci(int);
	void prefDeci(double);
	void setDefault();
	void listItemClicked(QTreeWidgetItem *, int);

signals:
	void settingsApply(bool);
};

#endif // PREFERENCES_H
