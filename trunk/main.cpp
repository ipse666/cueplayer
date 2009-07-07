#include <QApplication>
#include "cueplayer.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	app.setApplicationName("cueplayer");
	CuePlayer *player = new CuePlayer;
	player->show();
	return app.exec();
}
