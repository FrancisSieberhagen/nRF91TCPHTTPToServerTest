# nRF91TCPHTTPTest

## Test BSD library - NB-IoT TCP HTTP client connect to worldtimeapi.org

### nRF Connect SDK!
    https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/index.html

    Installing the nRF Connect SDK through nRF Connect for Desktop
    https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/gs_assistant.html

### Nordicsemi nRF9160 NB-IoT 
    https://www.nordicsemi.com/Software-and-tools/Development-Kits/nRF9160-DK

### CLion - A cross-platform IDE for C
    https://devzone.nordicsemi.com/f/nordic-q-a/49730/howto-nrf9160-development-with-clion

### Application Description
    curl Test 
    # curl 172.105.18.205:42501
    # {"ActionName":"BSDTest","LED1":false,"LED2":true}


    Client: Connect to nRF91ServerTest
    Client: Parse JSON and toggel LED's  
    Client: Close socket

### Test Server 
    # Test Server
    CONFIG_SERVER_HOST="72.105.18.205"
    CONFIG_SERVER_PORT=42501

### Build hex 
    $ export ZEPHYR_BASE=/????
    $ west build -b nrf9160_pca10090ns

### Program nRF9160-DK using nrfjprog
    $ nrfjprog --program build/zephyr/merged.hex -f nrf91 --chiperase --reset --verify


### nRF Connect
![alt text](https://raw.githubusercontent.com/FrancisSieberhagen/nRF91TCPHTTPToServerTest/master/images/nRFConnect.jpg)
