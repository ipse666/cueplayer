#include "apetoflac.h"

#define TIME 200

GstElement *audiobin;
ApeToFlac *apetoflac = 0;

static gboolean
bus_call (GstBus     *bus,
		  GstMessage *msg,
		  gpointer    data)
{
	bus = NULL;
	data = NULL;

  switch (GST_MESSAGE_TYPE (msg)) {

	case GST_MESSAGE_EOS:
	  g_print ("Конец потока\n");
	  apetoflac->stopCode();
	  break;
	case GST_MESSAGE_STATE_CHANGED: {
	  GstState oldstate;
	  GstState newstate;
	  GstState pending;
	  if (!strcmp(gst_element_get_name(GST_MESSAGE_SRC(msg)), "atf-transcoder"))
		{
			gst_message_parse_state_changed(msg, &oldstate, &newstate, &pending);
			//g_print ("%s changed state from %d to %d, pending %d\n", GST_ELEMENT_NAME(GST_MESSAGE_SRC(msg)), oldstate, newstate, pending);
			if (newstate == GST_STATE_PLAYING)
				apetoflac->decoding();
		}
	  break;
	}
	case GST_MESSAGE_ERROR: {
	  gchar  *debug;
	  GError *error;

	  gst_message_parse_error (msg, &error, &debug);
	  g_free (debug);

	  g_printerr ("Ошибка: %s\n", error->message);
	  g_error_free (error);

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

  audiopad = gst_element_get_static_pad (audiobin, "sink");
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

// Конструктор
ApeToFlac::ApeToFlac(QWidget *parent) : QDialog(parent)
{
	setupUi(this);
	apetoflac = this;

	timer = new QTimer(this);

	// Костыль для неюникодовых локалей
	localFileNamesEncoder = QTextCodec::codecForLocale()->makeEncoder();

	connect(okButton, SIGNAL(clicked()), this, SLOT(startDecoder()));
	connect(timer, SIGNAL(timeout()), this, SLOT(progressUpd()));
}

// Получение имен файлов
void ApeToFlac::setFileNames(QString cuefile, QString apefile)
{
	QString line;
	QRegExp rxFilename("(.*)\\.(.{3})");
	QRegExp rxFilename2(".*/([^/]*)$");
	QRegExp rxSoundfile("^FILE \"(.*)\"");
	QFile inCuefile(cuefile);

	atfCuefile = cuefile;
	atfApefile = apefile;

	rxFilename.indexIn(atfApefile);
	outFile = rxFilename.cap(1) + ".flac";

	rxFilename.indexIn(atfCuefile);
	outCue = rxFilename.cap(1);
	if (rxFilename.indexIn(outCue) != -1)
		outCue = rxFilename.cap(1) + ".flac.cue";
	else
		outCue += ".flac.cue";
	QFile outCueFile(outCue);
	outCueFile.open(QFile::WriteOnly | QFile::Truncate);
	inCuefile.open(QFile::ReadOnly);
	QTextStream cueInText(&inCuefile);
	QTextStream cueOutText(&outCueFile);
	cueInText.setCodec("Windows-1251");
	cueOutText.setCodec("Windows-1251");
	do {
		line = cueInText.readLine();
		if (rxSoundfile.indexIn(line) != -1)
		{
			if (rxFilename2.indexIn(atfApefile) != -1)
				if(rxFilename.indexIn(rxFilename2.cap(1)) != -1)
					cueOutText << "FILE \"" << rxFilename.cap(1) << ".flac\" WAVE" << endl;
		}
		else if (!line.isNull())
			cueOutText << line << endl;
	} while (!line.isNull());
	outCueFile.close();
	inCuefile.close();
	initDecoder();
}

// Инициализация конвеера
void ApeToFlac::initDecoder()
{
	GstElement *source, *dec, *conv, *encoder, *fileout;
	GstBus *bus;
	GstPad *audiopad;
	saveTotalTime = 0;
	progressBar->setValue(0);

	atfpipe = gst_pipeline_new ("atf-transcoder");

	// Входной файл
	source   = gst_element_factory_make ("filesrc", "file-source");
	g_object_set (source, "location", localFileNamesEncoder->fromUnicode(atfApefile).data(), NULL);

	dec = gst_element_factory_make ("decodebin", "decoder");
	g_signal_connect (dec, "new-decoded-pad", G_CALLBACK (cb_newpad), NULL);
	gst_bin_add_many (GST_BIN (atfpipe), source, dec, NULL);
	gst_element_link (source, dec);

	audiobin = gst_bin_new ("audiobin");
	conv = gst_element_factory_make ("audioconvert", "aconv");
	encoder = gst_element_factory_make ("flacenc", "audio-encoder");
	fileout = gst_element_factory_make ("filesink", "file-out");
	gst_bin_add_many (GST_BIN (audiobin), conv, encoder, fileout, NULL);
	gst_element_link_many (conv, encoder, fileout, NULL);
	audiopad = gst_element_get_static_pad (conv, "sink");
	gst_element_add_pad (audiobin, gst_ghost_pad_new ("sink", audiopad));
	gst_object_unref (audiopad);
	gst_bin_add (GST_BIN (atfpipe), audiobin);

	// Выходной файл
	g_object_set (fileout, "location", localFileNamesEncoder->fromUnicode(outFile).data(), NULL);

	bus = gst_pipeline_get_bus (GST_PIPELINE (atfpipe));
	gst_bus_add_watch (bus, bus_call, NULL);
	gst_object_unref (bus);

	g_object_set (G_OBJECT (encoder), "quality", 5, NULL);
	gst_element_set_state (atfpipe, GST_STATE_READY);
}

// Запуск конвеера
void ApeToFlac::startDecoder()
{
	gst_element_set_state (atfpipe, GST_STATE_PLAYING);
}

// Обновление прогрессбара
void ApeToFlac::progressUpd()
{
	gint64 pos, percent;
	int curpercent = 0;
	GstFormat fmt = GST_FORMAT_TIME;
	gst_element_query_position(atfpipe, &fmt, &pos);

	percent = (saveTotalTime / 100) * GST_MSECOND;
	curpercent = pos/percent;

	progressBar->setValue(curpercent);
}

void ApeToFlac::stopCode()
{
	progressBar->setValue(100);
	gst_element_set_state (atfpipe, GST_STATE_READY);
	timer->stop();
	if (checkBox->checkState() == Qt::Checked)
	{
		QFile::remove(atfCuefile);
		QFile::remove(atfApefile);
	}
	QStringList cuefilename;
	cuefilename << outCue;
	emit endHint(cuefilename);
}

int ApeToFlac::getDuration()
{
	gint64 basetime;
	GstFormat fmt = GST_FORMAT_TIME;
	if (gst_element_query_duration(atfpipe, &fmt, &basetime))
		return basetime / 1000000;
	else
		return -1;
}

void ApeToFlac::decoding()
{
	saveTotalTime = getDuration();
	if (saveTotalTime == -1)
	{
		g_print("Ошибка при запросе продолжительности\n");
		gst_element_set_state (atfpipe, GST_STATE_NULL);
		gst_object_unref (GST_OBJECT (atfpipe));
		close();
		return;
	}
	g_print("Продолжительность %d\n", saveTotalTime);
	timer->start(TIME);
}
