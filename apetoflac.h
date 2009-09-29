#ifndef APETOFLAC_H
#define APETOFLAC_H

#include <QDialog>
#include <QTimer>
#include <QTextStream>
#include <QFile>
#include <gst/gst.h>
#include "ui_apetoflacdialog.h"

class ApeToFlac : public QDialog, public Ui::ApeToFlac
{
	Q_OBJECT

public:
	ApeToFlac(QWidget *parent = 0);
	void setFileNames(QString, QString);
	void stopCode();
	void decoding();
private:
	QString atfCuefile;
	QString atfApefile;
	QString outFile;
	QString outCue;
	QTimer *timer;
	void initDecoder();
	int getDuration();
	int saveTotalTime;
	GstElement *atfpipe;
private slots:
	void startDecoder();
	void progressUpd();
signals:
	void endHint(QStringList);
};

#endif // APETOFLAC_H
