#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <FAST/Exception.hpp>
#include <FAST/Utility.hpp>
#include "DataHub.hpp"
#include <iostream>
#include <FAST/Visualization/Window.hpp>
#include <fstream>
#include <QStandardPaths>
#ifdef WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#endif
#include <iomanip>
#include <stack>

namespace fast {

DataHub::DataHub(std::string URL, std::string storageDirectory) {
    if(URL.empty()) {
        m_URL = "https://fast-datahub.sintef.no/";
    } else {
        m_URL = URL;
    }
    if(m_URL[m_URL.size()-1] != '/')
        m_URL += "/";
    if(storageDirectory.empty()) {
        m_storageDirectory = join(Config::getTestDataPath(), "..", "datahub");
    } else {
        m_storageDirectory = storageDirectory;
    }
    createDirectories(m_storageDirectory);
    Window::initializeQtApp();
    // TODO verify site is online
}

static QJsonDocument getJSONFromURL(const std::string& url) {
    QEventLoop eventLoop;

    // "quit()" the event-loop, when the network request "finished()"
    QNetworkAccessManager mgr;
    QObject::connect(&mgr, &QNetworkAccessManager::finished, &eventLoop, &QEventLoop::quit);

    // the HTTP request
    QNetworkRequest req(QUrl(QString::fromStdString(url)));
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    QNetworkReply *reply = mgr.get(req);
    eventLoop.exec(); // blocks stack until "finished()" has been called

    reply->deleteLater();
    QJsonDocument result;
    if (reply->error() == QNetworkReply::NoError) {
        auto status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        auto res = reply->readAll();
        //std::cout << "Response: " << res.toStdString() << std::endl;
        result = QJsonDocument::fromJson(res);
    } else if (reply->error() == QNetworkReply::ContentNotFoundError) {
        throw Exception("FAST DataHub content not found. Please check spelling.");
    } else {
        //failure
        throw Exception("Failed to retrieve data from FAST DataHub.\nServer could be down, or you may have no internet access.\nPlease try again later.");
    }
    return result;
}

void DataHub::downloadTextFile(const std::string& url, const std::string& destination, const std::string& name, int fileNr) {
    emit progress(fileNr, 0);
    QEventLoop eventLoop;

    // "quit()" the event-loop, when the network request "finished()"
    QNetworkAccessManager mgr;
    QObject::connect(&mgr, &QNetworkAccessManager::finished, &eventLoop, &QEventLoop::quit);

    // the HTTP request
    QNetworkRequest req(QUrl(QString::fromStdString(url)));
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    QNetworkReply *reply = mgr.get(req);
    eventLoop.exec(); // blocks stack until "finished()" has been called

    reply->deleteLater();
    QJsonDocument result;
    if (reply->error() == QNetworkReply::NoError) {
        auto status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        result = QJsonDocument::fromJson(reply->readAll());
        std::ofstream file(join(destination, "pipeline.fpl"), std::ofstream::out);
        file << result["text"].toString().toStdString();
        file.close();
        emit progress(fileNr, 100);
    } else if (reply->error() == QNetworkReply::ContentNotFoundError) {
        throw Exception("FAST DataHub item " + name + " not found. Please check spelling.");
    } else {
        //failure
        throw Exception("Failed to retrieve pipeline data from FAST DataHub.\nServer could be down, or you may have no internet access.\nPlease try again later.");
    }
}

void DataHub::downloadAndExtractZipFile(const std::string& URL, const std::string& destination, const std::string& name, int fileNr) {
    QNetworkAccessManager manager;
    QNetworkRequest request(QUrl(QString::fromStdString(URL)));
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    auto timer = new QElapsedTimer;
    timer->start();
    auto reply = manager.get(request);
    std::cout << "";
    std::cout.flush();
	const int totalWidth = getConsoleWidth();
    std::string blank;
    for(int i = 0; i < totalWidth-1; ++i)
        blank += " ";
    QObject::connect(reply, &QNetworkReply::downloadProgress, [=](quint64 current, quint64 max) {
        const float percent = ((float)current / max);
        const float speed = ((float)timer->elapsed() / 1000.0f)/percent;
        const float ETA = (speed * (1.0f - percent) / 60.0f);
        std::cout << "\r"; // Replace line
        std::stringstream ss;
        ss << std::setprecision(1)
            << std::fixed;
        ss << "] " << (int)(percent * 100.0f) << "% | " << current/(1024*1024) << "MB / " << max/(1024*1024) << "MB | ETA " << ETA << " mins";
        std::string startString = "Downloading [";
        const int progressbarWidth = totalWidth - ss.str().size() - startString.size() - 1; // have to leave last line open to avoid jumping to next line (windows)
        std::cout << startString;
        int pos = progressbarWidth * percent;
        for (int i = 0; i < progressbarWidth; ++i) {
            if (i < pos) std::cout << "=";
            else if (i == pos) std::cout << ">";
            else std::cout << " ";
        }
        std::cout << ss.str();
        std::cout.flush();
        if(percent == 1.0f) {
            std::cout << "\n";
            std::cout << "\rExtracting zip file...";
            std::cout.flush();
        }
        emit progress(fileNr, (int)(percent*100.0f));
    });
    auto tempLocation = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/fast-datahub-data.zip";
    QFile file(tempLocation);
    if(!file.open(QIODevice::WriteOnly)) {
        throw Exception("Could not write to " + tempLocation.toStdString());
    }
    QObject::connect(reply, &QNetworkReply::readyRead, [&reply, &file]() {
        file.write(reply->read(reply->bytesAvailable()));
    });
    QObject::connect(&manager, &QNetworkAccessManager::finished, [blank, reply, &file, destination]() {
        if(reply->error() != QNetworkReply::NoError) {
            std::cout << "\r" << blank;
            std::cout << "\rERROR: Download failed! : " << reply->errorString().toStdString() << std::endl;
            file.close();
            file.remove();
            return;
        }
        file.close();
        try {
            extractZipFile(file.fileName().toStdString(), destination);
        } catch(Exception& e) {
            std::cout << "\r" << blank;
            std::cout << "\rERROR: Zip extraction failed!" << std::endl;
            file.remove();
            return;
        }

        file.remove();
		std::cout << "\r" << blank;
        std::cout << "\rComplete." << std::endl;
    });

    auto eventLoop = new QEventLoop(&manager);

    // Make sure to quit the event loop when download is finished
    QObject::connect(&manager, &QNetworkAccessManager::finished, eventLoop, &QEventLoop::quit);

    // Wait for it to finish
    eventLoop->exec();
}

DataHub::Item DataHub::Item::fromJSON(QJsonObject json) {
    DataHub::Item itemObject;
    itemObject.id = json["id"].toString().toStdString();
    itemObject.name = json["name"].toString().toStdString();
    itemObject.type = json["type"].toString().toStdString();
    itemObject.author = json["author"].toString().toStdString();
    itemObject.downloads = json["downloads"].toInt();
    itemObject.copyright = json["copyright"].toString().toStdString();
    itemObject.description = json["description"].toString().toStdString();
    itemObject.license = json["license_name"].toString().toStdString();
    itemObject.licenseCustom = json["license_custom"].toString().toStdString();
    itemObject.licenseURL = json["license_url"].toString().toStdString();
    itemObject.thumbnailURL = json["thumbnail_url"].toString().toStdString();
    itemObject.downloadURL = json["download_url"].toString().toStdString();
    itemObject.minFASTVersion = json["min_fast_version"].toString().toStdString();
    itemObject.maxFASTVersion = json["max_fast_version"].toString().toStdString();
    for(auto jsonItem2 : json["needs"].toArray()) {
        itemObject.needs.push_back(fromJSON(jsonItem2.toObject()));
    }
    return itemObject;
}

    std::set<std::string> DataHub::Item::getAllAuthors() {
        std::set<std::string> result;
        std::stack<Item> items;
        items.push(*this);
        while(!items.empty()) {
            auto item = items.top();
            items.pop();
            result.insert(item.author);
            for(auto item2 : item.needs) {
                items.push(item2);
            }
        }
        return result;
    }

    std::set<std::string> DataHub::Item::getAllCopyrights() {
        std::set<std::string> result;
        std::stack<Item> items;
        items.push(*this);
        while(!items.empty()) {
            auto item = items.top();
            items.pop();
            result.insert(item.copyright);
            for(auto item2 : item.needs) {
                items.push(item2);
            }
        }
        return result;
    }

    std::map<std::string, std::string> DataHub::Item::getAllLicences() {
        std::map<std::string, std::string> result;
        std::stack<Item> items;
        items.push(*this);
        while(!items.empty()) {
            auto item = items.top();
            items.pop();
            if(item.type != "pipeline") // Exclude licences for pipelines
                result[item.license] = item.licenseURL;
            for(auto item2 : item.needs) {
                items.push(item2);
            }
        }
        return result;
    }

    std::vector<DataHub::Item> DataHub::getItems(std::string tag) {
    std::vector<DataHub::Item> items;
    auto json = getJSONFromURL(m_URL + "api/list/" + tag);
    //std::cout << json.toJson().toStdString() << std::endl;

    for(auto item : json["items"].toArray()) {
        DataHub::Item itemObject = DataHub::Item::fromJSON(item.toObject());

        // TODO license info
        items.push_back(itemObject);
    }

    return items;
}

static bool checkMinVersion(std::string version, std::string minVersion) {
    if(minVersion.empty())
        return true;
    auto parts1 = split(version, ".");
    auto parts2 = split(minVersion, ".");
    for(int i = 0; i < 3; ++i) {
        if(std::stoi(parts1[i]) < std::stoi(parts2[i])) {
            return false;
        } else if(std::stoi(parts1[i]) > std::stoi(parts2[i])) {
            return true;
        }
    }
    return true; // All equal
}


static bool checkMaxVersion(std::string version, std::string maxVersion) {
    if(maxVersion.empty())
        return true;
    auto parts1 = split(version, ".");
    auto parts2 = split(maxVersion, ".");
    for(int i = 0; i < 3; ++i) {
        if(std::stoi(parts1[i]) < std::stoi(parts2[i])) {
            return true;
        } else if(std::stoi(parts1[i]) > std::stoi(parts2[i])) {
            return false;
        }
    }
    return true; // All equal
}

DataHub::Download DataHub::download(std::string itemID, bool force) {
    DataHub::Download download;
    QJsonDocument json;
    try {
        json = getJSONFromURL(m_URL + "api/items/get/" + itemID);
    } catch(Exception &e) {
        throw e;
    }

    std::stack<DataHub::Item> toDownload;
    toDownload.push(Item::fromJSON(json.object()));
    int counter = 0;
    while(!toDownload.empty()) {
        auto itemObject = toDownload.top();
        toDownload.pop();
        for(auto needs : itemObject.needs) {
            toDownload.push(needs);
        }
        auto folder = join(m_storageDirectory, itemObject.id);
        auto downloadName = "'" + itemObject.name + "'";
        download.paths.push_back(folder);
        createDirectories(folder);
        if(!getDirectoryList(folder, true, true).empty()) {
            // Do not download if already exists
            std::cout << downloadName << " has already been downloaded from FAST Data Hub" << std::endl;
            continue;
        } else {
            std::cout << "Retrieving " << downloadName << " " << itemObject.type << " from FAST Data Hub" << std::endl;
        }
        const std::string version = std::to_string(FAST_VERSION_MAJOR) + "."  + std::to_string(FAST_VERSION_MINOR) + "." + std::to_string(FAST_VERSION_PATCH);
        if(!checkMinVersion(version, itemObject.minFASTVersion)) {
            std::string msg = "The data hub item " + itemObject.name + " requires at least version " + itemObject.minFASTVersion + " of FAST. Consider upgrading to use this item.";
            if(force) {
                Reporter::warning() << msg << Reporter::end();
            } else {
                throw Exception(msg);
            }
        }
        if(!checkMaxVersion(version, itemObject.maxFASTVersion)) {
            std::string msg = "The data hub item " + itemObject.name + " requires FAST version to be equal or below " + itemObject.maxFASTVersion + ". Consider downgrading to use this item.";
            if(force) {
                Reporter::warning() << msg << Reporter::end();
            } else {
                throw Exception(msg);
            }
        }
        if(itemObject.type == "pipeline") {
            downloadTextFile(itemObject.downloadURL, folder, downloadName, counter);
        } else {
            std::cout << "License: " << itemObject.license << std::endl;
            std::cout << "Copyright: " << itemObject.copyright << " - " << itemObject.author << std::endl;
            if(!itemObject.licenseCustom.empty()) {
                std::cout << "Additional license information: " << itemObject.licenseCustom << std::endl;
            }
            downloadAndExtractZipFile(itemObject.downloadURL, folder, downloadName, counter);
        }
        ++counter;
        // TODO download license
    }

    emit finished();

    return download;
}

bool DataHub::isDownloaded(std::string itemID) {
    QJsonDocument json;
    try {
        json = getJSONFromURL(m_URL + "api/items/get/" + itemID);
    } catch(Exception &e) {
        throw e;
    }

    bool isDownloaded = true;

    std::stack<DataHub::Item> toDownload;
    toDownload.push(Item::fromJSON(json.object()));
    while(!toDownload.empty()) {
        auto itemObject = toDownload.top();
        toDownload.pop();
        for(auto needs : itemObject.needs) {
            toDownload.push(needs);
        }
        auto folder = join(m_storageDirectory, itemObject.id);
        try {
            if(getDirectoryList(folder, true, true).empty()) {
                isDownloaded = false;
                break;
            }
        } catch(Exception& e) {
            isDownloaded = false;
            break;
        }
    }

    return isDownloaded;
}

DataHub::Item DataHub::getItem(std::string itemID) {
    QJsonDocument json;
    try {
        json = getJSONFromURL(m_URL + "api/items/get/" + itemID);
    } catch(Exception &e) {
        throw e;
    }

    return DataHub::Item::fromJSON(json.object());
}

std::string DataHub::getStorageDirectory() const {
    return m_storageDirectory;
}

std::string DataHub::getURL() const {
    return m_URL;
}

DataHubBrowser::DataHubBrowser(std::string tag, std::string URL, std::string storageDirectory, QWidget *parent) :
        QWidget(parent), m_hub(DataHub(URL, storageDirectory)) {
    setWindowTitle("Data Hub Browser");
    auto items = m_hub.getItems(tag);

    auto mainLayout = new QVBoxLayout();
    setLayout(mainLayout);

    auto title = new QLabel;
    title->setText("");
    mainLayout->addWidget(title);

    m_listWidget = new QListWidget();
    m_listWidget->setMinimumSize(1200, 1024);

    for(auto item : items) {
        auto itemWidget = new QListWidgetItem(m_listWidget);

        // Create custom widget here, and use listWidget->setItemWidget(itemWidget, customWidget)
        // https://stackoverflow.com/questions/948444/qlistview-qlistwidget-with-custom-items-and-custom-item-widgets
        auto customWidget = new DataHubItemWidget(item, downloadThumbnail(item.thumbnailURL), m_hub.isDownloaded(item.id));
        m_listWidget->setItemWidget(itemWidget, customWidget);
        m_listWidget->setMinimumWidth(customWidget->width()*1.2);
        itemWidget->setSizeHint(customWidget->sizeHint());
        connect(customWidget, &DataHubItemWidget::download, [this, item]() {
            std::cout << "Downloading.." << std::endl;
            download(item.id);
        });
    }
    mainLayout->addWidget(m_listWidget);
}

DataHub& DataHubBrowser::getDataHub() {
    return m_hub;
}

QPixmap DataHubBrowser::downloadThumbnail(const std::string& URL) {
    QEventLoop eventLoop;

    // "quit()" the event-loop, when the network request "finished()"
    QNetworkAccessManager mgr;
    QObject::connect(&mgr, &QNetworkAccessManager::finished, &eventLoop, &QEventLoop::quit);

    // the HTTP request
    QNetworkRequest req(QUrl(QString::fromStdString(URL)));
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    QNetworkReply *reply = mgr.get(req);
    eventLoop.exec(); // blocks stack until "finished()" has been called

    reply->deleteLater();
    if (reply->error() == QNetworkReply::NoError) {
        auto status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        QByteArray bytes = reply->readAll();  // bytes
        QPixmap pixmap;
        pixmap.loadFromData(bytes);
        return pixmap;
    } else {
        //failure
        throw Exception("Error getting thumbnail from URL");
    }
}

void DataHubBrowser::download(std::string itemID) {
    auto item = m_hub.getItem(itemID);
    auto progressWidget = new DownloadProgressWidget(item);
    connect(&m_hub, &DataHub::progress, progressWidget, &DownloadProgressWidget::updateProgress);
    progressWidget->show();
    connect(&m_hub, &DataHub::finished, [=]() {
        for(int i = 0; i < m_listWidget->count(); ++i) {
            DataHubItemWidget* widget = (DataHubItemWidget*)m_listWidget->itemWidget(m_listWidget->item(i));
            if(widget->id == itemID) {
                widget->setDownloaded(true);
            }
        }
        progressWidget->close();
    });
    m_hub.download(itemID);
}

}