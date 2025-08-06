import fast
import shutil
import os


def test_data_hub_available():
    items = fast.DataHub().getItems('wsi')
    #assert len(items) > 0


def test_download_item():
    name = 'jugular-carotid-ultrasound-segmentation-model'
    hub = fast.DataHub()

    # Delete if exists
    assert len(hub.getStorageDirectory()) > 0
    if os.path.isdir(os.path.join(hub.getStorageDirectory(), name)):
        shutil.rmtree(os.path.join(hub.getStorageDirectory(), name))

    # Download
    assert not hub.isDownloaded(name)
    download = hub.download(name)
    assert len(download.items) == 1
    assert len(download.paths) == 1
    #assert download.items[0] == name
    assert os.path.normpath(download.paths[0]) == os.path.normpath(os.path.join(hub.getStorageDirectory(), name))
    assert hub.isDownloaded(name)