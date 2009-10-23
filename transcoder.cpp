#include <QtGui>

#include "transcoder.h"

#define TIME 200
#define APPNAME "CuePlayer"
#define VERSION "0.15"

Q_EXPORT_PLUGIN2(trans_coder, TransCoder)
		TransCoder *transcoder = 0;

GstElement *pipeline, *audio;

static gboolean
bus_call (GstBus     *bus,
		  GstMessage *msg,
		  gpointer    data)
{
	bus = NULL;
  GMainLoop *loop = (GMainLoop *) data;

  switch (GST_MESSAGE_TYPE (msg)) {

	case GST_MESSAGE_EOS:
	  g_print ("Конец потока\n");
	  g_main_loop_quit (loop);
	  break;

	case GST_MESSAGE_ERROR: {
		gchar  *debug;
		GError *error;

		gst_message_parse_error (msg, &error, &debug);
		g_free (debug);

		g_printerr ("Ошибка: %s\n", error->message);
		g_error_free (error);
		transcoder->stopAllPub();
	  break;
	}
	default:
	  break;
  }

  return TRUE;
}

static void
cb_newpad (GstElement *decodebin,
	   GstPad     *pad,
	   gboolean    last,
	   gpointer    data)
{
  GstCaps *caps;
  GstStructure *str;
  GstPad *audiopad;

  audiopad = gst_element_get_static_pad (audio, "sink");
  if (GST_PAD_IS_LINKED (audiopad)) {
	g_object_unref (audiopad);
	decodebin = NULL;
	last = NULL;
	data = NULL;
	return;
  }

  caps = gst_pad_get_caps (pad);
  str = gst_caps_get_structure (caps, 0);
  if (!g_strrstr (gst_structure_get_name (str), "audio")) {
	gst_caps_unref (caps);
	gst_object_unref (audiopad);
	return;
  }
  gst_caps_unref (caps);

  gst_pad_link (pad, audiopad);
}

/*gboolean
cb_encoderpad (GstPad *pad,
			   GstBuffer *buffer,
			   gpointer u_data)
{
	g_print("БУФЕРОК %d\n", GST_BUFFER_SIZE(buffer));
	g_print("Данные %s\n", GST_BUFFER_DATA(buffer));
	if (GST_BUFFER_SIZE(buffer))
	{
		//gst_buffer_set_data(buffer, GST_BUFFER_DATA(buffer), 0);
	}
	//guint16 *data = (guint16 *) GST_BUFFER_DATA (buffer), t;
	//data = 0;
	//gst_buffer_unref (buffer);
	(void) pad;
	(void) u_data;
	return true;
}*/

enum {
	CODEC_VORBIS,
	CODEC_LAME,
	CODEC_FLAC,
	CODEC_NO
};

// Конструктор TransCoder
TransCoder::TransCoder(QWidget *parent) : QMainWindow(parent)
{
	setupUi(this);

	defaultContainer = 1;
	defaultCodec = 1;
	transcoder = this;

	treeWidget->setHeaderLabels(QStringList() << trUtf8("Композиция") << trUtf8("Время"));
	treeWidget->header()->setStretchLastSection(false);
	treeWidget->setColumnWidth(1, 75);
	treeWidget->header()->setResizeMode(0,QHeaderView::Stretch);

	containerBox->addItems(QStringList() << "ogg" << "mp3" << "flac");
	codecBox->addItems(QStringList() << "vorbis" << "lame" << "flac");
	bitrateBox->addItems(QStringList() << "8"  << "16" << "24" << "32" << "40" <<
						 "48" << "56" << "64" << "80" << "96" << "112" << "128" << "160" << "192" << "224" << "256" << "320");

	containerBox->setCurrentIndex(defaultContainer);
	codecBox->setCurrentIndex(defaultCodec);
	bitrateBox->setCurrentIndex(11);

	statusLabel = new QLabel();
	lineEdit->setText(QDir::homePath());
	dirdialog = new QFileDialog(this, trUtf8("Выбрать директорию для сохранения альбома"), QDir::homePath());
	dirdialog->setFileMode(QFileDialog::DirectoryOnly);
	statusBar()->addWidget(statusLabel, 1);
	selectAllAction->setStatusTip(trUtf8("Выбор всех композиций для кодирования"));

	selected = false;
	transcode = false;
	numTrack = 0;

	timer = new QTimer(this);

	restoreSettings();

	connect(selectDirButton, SIGNAL(clicked()), dirdialog, SLOT(exec()));
	connect(dirdialog, SIGNAL(directoryEntered(QString)), lineEdit, SLOT(setText(QString)));
	connect(dirdialog, SIGNAL(directoryEntered(QString)), this, SLOT(updateSettings()));
	connect(timer, SIGNAL(timeout()), this, SLOT(timerUpdate()));
	connect(selectAllAction, SIGNAL(triggered()), this, SLOT(selectAllTrigger()));
	connect(startButton, SIGNAL(clicked()), this, SLOT(startTranscode()));
	connect(stopButton, SIGNAL(clicked()), this, SLOT(stopAll()));
	connect(containerBox, SIGNAL(currentIndexChanged(int)), this, SLOT(formatError(int)));
	connect(containerBox, SIGNAL(activated(int)), codecBox, SLOT(setCurrentIndex(int)));
	connect(codecBox, SIGNAL(currentIndexChanged(int)), this, SLOT(formatError(int)));
	connect(codecBox, SIGNAL(activated(int)), containerBox, SLOT(setCurrentIndex(int)));
	connect(bitrateBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateSettings()));
	connect(treeWidget, SIGNAL(itemEntered(QTreeWidgetItem*,int)), this, SLOT(toolItem(QTreeWidgetItem*,int)));
	connect(treeWidget, SIGNAL(itemPressed(QTreeWidgetItem*,int)), this, SLOT(toolItem(QTreeWidgetItem*,int)));
}

// Заполнение списка композиций
void TransCoder::setFileName(QString filename, qint64 totalTime)
{
	treeWidget->clear();
	refparser = new CueParser(filename);
	setWindowTitle(trUtf8("[конвертор] ") + refparser->getTitle());
	qint64 startTime = refparser->getTrackIndex(1);
	qint64 currentTime;
	saveTotalTime = totalTime;
	QString strSec;
	for (int i = 1; i <= refparser->getTrackNumber(); i++)
	{
		if (i < refparser->getTrackNumber())
			currentTime = refparser->getTrackIndex(i + 1) - startTime;
		else
			currentTime = totalTime - startTime;
		int sec = currentTime / 1000;
		int min = sec / 60;
		sec %= 60;
		if (sec < 10)
			strSec = "0" + QString::number(sec,10);
		else
			strSec = QString::number(sec,10);
		QString numTrackList;
		numTrackList.setNum(i);
		playlistItem[i-1] = new QTreeWidgetItem;
		playlistItem[i-1]->setText(0, numTrackList + ". " + refparser->getTrackTitle(i));
		playlistItem[i-1]->setText(1, QString::number(min, 10) + ":" + strSec);
		playlistItem[i-1]->setTextAlignment(1, Qt::AlignRight);
		treeWidget->insertTopLevelItem(i-1, playlistItem[i-1]);
		startTime = refparser->getTrackIndex(i + 1);
	}
}

// Триггер выделения всех композиций
void TransCoder::selectAllTrigger()
{
	if(selected)
	{
		treeWidget->clearSelection();
		selectAllAction->setText(trUtf8("Выделить всё"));
	}
	else
	{
		treeWidget->selectAll();
		toolItem(treeWidget->topLevelItem(0), 0);
		selectAllAction->setText(trUtf8("Снять выделение"));
	}
	selected = selected ? false : true;
}

// Старт кодировщика
void TransCoder::startTranscode()
{
	bool selected = false;
	startButton->setDisabled(true);
	progressColor = Qt::yellow;
	finishedColor = Qt::green;
	selectAllAction->setDisabled(true);
	for (int i = 0; i < refparser->getTrackNumber(); i++)
	{
		if (treeWidget->topLevelItem(i)->isSelected())
		{
			treeWidget->setItemSelected(treeWidget->topLevelItem(i), false);
			treeWidget->topLevelItem(i)->setBackgroundColor(0, progressColor);
			treeWidget->topLevelItem(i)->setBackgroundColor(1, progressColor);
			treeWidget->topLevelItem(i)->setFlags(Qt::NoItemFlags);
			treeWidget->topLevelItem(i)->setForeground(0, QBrush(Qt::SolidPattern));
			treeWidget->topLevelItem(i)->setForeground(1, QBrush(Qt::SolidPattern));
			statusLabel->setText(trUtf8("Идет кодирование трека: ") + refparser->getTrackTitle(i + 1));
			transcode = true;
			pipeRun(i+1);
			if (transcode)
				this->stopTranscode();
			if (progressBar->value() == 100)
			{
				treeWidget->topLevelItem(i)->setBackgroundColor(0, finishedColor);
				treeWidget->topLevelItem(i)->setBackgroundColor(1, finishedColor);
			}
			selected = true;
		}
	}
	startButton->setDisabled(false);
	selectAllAction->setDisabled(false);
	if (selected)
		statusLabel->setText(trUtf8("Все задания завершены."));
	else
		statusLabel->setText(trUtf8("Выделите треки для кодирования."));
}

// Остановка кодирования трека
void TransCoder::stopTranscode()
{
	if (transcode)
	{
		progressBar->setValue(100);
		gst_element_set_state (pipeline, GST_STATE_NULL);
		timer->stop();
		gst_object_unref (GST_OBJECT (pipeline));
		g_main_loop_quit (loop);
	}
	transcode = false;
}

// Остановка кодировщика
void TransCoder::stopAll()
{
	errorColor = Qt::red;
	if (transcode)
	{
		gst_element_set_state (pipeline, GST_STATE_NULL);
		timer->stop();
		gst_object_unref (GST_OBJECT (pipeline));
		g_main_loop_quit (loop);
	}
	transcode = false;
	for (int i = 0; i < refparser->getTrackNumber(); i++)
	{
		treeWidget->topLevelItem(i)->setSelected(false);
	}
	treeWidget->topLevelItem(numTrack - 1)->setBackgroundColor(0, errorColor);
	treeWidget->topLevelItem(numTrack - 1)->setBackgroundColor(1, errorColor);
	treeWidget->topLevelItem(numTrack - 1)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsTristate);
	if (progressBar->value() == 100)
		progressBar->setValue(0);
}

void TransCoder::setTrack()
{

}

void TransCoder::setTrackTime(qint64 start, qint64 stop)
{
		GstClockTime nach   = (GstClockTime)(start * GST_MSECOND);
		GstClockTime kon   = (GstClockTime)(stop * GST_MSECOND);
		if (!gst_element_seek(pipeline, 1.0,
				GST_FORMAT_TIME,
				(GstSeekFlags)(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE),
				GST_SEEK_TYPE_SET, nach,
				GST_SEEK_TYPE_SET, kon ))
			qDebug() << QString(trUtf8("Ошибка поиска ")) << qint64(stop);
		else
			qDebug() << QString(trUtf8("Начало потока ")) << qint64(start);
}

void TransCoder::pipeRun(int ind)
{
	GstElement *source, *dec, *conv, *encoder, *muxer, *fileout, *tagger;
	GstBus *bus;
	GstState state;
	GstPad *audiopad;//, *encoderpad;

	numTrack = ind;

	loop = g_main_loop_new (NULL, FALSE);

	pipeline = gst_pipeline_new ("audio-transcoder");

	// Входной файл
	source   = gst_element_factory_make ("filesrc", "file-source");
	g_object_set (source, "location", refparser->getSoundFile().toUtf8().data(), NULL);

	dec = gst_element_factory_make ("decodebin", "decoder");
	g_signal_connect (dec, "new-decoded-pad", G_CALLBACK (cb_newpad), NULL);
	gst_bin_add_many (GST_BIN (pipeline), source, dec, NULL);
	gst_element_link (source, dec);

	audio = gst_bin_new ("audiobin");
	conv = gst_element_factory_make ("audioconvert", "aconv");
	audiopad = gst_element_get_static_pad (conv, "sink");
	fileout = gst_element_factory_make ("filesink", "file-out");

	switch (codecBox->currentIndex())
	{
		case CODEC_VORBIS:
			encoder = gst_element_factory_make ("vorbisenc", "audio-encoder");
			tagger = gst_element_factory_make ("vorbistag", "tagger");
			muxer = gst_element_factory_make ("oggmux", "audio-muxer");
			gst_bin_add_many (GST_BIN (audio), conv, encoder, tagger, muxer, fileout, NULL);
			gst_element_link_many (conv, encoder, tagger, muxer, fileout, NULL);
			gst_tag_setter_add_tags (GST_TAG_SETTER (tagger),
								GST_TAG_MERGE_REPLACE_ALL,
								GST_TAG_TITLE, refparser->getTrackTitle(ind).toUtf8().data(),
								GST_TAG_ARTIST, refparser->getPerformer().toUtf8().data(),
								GST_TAG_TRACK_NUMBER, ind,
								GST_TAG_TRACK_COUNT, refparser->getTrackNumber(),
								GST_TAG_ALBUM, refparser->getAlbum().toUtf8().data(),
								GST_TAG_ENCODER, APPNAME,
								GST_TAG_ENCODER_VERSION, VERSION,
								GST_TAG_COMMENT, APPNAME " " VERSION,
								GST_TAG_CODEC, "vorbis",
								NULL);
			containerBox->setCurrentIndex(CODEC_VORBIS);
			break;
		case CODEC_LAME:
			bool ok;
			encoder = gst_element_factory_make ("lame", "audio-encoder");
			muxer = gst_element_factory_make ("id3v2mux", "audio-muxer");
			gst_bin_add_many (GST_BIN (audio), conv, encoder, muxer, fileout, NULL);
			gst_element_link_many (conv, encoder, muxer, fileout, NULL);
			g_object_set (encoder, "bitrate", bitrateBox->currentText().toInt(&ok, 10), NULL);
			gst_tag_setter_add_tags (GST_TAG_SETTER (muxer),
								GST_TAG_MERGE_REPLACE_ALL,
								GST_TAG_TITLE, refparser->getTrackTitle(ind).toUtf8().data(),
								GST_TAG_ARTIST, refparser->getPerformer().toUtf8().data(),
								GST_TAG_TRACK_NUMBER, ind,
								GST_TAG_TRACK_COUNT, refparser->getTrackNumber(),
								GST_TAG_ALBUM, refparser->getAlbum().toUtf8().data(),
								GST_TAG_ENCODER, APPNAME,
								GST_TAG_ENCODER_VERSION, VERSION,
								GST_TAG_COMMENT, APPNAME " " VERSION,
								GST_TAG_CODEC, "lame",
								NULL);
			containerBox->setCurrentIndex(CODEC_LAME);
			break;
		case CODEC_FLAC:
			encoder = gst_element_factory_make ("flacenc", "audio-encoder");
			tagger = gst_element_factory_make ("flactag", "tagger");
			gst_bin_add_many (GST_BIN (audio), conv, encoder, tagger, fileout, NULL);
			gst_element_link_many (conv, encoder, tagger, fileout, NULL);
			gst_tag_setter_add_tags (GST_TAG_SETTER (tagger),
								GST_TAG_MERGE_REPLACE_ALL,
								GST_TAG_TITLE, refparser->getTrackTitle(ind).toUtf8().data(),
								GST_TAG_ARTIST, refparser->getPerformer().toUtf8().data(),
								GST_TAG_TRACK_NUMBER, ind,
								GST_TAG_TRACK_COUNT, refparser->getTrackNumber(),
								GST_TAG_ALBUM, refparser->getAlbum().toUtf8().data(),
								GST_TAG_ENCODER, APPNAME,
								GST_TAG_ENCODER_VERSION, VERSION,
								GST_TAG_COMMENT, APPNAME " " VERSION,
								GST_TAG_CODEC, "flac",
								NULL);
			/*encoderpad = gst_element_get_static_pad (encoder, "src");
			gst_pad_add_buffer_probe (encoderpad, G_CALLBACK (cb_encoderpad), NULL);
			gst_object_unref (encoderpad);*/

			containerBox->setCurrentIndex(CODEC_FLAC);
			break;
		case CODEC_NO:
		default:
			break;
	}

	gst_element_add_pad (audio, gst_ghost_pad_new ("sink", audiopad));
	gst_object_unref (audiopad);
	gst_bin_add (GST_BIN (pipeline), audio);

	// Выходной файл
	QRegExp rxFileSlash("/");
	QString filename;
	QString trackName = refparser->getTrackTitle(ind);
	trackName.replace(rxFileSlash, trUtf8("⁄"));
	QString dirname = lineEdit->text() + "/" + refparser->getTitle();
	QDir dir(dirname);
	if (!dir.exists())
		dir.mkdir(dirname);
	if (ind < 10)
		filename = dirname + "/" + "0" + QString::number(ind,10) + " - " + trackName + "." + containerBox->currentText();
	else
		filename = dirname + "/" + QString::number(ind,10) + " - " + trackName + "." + containerBox->currentText();
	g_object_set (fileout, "location", filename.toUtf8().data(), NULL);

	bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
	gst_bus_add_watch (bus, bus_call, loop);
	gst_object_unref (bus);

	//g_signal_connect (pipeline, "deep-notify", G_CALLBACK (gst_object_default_deep_notify), NULL); // Дебаг!
	g_print ("Кодируется: %s\n", refparser->getSoundFile().toUtf8().data());

	gst_element_set_state (pipeline, GST_STATE_PLAYING);

	gst_element_get_state( GST_ELEMENT(pipeline), &state, NULL, GST_MSECOND * 300);
	if (state == GST_STATE_PLAYING)
	{
		timer->start(TIME);
		if (ind == refparser->getTrackNumber())
			setTrackTime(refparser->getTrackIndex(ind),saveTotalTime);
		else
			setTrackTime(refparser->getTrackIndex(ind),refparser->getTrackIndex(ind+1));
		g_print ("Запущено...\n");
		g_main_loop_run (loop);
	}
	else
	{
		g_print ("Ошибка перехода в PLAYING\n");
		transcode = false;
		stopAll();
	}
}
void TransCoder::timerUpdate()
{
	gint64 startpos, pos, endpos, percent;
	int curpercent = 0;
	GstFormat fmt = GST_FORMAT_TIME;
	gst_element_query_position(pipeline, &fmt, &pos);
	if(numTrack)
	{
		if (numTrack == refparser->getTrackNumber())
			endpos = saveTotalTime;
		else
			endpos = refparser->getTrackIndex(numTrack+1);
		startpos = refparser->getTrackIndex(numTrack);
		percent = ((endpos - startpos) / 100) * GST_MSECOND;
		if (codecBox->currentIndex() <= 1)
			pos = pos - (startpos * GST_MSECOND);
		curpercent = pos/percent;
	}
	else
		g_print ("ОШИБКА!!!\n");
	//qDebug() << QString(trUtf8("Позиция ")) << pos;
	progressBar->setValue(curpercent);
}

void TransCoder::formatError(int index)
{
	if (!index)
	{
		QMessageBox *codecwarn = new QMessageBox(this);
		codecwarn->setIcon(QMessageBox::Warning);
		codecwarn->setWindowTitle(trUtf8("Внимание!"));
		codecwarn->setText(trUtf8("<h2>Формат не поддерживается.</h2>"
									"<p>Заданный формат временно не поддерживается транскодером."
									"<p>Будет применен формат по-умолчанию."));
		connect(codecwarn, SIGNAL(finished(int)), this, SLOT(setDefaultIndex()));
		codecwarn->exec();
	}
}

void TransCoder::setDefaultIndex()
{
	codecBox->setCurrentIndex(defaultCodec);
	containerBox->setCurrentIndex(defaultContainer);
}

void TransCoder::stopAllPub()
{
	stopAll();
}

void TransCoder::updateSettings()
{
	settings.setValue("transcoder/outdir", lineEdit->text());
	settings.setValue("transcoder/bitrate", bitrateBox->currentIndex());
}

void TransCoder::restoreSettings()
{
	bool ok;
	if (settings.value("transcoder/bitrate").toBool())
		bitrateBox->setCurrentIndex(settings.value("transcoder/bitrate").toInt(&ok));
	if (settings.value("transcoder/outdir").toBool())
		lineEdit->setText(settings.value("transcoder/outdir").toString());
}

void TransCoder::toolItem(QTreeWidgetItem*,int)
{
	statusLabel->setText(trUtf8("Нажмите \"Начать\" для кодирования."));
}
