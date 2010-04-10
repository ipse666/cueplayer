#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QWidget>

namespace Ui {
    class Preferences;
}

class Preferences : public QWidget {
    Q_OBJECT
public:
    Preferences(QWidget *parent = 0);
    ~Preferences();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::Preferences *ui;
};

#endif // PREFERENCES_H
