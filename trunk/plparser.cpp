#include "plparser.h"

PlParser::PlParser(QObject *parent) : QObject(parent)
{
	manager = new QNetworkAccessManager(this);

	connect(manager, SIGNAL(finished(QNetworkReply*)),
	 this, SLOT(readNmReply(QNetworkReply*)));
}

void PlParser::setPlsUri(QString uri)
{
	QRegExp rxUrl("^\\w{3,5}://.*");
	QRegExp rxFilePath("^/.*");

	if (rxUrl.indexIn(uri) != -1)
	{
		QUrl urlpoint = QUrl(uri);
		if (urlpoint.isValid())
			manager->get(QNetworkRequest(urlpoint));
		else
			emit plperror();
	}
	else if (rxFilePath.indexIn(uri) != -1)
	{
		QFile plfile(uri);
		QString line;
		QStringList linelist;

		if (!plfile.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			emit plperror();
			return;
		}
		QTextStream textstream(&plfile);
		do {
			line = textstream.readLine();
			linelist << line;
		} while (!line.isNull());
		parsePls(linelist);

		plfile.close();
	}
	else
	{
		emit plperror();
	}
}

unsigned short PlParser::getPlsEntries()
{
	return entries;
}

QStringList PlParser::getPlsStruct(unsigned short ind)
{
	return QStringList() << playlist[ind].uri << playlist[ind].title;
}

void PlParser::readNmReply(QNetworkReply *reply)
{
	QStringList stringlist;
	QString str;
	QByteArray result = reply->readAll();
	reply->close();
	QList<QByteArray> out = result.split('\n');
	while (out.size())
	{
		str = "";
		QByteArray bastr = out.last();
		out.pop_back();
		str.append(bastr);
		if (!str.isEmpty())
			stringlist << str.trimmed();
	}
}

void PlParser::parsePls(QStringList list)
{
	QRegExp rxEntry("numberofentries=(\\d+)");
	QRegExp rxUri("File\\d+=(.*)");
	QRegExp rxTitle("Title\\d+=(.*)");

	int ind = 0;

	foreach (QString str, list)
	{
		if (rxEntry.indexIn(str) != -1)
			entries = rxEntry.cap(1).toShort(0,10);
		if (rxUri.indexIn(str) != -1)
			playlist[++ind].uri = rxUri.cap(1);
		if (rxTitle.indexIn(str) != -1)
			playlist[ind].title = rxTitle.cap(1);
	}

	if (ind)
		emit ready();
	else
		emit plperror();
}
