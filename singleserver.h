#ifndef SINGLESERVER_H
#define SINGLESERVER_H

#include <QObject>
#include <QThread>
#include <QLocalServer>
#include <QLocalSocket>
#include <QRegExp>
#include <QStringList>
#include <QDir>

class SingleServer : public QObject
{
Q_OBJECT
public:
	explicit SingleServer(QObject *parent = 0);
	void startServer();

private:
	QLocalServer *server;
	QLocalSocket *clientConnection;

protected:


signals:
	void sendArgs(QStringList);

public slots:

private slots:
	void newConn();
	void readArgs();

};

#endif // SINGLESERVER_H
