#ifndef CUEPLAYER_H
#define CUEPLAYER_H
#include <QWidget>
#include <QFileDialog>
#include <QSystemTrayIcon>
#include <QThread>
#include <QX11Info>
#include <QProcess>
#include <QtNetwork>
#include <gst/interfaces/xoverlay.h>
#include "ui_cueplayer.h"
#include "cueparser.h"
#include "transcoder.h"
#include "apetoflac.h"
#include "videowindow.h"
#include "streamform.h"

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
	void setDvdAudio(gchar*, int);
	gchar* getDvdAudio(int);
private:
	GstThread *trd;
	void seekAndLCD(int);
	void createTrayIconMenu();
	void enableButtons(bool);
	void checkState();
	int getDuration();
	bool playProbe();
	void createDvdPipe();
	void initPlayer();
	QFileDialog *filedialog;
	QTreeWidgetItem *playlistItem[100];
	QSystemTrayIcon *trayIcon;
	QMenu *trayIconMenu;
	QAction *quitAction;
	QAction *aboutAction;
	QAction *extbutAction;
	QAction *transcodeAction;
	QAction *apetoflacAction;
	QWidget *desktop;
	QString filename;
	QTimer *timer;
	QStringList filters;
	QString mp3trackName;
	QSettings settings;
	QX11Info *xinfo;
	QString dvdAu[20];
	QFileInfoList saveFileList;
	QProcess *videoProcess;
	QString primaryDPMS;
	QNetworkAccessManager *manager;
	TransCoder *transcoder;
	ApeToFlac *apetoflac;
	VideoWindow *videowindow;
	StreamForm *streamform;
	int numTrack;
	int totalTime;
	int d_title;
	CueParser *refparser;
	GMainLoop *loop;
	GstElement *play;
	GstElement *videosink;
	GstElement *demuxer;
	GstBus *bus;
	int multiFiles[100];
private slots:
	void setNumLCDs(int);
	void cueFileSelected(QStringList);
	void discSet();
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
	void multiFileInit(QFileInfoList);
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
	void dtsPlayer();
	void extButtons(bool);
	void endBlock();
	void dpmsTrigger(bool);
	QString checkDPMS();
	QFileInfoList m3uParse(QString);
	void readNmReply(QNetworkReply*);
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