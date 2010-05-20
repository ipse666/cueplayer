#ifndef CUGSTBINDS_H
#define CUGSTBINDS_H

#include <QObject>
#include <QTextCodec>
#include <gst/gst.h>

class CuGstBinds : public QObject
{
Q_OBJECT
public:
    explicit CuGstBinds(QObject *parent = 0);
	bool checkElement(QString);

private:
	QTextEncoder *localFileNamesEncoder;

signals:

public slots:

};

#endif // CUGSTBINDS_H
