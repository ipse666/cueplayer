#include <QApplication>
#include <QMessageBox>
#include "cueplayer.h"

int main(int argc, char *argv[])
{
	QDir currentdir;
	QRegExp rxPath("^/.*");
	QString arg = QObject::trUtf8(argv[1]);
	QApplication app(argc, argv);
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
		if (rxPath.indexIn(arg) != -1)
			file << arg;
		else
			file << currentdir.currentPath() << "/" << arg;
		player->paramFile(file);
	}
	return app.exec();
}
