#include <QApplication>
#include "cueplayer.h"

bool isAlreadyRunning(QApplication *appref)
{
	QRegExp rxFile("/([^/]*)$");
	QFile pidfile;

	if (rxFile.indexIn(appref->applicationFilePath()) != -1)
		pidfile.setFileName(QDir::homePath() + "/." + rxFile.cap(1) + ".pid");
	if (!pidfile.exists())
	{
		qDebug() << QObject::trUtf8("pid файл не найден");
		if (!pidfile.open(QFile::WriteOnly))
			qDebug() << QObject::trUtf8("Не удалось создать pid файл") << pidfile.fileName()
			<< pidfile.errorString();
		QTextStream pidstream(&pidfile);
		pidstream << appref->applicationPid();
		pidfile.close();
	}
	else
	{
		pidfile.open(QFile::ReadOnly);
		qint64 pid = pidfile.readLine().toInt();
		pidfile.close();
		QProcess::execute("kill", QStringList() << QString::number(pid));
		pidfile.open(QFile::WriteOnly);
		QTextStream pidstream(&pidfile);
		pidstream << appref->applicationPid();
		pidfile.close();
	}
	return true;
}

int main(int argc, char *argv[])
{
	QDir currentdir;
	QRegExp rxPath("^/.*");
	QRegExp rxFilename3("^(mms://|http://|ftp://).*");

	QString arg = QObject::trUtf8(argv[1]);
	QApplication app(argc, argv);
	isAlreadyRunning(&app);
	if (!QSystemTrayIcon::isSystemTrayAvailable()) {
		/*QMessageBox::critical(0, QObject::trUtf8("Лоток"),
		QObject::trUtf8("Не найден системный лоток в вашей системе."
						"<p>Вероятна некорректная работа приложения."
						"<p>Работа приложения будет продолжена."));*/
		//return 1;
	}
	else
		QApplication::setQuitOnLastWindowClosed(false);
	QCoreApplication::setOrganizationName("CuePlayer");
	app.setApplicationName("cueplayer");
	gst_init(0,0);
	gst_registry_fork_set_enabled(true);
	CuePlayer *player = new CuePlayer;
	player->show();
	if(argc == 2)
	{
		QStringList file;
		if (rxPath.indexIn(arg) != -1 ||
			rxFilename3.indexIn(arg) != -1)
			file << arg;
		else
			file << currentdir.currentPath() << "/" << arg;
		player->paramFile(file);
	}
	return app.exec();
}