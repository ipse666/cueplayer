#include <QApplication>
#include <QTranslator>
#include "cueplayer.h"
#include "singleclient.h"

#define APPVERSION "0.24"

QList<int> checkrevision()
{
	QRegExp rxData("(\\d{4})-(\\d{2})-(\\d{2}).*");
	QRegExp rxRev("^(\\d+)$");
	QFile entfile("./.svn/entries");
	int year, month, day, rev;
	year = month = day = rev = 0;
	bool readnextline = false;
	QList<int> datarev;
	QString line;

	if (!entfile.open(QIODevice::ReadOnly | QIODevice::Text))
				return datarev;
	QTextStream enttext(&entfile);

	do {
		line = enttext.readLine();
		if (rxData.indexIn(line) != -1)
		{
			if (rxData.cap(1).toInt() >= year)
			{
				if (rxData.cap(1).toInt() > year)
					month = day = 0;
				year = rxData.cap(1).toInt();
				if (rxData.cap(2).toInt() >= month)
				{
					if (rxData.cap(2).toInt() > month)
						day = 0;
					month = rxData.cap(2).toInt();
					if (rxData.cap(3).toInt() >= day)
					{
						day = rxData.cap(3).toInt();
						readnextline = true;
					}
					else
						readnextline = false;
				}
				else
				{
					readnextline = false;
				}
			}
			else
			{
				readnextline = false;
			}
		}
		else if (rxRev.indexIn(line) != -1 && readnextline)
		{
			readnextline = false;
			rev = rxRev.cap(1).toInt();
		}

	} while (!line.isNull());

	entfile.close();
	datarev << day << month << year << rev;
	return datarev;
}

int main(int argc, char *argv[])
{
	bool tray;
	QDir currentdir;
	QRegExp rxPath("^/.*");
	QRegExp rxFilename3("^(mms://|http://|ftp://).*");
	QList<int> datarev;
	QString fullver;

	QString arg = QObject::trUtf8(argv[1]);
	QApplication app(argc, argv);
	SingleServer *sserver = new SingleServer;
	SingleClient *sclient = new SingleClient;
	sclient->connServer("cueplayer");
	if (sclient->getState() == "ConnectedState")
	{
		if (argc >= 2)
			sclient->setArgs(QByteArray(argv[1]));
		return 0;
	}
	else if (sclient->getState() == "UnconnectedState")
	{
		sserver->startServer();
	}
	else
	{
		qDebug() << QObject::trUtf8("Ошибка. Отпишитесь разработчику.");
		return 1;
	}

	if (!QSystemTrayIcon::isSystemTrayAvailable()) {
		/*QMessageBox::critical(0, QObject::trUtf8("Лоток"),
		QObject::trUtf8("Не найден системный лоток в вашей системе."
						"<p>Вероятна некорректная работа приложения."
						"<p>Работа приложения будет продолжена."));
		return 1;*/
		tray = false;
	}
	else
	{
		QApplication::setQuitOnLastWindowClosed(false);
		tray = true;
	}
	QCoreApplication::setOrganizationName("CuePlayer");
	QTranslator myappTranslator;
		 myappTranslator.load("/usr/share/cueplayer/loc/cueplayer_" + QLocale::system().name().left(2));
		 app.installTranslator(&myappTranslator);
	datarev = checkrevision();
	fullver = APPVERSION;
#ifdef BUILDER
	if (!datarev.isEmpty())
	{
		fullver += "-r";
		fullver += REVISION;
	}
#endif
	app.setApplicationName("cueplayer");
	app.setApplicationVersion(fullver);
	gst_init(0,0);
	gst_registry_fork_set_enabled(true);
	CuePlayer *player = new CuePlayer;
	player->setTray(tray);
	player->setServer(sserver);
	player->show();
	if(argc >= 2)
	{
		QStringList file;
		QString fullfile = currentdir.currentPath() + "/" + arg;
		if (rxPath.indexIn(arg) != -1 ||
			rxFilename3.indexIn(arg) != -1)
			file << arg;
		else
			file << fullfile;
		player->paramFile(file);
	}
	return app.exec();
}
