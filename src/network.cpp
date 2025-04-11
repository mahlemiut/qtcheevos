// RA API connection stuff

#include <QObject>
#include <iostream>
#include "network.h"

TrackerData::TrackerData() :
    user(""), 
    apikey(""), 
    url(""), 
    busy(false)
{
    manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, this, &TrackerData::transferDone);
    connect(manager, &QNetworkAccessManager::sslErrors, this, &TrackerData::transferSSLError);
    
}

TrackerData::~TrackerData() 
{
    delete manager;
}

/**
 * @brief Retrieves a summary of the user's recent activity from the server.
 *
 * This function constructs a URL using the user's API key and username, and sends
 * a GET request to the RetroAchievements API to retrieve the user's recent activity summary.
 * If either the username or API key is empty, the function returns without sending the request.
 */
void TrackerData::sendSummary()
{
    if(busy)
        return;
    if(user.isEmpty() || apikey.isEmpty())
        return;
    
    busy = true;

    // get recent activity
    url = "https://retroachievements.org/API/API_GetUserRecentlyPlayedGames.php?y=" + apikey + "&u=" + user + "&c=1";
//  std::cout << url.toStdString() << std::endl;
    reply = manager->get(QNetworkRequest(QUrl(url)));
    connect(reply, &QNetworkReply::errorOccurred, this, &TrackerData::transferError);
}

void TrackerData::transferDone(QNetworkReply *reply)
{
    response = reply->readAll();
    reply->deleteLater();
    busy = false;
    emit dataReady(response);
}

void TrackerData::sendBoxArt(QString u)
{
    QString url = "https://retroachievements.org" + u;
    if(busy)
        return;
    if(url.isEmpty())
        return;
    
    busy = true;

//  std::cout << url.toStdString() << std::endl;
    reply = manager->get(QNetworkRequest(QUrl(url)));
    connect(reply, &QNetworkReply::errorOccurred, this, &TrackerData::transferError);
}

void TrackerData::sendRank()
{
    if(busy)
        return;
    if(user.isEmpty() || apikey.isEmpty())
        return;
    
    busy = true;

    // get user rank
    url = "https://retroachievements.org/API/API_GetUserSummary.php?y=" + apikey + "&u=" + user  + "&g=0&a=0";
//  std::cout << url.toStdString() << std::endl;
    reply = manager->get(QNetworkRequest(QUrl(url)));
    connect(reply, &QNetworkReply::errorOccurred, this, &TrackerData::transferError);
}

void TrackerData::transferError(QNetworkReply::NetworkError error)
{
    std::cout << "Network error: " << error;
    busy = false;
}

void TrackerData::transferSSLError(QNetworkReply* reply, QList<QSslError> errors)
{
    std::cout << "SSL error: " << errors.at(0).errorString().toStdString();
    busy = false;
}