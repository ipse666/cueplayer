#include "preferences.h"
#include "ui_preferences.h"

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
	settings.setValue("preferences/lamebitrate", ui->lameBitrateBox->currentIndex());
	settings.setValue("preferences/lamecompressionratio", ui->lameCRSlider->value());
	settings.setValue("preferences/lamequality", ui->lameQuaSlider->value());
	settings.setValue("preferences/lamemode", ui->lameModeComboBox->currentIndex());
	settings.setValue("preferences/lameforcems", ui->lameForseMs->isChecked());
	settings.setValue("preferences/lamefreeformat", ui->lameFreeFormat->isChecked());
	settings.setValue("preferences/lamecopyright", ui->lameCopyight->isChecked());
	settings.setValue("preferences/lameoriginal", ui->lameOriginal->isChecked());
	settings.setValue("preferences/lameerrprot", ui->lameErrProt->isChecked());
	settings.setValue("preferences/lamepaddingtype", ui->lamePaddingType->currentIndex());
	settings.setValue("preferences/lameextension", ui->lameExtention->isChecked());
	settings.setValue("preferences/lamestrictiso", ui->lameStrictIso->isChecked());
	settings.setValue("preferences/lamedisrese", ui->lameDisRese->isChecked());
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
	{
		ui->lameBitrateBox->setCurrentIndex(settings.value("preferences/lamebitrate").toInt());
		ui->lameCRSlider->setValue(settings.value("preferences/lamecompressionratio").toInt());
		ui->lameQuaSlider->setValue(settings.value("preferences/lamequality").toInt());
		ui->lameModeComboBox->setCurrentIndex(settings.value("preferences/lamemode").toInt());
		ui->lameForseMs->setChecked(settings.value("preferences/lameforcems").toBool());
		ui->lameFreeFormat->setChecked(settings.value("preferences/lamefreeformat").toBool());
		ui->lameCopyight->setChecked(settings.value("preferences/lamecopyright").toBool());
		ui->lameOriginal->setChecked(settings.value("preferences/lameoriginal").toBool());
		ui->lameErrProt->setChecked(settings.value("preferences/lameerrprot").toBool());
		ui->lamePaddingType->setCurrentIndex(settings.value("preferences/lamepaddingtype").toInt());
		ui->lameExtention->setChecked(settings.value("preferences/lameextension").toBool());
		ui->lameStrictIso->setChecked(settings.value("preferences/lamestrictiso").toBool());
		ui->lameDisRese->setChecked(settings.value("preferences/lamedisrese").toBool());
		ui->lameVbrQuaSlider->setValue(settings.value("preferences/lamevbrquality").toInt());
	}

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
	switch (ui->stackedWidget->currentIndex())
	{
	case 0:
		// Основное. Видео
		ui->integrationBox->setChecked(false);
		// Основное. Аудио
		ui->equalizerBox->setChecked(false);
		ui->trayTextBox->setChecked(true);
		ui->coverBox->setChecked(false);
		// Основное. Кодировка CUE файла
		ui->autoRadioButton->setChecked(true);
		break;
	case 2:
		// Транскодер. vorbisenc
		ui->vorbisMaxBitrateSlider->setValue(-1);
		ui->vorbisBitrateSlider->setValue(-1);
		ui->vorbisMinBitrateSlider->setValue(-1);
		ui->vorbisQuaSlider->setValue(3);
		prefDeci(3);
		ui->vorbisManagedBox->setChecked(false);
		break;
	case 3:
		// Транскодер. lame
		ui->lameBitrateBox->setCurrentIndex(11);
		ui->lameCRSlider->setValue(0);
		ui->lameQuaSlider->setValue(3);
		ui->lameModeComboBox->setCurrentIndex(1);
		ui->lameForseMs->setChecked(false);
		ui->lameFreeFormat->setChecked(false);
		ui->lameCopyight->setChecked(false);
		ui->lameOriginal->setChecked(true);
		ui->lameErrProt->setChecked(false);
		ui->lamePaddingType->setCurrentIndex(0);
		ui->lameExtention->setChecked(false);
		ui->lameStrictIso->setChecked(false);
		ui->lameDisRese->setChecked(false);
		ui->lameVbrQuaSlider->setValue(4);
		break;
	case 4:
		// Транскодер. flacenc
		ui->flacQuaSlider->setValue(5);
		break;
	case 5:
		// Транскодер. faac
		ui->comboBox->setCurrentIndex(1);
		break;
	default:
		break;
	}
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
