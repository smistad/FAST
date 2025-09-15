#include <FAST/Testing.hpp>
#include <FAST/DataHub.hpp>
#include <QApplication>

using namespace fast;

TEST_CASE("Data hub download", "[fast][DataHub]")  {
    auto hub = DataHub();
    auto download = hub.download("brain-mri-t1");
}

/*
TEST_CASE("Data hub", "[fast][DataHub]") {
    auto hub = DataHub();
    hub.getItems("wsi");

    auto res = hub.getItem("breast-tumour-segmentation");

    auto browser = new DataHubBrowser("fast-pathology");
    browser->show();
    QApplication::instance()->exec();
}
*/