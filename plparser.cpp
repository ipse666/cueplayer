#include "plparser.h"

PlParser::PlParser(QObject *parent) : QObject(parent)
{
	manager = new QNetworkAccessManager(this);

	connect(manager, SIGNAL(finished(QNetworkReply*)),
	 this, SLOT(readNmReply(QNetworkReply*)));
}

void PlParser::setPlUri(QString uri)
{
	QRegExp rxUrl("^\\w{3,5}://.*");
	QRegExp rxFilePath("^/.*");

	QFileInfo fi(uri);
	entries = 0;

	if (rxUrl.indexIn(uri) != -1)
	{
		QUrl urlpoint = QUrl(uri);
		if (urlpoint.isValid())
			manager->get(QNetworkRequest(urlpoint));
		else
			emit plperror(trUtf8("Неверный url:\n") + urlpoint.toString());
	}
	else if (rxFilePath.indexIn(uri) != -1)
	{
		QFile plfile(uri);
		QString line;
		QStringList linelist;

		if (!plfile.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			emit plperror(trUtf8("Ошибка открытия файла:\n") + uri);
			return;
		}
		QTextStream textstream(&plfile);
		do {
			line = textstream.readLine();
			linelist << line;
		} while (!line.isNull());
		plfile.close();

		if (fi.suffix() == "pls")
			parsePls(linelist);
		else if(fi.suffix() == "wvx")
			parseWvx(linelist);
		else if(fi.suffix() == "m3u")
			parseM3u(linelist);
		else
			emit plperror(trUtf8("Неизвестный формат файла:\n") + uri);
	}
	else
	{
		emit plperror(trUtf8("Тип uri неизвестен:\n") + uri);
	}
}

unsigned short PlParser::getPlEntries()
{
	return entries;
}

QStringList PlParser::getPlStruct(unsigned short ind)
{
	return QStringList() << playlist[ind].uri << playlist[ind].title;
}

void PlParser::readNmReply(QNetworkReply *reply)
{
	QStringList stringlist;
	QUrl url = reply->url();
	QFileInfo fi(url.toString());
	QString str;
	QByteArray result = reply->readAll();
	reply->close();
	QList<QByteArray> out = result.split('\n');
	foreach (QByteArray bastr, out)
	{
		str = "";
		str.append(bastr);
		if (!str.isEmpty())
			stringlist << str.trimmed();
	}

	if (fi.suffix() == "pls")
		parsePls(stringlist);
	else if(fi.suffix() == "wvx")
		parseWvx(stringlist);
	else if(fi.suffix() == "m3u")
		parseM3u(stringlist);
	else
		emit plperror(trUtf8("Неизвестный формат файла:\n") + url.toString());
}

void PlParser::parsePls(QStringList list)
{
	QRegExp rxEntry("numberofentries=(\\d+)");
	rxEntry.setCaseSensitivity(Qt::CaseInsensitive);
	QRegExp rxUri("File\\d+=(.*)");
	QRegExp rxTitle("Title\\d+=(.*)");

	int ind = 0;

	foreach (QString str, list)
	{
		if (rxEntry.indexIn(str) != -1)
			entries = rxEntry.cap(1).toShort(0,10);
		if (rxUri.indexIn(str) != -1)
			playlist[ind].uri = rxUri.cap(1);
		if (rxTitle.indexIn(str) != -1)
			playlist[ind++].title = rxTitle.cap(1);
	}

	if (ind || entries)
		emit ready();
	else
		emit plperror(trUtf8("Ноль записей"));
}

void PlParser::parseWvx(QStringList list)
{
	int ind = 0;
	bool title = false;
	bool entry = false;

	QString fulldoc = list.join("\n");
	QXmlStreamReader xml(fulldoc);
	while (!xml.atEnd())
	{
		QString characters, attributes;
		QXmlStreamReader::TokenType tokentype = xml.readNext();
		switch (tokentype)
		{
			case QXmlStreamReader::StartElement:
				if (xml.name() == "ref")
					attributes = xml.attributes().value("href").toString();
				else if (xml.name() == "Title")
					title = true;
				else if (xml.name() == "entry")
					entry = true;
				break;
			case QXmlStreamReader::EndElement:
				if (xml.name() == "entry")
				{
					ind++;
					entry = false;
				}
				else if (xml.name() == "Title")
					title = false;
				break;
			case QXmlStreamReader::Characters:
				if (title && entry) characters = xml.text().toString();
				break;
			default:
				break;
		}
		if (!attributes.isNull()) playlist[ind].uri = attributes;
		if (!characters.isNull()) playlist[ind].title = characters;
	}

	entries = ind;

	if (ind)
		emit ready();
	else
		emit plperror(trUtf8("Ноль записей"));
}

void PlParser::parseM3u(QStringList list)
{
	QRegExp rxComment("^#.*");
	int ind = 0;

	foreach (QString line, list)
	{
		if (rxComment.indexIn(line) != -1)
			continue;
		playlist[ind].uri = line;
		playlist[ind].title = line;
		ind++;
	}
	entries = ind;

	if (ind)
		emit ready();
	else
		emit plperror(trUtf8("Ноль записей"));
}
