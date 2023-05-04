# LaRoomyApi_Nordic_nRF52
![](https://img.shields.io/badge/Platform-nordicnrf52-002f3e)
![](https://img.shields.io/badge/Environment-PlatformIO-9cf)
![](https://img.shields.io/badge/Framework-Arduino-informational)

This is the **LaRoomy Api** remote framework library. Implementing this library enables your Nano 33 Ble project to interact with the [LaRoomy App](https://www.laroomy.com). The library is written for the **Nordic nRF52** platform and is intended to be used with the **PlatformIO** embedded development platform or the Arduino IDE.

The complete documentation for the library is provided on the [LaRoomy Api Website](https://api.laroomy.com/).
 
# How to use the Library

## Usage with PlatformIO

### Prerequisites

This library requires [Platform IO](https://platformio.org/platformio-ide) to be installed in [Visual Studio Code](https://code.visualstudio.com/).

- [Install Visual Studio Code](https://code.visualstudio.com/)
- [Install Platform IO for VS Code ](https://docs.platformio.org/en/latest/integration/ide/vscode.html#installation)


### Implementation

- Add the dependency to the **platformio.ini** file of your project:

```ini
lib_deps = https://github.com/LaRoomy/LaRoomyApi_Nordic_nRF52
```

- Include the repective header in your main project file

```c
#include <LaRoomyApi_nRF52.h>
```

- Now you can use the LaRoomy Api with the respective access operator

```cpp
LaRoomyApi.begin();
```

## Usage with Arduino IDE:

### Prerequisites

This library requires the Arduino IDE to be installed

- [Download and install Arduino IDE](https://support.arduino.cc/hc/en-us/articles/360019833020-Download-and-install-Arduino-IDE)

### Implementation

- Download this repository as .zip file (click on '<>Code' and select 'Download ZIP')
- Open the Arduino IDE
- Click on the 'Sketch' entry on the menu and select 'Include Library' > 'Add .ZIP Library'
- Select the downloaded .zip file and press OK
- When the library was successful installed, it can be used by clicking on 'Sketch' > 'Include Library' in the menu and select the LaRoomy Api library in the section for the contributed libraries
- Now you can use the LaRoomy Api with the respective access operator:

```cpp
LaRoomyApi.begin();
```

**NOTE:** This library depends on the [ArduinoBLE](https://github.com/arduino-libraries/ArduinoBLE) library which must be also installed (if it isn't done automatically)

## Examples

A detailed description of the examples with test schematics can be found [here](https://github.com/LaRoomy/LaRoomyAPI_Remote_Examples).

## Further Information

[Click here for detailed instructions on how to use the LaRoomyApi.](https://api.laroomy.com/p/laroomy-api-class.html)


![](https://img.shields.io/github/license/LaRoomy/LaRoomyApi_Nordic_nRF52?color=009999&style=for-the-badge)


