#include "appendquestion.h"

AppendQuestion::AppendQuestion(QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);

    connect(appendButton, SIGNAL(clicked()), this, SLOT(appendSlot()));
    connect(reloadButton, SIGNAL(clicked()), this, SLOT(reloadSlot()));
    connect(rememberBox, SIGNAL(toggled(bool)), this, SLOT(checkedSave(bool)));
}

void AppendQuestion::setFileInfoLists(QFileInfoList savedfilelist, QFileInfoList newfilelist)
{
    savedFileInfoList = savedfilelist;
    newFileInfoList = newfilelist;
}

void AppendQuestion::appendSlot()
{
    QSettings settings;
    settings.setValue("appendquestion/append", true);
    savedFileInfoList.append(newFileInfoList);
    emit submit(savedFileInfoList);
    close();
}

void AppendQuestion::reloadSlot()
{
    QSettings settings;
    settings.setValue("appendquestion/append", false);
    emit submit(newFileInfoList);
    close();
}

void AppendQuestion::checkedSave(bool b)
{
    QSettings settings;
    settings.setValue("appendquestion/remember", b);
}
