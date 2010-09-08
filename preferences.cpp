#include "preferences.h"
#include "ui_preferences.h"

Preferences::Preferences(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Preferences)
{
    ui->setupUi(this);

	ui->treeWidget->setCurrentItem(ui->treeWidget->topLevelItem(0));
	readSettings();
	cugstbinds = new CuGstBinds(this);

	static QStringList codeclist;
	codeclist << "mad" << "vorbisdec" << "flacdec" << "ffdec_ape"
                        << "faad" << "ffdec_mpeg4" << "ffdec_h264" << "musepackdec";
	int counter = 0;

	foreach (QString str, codeclist)
	{
		if (cugstbinds->checkElement(str))
			ui->tableWidget->item(counter,0)->setText("X");
		else
			ui->tableWidget->item(counter,1)->setText("X");

		ui->tableWidget->item(counter,2)->setText(codeclist.at(counter));
		counter++;
	}

	ui->tableWidget->verticalHeader()->show();
	connect(ui->okButton, SIGNAL(clicked()), this, SLOT(saveSettings()));
	connect(ui->vorbisQuaSlider, SIGNAL(valueChanged(int)), this, SLOT(prefDeci(int)));
	connect(ui->vorbisQuaValue, SIGNAL(valueChanged(double)), this, SLOT(prefDeci(double)));
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

void Preferences::resizeEvent(QResizeEvent  *e)
{
	ui->treeWidget->setMinimumWidth(126);
	QWidget::resizeEvent(e);
}

void Preferences::saveSettings()
{
	QSettings settings;

	// Основное. Видео
	settings.setValue("preferences/integration", ui->integrationBox->isChecked());
	// Основное. Аудио
	settings.setValue("preferences/equalizer", ui->equalizerBox->isChecked());
	settings.setValue("preferences/traytext", ui->trayTextBox->isChecked());
	settings.setValue("preferences/traytimeout", ui->doubleSpinBox->value());
	settings.setValue("preferences/cover",ui->coverBox->isChecked());
        settings.setValue("preferences/showplaylist",ui->showPLBox->isChecked());
        settings.setValue("preferences/autoreplay",ui->autoReplayBox->isChecked());
        settings.setValue("preferences/autoplay",ui->autoPlayBox->isChecked());
	settings.setValue("preferences/audiooutput",ui->audioOutputBox->currentIndex());

	// Основное. Кодировка CUE файла
	settings.setValue("preferences/autocuec", ui->autoRadioButton->isChecked());
	settings.setValue("preferences/cp1250cuec", ui->cp1250RadioButton->isChecked());
	settings.setValue("preferences/cp1251cuec", ui->cp1251RadioButton->isChecked());
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
	settings.setValue("preferences/lamevbr", ui->lameVbrComboBox->currentIndex());
	settings.setValue("preferences/lamevbrquality", ui->lameVbrQuaSlider->value());

	// Транскодер. flacenc
	settings.setValue("preferences/flacquality", ui->flacQuaSlider->value());
	settings.setValue("preferences/flacstreamablesubset", ui->flacStreamableSubsetBox->isChecked());
	settings.setValue("preferences/flacmidsidestereo", ui->flacMidSideStereoBox->isChecked());
	settings.setValue("preferences/flacloosemidsidestereo", ui->flacLooseMidSideStereoBox->isChecked());
	settings.setValue("preferences/flacblocksize", ui->flacBlocksizeSlider->value());
	settings.setValue("preferences/flacmaxlpcorder", ui->flacMaxLpcOrderSlider->value());
	settings.setValue("preferences/flacqlpcoeffprecision", ui->flacQlpCoeffPrecisionSlider->value());
	settings.setValue("preferences/flacqlpcoeffprecsearch", ui->flacQlpCoeffPrecSearchCheckBox->isChecked());
	settings.setValue("preferences/flacescapecoding", ui->flacEscapeCodingCheckBox->isChecked());
	settings.setValue("preferences/flacexhaustivemodelsearch", ui->flacExhaustiveModelSearchCheckBox->isChecked());
	settings.setValue("preferences/flacminresidualpartitionorder", ui->flacMinResidualPartitionOrderSlider->value());
	settings.setValue("preferences/flacmaxresidualpartitionorder", ui->flacMaxResidualPartitionOrderSlider->value());
	settings.setValue("preferences/flacriceparametersearchdist", ui->flacRiceParameterSearchDistSlider->value());

	// Транскодер. faac
	settings.setValue("preferences/faacoutputformat", ui->faacOutputformatComboBox->currentIndex());
	settings.setValue("preferences/faacbitrate", ui->faacBitrateSlider->value());
	settings.setValue("preferences/faacprofile", ui->faacProfileComboBox->currentIndex());
	settings.setValue("preferences/faactns", ui->faacTnsCheckBox->isChecked());
	settings.setValue("preferences/faacmidside", ui->faacMidsideCheckBox->isChecked());
	settings.setValue("preferences/faacshortctl", ui->faacShortctlComboBox->currentIndex());

	// Ширина левого поля
	settings.setValue("preferences/treewidgetwidth", ui->treeWidget->width());

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
	if (!settings.value("preferences/traytimeout").isNull())
		ui->doubleSpinBox->setValue(settings.value("preferences/traytimeout").toDouble());
	ui->coverBox->setChecked(settings.value("preferences/cover").toBool());
        ui->showPLBox->setChecked(settings.value("preferences/showplaylist").toBool());
        ui->autoReplayBox->setChecked(settings.value("preferences/autoreplay").toBool());
        ui->autoPlayBox->setChecked(settings.value("preferences/autoplay").toBool());
	ui->audioOutputBox->setCurrentIndex(settings.value("preferences/audiooutput").toInt());

	// Основное. Кодировка CUE файла
	ui->autoRadioButton->setChecked(settings.value("preferences/autocuec").toBool());
	ui->cp1250RadioButton->setChecked(settings.value("preferences/cp1250cuec").toBool());
	ui->cp1251RadioButton->setChecked(settings.value("preferences/cp1251cuec").toBool());
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
		ui->lameVbrComboBox->setCurrentIndex(settings.value("preferences/lamevbr").toInt());
		ui->lameVbrQuaSlider->setValue(settings.value("preferences/lamevbrquality").toInt());
	}

	// Транскодер. flacenc
	if (!settings.value("preferences/flacquality").isNull())
	{
		ui->flacQuaSlider->setValue(settings.value("preferences/flacquality").toInt());
		ui->flacStreamableSubsetBox->setChecked(settings.value("preferences/flacstreamablesubset").toBool());
		ui->flacMidSideStereoBox->setChecked(settings.value("preferences/flacmidsidestereo").toBool());
		ui->flacLooseMidSideStereoBox->setChecked(settings.value("preferences/flacloosemidsidestereo").toBool());
		ui->flacBlocksizeSlider->setValue(settings.value("preferences/flacblocksize").toInt());
		ui->flacMaxLpcOrderSlider->setValue(settings.value("preferences/flacmaxlpcorder").toInt());
		ui->flacQlpCoeffPrecisionSlider->setValue(settings.value("preferences/flacqlpcoeffprecision").toInt());
		ui->flacQlpCoeffPrecSearchCheckBox->setChecked(settings.value("preferences/flacqlpcoeffprecsearch").toBool());
		ui->flacEscapeCodingCheckBox->setChecked(settings.value("preferences/flacescapecoding").toBool());
		ui->flacExhaustiveModelSearchCheckBox->setChecked(settings.value("preferences/flacexhaustivemodelsearch").toBool());
		ui->flacMinResidualPartitionOrderSlider->setValue(settings.value("preferences/flacminresidualpartitionorder").toInt());
		ui->flacMaxResidualPartitionOrderSlider->setValue(settings.value("preferences/flacmaxresidualpartitionorder").toInt());
		ui->flacRiceParameterSearchDistSlider->setValue(settings.value("preferences/flacriceparametersearchdist").toInt());
	}

	// Транскодер. faac
	if (!settings.value("preferences/faacprofile").isNull())
	{
		ui->faacOutputformatComboBox->setCurrentIndex(settings.value("preferences/faacoutputformat").toInt());
		ui->faacBitrateSlider->setValue(settings.value("preferences/faacbitrate").toInt());
		ui->faacProfileComboBox->setCurrentIndex(settings.value("preferences/faacprofile").toInt());
		ui->faacTnsCheckBox->setChecked(settings.value("preferences/faactns").toBool());
		ui->faacMidsideCheckBox->setChecked(settings.value("preferences/faacmidside").toBool());
		ui->faacShortctlComboBox->setCurrentIndex(settings.value("preferences/faacshortctl").toInt());
	}

	// Ширина левого поля
	if (!settings.value("preferences/treewidgetwidth").isNull())
		ui->treeWidget->setMinimumWidth(settings.value("preferences/treewidgetwidth").toInt());
}

// Внешний вызов настроек транскодера
void Preferences::setTrPref()
{
	ui->treeWidget->setCurrentItem(ui->treeWidget->topLevelItem(1));
	ui->treeWidget->expandItem(ui->treeWidget->topLevelItem(1));
	ui->stackedWidget->setCurrentIndex(1 + ui->treeWidget->topLevelItem(0)->childCount());
}

void Preferences::prefDeci(int i)
{
	ui->vorbisQuaValue->setValue((double)i/10);
}

void Preferences::prefDeci(double i)
{
	ui->vorbisQuaSlider->setValue(i * 10);
}

void Preferences::setDefault()
{
	switch (ui->stackedWidget->currentIndex())
	{
	case 2:
		// Аудио
		ui->equalizerBox->setChecked(false);
		ui->trayTextBox->setChecked(true);
		ui->doubleSpinBox->setValue(2.0);
		ui->coverBox->setChecked(false);
                ui->showPLBox->setChecked(false);
                ui->autoReplayBox->setChecked(false);
                ui->autoPlayBox->setChecked(false);
		ui->audioOutputBox->setCurrentIndex(0);
                emit defaultAudio();
		// Аудио. Кодировка CUE файла
		ui->autoRadioButton->setChecked(true);
		break;
	case 3:
		// Видео
		ui->integrationBox->setChecked(false);
	case 5:
		// Транскодер. vorbisenc
		ui->vorbisMaxBitrateSlider->setValue(-1);
		ui->vorbisBitrateSlider->setValue(-1);
		ui->vorbisMinBitrateSlider->setValue(-1);
		ui->vorbisQuaSlider->setValue(3);
		prefDeci(3);
		ui->vorbisManagedBox->setChecked(false);
		break;
	case 6:
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
		ui->lameVbrComboBox->setCurrentIndex(0);
		ui->lameVbrQuaSlider->setValue(4);
		break;
	case 7:
		// Транскодер. flacenc
		ui->flacQuaSlider->setValue(5);
		ui->flacStreamableSubsetBox->setChecked(true);
		ui->flacMidSideStereoBox->setChecked(true);
		ui->flacLooseMidSideStereoBox->setChecked(false);
		ui->flacBlocksizeSlider->setValue(4608);
		ui->flacMaxLpcOrderSlider->setValue(8);
		ui->flacQlpCoeffPrecisionSlider->setValue(0);
		ui->flacQlpCoeffPrecSearchCheckBox->setChecked(false);
		ui->flacEscapeCodingCheckBox->setChecked(false);
		ui->flacExhaustiveModelSearchCheckBox->setChecked(false);
		ui->flacMinResidualPartitionOrderSlider->setValue(3);
		ui->flacMaxResidualPartitionOrderSlider->setValue(3);
		ui->flacRiceParameterSearchDistSlider->setValue(0);
		break;
	case 8:
		// Транскодер. faac
		ui->faacOutputformatComboBox->setCurrentIndex(0);
		ui->faacBitrateSlider->setValue(128000);
		ui->faacProfileComboBox->setCurrentIndex(1);
		ui->faacTnsCheckBox->setChecked(false);
		ui->faacMidsideCheckBox->setChecked(true);
		ui->faacShortctlComboBox->setCurrentIndex(0);
		break;
	default:
		break;
	}
}

void Preferences::listItemClicked(QTreeWidgetItem *item, int column)
{
	int ind = ui->treeWidget->indexOfTopLevelItem(item);
	column = 0;

	if (ind == -1)
	{
		switch (ui->treeWidget->indexOfTopLevelItem(item->parent()))
		{
		case 0:
			ui->stackedWidget->setCurrentIndex(ui->treeWidget->indexOfTopLevelItem(item->parent()) + item->parent()->indexOfChild(item) + 1);
			break;
		case 1:
			ui->stackedWidget->setCurrentIndex(ui->treeWidget->indexOfTopLevelItem(item->parent()) +
											   item->parent()->indexOfChild(item) + 1 +
											   ui->treeWidget->topLevelItem(ui->treeWidget->indexOfTopLevelItem(item->parent()) - 1)->childCount());
			break;
		}
	}
	else if (ind <= 0)
		ui->stackedWidget->setCurrentIndex(ind);
	else
		ui->stackedWidget->setCurrentIndex(ind + ui->treeWidget->topLevelItem(ind - 1)->childCount());
}
