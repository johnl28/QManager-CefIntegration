# QManager-CefIntegration
[![badge](https://img.shields.io/badge/C%2B%2B-17-brightgreen)]() 

[![badge](https://img.shields.io/github/issues/johnl28/QManager-CefIntegration)]() 

[![badge](https://img.shields.io/github/license/johnl28/QManager-CefIntegration)](https://github.com/johnl28/QManager-CefIntegration/blob/main/LICENSE.txt)

QManager is an windows executable which provide a range of tools to personalise your desktop, such as:
* Auto-hide icons
* Tracking color
* Text highlight color
* Global text color

This software uses [CEF](https://bitbucket.org/chromiumembedded/cef/src/master/) for UI rendering and [JsonCPP](https://github.com/open-source-parsers/jsoncpp) for config files.


![no img](https://i.imgur.com/y2b9A9j.png)

### Dependencies

* `Chromium Version` 91.0.4472.101 (not compatbile with newer)
* `JsonCPP` 1.8.0
* `Bootstrap` Latest version
* `jQuery` Latest version


### Build

The project can be builded with Visual Studio 2019 a newer version.

**The project is linking the libraries using global environment variables:** `CEF_DIR` and `jsoncpp`.