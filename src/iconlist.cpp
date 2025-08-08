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

/**
 * @brief Fetches and processes the icons for a specific game and user.
 * 
 * This function retrieves game information and user progress from the RetroAchievements API
 * for the specified game ID and user credentials. If the game ID differs from the previously
 * loaded game, the icon list is cleared and reloaded. Otherwise, the icons are not reloaded.
 * 
 * @param gameID The unique identifier of the game for which icons are to be fetched.
 * @param username The username of the user whose progress is to be retrieved.
 * @param apiKey The API key used for authentication with the RetroAchievements API.
 * 
 * The function performs the following steps:
 * - Clears the icon list if the game ID has changed.
 * - Sets up the necessary connections for handling the API response.
 * - Sends a GET request to the RetroAchievements API to fetch the game information and user progress.
 */
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

/**
 * @brief Handles the response received from a network request for the list of achievements.
 * 
 * This function processes the JSON response from the network reply, extracts achievement data,
 * and updates the internal list of icons. It also emits a signal to notify that icons have been loaded.
 * 
 * @param reply A pointer to the QNetworkReply object containing the response data.
 * 
 * The function performs the following steps:
 * - Checks for errors in the network reply and logs the error message if any.
 * - Parses the JSON response to extract achievement data.
 * - If `loadicons` is true, populates the `icons` list with achievement details, including ID, URL, and unlocked status.
 * - If `loadicons` is false, updates the unlocked status of existing icons in the list.
 * - Emits the `iconLoad` signal to indicate that icons have been loaded.
 * - Disconnects the relevant signals and deletes the reply object to clean up resources.
 */
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
	request.setTransferTimeout(5000);
	//std::cout << pos << ":" << request.url().toString().toStdString() << std::endl;
	connect(manager, &QNetworkAccessManager::finished, this, &IconList::IconsReceived);
	manager->get(request);
}

void IconList::IconsReceived(QNetworkReply *reply)
{
	if (reply->error())
	{
		qDebug() << reply->errorString();
		if (reply->error() == QNetworkReply::TimeoutError || 
			reply->error() == QNetworkReply::OperationCanceledError)
		{
			// Immediately retry the download for this icon
			disconnect(manager, &QNetworkAccessManager::finished, this, &IconList::IconsReceived);
			reply->deleteLater();
			PopulateIcons(); // Retry the same icon (pos hasn't been incremented)
			return;
		}
		return;
	}

	QByteArray data = reply->readAll();
	QImage image;
	if (!image.loadFromData(data)) {
		// Handle failed image load by retrying
		disconnect(manager, &QNetworkAccessManager::finished, this, &IconList::IconsReceived);
		reply->deleteLater();
		PopulateIcons(); // Retry the same icon
		return;
	}
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