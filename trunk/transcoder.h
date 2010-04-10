#ifndef TRANSCODER_H
#define TRANSCODER_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <gst/gst.h>
#include "ui_transcoder.h"
#include "cueparser.h"

class TransCoder : public QMainWindow, public Ui::TransCoder
{
	Q_OBJECT

public:
	TransCoder(QWidget *parent = 0);
	virtual void setFileName(QString, qint64, int);
	virtual void stopAllPub();

private:
	qint64 numTrack;
	CueParser *refparser;
	QTreeWidgetItem *playlistItem[100];
	bool selected;
	bool transcode;
	QLabel *statusLabel;
	QColor progressColor;
	QColor finishedColor;
	QColor errorColor;
	QTimer *timer;
	GMainLoop *loop;
	qint64 saveTotalTime;
	QFileDialog *dirdialog;
	QSettings settings;
	QFile tmpfile;
	void setTrack();
	void setTrackTime(qint64,qint64);
	void pipeRun(int);
	int defaultContainer;
	int defaultCodec;

private slots:
	void on_closeAction_triggered();
	void selectAllTrigger();
	void startTranscode();
	void stopTranscode();
	void timerUpdate();
	void formatError(int);
	void setDefaultIndex();
	void stopAll();
	void updateSettings();
	void setVbr();
	void restoreSettings();
	void toolItem(QTreeWidgetItem*,int);
signals:
	void transQuit();
};

#endif // TRANSCODER_H
