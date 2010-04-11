#include "preferences.h"
#include "ui_preferences.h"

#include <QDebug>

Preferences::Preferences(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Preferences)
{
    ui->setupUi(this);

	ui->treeWidget->setCurrentItem(ui->treeWidget->topLevelItem(0));
	readSettings();

	connect(ui->okButton, SIGNAL(clicked()), this, SLOT(saveSettings()));
	connect(ui->vorbisQuaSlider, SIGNAL(valueChanged(int)), this, SLOT(prefDeci(int)));
	connect(ui->defaultButton, SIGNAL(clicked()), this, SLOT(setDefault()));
	connect(ui->treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(listItemClicked(QTreeWidgetItem*,int)));
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
	settings.setValue("preferences/vorbismaxbitrate", ui->vorbisMaxBitrateSlider->value());
	settings.setValue("preferences/vorbisbitrate", ui->vorbisBitrateSlider->value());
	settings.setValue("preferences/vorbisminbitrate", ui->vorbisMinBitrateSlider->value());
	settings.setValue("preferences/vorbisquality", ui->vorbisQuaSlider->value());
	settings.setValue("preferences/vorbismanaged", ui->vorbisManagedBox->isChecked());

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
		ui->vorbisMaxBitrateSlider->setValue(settings.value("preferences/vorbismaxbitrate").toInt());
		ui->vorbisBitrateSlider->setValue(settings.value("preferences/vorbisbitrate").toInt());
		ui->vorbisMinBitrateSlider->setValue(settings.value("preferences/vorbisminbitrate").toInt());
		ui->vorbisQuaSlider->setValue(settings.value("preferences/vorbisquality").toInt());
		prefDeci(settings.value("preferences/vorbisquality").toInt());
		ui->vorbisManagedBox->setChecked(settings.value("preferences/vorbismanaged").toBool());
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
	ui->treeWidget->setCurrentItem(ui->treeWidget->topLevelItem(1));
	ui->treeWidget->expandItem(ui->treeWidget->topLevelItem(1));
	ui->stackedWidget->setCurrentIndex(1);
}

void Preferences::prefDeci(int i)
{
	ui->vorbisQuaValue->setValue((double)i/10);
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

	// Транскодер. vorbisenc
	ui->vorbisMaxBitrateSlider->setValue(-1);
	ui->vorbisBitrateSlider->setValue(-1);
	ui->vorbisMinBitrateSlider->setValue(-1);
	ui->vorbisQuaSlider->setValue(3);
	prefDeci(3);
	ui->vorbisManagedBox->setChecked(false);

	// Транскодер. lame
	ui->lameQuaSlider->setValue(5);

	// Транскодер. lame vbr
	ui->lameVbrQuaSlider->setValue(5);

	// Транскодер. flacenc
	ui->flacQuaSlider->setValue(5);

	// Транскодер. faac
	ui->comboBox->setCurrentIndex(1);
}

void Preferences::listItemClicked(QTreeWidgetItem *item, int column)
{
	int ind = ui->treeWidget->indexOfTopLevelItem(item);
	column = 0;

	if (ind != -1)
		ui->stackedWidget->setCurrentIndex(ind);
	else
		ui->stackedWidget->setCurrentIndex(ui->treeWidget->indexOfTopLevelItem(item->parent()) + item->parent()->indexOfChild(item) + 1);
}
