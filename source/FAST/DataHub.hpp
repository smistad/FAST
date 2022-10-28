#pragma once

#include <string>
#include <vector>
#include <set>
#include <map>
#include <FASTExport.hpp>
#include <QObject>
#include <QWidget>

#include <QVBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QProgressBar>
#include <iostream>
#include <QApplication>
#include <QScreen>

#ifdef SWIG
// Swig and python doesn't understand nested classes, this fixes this:
%feature("flatnested");
%ignore fast::DataHub::Item::fromJSON;
#endif
namespace fast {

/**
 * @defgroup datahub DataHub
 * Objects and functions used for the FAST DataHub
 */

#ifdef SWIG
%rename(DataHubDownload) DataHub::Download;
%rename(DataHubItem) DataHub::Item;
#endif
/**
 * @brief Object uses to browse and download data, models and pipelines from the FAST DataHub
 *
 * @ingroup datahub
 */
class FAST_EXPORT DataHub : public QObject {
    Q_OBJECT
    public:
        /**
         * @brief Object representing and item on the DataHub
         * @ingroup datahub
         */
        class FAST_EXPORT Item {
            public:
                std::string id;
                std::string name;
                std::string description;
                std::string author;
                std::string copyright;
                std::string type;
                std::string license;
                std::string licenseURL;
                std::string thumbnailURL;
                std::string downloadURL;
                uint64_t downloads;
                std::vector<DataHub::Item> needs;
                /**
                 * @brief Convert this item to JSON string
                 * @return JSON string
                 */
                static Item fromJSON(QJsonObject);
                /**
                 * @brief Get all authors of this item and dependencies
                 * @return authors
                 */
                std::set<std::string> getAllAuthors();
                /**
                 * @brief Get all copyrights of this item and dependencies
                 * @return copyrights
                 */
                std::set<std::string> getAllCopyrights();
                /**
                 * @brief Get all licenses and URLS of this item and dependencies
                 * @return licenses
                 */
                std::map<std::string, std::string> getAllLicences();
        };
        /**
         * @brief Download results from DataHub
         * @ingroup datahub
         */
        class FAST_EXPORT Download {
            public:
                std::vector<std::string> items;
                std::vector<std::string> paths;
        };
        /**
         * @brief Setup DataHub object
         * @param URL Address to DataHub. If empty use default.
         * @param storageDirectory Where on disk to store downloaded items. If empty use default.
         */
        explicit DataHub(std::string URL = "", std::string storageDirectory = "");
        /**
         * @brief Get list of items for a given tag
         * @param tag
         * @return list of items
         */
        std::vector<DataHub::Item> getItems(std::string tag);
        /**
         * @brief Get item details from DataHub
         * @param id
         * @return item
         */
        DataHub::Item getItem(std::string id);
        /**
         * @brief Download an item and all dependencies from the DataHub
         * @param itemID
         * @return
         */
        DataHub::Download download(std::string itemID);
        /**
         * @brief Whether an item (including dependencies) has been downloaded or not.
         * @param itemID
         * @return
         */
        bool isDownloaded(std::string itemID);
        /**
         * @brief Get DataHub storage directory
         * @return
         */
        std::string getStorageDirectory() const;
        /**
         * @brief Get DataHub URL
         * @return
         */
        std::string getURL() const;
    Q_SIGNALS:
        void progress(int fileNr, int percent);
        void finished();
    private:
        std::string m_URL;
        std::string m_storageDirectory;
        void downloadTextFile(const std::string& url, const std::string& destination, const std::string& name, int fileNr);
        void downloadAndExtractZipFile(const std::string& URL, const std::string& destination, const std::string& name, int fileNr);
};


/**
 * @brief A widget to browse the DataHub
 * @ingroup datahub
 */
class FAST_EXPORT DataHubBrowser : public QWidget {
    Q_OBJECT
    public:
        explicit DataHubBrowser(std::string tag = "", std::string URL = "", std::string storageDirectory = "", QWidget* parent = nullptr);
        DataHub& getDataHub();
        QPixmap downloadThumbnail(const std::string& URL);
    public Q_SLOTS:
        void download(std::string itemID);
    private:
        DataHub m_hub;
        QListWidget* m_listWidget;
};

#ifndef SWIG
/**
 * @brief A widget to display progress of downloading and item from DataHub
 */
class DownloadProgressWidget : public QWidget {
    Q_OBJECT
    public:
        DownloadProgressWidget(DataHub::Item item, QWidget* parent = nullptr) : QWidget(parent) {
            setWindowTitle("Downloading from FAST Data Hub");
            setWindowModality(Qt::ApplicationModal);
            auto layout = new QVBoxLayout();
            setLayout(layout);

            auto label = new QLabel();
            label->setText("Downloading " + QString::fromStdString(item.name) + ": ");
            layout->addWidget(label);

            auto progressBar = new QProgressBar();
            progressBar->setRange(0, 100);
            layout->addWidget(progressBar);
            m_progressBars.push_back(progressBar);

            for(auto dependency : item.needs) {
                auto label = new QLabel();
                label->setText("Downloading " + QString::fromStdString(dependency.name) + ": ");
                layout->addWidget(label);

                auto progressBar = new QProgressBar();
                progressBar->setRange(0, 100);
                layout->addWidget(progressBar);
                m_progressBars.push_back(progressBar);
            }
            move(QGuiApplication::primaryScreen()->geometry().center() - rect().center());
        }
    public Q_SLOTS:
        void updateProgress(int fileNr, int percent) {
            m_progressBars[fileNr]->setValue(percent);
        };
    private:
        std::vector<QProgressBar*> m_progressBars;
};

/**
 * @brief A widget representing an item in the DataHubBrowser
 */
class DataHubItemWidget : public QWidget {
    Q_OBJECT
    public:
        std::string id;
        QPushButton* m_downloadButton;
        DataHubItemWidget(DataHub::Item item, const QPixmap& image, bool exists, QWidget* parent = nullptr) : QWidget(parent) {
            id = item.id;
            auto layout = new QHBoxLayout();
            setLayout(layout);

            auto imageLabel = new QLabel();
            imageLabel->setAlignment(Qt::AlignTop);
            imageLabel->setPixmap(image);
            layout->addWidget(imageLabel);

            auto title = new QLabel();
            title->setWordWrap(true);
            title->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
            std::string licenseString = "";
            for(auto license : item.getAllLicences()) {
                licenseString += "<a href=\"" + license.second + "\">" + license.first + "</a>";
            }
            auto glue = [](std::set<std::string> set, std::string delimeter) -> std::string {
                std::vector<std::string> list(set.begin(), set.end());
                std::string res = "";
                if(!list.empty()) {
                    for(int i = 0; i < list.size()-1; ++i) {
                        res += list[i] + delimeter;
                    }
                    if(!list.empty())
                        res += list[list.size()-1];
                }
                return res;
            };
            title->setOpenExternalLinks(true);
            title->setText(QString::fromStdString(
              "<h3>" + item.name + "</h3>" +
                "<p>" + item.description + "</p>" +
                "<p>Downloads: " + std::to_string(item.downloads) + "</p>" +
                "<p>Authors: " + glue(item.getAllAuthors(), ", ") + "</p>" +
                "<p>Copyrights: " + glue(item.getAllCopyrights(), ", ") + "</p>" +
                "<p>Licenses: " + licenseString + "</p>"
                )
            );
            title->setAlignment(Qt::AlignLeft);
            title->adjustSize();
            layout->addWidget(title);

            m_downloadButton = new QPushButton();
            layout->addWidget(m_downloadButton);
            connect(m_downloadButton, &QPushButton::clicked, this, &DataHubItemWidget::download);
            setDownloaded(exists);
            m_downloadButton->adjustSize();
        };
        void setDownloaded(bool downloaded) {
            if(downloaded) {
                m_downloadButton->setDisabled(true);
                m_downloadButton->setText("Already downloaded");
            } else {
                m_downloadButton->setDisabled(false);
                m_downloadButton->setText("Download");
            }
        }
    Q_SIGNALS:
        void download();
    };

#endif

}
#ifdef SWIG
// Swig and python doesn't understand nested classes, this fixes this:
%feature("flatnested", "");
#endif
