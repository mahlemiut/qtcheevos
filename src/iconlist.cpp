// IconList gatherer class

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <iostream>
#include "iconlist.h"

IconList::IconList(QObject *parent) : QObject(parent),
	game(0),
	loadicons(true),
	loadphase(Idle)
{
    manager = new QNetworkAccessManager(this);
}

IconList::~IconList()
{
    delete manager;
}

void IconList::getIcons(int gameID, QString username, QString apiKey)
{
    QNetworkRequest request;

	if(game != gameID)
	{
		icons.clear();
		game = gameID;
		loadicons = true;
	}
	else 
		loadicons = false;

	loadphase = List;
	connect(manager, &QNetworkAccessManager::finished, this, &IconList::ListReceived);
	connect(this, &IconList::iconLoad, this, &IconList::PopulateIcons);
	request.setUrl(QUrl("https://retroachievements.org/API/API_GetGameInfoAndUserProgress.php?u=" + username + "&g=" + QString("%1").arg(gameID) + "&y=" + apiKey));
	//std::cout << request.url().toString().toStdString() << std::endl;
	manager->get(request);
}

void IconList::clear()
{
	icons.clear();
	game = 0;
	loadicons = true;
}

void IconList::ListReceived(QNetworkReply *reply)
{
    if (reply->error())
    {
        qDebug() << reply->errorString();
        return;
    }

//	if(loadphase == List)
	{
		QByteArray data = reply->readAll();
		QJsonDocument doc = QJsonDocument::fromJson(data);
		QJsonObject cheevoArray = doc["Achievements"].toObject();

		// fill the array
		if(loadicons)
		{
			for(QJsonObject::iterator it = cheevoArray.begin(); it != cheevoArray.end(); ++it)
			{
				QJsonObject cheevo = it.value().toObject();
				Icon icon;
				icon.id = cheevo["ID"].toInt();
				icon.url = "/Badge/" + cheevo["BadgeName"].toString() + ".png";
				if(cheevo["DateEarned"].isNull())
					icon.unlocked = false;
				else
					icon.unlocked = true;
				icons.append(icon);
			}
			pos = 0;
			emit iconLoad();
		}
		else // just update the unlocked status
		{
			int i = 0;
			for(QJsonObject::iterator it = cheevoArray.begin(); it != cheevoArray.end(); ++it)
			{
				QJsonObject cheevo = it.value().toObject();
				if(cheevo["DateEarned"].isNull())
					icons[i++].unlocked = false;
				else
					icons[i++].unlocked = true;
			}
		}
	}
	disconnect(manager, &QNetworkAccessManager::finished, this, &IconList::ListReceived);
	disconnect(this, &IconList::iconLoad, this, &IconList::PopulateIcons);
	reply->deleteLater();
}

void IconList::PopulateIcons()
{
	QNetworkRequest request;
	loadphase = Icons;
	request.setUrl(QUrl("https://retroachievements.org" + icons[pos].url));
	//std::cout << pos << ":" << request.url().toString().toStdString() << std::endl;
	connect(manager, &QNetworkAccessManager::finished, this, &IconList::IconsReceived);
	manager->get(request);
}

void IconList::IconsReceived(QNetworkReply *reply)
{
	if (reply->error())
	{
		qDebug() << reply->errorString();
		return;
	}

	QByteArray data = reply->readAll();
	QImage image;
	image.loadFromData(data);
	if(reply->url().toString() == "https://retroachievements.org" + icons[pos].url)
	{
		icons[pos].image = image;
	}
//	std::cout << pos << ":" << "Loaded:" << icons[pos].url.toStdString() << std::endl;
	disconnect(manager, &QNetworkAccessManager::finished, this, &IconList::IconsReceived);
	pos++;
	if(pos < icons.size())
		PopulateIcons();  // so we can just download one icon at a time
	else
		loadphase = Idle;
	reply->deleteLater();
}