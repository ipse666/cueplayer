#ifndef APPENDQUESTION_H
#define APPENDQUESTION_H

#include <QWidget>
#include <QFileInfoList>
#include <QSettings>
#include "ui_appendquestion.h"

class AppendQuestion : public QDialog, public Ui::AppendQuestion
{
    Q_OBJECT
public:
    explicit AppendQuestion(QWidget *parent = 0);
    void setFileInfoLists(QFileInfoList, QFileInfoList);

private:
    QFileInfoList savedFileInfoList;
    QFileInfoList newFileInfoList;

private slots:
    void appendSlot();
    void reloadSlot();
    void checkedSave(bool);

signals:
    void submit(QFileInfoList);

public slots:

};

#endif // APPENDQUESTION_H
