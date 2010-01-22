#include "singleserver.h"

SingleServer::SingleServer(QObject *parent) :
	QObject(parent)
{
	server = new QLocalServer(this);
}

void SingleServer::startServer()
{
	if (!server->listen("cueplayer")) {
			 qDebug() << tr("CuePlayer Server") << trUtf8("Ошибка запуска сервера: %1.").arg(server->errorString());
			 QLocalServer::removeServer("cueplayer");
			 return;
	}
	connect(server, SIGNAL(newConnection()), this, SLOT(newConn()));
}

void SingleServer::newConn()
{
	clientConnection = server->nextPendingConnection();
	connect(clientConnection, SIGNAL(readyRead()), this, SLOT(readArgs()));
	connect(clientConnection, SIGNAL(disconnected()), clientConnection, SLOT(deleteLater()));
}

void SingleServer::readArgs()
{
	QDir currentdir;
	QRegExp rxPath("^/.*");
	QRegExp rxFilename3("^(mms://|http://|ftp://).*");
	QStringList file;

	QString arg = trUtf8(clientConnection->readLine());

	if (rxPath.indexIn(arg) != -1 ||
		rxFilename3.indexIn(arg) != -1)
		file << arg;
	else
		file << currentdir.currentPath() << "/" << arg;
	emit sendArgs(file);
	clientConnection->flush();
	clientConnection->disconnectFromServer();
}
