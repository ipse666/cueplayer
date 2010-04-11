#include "preferences.h"
#include "ui_preferences.h"

Preferences::Preferences(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Preferences)
{
    ui->setupUi(this);

	readSettings();

	connect(ui->okButton, SIGNAL(clicked()), this, SLOT(saveSettings()));
	connect(ui->vorbisQuaSlider, SIGNAL(valueChanged(int)), this, SLOT(prefDeci(int)));
	connect(ui->defaultButton, SIGNAL(clicked()), this, SLOT(setDefault()));
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

	// Основное. Видео
	settings.setValue("preferences/integration", ui->integrationBox->isChecked());
	// Основное. Аудио
	settings.setValue("preferences/equalizer", ui->equalizerBox->isChecked());
	settings.setValue("preferences/traytext", ui->trayTextBox->isChecked());
	settings.setValue("preferences/cover",ui->coverBox->isChecked());

	// Основное. Кодировка CUE файла
	settings.setValue("preferences/autocuec", ui->autoRadioButton->isChecked());
	settings.setValue("preferences/cpcuec", ui->cpRadioButton->isChecked());
	settings.setValue("preferences/utfcuec", ui->utfRadioButton->isChecked());


	// Транскодер. vorbisenc
	settings.setValue("preferences/vorbisquality", ui->vorbisQuaSlider->value());

	// Транскодер. lame
	settings.setValue("preferences/lamequality", ui->lameQuaSlider->value());
	// Транскодер. lame vbr
	settings.setValue("preferences/lamevbrquality", ui->lameVbrQuaSlider->value());

	// Транскодер. flacenc
	settings.setValue("preferences/flacquality", ui->flacQuaSlider->value());

	// Транскодер. faac
	settings.setValue("preferences/faacquality", ui->comboBox->currentIndex());

	emit settingsApply(ui->equalizerBox->isChecked());

	close();
}

void Preferences::readSettings()
{
	QSettings settings;

	// Основное. Видео
	ui->integrationBox->setChecked(settings.value("preferences/integration").toBool());
	// Основное. Аудио
	ui->equalizerBox->setChecked(settings.value("preferences/equalizer").toBool());
	ui->trayTextBox->setChecked(settings.value("preferences/traytext").toBool());
	ui->coverBox->setChecked(settings.value("preferences/cover").toBool());

	// Основное. Кодировка CUE файла
	ui->autoRadioButton->setChecked(settings.value("preferences/autocuec").toBool());
	ui->cpRadioButton->setChecked(settings.value("preferences/cpcuec").toBool());
	ui->utfRadioButton->setChecked(settings.value("preferences/utfcuec").toBool());


	// Транскодер. vorbisenc
	if (!settings.value("preferences/vorbisquality").isNull())
	{
		ui->vorbisQuaSlider->setValue(settings.value("preferences/vorbisquality").toInt());
		prefDeci(settings.value("preferences/vorbisquality").toInt());
	}

	// Транскодер. lame
	if (!settings.value("preferences/lamequality").isNull())
		ui->lameQuaSlider->setValue(settings.value("preferences/lamequality").toInt());
	// Транскодер. lame vbr
	if (!settings.value("preferences/lamevbrquality").isNull())
		ui->lameVbrQuaSlider->setValue(settings.value("preferences/lamevbrquality").toInt());

	// Транскодер. flacenc
	if (!settings.value("preferences/flacquality").isNull())
		ui->flacQuaSlider->setValue(settings.value("preferences/flacquality").toInt());

	// Транскодер. faac
	if (!settings.value("preferences/faacquality").isNull())
		ui->comboBox->setCurrentIndex(settings.value("preferences/faacquality").toInt());
}

// Внешний вызов настроек транскодера
void Preferences::setTrPref()
{
	ui->listWidget->setCurrentRow(1);
}

void Preferences::prefDeci(int i)
{
	double vorbisQvalue = (double)i/10;
	if (!vorbisQvalue)
		ui->vorbisQuaValue->setText("0.0");
	else if (vorbisQvalue == 1)
		ui->vorbisQuaValue->setText("1.0");
	else
		ui->vorbisQuaValue->setNum(vorbisQvalue);
}

void Preferences::setDefault()
{
	// Основное. Видео
	ui->integrationBox->setChecked(false);
	// Основное. Аудио
	ui->equalizerBox->setChecked(false);
	ui->trayTextBox->setChecked(true);
	ui->coverBox->setChecked(false);

	// Основное. Кодировка CUE файла
	ui->autoRadioButton->setChecked(true);
	//ui->cpRadioButton->setChecked(settings.value("preferences/cpcuec").toBool());
	//ui->utfRadioButton->setChecked(settings.value("preferences/utfcuec").toBool());


	// Транскодер. vorbisenc
	ui->vorbisQuaSlider->setValue(3);
	prefDeci(3);

	// Транскодер. lame
	ui->lameQuaSlider->setValue(5);

	// Транскодер. lame vbr
	ui->lameVbrQuaSlider->setValue(5);

	// Транскодер. flacenc
	ui->flacQuaSlider->setValue(5);

	// Транскодер. faac
	ui->comboBox->setCurrentIndex(1);
}
