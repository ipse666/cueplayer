#include "cueplayer.h"
#include "callbacks.h"

#define TIME 200
#define TIMEOUT 3

#define KEY_PLAY 162
#define KEY_PLAY_CURVE 172

#define KEY_STOP 164

#define KEY_PREV 144
#define KEY_PREV_CURVE 166

#define KEY_NEXT 153
#define KEY_NEXT_CURVE 167

#define KEY_VOL_UP 176
#define KEY_VOL_UP_CURVE 123

#define KEY_VOL_DOWN 174
#define KEY_VOL_DOWN_CURVE 122

#define KEY_MUTE 160
#define KEY_MUTE_CURVE 121

#define KEY_EJECT 204

enum Func {
	NULL_FUNC,
	POST_PLAY,
	POST_CHECK
};

enum EndOp {
	NULL_RET,
	ERR_STATE,
	FUNC_PLAY,
	FUNC_CHECK
};

CuePlayer::CuePlayer(QWidget *parent) : QWidget(parent), play(0)
{
	setupUi(this);
	cueplayer = this;
	timer = new QTimer(this);
	trdtimer = new QTimer(this);
	paramtimer = new QTimer(this);
	trd = new GstThread(this);

	// Иксы
	xinfo = new QX11Info();
	display = xinfo->display();
	videoProcess = new QProcess(this);
	primaryDPMS = checkDPMS();

	// Фильтр диалога
	filters << trUtf8("Все поддерживаемые файлы (*.cue *.ogg *.ogv *.avi *.mkv *.mp4 *.mp3 *.flac *.ogm)")
			<< trUtf8("CUE образы (*.cue)")
			<< trUtf8("Видеофайлы (*.ogg *.ogv *.avi *.mkv *.mp4)")
			<< trUtf8("Аудиофайлы (*.mp3 *.flac *.ogg *.ogm)")
			<< trUtf8("Все файлы (*.*)")
			<< trUtf8("Каталог с файлами или DVD каталог");

	//расположение главного окна по центру экрана
	desktop = QApplication::desktop();
	int appWidth = width();
	int appHeight = height();
	int appWidthPos = desktop->width()/2 - appWidth/2;
	int appHeightPos = desktop->height()/2 - appHeight/2;
	move(appWidthPos, appHeightPos);

	// Drag and Drop
	setAcceptDrops(true);

	label->setText(trUtf8("откройте файл"));
	treeWidget->hide();
	dvdButton->hide();
	streamButton->hide();
	createTrayIconMenu();
	layout()->setSizeConstraint(QLayout::SetFixedSize);
	refparser = NULL;
	filedialog = new QFileDialog(this, trUtf8("Открыть файл"));
	filedialog->setOption(QFileDialog::DontUseNativeDialog, true);
	filedialog->setNameFilters(filters);
	videowindow = new VideoWindow(this);
	winman = new WidgetManager(this);
	int vidWidth = videowindow->width();
	int vidHeight = videowindow->height();
	int vidWidthPos = desktop->width()/2 - vidWidth/2;
	videowindow->move(vidWidthPos, appHeightPos - vidHeight);
	win = videowindow->winId();
	streamform = new StreamForm(this);
	plparser = new PlParser(this);
	secNumLCD->display("00");
	enableButtons(false);
	restoreSettings();
	apetoflacAction->setEnabled(false);
	streamButton->setShortcut(trUtf8("Ctrl+u"));
	dvdButton->setShortcut(trUtf8("Ctrl+d"));
	openButton->setShortcut(trUtf8("Ctrl+o"));
	playButton->setShortcut(trUtf8("Ctrl+p"));
	pauseButton->setShortcut(trUtf8(" "));
	stopButton->setShortcut(trUtf8("Ctrl+s"));
	prewButton->setShortcut(trUtf8("Ctrl+r"));
	nextButton->setShortcut(trUtf8("Ctrl+n"));
	fileList->setShortcut(trUtf8("Ctrl+l"));

	connect(timeLineSlider, SIGNAL(valueChanged(int)),
	 this, SLOT(setNumLCDs(int)));
	connect(dvdButton, SIGNAL(clicked()), this, SLOT(discSet()));
	connect(streamButton, SIGNAL(clicked()),
	 streamform, SLOT(show()));
	connect(openButton, SIGNAL(clicked()),
	 filedialog, SLOT(exec()));
	connect(filedialog, SIGNAL(filesSelected(QStringList)),
	 this, SLOT(cueFileSelected(QStringList)));
	connect(apetoflac, SIGNAL(endHint(QStringList)),
	 this, SLOT(cueFileSelected(QStringList)));
	connect(streamform, SIGNAL(streamOk(QStringList)),
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
	connect(timeLineSlider, SIGNAL(actionTriggered(int)),
	 this, SLOT(sliderValue(int)));
	connect(volumeDial, SIGNAL(valueChanged(int)),
	 this, SLOT(volumeValue(int)));
	connect(treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
	 this, SLOT(listItemClicked(QTreeWidgetItem*,int)));
	connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
	 this, SLOT(trayClicked(QSystemTrayIcon::ActivationReason)));
	connect(timer, SIGNAL(timeout()), this, SLOT(timerUpdate()));
	connect(filedialog, SIGNAL(filterSelected(QString)),
	 this, SLOT(fileDialogFilter(QString)));
	connect(trd, SIGNAL(started()), this, SLOT(threadRunInd()));
	connect(trdtimer, SIGNAL(timeout()), this, SLOT(threadRunProgress()));
	connect(trd, SIGNAL(terminated()), this, SLOT(threadStop()));
	connect(trd, SIGNAL(finished()), this, SLOT(threadStop()));
	connect(paramtimer, SIGNAL(timeout()), this, SLOT(checkReady()));

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
	connect(videowindow, SIGNAL(videoExit()), this, SLOT(endBlock()));
	connect(videowindow, SIGNAL(draganddrop(QStringList)),
	 this, SLOT(cueFileSelected(QStringList)));
	connect(videowindow, SIGNAL(newTime(int)), this, SLOT(seekGst(int)));
	connect(videowindow, SIGNAL(showWin(bool)), this, SLOT(mover(bool)));
	connect(videowindow, SIGNAL(doubleClick(bool)), this, SLOT(dclIntVw(bool)));

	// Плейлист парсер
	connect(plparser, SIGNAL(ready()), this, SLOT(plInit()));
	connect(plparser, SIGNAL(plperror(QString)), this, SLOT(plError(QString)));

	// Эквалайзер
	connect(equalizer, SIGNAL(bandsValue(double*)), this, SLOT(equalizerChang(double*)));
}

void CuePlayer::setServer(SingleServer *ss)
{
	connect(ss, SIGNAL(sendArgs(QStringList)), this, SLOT(paramFile(QStringList)));
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
	QRegExp rxFilename("^/.*");
	QRegExp rxFilename2(".*/([^/]*)$");
	QRegExp rxFilename3("^(mms://|http://).*");
#ifdef FTPPLAY
	QRegExp rxFilename4("^ftp://.*");
#endif
	QRegExp rxFilename5("^http://www.youtube.com/watch\\?v=.*");
	QRegExp rxFilename6("^http://youtube.com/get_video\\?video_id=.*");
	QString nextTool = nextButton->toolTip();
	QString prewTool = prewButton->toolTip();

	initPlayer();
	filename = filenames.join("");

	QFileInfo fi(filename);

	if (!QString::compare(fi.suffix(), "cue", Qt::CaseInsensitive))
	{
		refparser = new CueParser(filename);
		setWindowsTitles(refparser->getTitle());
		label->setText("1. " + refparser->getTrackTitle(numTrack));
		cueFlag = true;
		if (refparser->getTrackNumber() > 1 && refparser->getTrackFile(1) != refparser->getTrackFile(2))
		{
			multiCueFlag = true;
			cueplayer->multiCueInit();
			settings.setValue("player/recentfile", filename);
			return;
		}
		preInit(refparser->getSoundFile());
		if (!videoFlag && setEqualizerAction->isChecked())
		{
			/* Это говно нихуя не работает,
			 надо писать нормальный playbin */
			progressiveMode(refparser->getSoundFile());
		}
		else
		{
			play = gst_element_factory_make ("playbin2", "play");
			g_object_set (G_OBJECT (play), "uri", ("file://" + refparser->getSoundFile()).toUtf8().data(), NULL);
		}
		nextButton->setToolTip(nextTool);
		prewButton->setToolTip(prewTool);
	}
	else if (!QString::compare(fi.suffix(), "aac", Qt::CaseInsensitive) ||
			 !QString::compare(fi.suffix(), "mp3", Qt::CaseInsensitive) ||
			 !QString::compare(fi.suffix(), "flac", Qt::CaseInsensitive) ||
			 !QString::compare(fi.suffix(), "ape", Qt::CaseInsensitive) ||
			 !QString::compare(fi.suffix(), "ogg", Qt::CaseInsensitive) ||
			 !QString::compare(fi.suffix(), "ogm", Qt::CaseInsensitive) ||
			 !QString::compare(fi.suffix(), "ogv", Qt::CaseInsensitive) ||
			 !QString::compare(fi.suffix(), "mp4", Qt::CaseInsensitive) ||
			 !QString::compare(fi.suffix(), "avi", Qt::CaseInsensitive) ||
			 !QString::compare(fi.suffix(), "ts", Qt::CaseInsensitive) ||
			 !QString::compare(fi.suffix(), "wv", Qt::CaseInsensitive) ||
			 !QString::compare(fi.suffix(), "3gp", Qt::CaseInsensitive) ||
			 !QString::compare(fi.suffix(), "m2ts", Qt::CaseInsensitive) ||
			 !QString::compare(fi.suffix(), "vob", Qt::CaseInsensitive) ||
			 !QString::compare(fi.suffix(), "wmv", Qt::CaseInsensitive) ||
			 !QString::compare(fi.suffix(), "wav", Qt::CaseInsensitive) ||
			 !QString::compare(fi.suffix(), "mpg", Qt::CaseInsensitive) ||
			 !QString::compare(fi.suffix(), "mov", Qt::CaseInsensitive) ||
			 !QString::compare(fi.suffix(), "m4v", Qt::CaseInsensitive) ||
			 !QString::compare(fi.suffix(), "m4a", Qt::CaseInsensitive) ||
			 !QString::compare(fi.suffix(), "mka", Qt::CaseInsensitive) ||
			 !QString::compare(fi.suffix(), "mkv", Qt::CaseInsensitive))
	{
		if (rxFilename2.indexIn(filename) != -1)
			mp3trackName = rxFilename2.cap(1);
#ifdef FTPPLAY
		if (rxFilename4.indexIn(filename) != -1)
		{
			ftpFlag = true;
			mp3trackName = rxFilename4.cap(1);
			createFtpPipe();
			settings.setValue("player/recentfile", filename);
			initFile();
			return;
		}
#endif
		setWindowsTitles(mp3trackName);
		label->setText(mp3trackName);

		if (fi.suffix() == "ts")
		{
			/* Этот блок с тонной костылей
			   создан исключительно для
			   перемотки ts файлов */

			GstElement *typefind, *audiosink, *ffmpegcolorspace;
			GstElement *audioconvert, *audioresample;
			GstElement *aqueue, *vqueue;
			GstPad *audiopad, *videopad;

			tsFlag = true;

			aqueue = gst_element_factory_make ("queue", NULL);
			vqueue = gst_element_factory_make ("queue", NULL);
			demuxer = gst_element_factory_make ("decodebin2", "bindecoder");

			play = gst_pipeline_new ("ts-player");
			tsfile = gst_element_factory_make ("filesrc", "file-source");
			g_object_set (tsfile, "location", filename.toUtf8().data(), NULL);

			typefind = gst_element_factory_make ("typefind", "typefinder");
			g_signal_connect (typefind, "have-type", G_CALLBACK (cb_typefound), NULL);

			audiosink = gst_element_factory_make ("autoaudiosink", "asink");
			audioconvert = gst_element_factory_make ("audioconvert", "aconv");
			audioresample = gst_element_factory_make ("audioresample", "asampl");
			d_volume = gst_element_factory_make ("volume", "volume");
			ffmpegcolorspace = gst_element_factory_make ("ffmpegcolorspace", "ffmpegcol");
			videosink = gst_element_factory_make ("xvimagesink", "xvideoout");
			g_object_set (G_OBJECT (videosink), "force-aspect-ratio", true, NULL);
			gst_x_overlay_set_xwindow_id (GST_X_OVERLAY(videosink), win);

			gst_bin_add_many (GST_BIN (play), tsfile, typefind, demuxer, NULL);
			gst_element_link_many (tsfile, typefind,  demuxer, NULL);
			g_signal_connect (demuxer, "new-decoded-pad", G_CALLBACK (new_pad_added), NULL);

			d_audio = gst_bin_new ("audiobin");
			gst_bin_add_many (GST_BIN (d_audio), aqueue, audioconvert, audioresample, d_volume, audiosink, NULL);
			gst_element_link_many(aqueue, audioconvert, audioresample, d_volume, audiosink, NULL);

			audiopad = gst_element_get_static_pad (aqueue, "sink");
			gst_element_add_pad (d_audio, gst_ghost_pad_new ("sink", audiopad));
			gst_object_unref (audiopad);
			gst_bin_add (GST_BIN (play), d_audio);

			d_video = gst_bin_new ("videobin");
			gst_bin_add_many (GST_BIN (d_video), vqueue, ffmpegcolorspace, videosink, NULL);
			gst_element_link_many (vqueue, ffmpegcolorspace, videosink, NULL);

			videopad = gst_element_get_static_pad (vqueue, "sink");
			gst_element_add_pad (d_video, gst_ghost_pad_new ("sink", videopad));
			gst_object_unref (videopad);
			gst_bin_add (GST_BIN (play), d_video);
		}
		else
		{
			preInit(filename);

			if (!videoFlag && setEqualizerAction->isChecked())
			{
				progressiveMode(filename);
			}
			else
			{
				play = gst_element_factory_make ("playbin2", "play");
				if (rxFilename.indexIn(filename) != -1)
					g_object_set (G_OBJECT (play), "uri", ("file://" + filename).toUtf8().data(), NULL);
				else
					g_object_set (G_OBJECT (play), "uri", filename.toUtf8().data(), NULL);
			}
		}
	}
	else if (!QString::compare(fi.suffix(), "flv", Qt::CaseInsensitive))
	{
		videoFlag = true;
		streamFlag = true;
		setWindowsTitles(trUtf8("FLV-видео"));
		label->setText(fi.fileName());
		play = gst_element_factory_make ("playbin2", "play");
		if (rxFilename.indexIn(filename) != -1)
			g_object_set (G_OBJECT (play), "uri", ("file://" + filename).toUtf8().data(), NULL);
		else
			g_object_set (G_OBJECT (play), "uri", filename.toUtf8().data(), NULL);
	}
	else if (!QString::compare(fi.suffix(), "pls", Qt::CaseInsensitive) ||
			 !QString::compare(fi.suffix(), "wvx", Qt::CaseInsensitive) ||
			 !QString::compare(fi.suffix(), "m3u", Qt::CaseInsensitive))
	{
		setWindowsTitles(trUtf8("Список воспроизведения"));
		label->setText(trUtf8("Подождите. Инициализация списка."));
		plparser->setPlUri(filename);
		return;
	}
	else if (fi.isDir())
	{
		bool isDvd = false;
		int counterVOB = 0;
		int counterFiles = 0;
		QDir dir = fi.filePath();
		QFileInfoList fileInfoList = dir.entryInfoList();
		QFileInfoList filesList;
		while (fileInfoList.size())
		{
			QFileInfo filetu = fileInfoList.last();
			fileInfoList.pop_back();
			if (!QString::compare(filetu.suffix(), "VOB", Qt::CaseSensitive))
				counterVOB++;
			else if (!QString::compare(filetu.suffix(), "aac", Qt::CaseInsensitive) ||
					 !QString::compare(filetu.suffix(), "mp3", Qt::CaseInsensitive) ||
					 !QString::compare(filetu.suffix(), "flac", Qt::CaseInsensitive) ||
					 !QString::compare(filetu.suffix(), "ogg", Qt::CaseInsensitive) ||
					 !QString::compare(filetu.suffix(), "ogm", Qt::CaseInsensitive) ||
					 !QString::compare(filetu.suffix(), "ogv", Qt::CaseInsensitive) ||
					 !QString::compare(filetu.suffix(), "mp4", Qt::CaseInsensitive) ||
					 !QString::compare(filetu.suffix(), "avi", Qt::CaseInsensitive) ||
					 !QString::compare(filetu.suffix(), "ts", Qt::CaseInsensitive) ||
					 !QString::compare(filetu.suffix(), "wv", Qt::CaseInsensitive) ||
					 !QString::compare(filetu.suffix(), "3gp", Qt::CaseInsensitive) ||
					 !QString::compare(filetu.suffix(), "m2ts", Qt::CaseInsensitive) ||
					 !QString::compare(filetu.suffix(), "vob", Qt::CaseInsensitive) ||
					 !QString::compare(filetu.suffix(), "wmv", Qt::CaseInsensitive) ||
					 !QString::compare(filetu.suffix(), "wav", Qt::CaseInsensitive) ||
					 !QString::compare(filetu.suffix(), "mpg", Qt::CaseInsensitive) ||
					 !QString::compare(filetu.suffix(), "mov", Qt::CaseInsensitive) ||
					 !QString::compare(filetu.suffix(), "m4v", Qt::CaseInsensitive) ||
					 !QString::compare(filetu.suffix(), "m4a", Qt::CaseInsensitive) ||
					 !QString::compare(filetu.suffix(), "mka", Qt::CaseInsensitive) ||
					 !QString::compare(filetu.suffix(), "mkv", Qt::CaseInsensitive))
			{
				filesList.prepend(filetu);
				counterFiles++;
			}
			if (counterVOB == 3)
			{
				isDvd = true;
				break;
			}
		}
		if (isDvd)
		{
			d_title = 1;
			createDvdPipe();
			nextButton->setEnabled(true);
			nextButton->setToolTip(trUtf8("Следующая дорожка"));
			prewButton->setEnabled(true);
			prewButton->setToolTip(trUtf8("Предыдущая дорожка"));
		}
		else if (counterFiles)
		{
			multiFileFlag = true;
			settings.setValue("player/recentfile", filename);
			multiFileInit(filesList);
			return;
		}
		else
		{
			setWindowsTitles(trUtf8("Медиафайлы не обнаружены"));
			label->setText(trUtf8("откройте файл"));
			return;
		}
	}
	else if (rxFilename5.indexIn(filename) != -1)
	{
		videoFlag = true;
		setWindowsTitles(trUtf8("Ютуб"));
		label->setText(trUtf8("Ютуб"));
		youtuber = new YouTubeDL(filename);
		connect(youtuber, SIGNAL(putUrl(QStringList)), this, SLOT(cueFileSelected(QStringList)));
		return;
	}
	else if (rxFilename6.indexIn(filename) != -1)
	{
		videoFlag = true;
		setWindowsTitles(trUtf8("Ютуб"));
		label->setText(trUtf8("Ютуб"));
		play = gst_element_factory_make ("playbin2", "play");
		g_object_set (G_OBJECT (play), "uri", filename.toUtf8().data(), NULL);
	}
	else if (rxFilename3.indexIn(filename) != -1)
	{
		setWindowsTitles(trUtf8("Радио"));
		label->setText(trUtf8("Радио"));
		play = gst_element_factory_make ("playbin2", "play");

		g_object_set (G_OBJECT (play), "uri", filename.toUtf8().data(), NULL);

		settings.setValue("player/recentfile", filename);

		bus = gst_pipeline_get_bus (GST_PIPELINE (play));
		gst_bus_add_watch (bus, bus_callback, play);
		gst_object_unref (bus);

		initFile();

		return;
	}
	else
	{
		setWindowsTitles(trUtf8("Формат не поддерживается"));
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

	if (!streamFlag)
		playProbe();
	//g_signal_connect (play, "deep-notify", G_CALLBACK (gst_object_default_deep_notify), NULL); // Дебаг
}

// Инициализация диска
void CuePlayer::discSet()
{
	initPlayer();
	d_title = 1;
	discFlag = true;
	createDvdPipe();
	nextButton->setEnabled(true);
	nextButton->setToolTip(trUtf8("Следующая дорожка"));
	prewButton->setEnabled(true);
	prewButton->setToolTip(trUtf8("Предыдущая дорожка"));
	bus = gst_pipeline_get_bus (GST_PIPELINE (play));
	gst_bus_add_watch (bus, bus_callback, play);
	gst_object_unref (bus);

	playProbe();
}

void CuePlayer::initPlayer()
{
	treeWidget->clear();
	treeWidget->hide();
	enableButtons(false);
	apetoflacAction->setEnabled(false);
	fileList->setChecked(false);
	cueFlag = false;
	multiCueFlag = false;
	multiFileFlag = false;
	videoFlag = false;
	dvdFlag = false;
	preInitFlag = false;
	discFlag = false;
	streamFlag = false;
	ftpFlag = false;
	tsFlag = false;
	progFlag = false;
	playButtonFlag = false;
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
	if (trd->isRunning())
	{
		trd->terminate();
		threadStop();
	}
	timeLineSlider->setSliderPosition(0);
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
	if (dvdFlag && d_title < 99)
		{
			g_object_set (G_OBJECT (dvdsrc), "title", ++d_title, NULL);
			label->setText(trUtf8("DVD видео : ") + QString::number(d_title));
			stopTrack();
			if(playProbe())
			{
				if (!nextButton->isEnabled())
					nextButton->setEnabled(true);
				return;
			}
			else
				createDvdPipe();
			playPrewTrack();
			nextButton->setEnabled(false);
		}
	if (multiFileFlag)
	{
		if (numTrack < saveFileList.size())
		{
			++numTrack;
			checkState();
		}
		else
			stopAll();
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
			{
				if (!nextButton->isEnabled())
					nextButton->setEnabled(true);
				return;
			}
			else
				createDvdPipe();
		}
	if (multiFileFlag)
	{
		if (numTrack > 1)
		{
			--numTrack;
			checkState();
		}
	}
}

// воспроизведение трека
void CuePlayer::playTrack()
{
	if (videoFlag || dvdFlag)
	{
		if (intWindAction->isChecked())
			integVideo(true);
		else
			integVideo(false);
		gst_x_overlay_set_xwindow_id (GST_X_OVERLAY (videosink), win);
		gst_x_overlay_expose (GST_X_OVERLAY (videosink));
		videowindow->show();
		videowindow->setWindowTitle(savetitle);
		dpmsTrigger(false);
		winman->raiseWidget(this);
	}
	trd->setPlayBin(play);
	trd->setFunc(POST_PLAY);
	trd->start();
	playButtonFlag = true;
}

// остановка воспроизведения трека/альбома
void CuePlayer::stopTrack()
{
	playButtonFlag = false;
	if (state == GST_STATE_PLAYING)
	{
		if (multiCueFlag || multiFileFlag)
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
	gst_element_get_state( GST_ELEMENT(play), &state, NULL, GST_SECOND * TIMEOUT);
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
	if (timeLineSlider->value() == timeLineSlider->maximum() &&
		!streamFlag &&
		!multiFileFlag)
		CuePlayer::playNextTrack();
}

// ручное перемещение слайдера
void CuePlayer::sliderRelease()
{
	int time = (totalTime + timeLineSlider->value()) * 1000;
	seekGst(time);
}

// ручное перемещение слайдера (кликом)
void CuePlayer::sliderValue(int i)
{
	int time;
	switch (i)
	{
	case QAbstractSlider::SliderPageStepAdd:
		break;
	case QAbstractSlider::SliderPageStepSub:
		break;
	default:
		return;
	}
	QCursor cursor = timeLineSlider->cursor();
	QPoint pos = this->pos() + timeLineSlider->pos();
	int cursorpixpos = cursor.pos().x() - pos.x();
	double percentsld = (double)timeLineSlider->maximum()/100;
	int percentpix = cursorpixpos / ((double)timeLineSlider->size().width()/100);
	time = (totalTime + (percentsld * percentpix)) * 1000;
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
	if (dvdFlag || ftpFlag || tsFlag || progFlag)
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
		if (playButtonFlag)
		{
			qDebug() << trUtf8("Играет: ") +
								refparser->getPerformer() + " - " +
								refparser->getAlbum() + " / " +
								refparser->getTrackTitle(num);
			if (tray) trayIcon->showMessage(trUtf8("Играет"),
								refparser->getTrackTitle(num) + "\n" +
								refparser->getPerformer() + " - " +
								refparser->getAlbum(),
								QSystemTrayIcon::Information,
								2000);
		}
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
	else if (multiFileFlag)
	{
		totalTimeNext = multiFiles[num]/1000;
		totalTime = 0;
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
	if (progFlag)
		g_object_set(d_volume, "volume", (double)volumeDial->value() / 100, NULL);
	else
		g_object_set(play, "volume", (double)volumeDial->value() / 100, NULL);
}

// инициализация после загрузки аудиофайла (mp3)
void CuePlayer::initFile()
{
	transcoder->close();
	playButton->setEnabled(true);
	pauseButton->setEnabled(true);
	stopButton->setEnabled(true);
	if(dvdFlag || ftpFlag || tsFlag || progFlag)
		g_object_set(d_volume, "volume", (double)volumeDial->value() / 100, NULL);
	else
		g_object_set(play, "volume", (double)volumeDial->value() / 100, NULL);
}

// создание иконки в лотке и контекстных меню
void CuePlayer::createTrayIconMenu()
{
	quitAction = new QAction(trUtf8("&Выход"), this);
	quitAction->setIcon(QIcon(":/images/application-exit.png"));
	quitAction->setShortcut(trUtf8("Ctrl+q"));
	connect(quitAction, SIGNAL(triggered()), this, SLOT(endBlock()));

	aboutAction = new QAction(trUtf8("&О программе"), this);
	aboutAction->setIcon(QIcon(":/images/help-about.png"));
	aboutAction->setShortcut(trUtf8("Ctrl+a"));
	connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));

	intWindAction = new QAction(trUtf8("&Интеграция"), this);
	intWindAction->setCheckable(true);
	intWindAction->setShortcut(trUtf8("Ctrl+i"));
	connect(intWindAction, SIGNAL(triggered(bool)), this, SLOT(intWindCheck(bool)));

	extbutAction = new QAction(trUtf8("&Расширенный"), this);
	extbutAction->setCheckable(true);
	extbutAction->setShortcut(trUtf8("Ctrl+e"));
	connect(extbutAction, SIGNAL(triggered(bool)), this, SLOT(extButtons(bool)));

	transcoder = new TransCoder(this);
	transcodeAction = new QAction(trUtf8("Конвертор"), this);
	transcodeAction->setIcon(QIcon(":/images/convertor.png"));
	transcodeAction->setShortcut(trUtf8("Ctrl+c"));
	connect(transcodeAction, SIGNAL(triggered()), transcoder, SLOT(show()));
	connect(transcoder, SIGNAL(transQuit()), this, SLOT(endBlock()));

	apetoflac = new ApeToFlac(this);
	apetoflacAction = new QAction(trUtf8("Ape->Flac"), this);
	apetoflacAction->setIcon(QIcon(":/images/hint.png"));
	apetoflacAction->setShortcut(trUtf8("Ctrl+h"));
	connect(apetoflacAction, SIGNAL(triggered()), this, SLOT(ape2flacShow()));

	equalizer = new Equalizer(0);
	int appHeight = height();
	int eqWidth = equalizer->width();
	int eqHeight = equalizer->height();
	int eqWidthPos = desktop->width()/2 - eqWidth/2;
	int appHeightPos = desktop->height()/2 - appHeight/2;
	equalizer->move(eqWidthPos, appHeightPos - eqHeight);

	equalizerAction = new QAction(trUtf8("Эквалайзер"), this);
	equalizerAction->setIcon(QIcon(":/images/equalizer.png"));
	equalizerAction->setShortcut(trUtf8("Ctrl+z"));

	setEqualizerAction = new QAction(trUtf8("&Включить"), this);
	setEqualizerAction->setCheckable(true);
	setEqualizerAction->setShortcut(trUtf8("Ctrl+v"));
	connect(setEqualizerAction, SIGNAL(triggered(bool)), this, SLOT(equalizerCheck(bool)));
	connect(setEqualizerAction, SIGNAL(toggled(bool)), this, SLOT(equalizerCheck(bool)));

	editEqualizerAction = new QAction(trUtf8("&Настройки"), this);
	editEqualizerAction->setIcon(QIcon(":/images/equalizer.png"));
	editEqualizerAction->setShortcut(trUtf8("Ctrl+j"));
	editEqualizerAction->setEnabled(false);
	connect(editEqualizerAction, SIGNAL(triggered()), equalizer, SLOT(show()));

	equalizerMenu = new QMenu(this);
	equalizerAction->setMenu(equalizerMenu);
	equalizerMenu->addAction(setEqualizerAction);
	equalizerMenu->addAction(editEqualizerAction);

	// Кнопки
	playAction = new QAction(trUtf8("&Играть"), this);
	playAction->setIcon(QIcon(":/images/play.png"));
	playAction->setShortcut(trUtf8("Ctrl+p"));
	connect(playAction, SIGNAL(triggered(bool)), this, SLOT(playTrack()));

	pauseAction = new QAction(trUtf8("&Пауза"), this);
	pauseAction->setIcon(QIcon(":/images/pause.png"));
	pauseAction->setShortcut(trUtf8(" "));
	connect(pauseAction, SIGNAL(triggered(bool)), this, SLOT(pauseTrack()));

	stopAction = new QAction(trUtf8("&Стоп"), this);
	stopAction->setIcon(QIcon(":/images/stop.png"));
	stopAction->setShortcut(trUtf8("Ctrl+s"));
	connect(stopAction, SIGNAL(triggered(bool)), this, SLOT(stopTrack()));

	prewAction = new QAction(trUtf8("П&редыдущий"), this);
	prewAction->setIcon(QIcon(":/images/prev.png"));
	prewAction->setShortcut(trUtf8("Ctrl+r"));
	connect(prewAction, SIGNAL(triggered(bool)), this, SLOT(playPrewTrack()));

	nextAction = new QAction(trUtf8("С&ледующий"), this);
	nextAction->setIcon(QIcon(":/images/next.png"));
	nextAction->setShortcut(trUtf8("Ctrl+n"));
	connect(nextAction, SIGNAL(triggered(bool)), this, SLOT(playNextTrack()));

	trayIcon = new QSystemTrayIcon(this);
	trayIconMenu = new QMenu(this);
	trayIconMenu->addAction(playAction);
	trayIconMenu->addAction(pauseAction);
	trayIconMenu->addAction(stopAction);
	trayIconMenu->addAction(prewAction);
	trayIconMenu->addAction(nextAction);
	trayIconMenu->addAction(aboutAction);
	trayIconMenu->addAction(quitAction);
	trayIcon = new QSystemTrayIcon(this);
	trayIcon->setContextMenu(trayIconMenu);
	trayIcon->setIcon(QIcon(":/images/cueplayer.png"));
	trayIcon->show();
	trayIcon->setToolTip(trUtf8("CuePlayer"));

	addAction(extbutAction);
	addAction(intWindAction);
	addAction(equalizerAction);
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
	QDate curdate = QDate::currentDate();
	QMessageBox::information(this, trUtf8("О программе"),
							 trUtf8("<h2>CuePlayer 0.23-svn</h2>"
									"<p>Дата ревизии: ")
									+ QString::number(27) +  " "
									+ QString(curdate.longMonthName(3)) +  " "
									+ QString::number(2010) +
									trUtf8("<p>Мультимедиа проигрыватель."
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
		dpmsTrigger(true);
	}
	timeLineSlider->setSliderPosition(0);
	if (multiFileFlag)
	{
		QRegExp rxFilename("^\\w{3,5}://.*");
		QFileInfo filetu = saveFileList.at(numTrack-1);
		gst_element_set_state (play, GST_STATE_NULL);
		g_object_set(play, "mute", true, NULL);
		if (rxFilename.indexIn(filetu.filePath()) != -1)
			g_object_set (G_OBJECT (play), "uri", filetu.filePath().toUtf8().data(), NULL);
		else
			g_object_set (G_OBJECT (play), "uri", ("file://" + filetu.absoluteFilePath()).toUtf8().data(), NULL);
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
			gst_element_set_state (play, GST_STATE_READY);
			g_object_set(play, "mute", false, NULL);
		}
	}
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
	if (!tsFlag)
	{
		GstClockTime nach   = (GstClockTime)(time * GST_MSECOND);
		if (!gst_element_seek(play, 1.0,
				GST_FORMAT_TIME,
				(GstSeekFlags)(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE),
				GST_SEEK_TYPE_SET, nach,
				GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE))
			qDebug() << QString(trUtf8("Ошибка поиска"));
	}
	else
	{
		gint64 pos, dur;
		gst_element_set_state (play, GST_STATE_READY);
		GstFormat fmt = GST_FORMAT_BYTES;
		gst_element_query_position (tsfile,
									&fmt,
									&pos);
		gst_element_query_duration (tsfile,
									&fmt,
									&dur);
		pos = (pos/188)*188;
		gint64 shift = pos+time*1880;
		if (!gst_element_seek_simple(tsfile,
					GST_FORMAT_BYTES,
					(GstSeekFlags)(GST_SEEK_FLAG_SKIP),
					shift))
			qDebug() << QString(trUtf8("Ошибка поиска ts"));
		gst_element_set_state (play, GST_STATE_PLAYING);
		gst_bin_recalculate_latency ((GstBin*)d_video);
		//g_print("Позиция: %ld, смещение %ld, размер %ld. Время: %d\n", pos, shift, dur, time);
	}
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
	}
	else if (multiFileFlag)
	{
		QRegExp rxFilename("^\\w{3,5}://.*");
		QString finame;
		qDebug() << numTrack;
		QFileInfo filetu = saveFileList.at(numTrack-1);
		gst_element_set_state (play, GST_STATE_NULL);

		if (rxFilename.indexIn(filetu.filePath()) != -1)
		{
			setEqualizerAction->setChecked(false);
			finame = filetu.filePath();
		}
		else
			finame = "file://" + filetu.absoluteFilePath();

		if (!videoFlag && setEqualizerAction->isChecked())
			g_object_set (aufile, "location", filetu.absoluteFilePath().toUtf8().data(), NULL);
		else
			g_object_set (G_OBJECT (play), "uri", finame.toUtf8().data(), NULL);
		gst_element_set_state (play, GST_STATE_READY);
	}
	trd->setPlayBin(play);
	trd->setFunc(POST_CHECK);
	trd->start();
}

void CuePlayer::setMp3Title(GValue *vtitle, GValue *valbum, GValue *vartist)
{
	if (vtitle)
	{
		label->setText(trUtf8(g_value_get_string(vtitle)));
		if (trUtf8(g_value_get_string(vartist)).isEmpty() || trUtf8(g_value_get_string(valbum)).isEmpty())
			qDebug() << trUtf8("Играет: ") + trUtf8(g_value_get_string(vtitle));
		else
			qDebug() << trUtf8("Играет: ") +
				trUtf8(g_value_get_string(vartist)) + " - " +
				trUtf8(g_value_get_string(valbum)) + " / " +
				trUtf8(g_value_get_string(vtitle));
		prewlabel = label->text(); // Необходимо сохранить асинхронную метку
	}
	if (valbum && vartist)
		setWindowsTitles(trUtf8(g_value_get_string(vartist)) + " - " + trUtf8(g_value_get_string(valbum)));
}

// Обнаружение типа файла
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
	if (!ape) apetoflac->close();
}

void CuePlayer::ape2flacShow()
{
	apetoflac->setFileNames(filename, refparser->getSoundFile());
	apetoflac->show();
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
		gst_element_get_state( GST_ELEMENT(play), &st, NULL, GST_SECOND * TIMEOUT);
		if (st == GST_STATE_PLAYING)
		{
			duration = getDuration();
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

void CuePlayer::multiFileInit(QFileInfoList fileInfoList)
{
	QRegExp rxFilename("^\\w{3,5}://.*");
	GstState st;
	GstElement *fakesink;
	int duration = 0;
	QString strSec;
	QString finame;
	saveFileList = fileInfoList;
	treeWidget->clear();

	treeWidget->setHeaderLabels(QStringList() << trUtf8("Композиция") << trUtf8("Время"));
	treeWidget->setColumnWidth(0, 380);
	treeWidget->setColumnWidth(1, 30);

	for (int i = 1; i <= fileInfoList.size(); i++)
	{
		QFileInfo filetu = fileInfoList.at(i-1);

		play = gst_element_factory_make ("playbin2", "play");
		fakesink = gst_element_factory_make ("fakesink", "fake");

		if (rxFilename.indexIn(filetu.filePath()) != -1)
		{
			qDebug() << trUtf8("Инициализация канала: ") + filetu.filePath();
			g_object_set (G_OBJECT (play), "uri", filetu.filePath().toUtf8().data(), NULL);
		}
		else
			g_object_set (G_OBJECT (play), "uri", ("file://" + filetu.absoluteFilePath()).toUtf8().data(), NULL);
		g_object_set(play, "audio-sink", fakesink, NULL);
		gst_element_set_state (play, GST_STATE_PLAYING);
		gst_element_get_state( GST_ELEMENT(play), &st, NULL, GST_SECOND * TIMEOUT);
		if (st == GST_STATE_PLAYING)
		{
			duration = getDuration();
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
		playlistItem[i-1]->setText(0, numTrackList + ". " + filetu.fileName());
		playlistItem[i-1]->setText(1, QString::number(min, 10) + ":" + strSec);
		playlistItem[i-1]->setTextAlignment(1, Qt::AlignRight);
		treeWidget->insertTopLevelItem(i-1, playlistItem[i-1]);
		multiFiles[i] = duration;
	}
	treeWidget->setCurrentItem(treeWidget->topLevelItem(0));
	numTrack = 1;

	QFileInfo filetu = fileInfoList.at(numTrack - 1);

	if (rxFilename.indexIn(filetu.filePath()) != -1)
	{
		finame = filetu.filePath();
		setEqualizerAction->setChecked(false);
	}
	else
		finame = "file://" + filetu.absoluteFilePath();

	if (!videoFlag && setEqualizerAction->isChecked())
	{
		progressiveMode(filetu.absoluteFilePath());
		g_object_set(d_volume, "volume", (double)volumeDial->value() / 100, NULL);
	}
	else
	{
		play = gst_element_factory_make ("playbin2", "play");
		g_object_set (G_OBJECT (play), "uri", finame.toUtf8().data(), NULL);
		g_object_set(play, "volume", (double)volumeDial->value() / 100, NULL);
	}

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
	paramtimer->start(TIME);
}

void CuePlayer::restoreSettings()
{
	bool ok;
	setEqualizerAction->setChecked(settings.value("player/equalizer").toBool());
	editEqualizerAction->setEnabled(settings.value("player/equalizer").toBool());
	if (settings.value("player/recentfile").toBool() && qApp->argc() <= 1)
		cueFileSelected(settings.value("player/recentfile").toStringList());
	if (settings.value("player/volume").toBool())
	{
		volumeDial->setValue(settings.value("player/volume").toInt(&ok));
		if (videoFlag || dvdFlag) videowindow->setVolumePos(settings.value("player/volume").toInt(&ok));
	}
	intWindAction->setChecked(settings.value("player/intwin").toBool());
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

// Выбор аудиодорожки видеофайла
void CuePlayer::setAid(int n)
{
	// Выбор аудио playbin
	if (videoFlag && !tsFlag)
	{
		gint flags;
		g_object_get (G_OBJECT (play), "flags", &flags, NULL);
		if (n == 20)
		{
			flags &= ~0x00000002;
			g_object_set (G_OBJECT (play), "flags", flags, "current-audio", -1, NULL);
		}
		else
		{
			flags |= 0x00000002;
			g_object_set(G_OBJECT (play), "flags", flags, "current-audio", n, NULL);
		}
	}
	// Выбор аудио DVD
	else if (dvdFlag || tsFlag)
	{
		gchar *curpad, *nextpad;
		curpad = getDvdAudio(dvdAudioCurrentPad);
		nextpad = getDvdAudio(n);
		//g_print("текущий пад %s, запрашиваемый пад %s\n", curpad, nextpad); // Дебаг!
		if (gst_pad_unlink(gst_element_get_static_pad (demuxer, curpad), gst_element_get_static_pad (d_audio, "sink")))
		{
			if (!gst_pad_link (gst_element_get_static_pad (demuxer, nextpad), gst_element_get_static_pad (d_audio, "sink")))
				dvdAudioCurrentPad = n;
			else
			{
				videowindow->createAudioMenu(dvdAudioPads - 1, dvdAudioCurrentPad);
				gst_pad_link (gst_element_get_static_pad (demuxer, curpad), gst_element_get_static_pad (d_audio, "sink"));
			}
		}
		g_free(curpad);
		g_free(nextpad);
	}
}

// Выбор субтитров видеофайла
void CuePlayer::setTid(int n)
{
	gint flags;
	g_object_get (G_OBJECT (play), "flags", &flags, NULL);
	if (n == 20)
	{
		flags &= ~0x00000004;
		g_object_set (G_OBJECT (play), "flags", flags, NULL);
	}
	else
	{
		flags |= 0x00000004;
		g_object_set(G_OBJECT (play), "flags", flags, "current-text", n, NULL);
	}
}

void CuePlayer::fileDialogFilter(QString filter)
{
	if (filter == trUtf8("Каталог с файлами или DVD каталог"))
	{
		filedialog->setFileMode(QFileDialog::Directory);
		filedialog->setNameFilters(filters);
		filedialog->selectFilter(trUtf8("Каталог с файлами или DVD каталог"));
	}
	else
		filedialog->setFileMode(QFileDialog::AnyFile);
}

// Пробный запуск конвеера, инициализация
bool CuePlayer::playProbe()
{
	if (!dvdFlag && !tsFlag && !progFlag) g_object_set(play, "mute", true, NULL);
	preInitFlag = true;
	dvdAudioPads = 0;
	dvdAudioCurrentPad = 0;
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
		preInitFlag = false;
		if (!dvdFlag && !tsFlag && !progFlag) g_object_set(play, "mute", false, NULL);
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
	GstElement *typefind, *audiosink, *vdecoder, *adecoder, *ffmpegcolorspace;
	GstElement *audioconvert, *audioresample;
	GstElement *aqueue, *vqueue;
	GstPad *audiopad, *videopad;


	dvdFlag = true;
	setWindowsTitles(trUtf8("DVD видео"));
	label->setText(trUtf8("DVD видео : 1"));

	aqueue = make_queue ();
	vqueue = make_queue ();

	g_print("Выбран формат DVD\n");
	play = gst_pipeline_new ("pipe");
	dvdsrc = gst_element_factory_make ("dvdreadsrc", "dvdsrc");
	if(!discFlag)
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

	d_audio = gst_bin_new ("audiobin");
	gst_bin_add_many (GST_BIN (d_audio), aqueue, adecoder, audioconvert, audioresample, d_volume, audiosink, NULL);
	gst_element_link_many(aqueue, adecoder, audioconvert, audioresample, d_volume, audiosink, NULL);

	audiopad = gst_element_get_static_pad (aqueue, "sink");
	gst_element_add_pad (d_audio, gst_ghost_pad_new ("sink", audiopad));
	gst_object_unref (audiopad);
	gst_bin_add (GST_BIN (play), d_audio);

	d_video = gst_bin_new ("videobin");
	gst_bin_add_many (GST_BIN (d_video), vqueue, vdecoder, ffmpegcolorspace, videosink, NULL);
	gst_element_link_many (vqueue, vdecoder, ffmpegcolorspace, videosink, NULL);

	videopad = gst_element_get_static_pad (vqueue, "sink");
	gst_element_add_pad (d_video, gst_ghost_pad_new ("sink", videopad));
	gst_object_unref (videopad);
	gst_bin_add (GST_BIN (play), d_video);
	//g_signal_connect (play, "deep-notify", G_CALLBACK (gst_object_default_deep_notify), NULL); // Дебаг
}

#ifdef FTPPLAY
void CuePlayer::createFtpPipe()
{
	GstElement *gnomevfs, *fqueue, *fdecoder, *faout;
	GstPad *audiopad;

	label->setText(trUtf8("Открыт ftp файл"));

	play = gst_pipeline_new ("pipe");
	gnomevfs = gst_element_factory_make ("gnomevfssrc", "gvfs");
	g_object_set (gnomevfs, "location", filename.toUtf8().data(), NULL);

	fqueue = make_queue ();
	fdecoder = gst_element_factory_make ("decodebin", "fdecode");
	d_volume = gst_element_factory_make ("volume", "volume");
	faout = gst_element_factory_make ("autoaudiosink", "fasink");

	gst_bin_add_many (GST_BIN (play), gnomevfs, fqueue, fdecoder, NULL);
	gst_element_link_many(gnomevfs, fqueue, fdecoder, NULL);

	g_signal_connect (fdecoder, "pad-added", G_CALLBACK (audio_pad_added), NULL);

	d_audio = gst_bin_new ("audiobin");
	gst_bin_add_many (GST_BIN (d_audio), d_volume, faout, NULL);
	gst_element_link_many(d_volume, faout, NULL);

	audiopad = gst_element_get_static_pad (d_volume, "sink");
	gst_element_add_pad (d_audio, gst_ghost_pad_new ("sink", audiopad));
	gst_object_unref (audiopad);
	gst_bin_add (GST_BIN (play), d_audio);


	bus = gst_pipeline_get_bus (GST_PIPELINE (play));
	gst_bus_add_watch (bus, bus_callback, play);
	gst_object_unref (bus);
	//g_signal_connect (play, "deep-notify", G_CALLBACK (gst_object_default_deep_notify), NULL); // Дебаг
}
#endif

void CuePlayer::dtsPlayer()
{
	GstElement* playbin;
	playbin = gst_element_factory_make ("playbin2", "dtsplay");

}

void CuePlayer::setDvdAudio(gchar* auname, int ind)
{
	dvdAu[ind] = auname;
}

gchar* CuePlayer::getDvdAudio(int ind)
{
	return strdup(dvdAu[ind].toAscii().data());
}

void CuePlayer::extButtons(bool b)
{
	if(b)
	{
		minwin = getLayoutSize();
		dvdButton->show();
		streamButton->show();
		if (intWindAction->isChecked())
				videowindow->setFixedSize(getLayoutSize());
	}
	else
	{
		dvdButton->hide();
		streamButton->hide();
		if (intWindAction->isChecked())
				videowindow->setFixedSize(minwin);
	}
}

void CuePlayer::endBlock()
{
	stopTrack();
	if (primaryDPMS != checkDPMS())
	{
		if (primaryDPMS == "-dpms")
		{
			dpmsTrigger(false);
			qDebug() << trUtf8("Внимание! Энергосбережение выключено.");
		}
		else
		{
			dpmsTrigger(true);
		}
	}
	QLocalServer::removeServer("cueplayer");
	qApp->quit();
}

void CuePlayer::dpmsTrigger(bool dpms)
{
	QStringList arguments;
	if (dpms)
		arguments << "+dpms";
	else
		arguments << "-dpms";
	if (videoProcess->state())
		videoProcess->kill();
	videoProcess->start("xset", arguments);
	videoProcess->waitForFinished(1000);
	//qDebug() << arguments;
}

QString CuePlayer::checkDPMS()
{
	if (videoProcess->state())
		videoProcess->kill();
	videoProcess->start("xset", QStringList() << "q");
	if (!videoProcess->waitForFinished(1000))
			 return "+dpms";
	QByteArray result = videoProcess->readAll();
	videoProcess->close();
	QList<QByteArray> out = result.split('\n');
	foreach (QByteArray bastr, out)
	{
		if (!strcmp (bastr, "  DPMS is Disabled"))
			return "-dpms";
		else if (!strcmp (bastr, "  DPMS is Enabled"))
			return "+dpms";
	}
	return "+dpms";
}

void CuePlayer::threadRunInd()
{
	prewlabel = label->text();
	loadpoints = 0;
	trdtimer->start(TRDTIME);
}

void CuePlayer::threadRunProgress()
{
	loadpoints++;
	switch (loadpoints)
	{
		case 1:
			label->setText(trUtf8(" Подождите."));
			break;
		case 2:
			label->setText(trUtf8("  Подождите.."));
			break;
		case 3:
			label->setText(trUtf8("   Подождите..."));
			loadpoints = 0;
		break;
	}
}

void CuePlayer::threadStop()
{
	trdtimer->stop();
	label->setText(prewlabel);
	loadpoints = 0;
	switch (threadRet)
	{
		case ERR_STATE:
			if (multiCueFlag || multiFileFlag)
				cueplayer->playPrewTrack(); // Костыли
			else
				cueplayer->stopAll();
			break;
		case FUNC_PLAY:
			timer->start(TIME);
			postPlay();
		case FUNC_CHECK:
			postCheck();
			break;
		default:
			break;
	}
}

void CuePlayer::postPlay()
{
	if (videoFlag && !tsFlag)
	{
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
	else if (dvdFlag || tsFlag)
	{
		if (state == GST_STATE_PLAYING)
		{
			videowindow->createAudioMenu(dvdAudioPads - 1, 0);
		}
	}
	if (progFlag)
	{
		if (state == GST_STATE_PLAYING)
		{
			equalizer->restoreValue();
		}
	}
}

void CuePlayer::postCheck()
{
	if (state == GST_STATE_READY)
		playTrack();
	gst_element_get_state( GST_ELEMENT(play), &state, NULL, GST_SECOND * TIMEOUT);
	if (state == GST_STATE_PLAYING)
		seekAndLCD(numTrack);
	else if (state == GST_STATE_PAUSED)
	{
		seekAndLCD(numTrack);
		playTrack();
	}
}

void CuePlayer::checkReady()
{
	if (playButton->isEnabled())
	{
		paramtimer->stop();
		playTrack();
	}
}

void CuePlayer::plInit()
{
	QFileInfoList filelist;
	multiFileFlag = true;
	streamFlag = true;

	for (int i = 0; i < plparser->getPlEntries(); i++)
		filelist << plparser->getPlStruct(i).at(0);
	multiFileInit(filelist);
	if(!playlistItem)
		return;
	for (int i = 0; i < plparser->getPlEntries(); i++)
		playlistItem[i]->setText(0, plparser->getPlStruct(i).at(1));
	label->setText(plparser->getPlStruct(0).at(1));
	settings.setValue("player/recentfile", filename);
}

void CuePlayer::plError(QString msg)
{
	label->setText(trUtf8("Ошибка чтения списка воспроизведения\n") + msg);
}

void CuePlayer::dragEnterEvent(QDragEnterEvent *event)
{
	if (event->mimeData()->hasFormat("text/uri-list"))
			event->acceptProposedAction();
}

void CuePlayer::dropEvent(QDropEvent *event)
{
	QRegExp rxFilename("^file://.*");
	QList<QUrl> urls = event->mimeData()->urls();
	if (urls.isEmpty())
		return;
	if (rxFilename.indexIn(urls.first().toString()) != -1)
		cueFileSelected(QStringList() << urls.first().toLocalFile());
	else
		cueFileSelected(QStringList() << urls.first().toString());
}

void CuePlayer::keyPressEvent (QKeyEvent  *event)
{
	static int volume;
	//qDebug() << event->nativeScanCode();
	switch (event->nativeScanCode())
	{
	case KEY_PREV:
	case KEY_PREV_CURVE:
		if (prewButton->isEnabled())
			playPrewTrack();
		break;
	case KEY_NEXT:
	case KEY_NEXT_CURVE:
		if (nextButton->isEnabled())
			playNextTrack();
		break;
	case KEY_PLAY:
	case KEY_PLAY_CURVE:
		gst_element_get_state( GST_ELEMENT(play), &state, NULL, GST_SECOND * 1);
		if (state > GST_STATE_READY)
			pauseTrack();
		else
			playTrack();
		break;
	case KEY_STOP:
		stopTrack();
		break;
	case KEY_VOL_DOWN:
	case KEY_VOL_DOWN_CURVE:
		if (volumeDial->isEnabled())
			volumeDial->setValue(volumeDial->value() - 1);
		break;
	case KEY_VOL_UP:
	case KEY_VOL_UP_CURVE:
		if (volumeDial->isEnabled())
			volumeDial->setValue(volumeDial->value() + 1);
		break;
	case KEY_MUTE:
	case KEY_MUTE_CURVE:
		if (volumeDial->isEnabled())
		{
			volume = volumeDial->value();
			volumeDial->setValue(0);
			volumeDial->setEnabled(false);
		}
		else
		{
			volumeDial->setEnabled(true);
			volumeDial->setValue(volume);
		}
		break;
	case KEY_EJECT:
		// eject
		break;
	default:
		break;
	}
}

void CuePlayer::mover(bool b)
{
	if (videowindow->parentWidget() == this)
	{
		if (b)
			this->move(QPoint(this->frameGeometry().x(),this->frameGeometry().y() - getLayoutSize().height() - 7));
		else
			this->move(QPoint(this->frameGeometry().x(),this->frameGeometry().y() + getLayoutSize().height() + 7));
	}
}

void CuePlayer::setWindowsTitles(QString s)
{
	setWindowTitle(s);
	if (videoFlag) videowindow->setWindowTitle(s);
	savetitle = s;
}

void CuePlayer::integVideo(bool b)
{
	if (!videoFlag && !dvdFlag)
		return;
	if (!b)
	{
		verticalLayout_3->removeWidget(videowindow);
		if (!vidwingeom.isNull())
		{
			videowindow->setGeometry(vidwingeom);
			videowindow->setParent(0);
		}
		win = videowindow->winId();
		return;
	}
	vidwingeom = videowindow->frameGeometry();
	verticalLayout_3->insertWidget(0, videowindow, 0, Qt::AlignTop);
	win = videowindow->winId();
	videowindow->setFixedSize(getLayoutSize());
}

QSize CuePlayer::getLayoutSize()
{
	int left, top, right, bottom;
	layout()->getContentsMargins(&left, &top, &right, &bottom);
	int videosize = this->width() - left - right;
	QSize recentsize(videosize, videosize/16*10);
	return recentsize;
}

void CuePlayer::intWindCheck(bool b)
{
	if (videowindow->isVisible())
	{
		integVideo(b);
		gst_x_overlay_set_xwindow_id (GST_X_OVERLAY (videosink), win);
		gst_x_overlay_expose (GST_X_OVERLAY (videosink));
		videowindow->show();
	}
	settings.setValue("player/intwin", b);
}

void CuePlayer::dclIntVw(bool b)
{
	if (intWindAction->isChecked())
	{
		integVideo(!b);
		gst_x_overlay_set_xwindow_id (GST_X_OVERLAY (videosink), win);
		gst_x_overlay_expose (GST_X_OVERLAY (videosink));
		videowindow->show();
	}
}

void CuePlayer::equalizerCheck(bool b)
{
	settings.setValue("player/equalizer", b);
	if (b)
	{
		editEqualizerAction->setEnabled(true);
	}
	else
	{
		editEqualizerAction->setEnabled(false);
		equalizer->close();
	}
	cueFileSelected(QStringList() << filename);
}

void CuePlayer::equalizerChang(double *band)
{
	g_object_set (G_OBJECT (equalizer10bands),
				  "band0", band[0],
				  "band1", band[1],
				  "band2", band[2],
				  "band3", band[3],
				  "band4", band[4],
				  "band5", band[5],
				  "band6", band[6],
				  "band7", band[7],
				  "band8", band[8],
				  "band9", band[9],
				  NULL);
}

void CuePlayer::progressiveMode(QString fname)
{
	qDebug() << trUtf8("Внимание! Включен прогрессивный режим.");
	GstElement *typefind, *decoder, *audioconvert, *audiosink;
	GstElement *aqueue, *multiqueue;
	GstPad *audiopad;

	/* Это пиздец какой хуёвый playbin,
	   на самом деле очень хуёвый playbin,
	   могло бы быть намного лучше это всё */

	progFlag = true;
	play = gst_pipeline_new ("player");
	aqueue = gst_element_factory_make ("queue", NULL);
	multiqueue = gst_element_factory_make ("multiqueue", NULL);
	aufile = gst_element_factory_make ("filesrc", "file-source");
	g_object_set (aufile, "location", fname.toUtf8().data(), NULL);
	typefind = gst_element_factory_make ("typefind", "typefinder");
	g_signal_connect (typefind, "have-type", G_CALLBACK (cb_typefound), NULL);
	decoder = gst_element_factory_make ("decodebin2", "bindecoder");
	audioconvert = gst_element_factory_make ("audioconvert", "aconv");
	equalizer10bands = gst_element_factory_make ("equalizer-10bands", "equalizer");
	d_volume = gst_element_factory_make ("volume", "volume");
	audiosink = gst_element_factory_make ("autoaudiosink", "asink");
	gst_bin_add_many (GST_BIN (play), aufile, typefind, multiqueue, decoder, NULL);
	gst_element_link_many (aufile, typefind, multiqueue, decoder, NULL);
	g_signal_connect (decoder, "new-decoded-pad", G_CALLBACK (new_pad_added), NULL);
	d_audio = gst_bin_new ("audiobin");
	gst_bin_add_many (GST_BIN (d_audio), aqueue, audioconvert, equalizer10bands, d_volume, audiosink, NULL);
	gst_element_link_many(aqueue, audioconvert, equalizer10bands, d_volume, audiosink, NULL);
	audiopad = gst_element_get_static_pad (aqueue, "sink");
	gst_element_add_pad (d_audio, gst_ghost_pad_new ("sink", audiopad));
	gst_object_unref (audiopad);
	gst_bin_add (GST_BIN (play), d_audio);
}

void CuePlayer::setTray(bool b)
{
	tray = b;
}

GstThread::GstThread(QObject *parent) : QThread(parent)
{
	thplay = NULL;
	numfunc = 0;
}

void GstThread::run()
{
	threadRet = NULL_RET;
	switch (numfunc)
	{
		case POST_PLAY:
			setState();
			threadRet = FUNC_PLAY;
			break;
		case POST_CHECK:
			threadRet = FUNC_CHECK;
			break;
		default:
			break;
	}
	if (gst_element_get_state( GST_ELEMENT(thplay), &state, NULL, GST_SECOND * 10) != GST_STATE_CHANGE_SUCCESS)
	{
		g_print ("Аварийный останов в треде.\n");
		threadRet = ERR_STATE;
	}
	numfunc = 0;
}

void GstThread::setPlayBin(GstElement *playbin)
{
	thplay = playbin;
}

void GstThread::setFunc(int func)
{
	numfunc = func;
}

void GstThread::setState()
{
	gst_element_set_state (thplay, GST_STATE_PLAYING);
}
