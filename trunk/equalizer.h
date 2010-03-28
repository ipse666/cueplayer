#ifndef EQUALIZER_H
#define EQUALIZER_H

#include <QWidget>
#include <QSettings>

namespace Ui {
    class Equalizer;
}

class Equalizer : public QWidget {
    Q_OBJECT
public:
    Equalizer(QWidget *parent = 0);
	void restoreValue();
    ~Equalizer();

protected:
    void changeEvent(QEvent *e);

private:
	Ui::Equalizer *ui;

private slots:
	void valueChan(int);
	void saveValue();
	void getValue(double*);

signals:
	void bandsValue(double*);
};

#endif // EQUALIZER_H
