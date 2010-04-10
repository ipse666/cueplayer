#include "preferences.h"
#include "ui_preferences.h"

enum Codec {
	AUTO,
	CP1251,
	UTF8
};

Preferences::Preferences(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Preferences)
{
    ui->setupUi(this);

	readSettings();

	connect(ui->okButton, SIGNAL(clicked()), this, SLOT(saveSettings()));
}

Preferences::~Preferences()
{
    delete ui;
}

void Preferences::changeEvent(QEvent *e)
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

void Preferences::saveSettings()
{
	QSettings settings;

	settings.setValue("preferences/integration", ui->integrationBox->isChecked());
	settings.setValue("preferences/equalizer", ui->equalizerBox->isChecked());
	settings.setValue("preferences/traytext", ui->trayTextBox->isChecked());

	settings.setValue("preferences/autocuec", ui->autoRadioButton->isChecked());
	settings.setValue("preferences/cpcuec", ui->cpRadioButton->isChecked());
	settings.setValue("preferences/utfcuec", ui->utfRadioButton->isChecked());

	emit settingsApply(ui->equalizerBox->isChecked());

	close();
}

void Preferences::readSettings()
{
	QSettings settings;

	ui->integrationBox->setChecked(settings.value("preferences/integration").toBool());
	ui->equalizerBox->setChecked(settings.value("preferences/equalizer").toBool());
	ui->trayTextBox->setChecked(settings.value("preferences/traytext").toBool());

	ui->autoRadioButton->setChecked(settings.value("preferences/autocuec").toBool());
	ui->cpRadioButton->setChecked(settings.value("preferences/cpcuec").toBool());
	ui->utfRadioButton->setChecked(settings.value("preferences/utfcuec").toBool());
}

int Preferences::checkCodec()
{
	QSettings settings;
	int codec = AUTO;

	if (settings.value("preferences/autocuec").toBool())
		codec = AUTO;
	else if (settings.value("preferences/cpcuec").toBool())
		codec = CP1251;
	else if (settings.value("preferences/utfcuec").toBool())
		codec = UTF8;

	return codec;
}
