Clarius module {#clarius-module}
===================================

The Clarius ultrasound module features a ClariusStreamer object which can be used to stream B-mode and Doppler data in real-time from a [Clarius ultrasound scanner](https://clarius.com/). 

FAST uses the [Clarius listen API](https://clarius.com/research-toolkits/) for streaming, and you need a research license from Clarius to use this API.

In the clarius app, make sure you have selected "Research (5828)" under Clarius Cast in settings. After this done connect to the clarius probe's wifi access point with your machine running FAST. You should then be able to use the clarius streamer in FAST. The password for the access point is added to the clipboard on your mobile device when connecting using the device.

**Note for Windows:** The windows firewall will block the images sent from the Clarius scanner. Thus you need to disable the windows firewall or add an exception in the firewall.