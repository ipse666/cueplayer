#ifndef CUEPLAYER_H
#define CUEPLAYER_H
#include <QWidget>
#include <QFileDialog>
#include <QSystemTrayIcon>
#include "ui_cueplayer.h"
#include "cueparser.h"
#include "transcoder.h"
#include "apetoflac.h"


class CuePlayer : public QWidget, public Ui::CuePlayer
{
	Q_OBJECT

protected:
	void closeEvent(QCloseEvent *event);

public:
	CuePlayer(QWidget *parent = 0);
	void stopAll();
	void setMp3Title(GValue *, GValue *, GValue *);
	void apeFound(bool);
private:
	void seekAndLCD(int);
	int getDuration();
	void createTrayIconMenu();
	void enableButtons(bool);
	void checkState();
	QFileDialog *filedialog;
	QTreeWidgetItem *playlistItem[100];
	QSystemTrayIcon *trayIcon;
	QMenu *trayIconMenu;
	QAction *quitAction;
	QAction *aboutAction;
	QAction *transcodeAction;
	QAction *apetoflacAction;
	QWidget *desktop;
	QString filename;
	QTimer *timer;
	QStringList filters;
	QString mp3trackName;
	TransCoder *transcoder;
	ApeToFlac *apetoflac;
	int numTrack;
	int totalTime;
	CueParser *refparser;
	GMainLoop *loop;
	GstElement *play;
	GstBus *bus;
private slots:
	void setNumLCDs(int);
	void cueFileSelected(QStringList);
	void playNextTrack();
	void playPrewTrack();
	void playTrack();
	void stopTrack();
	void pauseTrack();
	void tick(qint64);
	void sliderRelease();
	void volumeValue(int);
	void listItemClicked(QTreeWidgetItem *, int);
	void initAlbum(int);
	void initFile();
	void trayClicked(QSystemTrayIcon::ActivationReason);
	void about();
	void timerUpdate();
	void seekGst(int);
	void preInit(QString);
signals:
	void gstError();
};

#endif // __CUEPLAYER_H__
