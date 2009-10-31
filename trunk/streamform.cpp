#include "streamform.h"
#include "ui_streamform.h"

StreamForm::StreamForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StreamForm)
{
    ui->setupUi(this);
	this->setWindowFlags(Qt::Dialog);

	connect(ui->okButton, SIGNAL(clicked()), this, SLOT(readUrl()));
}

StreamForm::~StreamForm()
{
    delete ui;
}

void StreamForm::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void StreamForm::readUrl()
{
	QStringList url;
	url << ui->lineEdit->text();
	emit streamOk(url);
	close();
}
