#ifndef CUEPLAYER_H
#define CUEPLAYER_H
#include <QWidget>
#include <QFileDialog>
#include <QSystemTrayIcon>
#include <QThread>
#include <QX11Info>
#include <gst/interfaces/xoverlay.h>
#include "ui_cueplayer.h"
#include "cueparser.h"
#include "transcoder.h"
#include "apetoflac.h"
#include "videowindow.h"

class GstThread;

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
	void paramFile(QStringList);
private:
	GstThread *trd;
	void seekAndLCD(int);
	void createTrayIconMenu();
	void enableButtons(bool);
	void checkState();
	int getDuration();
	void playProbe();
	void createDvdPipe();
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
	QSettings settings;
	QX11Info *xinfo;
	TransCoder *transcoder;
	ApeToFlac *apetoflac;
	VideoWindow *videowindow;
	int numTrack;
	int totalTime;
	CueParser *refparser;
	GMainLoop *loop;
	GstElement *play;
	GstElement *videosink;
	GstBus *bus;
	int multiFiles[100];
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
	void multiCueInit();
	void restoreSettings();
	int getPosition();
	void nminSeek();
	void nmidSeek();
	void nmaxSeek();
	void pminSeek();
	void pmidSeek();
	void pmaxSeek();
	void setAid(int);
	void setTid(int);
	void sliderVideoRelease();
	void fileDialogFilter(QString);
signals:
	void gstError();
};

class GstThread : public QThread
{
	Q_OBJECT

public:
	GstThread(QObject * parent = 0);
protected:
	void run();
};

#endif // __CUEPLAYER_H__
