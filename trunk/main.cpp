#include <QApplication>
#include <QMessageBox>
#include "cueplayer.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	if (!QSystemTrayIcon::isSystemTrayAvailable()) {
		QMessageBox::critical(0, QObject::trUtf8("Лоток"),
		QObject::trUtf8("Не найден системный лоток в вашей системе."));
		return 1;
	}
	QApplication::setQuitOnLastWindowClosed(false);
	app.setApplicationName("cueplayer");
	CuePlayer *player = new CuePlayer;
	player->show();
	return app.exec();
}
