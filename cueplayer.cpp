#include <QtGui>
#include "cueplayer.h"

#define TIME 200

GstState state;
bool cueFlag;
bool multiCueFlag;

CuePlayer *cueplayer = 0;

static gboolean
bus_callback (GstBus     *bus,
		  GstMessage *msg,
		  gpointer    data)
{
	bus = NULL;
	GstElement *playbin = (GstElement *) data;
	GValue *valartist, *valalbum, *valtitle;

  switch (GST_MESSAGE_TYPE (msg)) {

	  case GST_MESSAGE_EOS: {
		g_print ("Конец потока\n");
		if (!multiCueFlag) cueplayer->stopAll();
		break;
	}
	case GST_MESSAGE_STATE_CHANGED: {
		gst_element_get_state( GST_ELEMENT(playbin), &state, NULL, GST_CLOCK_TIME_NONE);
		break;
	}
	case GST_MESSAGE_TAG: {
		GstTagList *taglist=gst_tag_list_new();
		gst_message_parse_tag(msg, &taglist);
		valartist = (GValue*)gst_tag_list_get_value_index (taglist, GST_TAG_ARTIST, 0);
		valalbum = (GValue*)gst_tag_list_get_value_index (taglist, GST_TAG_ALBUM, 0);
		valtitle = (GValue*)gst_tag_list_get_value_index(taglist, GST_TAG_TITLE, 0);
		if (!cueFlag)
			cueplayer->setMp3Title(valtitle, valalbum, valartist);
		break;
	}
	case GST_MESSAGE_ERROR: {
		gchar  *debug;
		GError *error;

		gst_message_parse_error (msg, &error, &debug);
		g_free (debug);

		QMessageBox::critical(0, QObject::trUtf8("Ошибка"), QObject::trUtf8(error->message));
		g_error_free (error);

		gst_element_set_state (playbin, GST_STATE_NULL);
		gst_object_unref (GST_OBJECT (playbin));
		playbin = NULL;
		break;
	}
	default:
	  break;
  }

  return TRUE;
}

static void
cb_typefound (GstElement *typefind,
		  guint       probability,
		  GstCaps    *caps,
		  gpointer    data)
{
	typefind = NULL;
	data = NULL;
	gchar *type;

	type = gst_caps_to_string (caps);
	g_print ("Обнаружен файл типа %s, инфа %d%%\n", type, probability);
	if (!strcmp(type ,"application/x-ape"))
		cueplayer->apeFound(true);
	else
		cueplayer->apeFound(false);
	g_free (type);
}

CuePlayer::CuePlayer(QWidget *parent) : QWidget(parent), play(0)
{
	setupUi(this);
	cueplayer = this;
	timer = new QTimer(this);
	filters << trUtf8("CUE образы и mp3 (*.cue *.mp3)")
		 << trUtf8("CUE образы (*.cue)");

	//расположение главного окна по центру экрана
	desktop = QApplication::desktop();
	int appWidth = width();
	int appHeight = height();
	int appWidthPos = desktop->width()/2 - appWidth/2;
	int appHeightPos = desktop->height()/2 - appHeight/2;
	move(appWidthPos, appHeightPos);

	label->setText(trUtf8("откройте файл"));
	treeWidget->hide();
	createTrayIconMenu();
	layout()->setSizeConstraint(QLayout::SetFixedSize);
	refparser = NULL;
	filedialog = new QFileDialog(this, trUtf8("Открыть файл"));
	filedialog->setNameFilters(filters);
	secNumLCD->display("00");
	enableButtons(false);
	apetoflacAction->setEnabled(false);
	pauseButton->setShortcut(trUtf8(" "));

	connect(timeLineSlider, SIGNAL(valueChanged(int)),
	 this, SLOT(setNumLCDs(int)));
	connect(openButton, SIGNAL(clicked()),
	 filedialog, SLOT(exec()));
	connect(filedialog, SIGNAL(filesSelected(QStringList)),
	 this, SLOT(cueFileSelected(QStringList)));
	connect(apetoflac, SIGNAL(endHint(QStringList)),
	 this, SLOT(cueFileSelected(QStringList)));
	connect(nextButton, SIGNAL(clicked()), 
	 this, SLOT(playNextTrack()));
	connect(prewButton, SIGNAL(clicked()), 
	 this, SLOT(playPrewTrack()));
	connect(playButton, SIGNAL(clicked()), 
	 this, SLOT(playTrack()));
	connect(stopButton, SIGNAL(clicked()), 
	 this, SLOT(stopTrack()));
	connect(pauseButton, SIGNAL(clicked()), 
	 this, SLOT(pauseTrack()));
	connect(timeLineSlider, SIGNAL(sliderReleased()),
	 this, SLOT(sliderRelease()));
	connect(volumeDial, SIGNAL(valueChanged(int)),
	 this, SLOT(volumeValue(int)));
	connect(treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
	 this, SLOT(listItemClicked(QTreeWidgetItem*,int)));
	connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
	 this, SLOT(trayClicked(QSystemTrayIcon::ActivationReason)));
	connect(timer, SIGNAL(timeout()), this, SLOT(timerUpdate()));
}

void CuePlayer::setNumLCDs(int sec)
{
	int min = sec / 60;
	if (sec >= 60)
		sec %= 60;
	if (sec < 10)
	{
		QString newvalue = "0" + QString::number(sec,10);
		secNumLCD->display(newvalue);
	}
	else
		secNumLCD->display(sec);
	minNumLCD->display(min);
}

// инициализация аудиофайла
void CuePlayer::cueFileSelected(QStringList filenames)
{
	QRegExp rxFilename(".*\\.(.{3})");
	QRegExp rxFilename2(".*/([^/]*)$");

	treeWidget->clear();
	treeWidget->hide();
	enableButtons(false);
	apetoflacAction->setEnabled(false);
	fileList->setChecked(false);
	cueFlag = false;
	multiCueFlag = false;
	memset(multiFiles,0,100);
	mp3trackName = trUtf8("неизвестно");
	numTrack = 1;
	if (refparser)
	{
	    delete refparser;
		refparser = NULL;
	}
	filename = filenames.join("");
	if (rxFilename.indexIn(filename) != -1)
	{
		if (rxFilename.cap(1) == "cue")
		{
			refparser = new CueParser(filename);
			setWindowTitle(refparser->getTitle());
			label->setText("1. " + refparser->getTrackTitle(numTrack));
			cueFlag = true;
		}
		else if (rxFilename.cap(1) == "mp3")
		{
			if (rxFilename2.indexIn(filename) != -1)
				mp3trackName = rxFilename2.cap(1);
			setWindowTitle(mp3trackName);
			label->setText(mp3trackName);
		}
	}
	timeLineSlider->setSliderPosition(0);

	if(play)
	{
		timer->stop();
		gst_element_set_state (play, GST_STATE_NULL);
		gst_object_unref (GST_OBJECT (play));
		play = NULL;
		g_print("Останов конвеера\n");
	}

	if (cueFlag && refparser->getTrackNumber() > 1 && refparser->getTrackFile(1) != refparser->getTrackFile(2))
	{
		multiCueFlag = true;
		multiCueInit();
		return;
	}

	play = gst_element_factory_make ("playbin2", "play");
	if (cueFlag)
		g_object_set (G_OBJECT (play), "uri", ("file://" + refparser->getSoundFile()).toUtf8().data(), NULL);
	else
		g_object_set (G_OBJECT (play), "uri", ("file://" + filename).toUtf8().data(), NULL);


	bus = gst_pipeline_get_bus (GST_PIPELINE (play));
	gst_bus_add_watch (bus, bus_callback, play);
	gst_object_unref (bus);

	if (cueFlag)
		preInit(refparser->getSoundFile());
	else
		preInit(filename);
	g_object_set(play, "mute", true, NULL);
	gst_element_set_state (play, GST_STATE_PLAYING);
	gst_element_get_state( GST_ELEMENT(play), &state, NULL, GST_CLOCK_TIME_NONE);
	if (state == GST_STATE_PLAYING)
	{
		int duration = getDuration();
		if (cueFlag)
			initAlbum(duration);
		else
			initFile();
		stopAll();
		g_object_set(play, "mute", false, NULL);
	}
	else
	{
		label->setText(trUtf8("ошибка инициализации аудиофайла"));
		emit gstError();
		play = NULL;
	}
}

// переключение на следующий трек
void CuePlayer::playNextTrack()
{
	if (refparser)
		if (numTrack < refparser->getTrackNumber())
		{
			++numTrack;
			checkState();
		}
}

// переключение на предыдущий трек
void CuePlayer::playPrewTrack()
{
	if (refparser)
		if (numTrack > 1)
		{
			--numTrack;
			checkState();
		}
}

// воспроизведение трека
void CuePlayer::playTrack()
{
	gst_element_set_state (play, GST_STATE_PLAYING);
	timer->start(TIME);
}

// остановка воспроизведения трека/альбома
void CuePlayer::stopTrack()
{
	if (state == GST_STATE_PLAYING)
	{
		if (multiCueFlag)
		{
			gst_element_set_state (play, GST_STATE_READY);
			timer->stop();
			timeLineSlider->setSliderPosition(0);
		}
		else if (cueFlag)
			gst_element_set_state (play, GST_STATE_PAUSED);
		else
			stopAll();
		seekAndLCD(numTrack);
	}
	else
	{
		stopAll();
		if(multiCueFlag)
		{
			gst_element_set_state (play, GST_STATE_NULL);
			g_object_set (G_OBJECT (play), "uri", ("file://" + refparser->getTrackFile(numTrack)).toUtf8().data(), NULL);
			gst_element_set_state (play, GST_STATE_READY);
		}
	}
}

// триггер паузы
void CuePlayer::pauseTrack()
{
	if (state == GST_STATE_PAUSED)
		gst_element_set_state (play, GST_STATE_PLAYING);
	else
		gst_element_set_state (play, GST_STATE_PAUSED);
}

// автоматическое перемещение слайдера
void CuePlayer::tick(qint64 time)
{
	time /= 1000;
	if (!timeLineSlider->isSliderDown())
		timeLineSlider->setSliderPosition(time - totalTime);
	if (timeLineSlider->value() == timeLineSlider->maximum())
		CuePlayer::playNextTrack();
}

// ручное перемещение слайдера
void CuePlayer::sliderRelease()
{
		int time = (totalTime + timeLineSlider->value()) * 1000;
		seekGst(time);
}

// регулировка громкости
void CuePlayer::volumeValue(int volume)
{
	g_object_set(play, "volume", (double)volume / 100, NULL);
	volumeDial->setToolTip(QString::number(volume) + "%");
}

// переключение между треками и индикация
void CuePlayer::seekAndLCD(int num)
{
	QString stringNumTrack;
	int totalTimeNext = 0;
	int totalTimeAlbum = getDuration();
	stringNumTrack.setNum(num);
	if (refparser)
	{
		label->setText(stringNumTrack + ". " + refparser->getTrackTitle(num));
		if (!multiCueFlag)
		{
			seekGst(refparser->getTrackIndex(num));
			totalTime = refparser->getTrackIndex(num);
			totalTime /= 1000;
			if (num < refparser->getTrackNumber())
				totalTimeNext = refparser->getTrackIndex(num+1);
			else
				totalTimeNext = totalTimeAlbum;
			totalTimeNext /= 1000;
			totalTimeNext -= totalTime;
		}
		else
		{
			totalTimeNext = multiFiles[num]/1000;
			totalTime = 0;
		}
		timeLineSlider->setMaximum(totalTimeNext);
		treeWidget->setCurrentItem(treeWidget->topLevelItem(num - 1));
	}
	else
	{
		totalTime = 0;
		timeLineSlider->setMaximum(totalTimeAlbum/1000);
	}
}

// переключение в списке воспроизведения
void CuePlayer::listItemClicked(QTreeWidgetItem *item, int column)
{
	column = 0;
	if (numTrack != (treeWidget->indexOfTopLevelItem(item) + 1))
	{
		numTrack = treeWidget->indexOfTopLevelItem(item) + 1;
		checkState();
	}
}

// инициализация после загрузки аудиофайла (CUE)
void CuePlayer::initAlbum(int totalTimeAlbum)
{
	treeWidget->setHeaderLabels(QStringList() << trUtf8("Композиция") << trUtf8("Время"));
	treeWidget->setColumnWidth(0, 380);
	treeWidget->setColumnWidth(1, 30);
	qint64 startTime = refparser->getTrackIndex(1);
	qint64 currentTime;
	QString strSec;
	for (int i = 1; i <= refparser->getTrackNumber(); i++)
	{
		if (i < refparser->getTrackNumber())
			currentTime = refparser->getTrackIndex(i + 1) - startTime;
		else
			currentTime = totalTimeAlbum - startTime;
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
	treeWidget->setCurrentItem(treeWidget->topLevelItem(0));
	enableButtons(true);
	transcoder->setFileName(filename, totalTimeAlbum);
	connect(this, SIGNAL(gstError()), transcoder, SLOT(close()));
	g_object_set(play, "volume", (double)volumeDial->value() / 100, NULL);
}

// инициализация после загрузки аудиофайла (mp3)
void CuePlayer::initFile()
{
	transcoder->close();
	playButton->setEnabled(true);
	pauseButton->setEnabled(true);
	stopButton->setEnabled(true);
	g_object_set(play, "volume", (double)volumeDial->value() / 100, NULL);
}

// создание иконки в лотке и контекстных меню
void CuePlayer::createTrayIconMenu()
{
	quitAction = new QAction(trUtf8("&Выход"), this);
	quitAction->setIcon(QIcon(":/images/application-exit.png"));
	connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));

	aboutAction = new QAction(trUtf8("&О программе"), this);
	aboutAction->setIcon(QIcon(":/images/help-about.png"));
	connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));

	transcoder = new TransCoder(this);
	transcodeAction = new QAction(trUtf8("Конвертор"), this);
	transcodeAction->setIcon(QIcon(":/images/convertor.png"));
	connect(transcodeAction, SIGNAL(triggered()), transcoder, SLOT(show()));

	apetoflac = new ApeToFlac(this);
	apetoflacAction = new QAction(trUtf8("Ape->Flac"), this);
	apetoflacAction->setIcon(QIcon(":/images/hint.png"));
	connect(apetoflacAction, SIGNAL(triggered()), apetoflac, SLOT(show()));

	trayIcon = new QSystemTrayIcon(this);
	trayIconMenu = new QMenu(this);
	trayIconMenu->addAction(apetoflacAction);
	trayIconMenu->addAction(transcodeAction);
	trayIconMenu->addAction(aboutAction);
	trayIconMenu->addAction(quitAction);
	trayIcon = new QSystemTrayIcon(this);
	trayIcon->setContextMenu(trayIconMenu);
	trayIcon->setIcon(QIcon(":/images/knotify.png"));
	trayIcon->show();

	addAction(apetoflacAction);
	addAction(transcodeAction);
	addAction(aboutAction);
	addAction(quitAction);
	setContextMenuPolicy(Qt::ActionsContextMenu);
}

void CuePlayer::closeEvent(QCloseEvent *event)
{
	if (!trayIcon->isVisible())
	{
		hide();
		event->ignore();
	}
}

void CuePlayer::trayClicked(QSystemTrayIcon::ActivationReason reason)
{
	if (reason == QSystemTrayIcon::Trigger)
	{
	if (isVisible())
		hide();
	else
		show();
	}
}

void CuePlayer::about()
{
	QMessageBox::information(this, trUtf8("О программе"),
							 trUtf8("<h2>CUE проигрыватель</h2>"
									"<p>Проигрыватель музыкальных медиаобразов."
									"<p><p>Разработчик: <a href=xmpp:ipse@ipse.zapto.org name=jid type=application/xmpp+xml>ipse</a>"));
}

void CuePlayer::stopAll()
{
	numTrack = 1;
	gst_element_set_state (play, GST_STATE_READY);
	timer->stop();
	timeLineSlider->setSliderPosition(0);
	seekAndLCD(numTrack);
}

void CuePlayer::timerUpdate()
{
	gint64 p;
	GstFormat fmt = GST_FORMAT_TIME;
	gst_element_query_position(play, &fmt, &p);
	tick((qint64) p / 1000000);
	//emit position((double)(p - Gstart) / Glength);
}

int CuePlayer::getDuration()
{
	static gint64 basetime;
	GstFormat fmt = GST_FORMAT_TIME;
	gst_element_query_duration(play, &fmt, &basetime);
	return basetime / 1000000;
}

void CuePlayer::seekGst(int time)
{
	GstClockTime nach   = (GstClockTime)(time * GST_MSECOND);
	if (!gst_element_seek(play, 1.0,
			GST_FORMAT_TIME,
			(GstSeekFlags)(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE),
			GST_SEEK_TYPE_SET, nach,
			GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE))
		qDebug() << QString(trUtf8("Ошибка поиска"));
	else
		g_print("Перемещение на позицию %d\n", time);
}

void CuePlayer::enableButtons(bool a)
{
	playButton->setEnabled(a);
	pauseButton->setEnabled(a);
	stopButton->setEnabled(a);
	prewButton->setEnabled(a);
	nextButton->setEnabled(a);
	fileList->setEnabled(a);
	transcodeAction->setEnabled(a);
}

void CuePlayer::checkState()
{
	if (multiCueFlag)
	{
			gst_element_set_state (play, GST_STATE_NULL);
			g_object_set (G_OBJECT (play), "uri", ("file://" + refparser->getTrackFile(numTrack)).toUtf8().data(), NULL);
			gst_element_set_state (play, GST_STATE_READY);
			gst_element_get_state( GST_ELEMENT(play), &state, NULL, GST_CLOCK_TIME_NONE);
	}
	if (state == GST_STATE_READY)
		playTrack();
	gst_element_get_state( GST_ELEMENT(play), &state, NULL, GST_CLOCK_TIME_NONE);
	if (state == GST_STATE_PLAYING)
		seekAndLCD(numTrack);
	else if (state == GST_STATE_PAUSED)
	{
		seekAndLCD(numTrack);
		playTrack();
	}
}

void CuePlayer::setMp3Title(GValue *vtitle, GValue *valbum, GValue *vartist)
{
	if (vtitle)
		label->setText(trUtf8(g_value_get_string(vtitle)));
	if (valbum && vartist)
		setWindowTitle(trUtf8(g_value_get_string(vartist)) + " - " + trUtf8(g_value_get_string(valbum)));
}

void CuePlayer::preInit(QString filename)
{
	GstElement *pipeline, *filesrc, *typefind, *fakesink;

	pipeline = gst_pipeline_new ("pipe");

	filesrc = gst_element_factory_make ("filesrc", "source");
	g_object_set (G_OBJECT (filesrc), "location", filename.toUtf8().data(), NULL);
	typefind = gst_element_factory_make ("typefind", "typefinder");
	g_signal_connect (typefind, "have-type", G_CALLBACK (cb_typefound), NULL);
	fakesink = gst_element_factory_make ("fakesink", "sink");

	gst_bin_add_many (GST_BIN (pipeline), filesrc, typefind, fakesink, NULL);
	gst_element_link_many (filesrc, typefind, fakesink, NULL);
	gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PLAYING);

	gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_NULL);
	gst_object_unref (GST_OBJECT (pipeline));
}

void CuePlayer::apeFound(bool ape)
{
	if (ape) apetoflacAction->setEnabled(true);
	if (ape) apetoflac->setFileNames(filename, refparser->getSoundFile());
	if (!ape) apetoflac->close();
}

void CuePlayer::multiCueInit()
{
	GstState st;
	GstElement *fakesink;
	int duration = 0;
	QString strSec;

	treeWidget->setHeaderLabels(QStringList() << trUtf8("Композиция") << trUtf8("Время"));
	treeWidget->setColumnWidth(0, 380);
	treeWidget->setColumnWidth(1, 30);

	for (int i = 1; i <= refparser->getTrackNumber(); i++)
	{
		play = gst_element_factory_make ("playbin2", "play");
		fakesink = gst_element_factory_make ("fakesink", "fake");
		g_object_set (G_OBJECT (play), "uri", ("file://" + refparser->getTrackFile(i)).toUtf8().data(), NULL);
		g_object_set(play, "audio-sink", fakesink, NULL);
		gst_element_set_state (play, GST_STATE_PLAYING);
		gst_element_get_state( GST_ELEMENT(play), &st, NULL, GST_CLOCK_TIME_NONE);
		if (st == GST_STATE_PLAYING)
		{
			duration = getDuration();
			//g_print ("Продолжительность трека %d: %d\n", i, duration);
			gst_element_set_state (play, GST_STATE_NULL);
			gst_object_unref (GST_OBJECT (play));
			play = NULL;
		}
		int sec = duration / 1000;
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
		multiFiles[i] = duration;
	}
	treeWidget->setCurrentItem(treeWidget->topLevelItem(0));
	numTrack = 1;
	play = gst_element_factory_make ("playbin2", "play");
	g_object_set (G_OBJECT (play), "uri", ("file://" + refparser->getTrackFile(numTrack)).toUtf8().data(), NULL);
	bus = gst_pipeline_get_bus (GST_PIPELINE (play));
	gst_bus_add_watch (bus, bus_callback, play);
	gst_object_unref (bus);
	gst_element_set_state (play, GST_STATE_PAUSED);
	seekAndLCD(numTrack);
	playButton->setEnabled(true);
	pauseButton->setEnabled(true);
	stopButton->setEnabled(true);
	prewButton->setEnabled(true);
	nextButton->setEnabled(true);
	fileList->setEnabled(true);
}
