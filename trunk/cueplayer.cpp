#include <QtGui>
#include "cueplayer.h"
#include "callbacks.h"

#define TIME 200
#define TIMEOUT 3

CuePlayer::CuePlayer(QWidget *parent) : QWidget(parent), play(0)
{
	setupUi(this);
	cueplayer = this;
	timer = new QTimer(this);

	// Иксы
	xinfo = new QX11Info();
	display = xinfo->display();

	// Фильтр диалога
	filters << trUtf8("CUE образы (*.cue)")
			<< trUtf8("Видеофайлы (*.ogg *.ogv *.avi *.mkv *.mp4)")
			<< trUtf8("Аудиофайлы (*.mp3 *.flac *.ogg *.ogm)")
			<< trUtf8("Все файлы (*.*)")
			<< trUtf8("DVD каталог");

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
	videowindow = new VideoWindow(this);
	int vidWidth = videowindow->width();
	int vidHeight = videowindow->height();
	int vidWidthPos = desktop->width()/2 - vidWidth/2;
	videowindow->move(vidWidthPos, appHeightPos - vidHeight);
	win = videowindow->winId();
	secNumLCD->display("00");
	enableButtons(false);
	restoreSettings();
	apetoflacAction->setEnabled(false);
	openButton->setShortcut(trUtf8("Ctrl+o"));
	playButton->setShortcut(trUtf8("Ctrl+p"));
	pauseButton->setShortcut(trUtf8(" "));
	stopButton->setShortcut(trUtf8("Ctrl+s"));
	prewButton->setShortcut(trUtf8("Ctrl+r"));
	nextButton->setShortcut(trUtf8("Ctrl+n"));
	fileList->setShortcut(trUtf8("Ctrl+l"));

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
	connect(filedialog, SIGNAL(filterSelected(QString)),
	 this, SLOT(fileDialogFilter(QString)));

	// Видео окно
	connect(videowindow, SIGNAL(pauseEvent()), this, SLOT(pauseTrack()));
	connect(videowindow, SIGNAL(stopEvent()), this, SLOT(stopTrack()));
	connect(videowindow, SIGNAL(pressKeyRight()), this, SLOT(nminSeek()));
	connect(videowindow, SIGNAL(pressKeyUp()), this, SLOT(nmidSeek()));
	connect(videowindow, SIGNAL(pressKeyPgUp()), this, SLOT(nmaxSeek()));
	connect(videowindow, SIGNAL(pressKeyLeft()), this, SLOT(pminSeek()));
	connect(videowindow, SIGNAL(pressKeyDown()), this, SLOT(pmidSeek()));
	connect(videowindow, SIGNAL(pressKeyPgDown()), this, SLOT(pmaxSeek()));
	connect(videowindow, SIGNAL(aboutSig()), this, SLOT(about()));
	connect(videowindow, SIGNAL(sendAid(int)), this, SLOT(setAid(int)));
	connect(videowindow, SIGNAL(sendTid(int)), this, SLOT(setTid(int)));
	connect(videowindow, SIGNAL(sliderRelease()), this, SLOT(sliderVideoRelease()));
	connect(videowindow, SIGNAL(volumeChan(int)), this, SLOT(volumeValue(int)));
	connect(videowindow, SIGNAL(volumeChan(int)), volumeDial, SLOT(setValue(int)));
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
	QRegExp rxFilename2(".*/([^/]*)$");
	QString nextTool = nextButton->toolTip();
	QString prewTool = prewButton->toolTip();

	treeWidget->clear();
	treeWidget->hide();
	enableButtons(false);
	apetoflacAction->setEnabled(false);
	fileList->setChecked(false);
	cueFlag = false;
	multiCueFlag = false;
	videoFlag = false;
	dvdFlag = false;
	videowindow->hide();
	memset(multiFiles,0,100);
	mp3trackName = trUtf8("неизвестно");
	numTrack = 1;
	if (refparser)
	{
	    delete refparser;
		refparser = NULL;
	}
	if(play)
	{
		timer->stop();
		gst_element_set_state (play, GST_STATE_NULL);
		gst_object_unref (GST_OBJECT (play));
		play = NULL;
		g_print("Останов конвеера\n");
	}
	timeLineSlider->setSliderPosition(0);
	filename = filenames.join("");
	QFileInfo fi(filename);

	if (fi.suffix() == "cue" ||
		fi.suffix() == "CUE")
	{
		refparser = new CueParser(filename);
		setWindowTitle(refparser->getTitle());
		label->setText("1. " + refparser->getTrackTitle(numTrack));
		cueFlag = true;
		if (refparser->getTrackNumber() > 1 && refparser->getTrackFile(1) != refparser->getTrackFile(2))
		{
			multiCueFlag = true;
			cueplayer->multiCueInit();
			return;
		}
		preInit(refparser->getSoundFile());
		play = gst_element_factory_make ("playbin2", "play");
		g_object_set (G_OBJECT (play), "uri", ("file://" + refparser->getSoundFile()).toUtf8().data(), NULL);
		nextButton->setToolTip(nextTool);
		prewButton->setToolTip(prewTool);
	}
	else if (fi.suffix() == "mp3" ||
			 fi.suffix() == "flac" ||
			 fi.suffix() == "ogg" ||
			 fi.suffix() == "ogm" ||
			 fi.suffix() == "ogv" ||
			 fi.suffix() == "mp4" ||
			 fi.suffix() == "avi" ||
			 fi.suffix() == "ts" ||
			 fi.suffix() == "wv" ||
			 fi.suffix() == "mkv")
	{
		if (rxFilename2.indexIn(filename) != -1)
			mp3trackName = rxFilename2.cap(1);
		setWindowTitle(mp3trackName);
		label->setText(mp3trackName);
		preInit(filename);
		play = gst_element_factory_make ("playbin2", "play");
		g_object_set (G_OBJECT (play), "uri", ("file://" + filename).toUtf8().data(), NULL);
	}
	else if (fi.isDir())
	{
		d_title = 1;
		createDvdPipe();
		nextButton->setEnabled(true);
		nextButton->setToolTip(trUtf8("Следующая дорожка"));
		prewButton->setEnabled(true);
		prewButton->setToolTip(trUtf8("Предыдущая дорожка"));
	}
	else
	{
		setWindowTitle(trUtf8("Формат не поддерживается"));
		label->setText(trUtf8("откройте файл"));
		return;
	}

	settings.setValue("player/recentfile", filename);

	if (videoFlag)
	{
		videosink = gst_element_factory_make ("xvimagesink", "xvideoout");
		g_object_set(G_OBJECT (videosink), "display", display, NULL);
		g_object_set(G_OBJECT (videosink), "force-aspect-ratio", true, NULL);
		g_object_set (G_OBJECT (play), "video-sink", videosink, NULL);
		gst_x_overlay_set_xwindow_id (GST_X_OVERLAY(videosink), win);
	}

	bus = gst_pipeline_get_bus (GST_PIPELINE (play));
	gst_bus_add_watch (bus, bus_callback, play);
	gst_object_unref (bus);

	playProbe();
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
	if (dvdFlag && d_title < 10)
		{
			g_object_set (G_OBJECT (dvdsrc), "title", ++d_title, NULL);
			label->setText(trUtf8("DVD видео : ") + QString::number(d_title));
			stopTrack();
			if(playProbe())
				return;
			else
				createDvdPipe();
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
	if (dvdFlag && d_title > 1)
		{
			g_object_set (G_OBJECT (dvdsrc), "title", --d_title, NULL);
			label->setText(trUtf8("DVD видео : ") + QString::number(d_title));
			stopTrack();
			if(playProbe())
				return;
			else
				createDvdPipe();
		}
}

// воспроизведение трека
void CuePlayer::playTrack()
{
	if (videoFlag || dvdFlag)
	{
		gst_x_overlay_set_xwindow_id (GST_X_OVERLAY (videosink), win);
		gst_x_overlay_expose (GST_X_OVERLAY (videosink));
		videowindow->show();
	}
	gst_element_set_state (play, GST_STATE_PLAYING);
	timer->start(TIME);
	if (videoFlag)
	{
		if (gst_element_get_state( GST_ELEMENT(play), &state, NULL, GST_SECOND * TIMEOUT) != GST_STATE_CHANGE_SUCCESS)
		{
			g_print ("Аварийный останов.\n");
			cueplayer->stopAll();
		}
		if (state == GST_STATE_PLAYING)
		{
			int naudio, currentaudio;
			// Количество аудио дорожек
			g_object_get (G_OBJECT (play), "n-audio", &naudio, NULL);
			// Текущая дорожка
			g_object_get (G_OBJECT (play), "current-audio", &currentaudio, NULL);
			videowindow->createAudioMenu(naudio - 1, currentaudio);

			int ntext, currenttext;
			// Количество cубтитров
			g_object_get (G_OBJECT (play), "n-text", &ntext, NULL);
			// Текущие субтитры
			g_object_get (G_OBJECT (play), "current-text", &currenttext, NULL);
			videowindow->createTitleMenu(ntext - 1, currenttext);
		}
	}
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
	{
		timeLineSlider->setSliderPosition(time - totalTime);
		if (videoFlag || dvdFlag) videowindow->setSliderPos(time - totalTime);
	}
	if (timeLineSlider->value() == timeLineSlider->maximum())
		CuePlayer::playNextTrack();
}

// ручное перемещение слайдера
void CuePlayer::sliderRelease()
{
		int time = (totalTime + timeLineSlider->value()) * 1000;
		seekGst(time);
}

// ручное перемещение слайдера видеоокна
void CuePlayer::sliderVideoRelease()
{
	int time = videowindow->getSliderPos() * 1000;
	seekGst(time);
}

// регулировка громкости
void CuePlayer::volumeValue(int volume)
{
	if (dvdFlag)
		g_object_set(d_volume, "volume", (double)volume / 100, NULL);
	else
		g_object_set(play, "volume", (double)volume / 100, NULL);
	volumeDial->setToolTip(QString::number(volume) + "%");
	settings.setValue("player/volume", volume);
	if (videoFlag || dvdFlag) videowindow->setVolumePos(volume);
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
		videowindow->setSliderMaximum(totalTimeAlbum);
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
	if(dvdFlag)
		g_object_set(d_volume, "volume", (double)volumeDial->value() / 100, NULL);
	else
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
	if (videoFlag || dvdFlag)
	{
		videowindow->hide();
		videowindow->newTrack();
	}
	timeLineSlider->setSliderPosition(0);
	seekAndLCD(numTrack);
}

void CuePlayer::timerUpdate()
{
	gint64 p;
	GstFormat fmt = GST_FORMAT_TIME;
	gst_element_query_position(play, &fmt, &p);
	tick((qint64) p / 1000000);
}

int CuePlayer::getDuration()
{
	static gint64 basetime;
	GstFormat fmt = GST_FORMAT_TIME;
	gst_element_query_duration(play, &fmt, &basetime);
	return basetime / 1000000;
}

int CuePlayer::getPosition()
{
	gint64 p;
	GstFormat fmt = GST_FORMAT_TIME;
	gst_element_query_position(play, &fmt, &p);
	return p / 1000000;
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

void CuePlayer::paramFile(QStringList list)
{
	cueFileSelected(list);
	playTrack();
}

void CuePlayer::restoreSettings()
{
	bool ok;
	if (settings.value("player/recentfile").toBool())
		cueFileSelected(settings.value("player/recentfile").toStringList());
	if (settings.value("player/volume").toBool())
	{
		volumeDial->setValue(settings.value("player/volume").toInt(&ok));
		if (videoFlag || dvdFlag) videowindow->setVolumePos(settings.value("player/volume").toInt(&ok));
	}
}

void CuePlayer::nminSeek()
{
	seekGst(getPosition() + 10000);
}

void CuePlayer::nmidSeek()
{
	seekGst(getPosition() + 60000);
}

void CuePlayer::nmaxSeek()
{
	seekGst(getPosition() + 600000);
}

void CuePlayer::pminSeek()
{
	seekGst(getPosition() - 10000);
}

void CuePlayer::pmidSeek()
{
	seekGst(getPosition() - 60000);
}

void CuePlayer::pmaxSeek()
{
	seekGst(getPosition() - 600000);
}

void CuePlayer::setAid(int n)
{
	g_object_set(G_OBJECT (play), "current-audio", n, NULL);
}

void CuePlayer::setTid(int n)
{
	if (n == 20)
		g_object_set (G_OBJECT (play), "flags", 0x00000013, NULL);
	else
		g_object_set(G_OBJECT (play), "current-text", n, "flags", 0x00000017, NULL);
}

void CuePlayer::fileDialogFilter(QString filter)
{
	if (filter == trUtf8("DVD каталог"))
	{
		filedialog->setFileMode(QFileDialog::Directory);
		filedialog->setNameFilters(filters);
		filedialog->selectFilter(trUtf8("DVD каталог"));
	}
	else
		filedialog->setFileMode(QFileDialog::AnyFile);
}

bool CuePlayer::playProbe()
{
	if (!dvdFlag) g_object_set(play, "mute", true, NULL);
	gst_element_set_state (play, GST_STATE_PLAYING);
	if (gst_element_get_state( GST_ELEMENT(play), &state, NULL, GST_SECOND * TIMEOUT) != GST_STATE_CHANGE_SUCCESS)
	{
		timer->stop();
		gst_element_set_state (play, GST_STATE_NULL);
		gst_object_unref (GST_OBJECT (play));
		play = NULL;
		g_print("Останов конвеера\n");
	}

	if (state == GST_STATE_PLAYING)
	{
		int duration = getDuration();
		if (cueFlag)
			initAlbum(duration);
		else
			initFile();

		stopAll();
		if (!dvdFlag) g_object_set(play, "mute", false, NULL);
		return true;
	}
	else
	{
		label->setText(trUtf8("ошибка инициализации файла"));
		emit gstError();
		play = NULL;
		return false;
	}
}

void CuePlayer::createDvdPipe()
{
	GstElement *typefind, *demuxer, *audiosink, *vdecoder, *adecoder, *ffmpegcolorspace;
	GstElement *audioconvert, *audioresample;
	GstElement *aqueue, *vqueue;
	GstPad *audiopad, *videopad;

	dvdFlag = true;
	setWindowTitle(trUtf8("DVD видео"));
	label->setText(trUtf8("DVD видео : 1"));

	aqueue = make_queue ();
	vqueue = make_queue ();

	g_print("Выбран каталог, формат DVD\n");
	play = gst_pipeline_new ("pipe");
	dvdsrc = gst_element_factory_make ("dvdreadsrc", "dvdsrc");
	g_object_set (G_OBJECT (dvdsrc), "device", filename.toUtf8().data(), NULL);
	typefind = gst_element_factory_make ("typefind", "typefinder");
	g_signal_connect (typefind, "have-type", G_CALLBACK (cb_typefound), NULL);
	demuxer = gst_element_factory_make ("dvddemux", "demux");
	audiosink = gst_element_factory_make ("autoaudiosink", "asink");
	vdecoder = gst_element_factory_make ("mpeg2dec", "decoder");
	adecoder = gst_element_factory_make ("a52dec", "adec");
	audioconvert = gst_element_factory_make ("audioconvert", "aconv");
	audioresample = gst_element_factory_make ("audioresample", "asampl");
	d_volume = gst_element_factory_make ("volume", "volume");
	ffmpegcolorspace = gst_element_factory_make ("ffmpegcolorspace", "ffmpegcol");
	videosink = gst_element_factory_make ("xvimagesink", "xvideoout");
	g_object_set (G_OBJECT (videosink), "force-aspect-ratio", true, NULL);
	gst_x_overlay_set_xwindow_id (GST_X_OVERLAY(videosink), win);

	gst_bin_add_many (GST_BIN (play), dvdsrc, typefind, demuxer, NULL);
	gst_element_link_many (dvdsrc, typefind, demuxer, NULL);
	g_signal_connect (demuxer, "pad-added", G_CALLBACK (on_pad_added), NULL);
	//g_signal_connect (play, "deep-notify",
	//	G_CALLBACK (gst_object_default_deep_notify), NULL);

	/* create audio output */
	d_audio = gst_bin_new ("audiobin");
	gst_bin_add_many (GST_BIN (d_audio), aqueue, adecoder, audioconvert, audioresample, d_volume, audiosink, NULL);
	gst_element_link_many(aqueue, adecoder, audioconvert, audioresample, d_volume, audiosink, NULL);

	audiopad = gst_element_get_static_pad (aqueue, "sink");
	gst_element_add_pad (d_audio, gst_ghost_pad_new ("sink", audiopad));
	gst_object_unref (audiopad);
	gst_bin_add (GST_BIN (play), d_audio);

	/* create video output */
	d_video = gst_bin_new ("videobin");
	gst_bin_add_many (GST_BIN (d_video), vqueue, vdecoder, ffmpegcolorspace, videosink, NULL);
	gst_element_link_many (vqueue, vdecoder, ffmpegcolorspace, videosink, NULL);

	videopad = gst_element_get_static_pad (vqueue, "sink");
	gst_element_add_pad (d_video, gst_ghost_pad_new ("sink", videopad));
	gst_object_unref (videopad);
	gst_bin_add (GST_BIN (play), d_video);
}

GstThread::GstThread(QObject *parent) : QThread(parent)
{

}

void GstThread::run()
{

}
