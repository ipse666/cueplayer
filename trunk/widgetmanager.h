#ifndef WIDGETMANAGER_H
#define WIDGETMANAGER_H

#include <QWidget>

class WidgetManager : public QWidget
{
Q_OBJECT
public:
    explicit WidgetManager(QWidget *parent = 0);
	void raiseWidget(QWidget *);

signals:

public slots:

};

#endif // WIDGETMANAGER_H
