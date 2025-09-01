import fast
import pytest
import sys
import os


def test_cast():
    should_fail = False
    if sys.platform == 'linux':
        import lsb_release
        version = lsb_release.get_os_release()['RELEASE']
        id = lsb_release.get_os_release()['ID']
        if id != 'Ubuntu':
            return
        major, minor = version.split('.')
        if major not in ('20', '22', '24'):
            should_fail = True
        if minor != '04':
            should_fail = True
        print(major, minor)

    fast.Reporter.setGlobalReportMethod(fast.Reporter.COUT)
    if should_fail:
        with pytest.raises(Exception):
            streamer = fast.ClariusStreamer.create()
    else:
        streamer = fast.ClariusStreamer.create()

    # Check that files exist
    name = {'darwin': 'libcast.dylib', 'linux': 'libcast.so', 'windows': 'cast.dll'}
    assert os.path.exists(fast.Config.getKernelBinaryPath() + '/../lib/cast/LICENSE.txt')
    assert os.path.exists(fast.Config.getKernelBinaryPath() + '/../lib/cast/' + name[sys.platform])

    streamer.run()

