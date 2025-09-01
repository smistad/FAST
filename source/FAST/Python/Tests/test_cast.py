import subprocess
import fast
import pytest
import sys
import os


def get_ubuntu_version():
    result = subprocess.run(['cat', '/etc/os-release'], stdout=subprocess.PIPE).stdout.decode('utf-8')
    values = {}
    for line in result.split('\n'):
        parts = line.split('=')
        if len(parts) == 2:
            values[parts[0]] = parts[1].replace('"', '')
    return values['ID'], values['VERSION_ID']


def test_cast():
    should_fail = False
    if sys.platform == 'linux':
        id, version = get_ubuntu_version()
        if id != 'ubuntu':
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
        name = {'darwin': 'libcast.dylib', 'linux': 'libcast.so', 'win32': 'cast.dll'}
        assert os.path.exists(fast.Config.getKernelBinaryPath() + '/../lib/cast/LICENSE.txt')
        assert os.path.exists(fast.Config.getKernelBinaryPath() + '/../lib/cast/' + name[sys.platform])

        streamer.run()

