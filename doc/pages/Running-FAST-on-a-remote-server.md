Running FAST on a remote server {#fast-remote-server}
========================================

FAST can run on a remote Linux server using SSH.

**If you don't want to do any visualization, you don't have to do anything extra.** FAST will detect that there is no display to render and thus disable visualization (i.e. OpenGL) in FAST. If you try to do anything that requires OpenGL (rendering etc.) FAST will crash.

**If you want to run FAST with visualization on a remote server you need to use [VirtualGL](https://virtualgl.org/).** Indirect rendering over SSH with X forwarding (ssh -X) will not work because FAST uses modern OpenGL (3+), which indirect GLX does not support (however some have reported that modern Mesa OpenGL does support this).

### Setting up VirtualGL
Install VirtualGL on the client machine (If windows, see [documentation here](https://cdn.rawgit.com/VirtualGL/virtualgl/2.6.5/doc/index.html#hd008)):
```bash
sudo apt install virtualgl
```
Then, install VirtualGL on the server machine:
```bash
sudo apt install virtualgl
```
If your server is *headless*, meaning it has no display connected, you need to setup an "empty" X display.
If your server uses the NVIDIA driver, you can do this with the following command:
```bash
sudo nvidia-xconfig -a --use-display-device=None --virtual=1280x1024
```
This will create a new xorg.conf config file in the folder /etc/X11/.

Then run the virtualgl config on your server:
```
vglserver_config
```
Finally restart X on your server, or just reboot:
```bash
sudo systemctl restart gdm
```
If something goes wrong, remove the xorg.conf that was created (or replace it with the backup /etc/X11/xorg.conf.backup) and reboot.

### Using FAST with VirtualGL
Instead of using the ssh command to connect to the server, connect with the following virtualgl command instead:
```bash
vglconnect -s user@domain
```
If you are on secure local area network, you can drop the -s parameter which can increase performance.
Then, when you want to run FAST, add vglrun to your command, e.g. to run the importImageFromFile example:
```bash
vglrun /opt/fast/bin/importImageFromFile
```
The same for python:
```bash
# Interactive python:
vglrun python
# Running a python script file:
vglrun python your_script.py
```

For more information on VirtualGL over SSH, see [documentation here](https://cdn.rawgit.com/VirtualGL/virtualgl/2.6.5/doc/index.html#hd008).