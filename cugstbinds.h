#ifndef CUGSTBINDS_H
#define CUGSTBINDS_H

#include <QObject>
#include <gst/gst.h>

class CuGstBinds : public QObject
{
Q_OBJECT
public:
    explicit CuGstBinds(QObject *parent = 0);
	bool checkElement(QString);

signals:

public slots:

};

#endif // CUGSTBINDS_H
