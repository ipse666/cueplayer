#include "equalizer.h"
#include "ui_equalizer.h"

Equalizer::Equalizer(QWidget *parent) :
	QWidget(parent),
    ui(new Ui::Equalizer)
{
    ui->setupUi(this);
	connect(ui->band0, SIGNAL(valueChanged(int)), this, SLOT(valueChan(int)));
	connect(ui->band1, SIGNAL(valueChanged(int)), this, SLOT(valueChan(int)));
	connect(ui->band2, SIGNAL(valueChanged(int)), this, SLOT(valueChan(int)));
	connect(ui->band3, SIGNAL(valueChanged(int)), this, SLOT(valueChan(int)));
	connect(ui->band4, SIGNAL(valueChanged(int)), this, SLOT(valueChan(int)));
	connect(ui->band5, SIGNAL(valueChanged(int)), this, SLOT(valueChan(int)));
	connect(ui->band6, SIGNAL(valueChanged(int)), this, SLOT(valueChan(int)));
	connect(ui->band7, SIGNAL(valueChanged(int)), this, SLOT(valueChan(int)));
	connect(ui->band8, SIGNAL(valueChanged(int)), this, SLOT(valueChan(int)));
	connect(ui->band9, SIGNAL(valueChanged(int)), this, SLOT(valueChan(int)));
	connect(ui->okButton, SIGNAL(clicked()), this, SLOT(saveValue()));
}

Equalizer::~Equalizer()
{
    delete ui;
}

void Equalizer::changeEvent(QEvent *e)
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

void Equalizer::valueChan(int i)
{
	double band[10];

	QWidget *widget = qobject_cast<QWidget*>(sender());
	widget->setToolTip(QString::number(i));

	getValue(band);
	emit bandsValue(band);
}

void Equalizer::saveValue()
{
	QSettings settings;
	double band[10];
	getValue(band);

	for (int i = 0; i < 10; i++)
		settings.setValue("equalizer/band"+QString::number(i), band[i]);
	close();
}

void Equalizer::getValue(double* band)
{
	band[0] = ui->band0->value();
	band[1] = ui->band1->value();
	band[2] = ui->band2->value();
	band[3] = ui->band3->value();
	band[4] = ui->band4->value();
	band[5] = ui->band5->value();
	band[6] = ui->band6->value();
	band[7] = ui->band7->value();
	band[8] = ui->band8->value();
	band[9] = ui->band9->value();
}

void Equalizer::restoreValue()
{
	double band[10];
	QSettings settings;

	ui->band0->setValue(settings.value("equalizer/band0").toInt());
	ui->band1->setValue(settings.value("equalizer/band1").toInt());
	ui->band2->setValue(settings.value("equalizer/band2").toInt());
	ui->band3->setValue(settings.value("equalizer/band3").toInt());
	ui->band4->setValue(settings.value("equalizer/band4").toInt());
	ui->band5->setValue(settings.value("equalizer/band5").toInt());
	ui->band6->setValue(settings.value("equalizer/band6").toInt());
	ui->band7->setValue(settings.value("equalizer/band7").toInt());
	ui->band8->setValue(settings.value("equalizer/band8").toInt());
	ui->band9->setValue(settings.value("equalizer/band9").toInt());

	getValue(band);
	emit bandsValue(band);
}
