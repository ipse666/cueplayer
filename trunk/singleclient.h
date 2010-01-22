#ifndef SINGLECLIENT_H
#define SINGLECLIENT_H

#include <QObject>
#include <QLocalSocket>
#include <QApplication>

class SingleClient : public QObject
{
Q_OBJECT
public:
	explicit SingleClient(QObject *parent = 0);
	void connServer(QString);
	QString getState();
	void setArgs(QByteArray);

private:
	QLocalSocket *socket;

private slots:
	void paramSender();

signals:

public slots:

};

#endif // SINGLECLIENT_H
