#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QWidget>
#include <QSettings>

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

signals:
	void settingsApply(bool);
};

#endif // PREFERENCES_H
