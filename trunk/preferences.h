#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QWidget>
#include <QSettings>
#include <QTreeWidgetItem>

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

private:
    Ui::Preferences *ui;

private slots:
	void saveSettings();
	void readSettings();
	void prefDeci(int);
	void setDefault();
	void listItemClicked(QTreeWidgetItem *, int);

signals:
	void settingsApply(bool);
};

#endif // PREFERENCES_H
