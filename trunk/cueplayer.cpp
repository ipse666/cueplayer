#include <QtGui>
#include "cueplayer.h"

CuePlayer::CuePlayer(QWidget *parent) : QWidget(parent)
{
	setupUi(this);

	//расположение главного окна по центру экрана
	desktop = QApplication::desktop();
	int appWidth = width();
	int appHeight = height();
	int appWidthPos = desktop->width()/2 - appWidth/2;
	int appHeightPos = desktop->height()/2 - appHeight/2;
	move(appWidthPos, appHeightPos);

	treeWidget->hide();
	createTrayIconMenu();
	layout()->setSizeConstraint(QLayout::SetFixedSize);
	refparser = NULL;
	mediaObject = new Phonon::MediaObject(this);
	mediaObject->setTickInterval(200);
	audioDevice = new Phonon::AudioOutput(Phonon::MusicCategory, this);
	Phonon::createPath(mediaObject, audioDevice);
	filedialog = new QFileDialog(this, trUtf8("Открыть файл"));
	filedialog->setNameFilter(trUtf8("CUE образы (*.cue)"));
	secNumLCD->display("00");
	volumeDial->setValue(qint32(audioDevice->volume() * 100));
	fileList->setEnabled(false);
	pauseButton->setShortcut(trUtf8(" "));

	connect(timeLineSlider, SIGNAL(valueChanged(int)),
	 this, SLOT(setNumLCDs(int)));
	connect(openButton, SIGNAL(clicked()),
	 filedialog, SLOT(exec()));
	connect(filedialog, SIGNAL(filesSelected(QStringList)),
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
	connect(mediaObject, SIGNAL(stateChanged(Phonon::State, Phonon::State)),
	 this, SLOT(stateChanged(Phonon::State, Phonon::State)));
	connect(mediaObject, SIGNAL(tick(qint64)),
	 this, SLOT(tick(qint64)));
	connect(timeLineSlider, SIGNAL(sliderReleased()),
	 this, SLOT(sliderRelease()));
	connect(volumeDial, SIGNAL(sliderMoved(int)),
	 this, SLOT(volumeValue(int)));
	connect(treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
	 this, SLOT(listItemClicked(QTreeWidgetItem*,int)));
	connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
	 this, SLOT(trayClicked(QSystemTrayIcon::ActivationReason)));
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
	treeWidget->clear();
	treeWidget->hide();
	fileList->setEnabled(false);
	fileList->setChecked(false);
	numTrack = 1;
	if (refparser)
	    delete refparser;
	QString filename = filenames.join("");
	refparser = new CueParser(filename);
	setWindowTitle(refparser->getTitle());
	label->setText("1. " + refparser->getTrackTitle(numTrack));
	mediaObject->setCurrentSource(QString(refparser->getSoundFile()));
	timeLineSlider->setSliderPosition(0);
}

// переключение на следующий трек
void CuePlayer::playNextTrack()
{
	if (refparser)
		if (numTrack < refparser->getTrackNumber())
		{
			++numTrack;
			seekAndLCD(numTrack);
		}
}

// переключение на предыдущий трек
void CuePlayer::playPrewTrack()
{
	if (refparser)
		if (numTrack > 1)
		{
			--numTrack;
			seekAndLCD(numTrack);
		}
}

// воспроизведение трека
void CuePlayer::playTrack()
{
	mediaObject->play();
}

// остановка воспроизведения трека/альбома
void CuePlayer::stopTrack()
{
	if (mediaObject->state() == Phonon::PlayingState)
	{
		mediaObject->pause();
		seekAndLCD(numTrack);
	}
	else
		mediaObject->stop();
}

// триггер паузы
void CuePlayer::pauseTrack()
{
	if (mediaObject->state() == Phonon::PausedState)
		mediaObject->play();
	else
		mediaObject->pause();
}

// обработка смены состояний медиаобъекта
void CuePlayer::stateChanged(Phonon::State newState, Phonon::State oldState)
{
	switch (newState)
	{
		case Phonon::LoadingState:
			//QMessageBox::information(this, trUtf8("Загружен файл"), refparser->getSoundFile());
		break;
		
		case Phonon::StoppedState:
			if (oldState == Phonon::LoadingState && mediaObject->totalTime() != -1)
				initAlbum();
			numTrack = 1;
			seekAndLCD(numTrack);
		break;
		
		case Phonon::PlayingState:
			if (oldState != Phonon::PausedState)
				seekAndLCD(numTrack);
		break;
		
		case Phonon::BufferingState:
			//QMessageBox::information(this, trUtf8("Буферизация"), mediaObject->errorString());
		break;
		
		case Phonon::PausedState:
			
		break;
		
		case Phonon::ErrorState:
			QMessageBox::critical(this, trUtf8("Ошибка"), mediaObject->errorString());
		break;
		
		default:
			
		break;
	}
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
		mediaObject->seek(time);
}

// регулировка громкости
void CuePlayer::volumeValue(int volume)
{
	audioDevice->setVolume(qreal((double)volume / 100));
	volumeDial->setToolTip(QString::number(volume) + "%");
}

// переключение между треками и индикация
void CuePlayer::seekAndLCD(int num)
{
	QString stringNumTrack;
	int totalTimeNext;
	stringNumTrack.setNum(num);
	label->setText(stringNumTrack + ". " + refparser->getTrackTitle(num));
	mediaObject->seek(refparser->getTrackIndex(num));
	totalTime = refparser->getTrackIndex(num);
	totalTime /= 1000;
	if (num < refparser->getTrackNumber())
		totalTimeNext = refparser->getTrackIndex(num+1);
	else
		totalTimeNext = mediaObject->totalTime();
	totalTimeNext /= 1000;
	totalTimeNext -= totalTime;
	timeLineSlider->setMaximum(totalTimeNext);
	treeWidget->setCurrentItem(treeWidget->topLevelItem(num - 1));
}

// переключение в списке воспроизведения
void CuePlayer::listItemClicked(QTreeWidgetItem *item, int column)
{
	column = 0;
	if (numTrack != (treeWidget->indexOfTopLevelItem(item) + 1))
	{
		numTrack = treeWidget->indexOfTopLevelItem(item) + 1;
		seekAndLCD(numTrack);
	}
}

// инициализация после загрузки аудиофайла
void CuePlayer::initAlbum()
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
			currentTime = mediaObject->totalTime() - startTime;
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
	fileList->setEnabled(true);
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

	trayIcon = new QSystemTrayIcon(this);
	trayIconMenu = new QMenu(this);
	trayIconMenu->addAction(aboutAction);
	trayIconMenu->addAction(quitAction);
	trayIcon = new QSystemTrayIcon(this);
	trayIcon->setContextMenu(trayIconMenu);
	trayIcon->setIcon(QIcon(":/images/knotify.png"));
	trayIcon->show();

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
