#ifndef CUEPLAYER_H
#define CUEPLAYER_H
#include <QWidget>
#include <QFileDialog>
#include <QSystemTrayIcon>
#include <KDE/Phonon/MediaObject>
#include <KDE/Phonon/AudioOutput>
#include "ui_cueplayer.h"
#include "cueparser.h"

class CuePlayer : public QWidget, public Ui::CuePlayer
{
	Q_OBJECT

protected:
	void closeEvent(QCloseEvent *event);

public:
	CuePlayer(QWidget *parent = 0);
private:
	void seekAndLCD(int);
	void createTrayIconMenu();
	QFileDialog *filedialog;
	QTreeWidgetItem *playlistItem[100];
	QSystemTrayIcon *trayIcon;
	QMenu *trayIconMenu;
	QAction *quitAction;
	QAction *aboutAction;
	QWidget *desktop;
	int numTrack;
	int totalTime;
	CueParser *refparser;
	Phonon::MediaObject *mediaObject;
	Phonon::AudioOutput *audioDevice;
private slots:
	void setNumLCDs(int);
	void cueFileSelected(QStringList);
	void playNextTrack();
	void playPrewTrack();
	void playTrack();
	void stopTrack();
	void pauseTrack();
	void stateChanged(Phonon::State, Phonon::State);
	void tick(qint64);
	void sliderRelease();
	void volumeValue(int);
	void listItemClicked(QTreeWidgetItem *, int);
	void initAlbum();
	void trayClicked(QSystemTrayIcon::ActivationReason);
	void about();
};

#endif // __CUEPLAYER_H__
