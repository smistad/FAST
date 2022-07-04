#include <FAST/Testing.hpp>
#include <FAST/DataHub.hpp>
#include <QApplication>

using namespace fast;

/*
TEST_CASE("Data hub", "[fast][DataHub]") {
    auto hub = DataHub("http://localhost:8000/");
    hub.getItems("wsi");

    hub.download("nuclei-segmentation");

    auto browser = new DataHubBrowser(std::move(hub), "fast-pathology");
    browser->show();
    QApplication::instance()->exec();
}
 */