#include "singleclient.h"

SingleClient::SingleClient(QObject *parent) :
	QObject(parent)
{
	socket = new QLocalSocket(this);
	connect(socket, SIGNAL(connected()), this, SLOT(paramSender()));
	connect(socket, SIGNAL(disconnected()),
				 socket, SLOT(deleteLater()));
}

void SingleClient::connServer(QString s)
{
	socket->connectToServer(s);
}

void SingleClient::paramSender()
{
	qDebug() << trUtf8("Отправляю аргуметы запущенной копии проигрывателя.");
}

QString SingleClient::getState()
{
	switch (socket->state())
	{
		case QLocalSocket::UnconnectedState:
			return "UnconnectedState";
		case QLocalSocket::ConnectingState:
			return "ConnectingState";
		case QLocalSocket::ConnectedState:
			return "ConnectedState";
		case QLocalSocket::ClosingState:
			return "ClosingState";
		default:
			return "undef";
	}
}

void SingleClient::setArgs(QByteArray a)
{
	socket->write(a);
	socket->flush();
	socket->disconnectFromServer();
}
