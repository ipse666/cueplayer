#include <QApplication>
#include <QMessageBox>
#include "cueplayer.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	if (!QSystemTrayIcon::isSystemTrayAvailable()) {
		QMessageBox::critical(0, QObject::trUtf8("Лоток"),
		QObject::trUtf8("Не найден системный лоток в вашей системе."
						"<p>Вероятна некорректная работа приложения."
						"<p>Работа приложения будет продолжена."));
		//return 1;
	}
	else
		QApplication::setQuitOnLastWindowClosed(false);
	app.setApplicationName("cueplayer");
	gst_init(0,0);
	gst_registry_fork_set_enabled(true);
	CuePlayer *player = new CuePlayer;
	player->show();
	return app.exec();
}
