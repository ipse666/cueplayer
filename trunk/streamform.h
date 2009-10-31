#ifndef STREAMFORM_H
#define STREAMFORM_H

#include <QWidget>

namespace Ui {
    class StreamForm;
}

class StreamForm : public QWidget {
    Q_OBJECT
public:
    StreamForm(QWidget *parent = 0);
    ~StreamForm();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::StreamForm *ui;

private slots:
	void readUrl();

signals:
	void streamOk(QStringList);
};

#endif // STREAMFORM_H
