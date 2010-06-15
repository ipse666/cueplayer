#ifndef APETOFLAC_H
#define APETOFLAC_H

#include <QDialog>
#include <QTimer>
#include <QTextStream>
#include <QTextCodec>
#include <QFile>
#include <QDebug>
#include <gst/gst.h>
#include "cueparser.h"
#include "ui_apetoflacdialog.h"

class ApeToFlac : public QDialog, public Ui::ApeToFlac
{
	Q_OBJECT

public:
	ApeToFlac(QWidget *parent = 0);
        void setFileNames(QString, CueParser*);
	void stopCode();
	void decoding();
private:
	QString atfCuefile;
	QString atfApefile;
	QString outFile;
	QString outCue;
        QString internalcue;
	QTimer *timer;
	QTextEncoder *localFileNamesEncoder;
        void initDecoder(CueParser*);
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
