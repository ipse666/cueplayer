#include "youtubedl.h"

YouTubeDL::YouTubeDL(QString url)
{
	http = new QHttp(this);
	lineUrl = url;
	startdownload();

	connect(http, SIGNAL(requestFinished(int, bool)),
			this, SLOT(httpRequestFinished(int, bool)));

	connect(http, SIGNAL(responseHeaderReceived(const QHttpResponseHeader &)),
			this, SLOT(readResponseHeader(const QHttpResponseHeader &)));
}

void YouTubeDL::abortDownload()
{
	http->abort();
}

void YouTubeDL::downloader(QString img_url, QString argument, QString dest)
{
	QUrl url(img_url);
	QFileInfo fileInfo(url.path());
	QString fileName = fileInfo.fileName();
	if (fileName.isEmpty())
		fileName = "index.html";

	file = new QFile(dest);

	QFileInfo fi(dest);
	currentFileName = fi.fileName();

	QHttp::ConnectionMode mode = url.scheme().toLower() == "https" ? QHttp::ConnectionModeHttps : QHttp::ConnectionModeHttp;
	http->setHost(url.host(), mode, url.port() != -1 ? url.port() : 80);

	if (!url.userName().isEmpty())
		http->setUser(url.userName(), url.password());

	httpRequestAborted = false;
	QByteArray path = QUrl::toPercentEncoding(url.path(), "!$&'()*+,;=:@/");
	if (path.isEmpty())
		path = "/";
	httpGetId = http->get(path + argument, file);
}

void YouTubeDL::httpRequestFinished(int requestId, bool error)
{
	if (requestId != httpGetId)
		return;
	if (httpRequestAborted) {
		if (file) {
			file->close();
			file->remove();
			delete file;
			file = 0;
		}
	}

	if (requestId != httpGetId)
		return;

	file->close();

	if (error) {
		qDebug() << trUtf8("Ошибка загрузки: %1.") << http->errorString();
	} else {
		qDebug() << trUtf8("Загрузка завершена");
	}
	delete file;
	file = 0;

	downResponse(false, reasonPhrase);
}

void YouTubeDL::readResponseHeader(const QHttpResponseHeader &responseHeader)
{
	switch (responseHeader.statusCode()) {
	case 200:
		reasonPhrase = trUtf8("Хорошо");
		break;
	case 301:
		reasonPhrase = trUtf8("Перемещёно окончательно");
		break;
	case 302:
		reasonPhrase = trUtf8("Найдено");
		break;
	case 303:
		reasonPhrase = trUtf8("Смотреть другое");
		break;
	case 307:
		reasonPhrase = trUtf8("Временное перенаправление");
		break;

	default:
		reasonPhrase = responseHeader.reasonPhrase();
	}

	qDebug() << trUtf8("Ответ сервера: ") << reasonPhrase;
}

void YouTubeDL::startdownload()
{
	if ((!lineUrl.isNull()) || (lineUrl.startsWith("http://www.youtube.com/watch?v=")))
		downloader("http://www.youtube.com/get_video_info", "?&video_id=" + extractVideoId(), QDir::tempPath() + "/get_video_info.txt");
	else
		qDebug() << trUtf8("Пожалуйста введите правильный адрес!");
}

QString YouTubeDL::extractVideoId()
{
	QString videoId = lineUrl.split("?v=").at(1);
	videoId = videoId.split("&").at(0);
	return videoId;
}

void YouTubeDL::downResponse(bool error, QString reasonPhrase)
{
	if (error) {
		qDebug() << trUtf8("Ошибка") << tr("Ошибка загрузки: %1.") << reasonPhrase;
	}
	else
	{
		 extractVideoUrl();
	}
}

VideoInfos YouTubeDL::extract_get_video_info_Infos()
{
	VideoInfos videoInfos;

	QFile htmlFile(QDir::tempPath() + "/" + "get_video_info.txt");
	if (!htmlFile.open(QIODevice::ReadOnly | QIODevice::Text))
		 qDebug() << trUtf8("Ошибка открытия файла!");

	QTextStream in(&htmlFile);
	while (!in.atEnd()) {
		QString line = in.readLine();

		QString author = line.split("&author=").at(1);
		videoInfos.author = author.split("&watermark").at(0);

		QString title = line.split("&title=").at(1);
		title = title.split("&title=").at(0);
		title = title.split("&ftoken=").at(0);
		videoInfos.title = title.replace("+", " ");

		QString thumbnail = line.split("&thumbnail_url=http%3A%2F%2F").at(1);
		thumbnail = thumbnail.split("&allow_ratings").at(0);
		thumbnail = "http://" + thumbnail.replace("%2F", "/");
		videoInfos.thumbnailUrl = thumbnail;

		QString token = line.split("&token=").at(1);
		videoInfos.token = token.split("&thumbnail_url").at(0);
	}


	htmlFile.close();

	return videoInfos;
}

void YouTubeDL::extractVideoUrl()
{
	QString qualiy = "&fmt=5";
	VideoInfos videoInfos = extract_get_video_info_Infos();

	if (!videoInfos.token.isEmpty()) {
		QString videoUrl = "http://youtube.com/get_video";
		QString videoUrlArgs = QString("?video_id=%1&t=%2%3")
						   .arg(extractVideoId())
						   .arg(videoInfos.token.replace("%3D", "="))
						   .arg(qualiy);

		emit putUrl(QStringList() << videoUrl + videoUrlArgs);
		qDebug() << trUtf8("Прямой линк ") << videoUrl + videoUrlArgs;
	}
	else
		qDebug() << trUtf8("Ошибка") << tr("Неверный адрес!");
}
