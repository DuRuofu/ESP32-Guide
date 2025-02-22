// pages/blueWifi/blueWifi.js
//获取应用实例
const app = getApp();
const util = require('../../utils/util.js');
const crypto = require('../../crypto/crypto-dh.js');
const md5 = require('../../crypto/md5.min.js');
var client = "";
Page({

  /**
   * 页面的初始数据
   */
  data: {
    deviceId: "",
    wifiList: [],
    hiddenModal: true,
    blueConnectNum: 0,
    frag: 16,
    flag: false,
    flagEnd: false,
    fragList: [],
    frameControl: 0,
    ssid: "",
    bssid: "",
    serviceId: "",
    uuid: "",
    password: "",
  },
  bindViewWifi: function(event) {
    var self = this,
      ssid = event.currentTarget.dataset.ssid,
      bssid = event.currentTarget.dataset.bssid;
    self.setData({
      hiddenModal: false,
      ssid: ssid,
      bssid: bssid,
      password: ""
    })
  },
  bindViewConfirm: function() {
    var self = this;
    wx.navigateTo({
      url: '/pages/blueConnect/blueConnect?deviceId=' + self.data.deviceId + "&ssid=" + escape(self.data.ssid) + "&bssid=" + self.data.bssid + "&password=" + escape(self.data.password) + "&serviceId=" + self.data.serviceId + "&uuid=" + self.data.uuid,
    })
    this.setData({
      hiddenModal: true,
    })
  },
  bindViewCancel: function(){
    this.setData({
      hiddenModal: true,
      ssid: "",
      bssid: "",
      password: ""
    })
  },
  bindViewInput: function (e) {
    this.setData({ password: e.detail.value })
  },
  blueConnect: function (event) {
    var self = this;
    app.data.sequenceControl = -1;
    app.data.sequenceNumber = -1;
    self.setData({
      fragList: [],
      wifiList: [],
      flagEnd: false,
      serviceId: "",
      uuid: ""
    });
    wx.createBLEConnection({
      deviceId: self.data.deviceId,
      timeout: 10000,
      success: function (res) {
        console.info("app.data.platform", app.data.platform)
        if (app.data.platform == 'android') {
          app.data.mtu = app.data.constMtu;
          wx.setBLEMTU({
            deviceId: self.data.deviceId,
            mtu: app.data.constMtu,
            success (res) {
              console.info('setBLEMTU-suc', res)
            },
            fail (res) {
              console.info('setBLEMTU-fail', res)
            }
          })
        }
        self.getDeviceServices(self.data.deviceId);
      },
      fail: function (res) {
        var num = self.data.blueConnectNum;
        if (num < 3) {
          self.blueConnect();
          num++;
          self.setData({
            blueConnectNum: num
          })
        } else {
          self.showFailToast();
        }
      }
    })
  },
  getDeviceServices: function (deviceId) {
    var self = this;
    wx.getBLEDeviceServices({
      deviceId: deviceId,
      success: function (res) {
        var services = res.services;
        if (services.length > 0) {
          for (var i = 0; i < services.length; i++) {
            var uuid = services[i].uuid;
            if (uuid == app.data.service_uuid) {
              self.getDeviceCharacteristics(deviceId, uuid);
              return false;
            }
          }
        }
      },
      fail: function (res) {
        self.showFailToast();
      }
    })
  },
  getDeviceCharacteristics: function (deviceId, serviceId) {
    var self = this;
    wx.getBLEDeviceCharacteristics({
      deviceId: deviceId,
      serviceId: serviceId,
      success: function (res) {
        var list = res.characteristics;
        if (list.length > 0) {
          for (var i = 0; i < list.length; i++) {
            var uuid = list[i].uuid;
            if (uuid == app.data.characteristic_write_uuid) {
              self.openNotify(deviceId, serviceId, uuid);
              self.setData({
                serviceId: serviceId,
                uuid: uuid,
              })
              return false;
            }
          }
        }
      },
      fail: function (res) {
        self.showFailToast();
      }
    })
  },
  //通知设备交互方式（是否加密）
  notifyDevice: function (deviceId, serviceId, characteristicId) {
    var self = this;
    client = util.blueDH(util.DH_P, util.DH_G, crypto);
    var kBytes = util.uint8ArrayToArray(client.getPublicKey());
    var pBytes = util.hexByInt(util.DH_P);
    var gBytes = util.hexByInt(util.DH_G);
    var pgkLength = pBytes.length + gBytes.length + kBytes.length + 6;
    var pgkLen1 = (pgkLength >> 8) & 0xff;
    var pgkLen2 = pgkLength & 0xff;
    var data = [];
    data.push(util.NEG_SET_SEC_TOTAL_LEN);
    data.push(pgkLen1);
    data.push(pgkLen2);
    var frameControl = util.getFrameCTRLValue(false, false, util.DIRECTION_OUTPUT, false, false);
    var value = util.writeData(util.PACKAGE_VALUE, util.SUBTYPE_NEG, frameControl, app.data.sequenceControl, data.length, data);
    var typedArray = new Uint8Array(value);
    console.log("notifyDevice:", value)
    wx.writeBLECharacteristicValue({
      deviceId: deviceId,
      serviceId: serviceId,
      characteristicId: characteristicId,
      value: typedArray.buffer,
      success: function (res) {
        console.log("notifyDevice-suc:", res)
        self.getSecret(deviceId, serviceId, characteristicId, client, kBytes, pBytes, gBytes, null);
      },
      fail: function (res) {
        console.log("notifyDevice-fail:", res)
        self.showFailToast();
      }
    })
  },
  getSecret: function (deviceId, serviceId, characteristicId, client, kBytes, pBytes, gBytes, data) {
    var self = this, obj = {}, frameControl = 0;
    app.data.sequenceControl = parseInt(app.data.sequenceControl) + 1;
    const encrypt = false
    const checksum = false
    if (!util._isEmpty(data)) {
      obj = util.isSubcontractor(data, checksum, app.data.sequenceControl);
      frameControl = util.getFrameCTRLValue(encrypt, checksum, util.DIRECTION_OUTPUT, false, obj.flag);
    } else {
      data = [];
      data.push(util.NEG_SET_SEC_ALL_DATA);
      var pLength = pBytes.length;
      var pLen1 = (pLength >> 8) & 0xff;
      var pLen2 = pLength & 0xff;
      data.push(pLen1);
      data.push(pLen2);
      data = data.concat(pBytes);
      var gLength = gBytes.length;
      var gLen1 = (gLength >> 8) & 0xff;
      var gLen2 = gLength & 0xff;
      data.push(gLen1);
      data.push(gLen2);
      data = data.concat(gBytes);
      var kLength = kBytes.length;
      var kLen1 = (kLength >> 8) & 0xff;
      var kLen2 = kLength & 0xff;
      data.push(kLen1);
      data.push(kLen2);
      data = data.concat(kBytes);
      obj = util.isSubcontractor(data, checksum, app.data.sequenceControl);
      frameControl = util.getFrameCTRLValue(encrypt, checksum, util.DIRECTION_OUTPUT, false, obj.flag);
    }
    var value = util.writeData(util.PACKAGE_VALUE, util.SUBTYPE_NEG, frameControl, app.data.sequenceControl, obj.len, obj.lenData);
    var typedArray = new Uint8Array(value);
    wx.writeBLECharacteristicValue({
      deviceId: deviceId,
      serviceId: serviceId,
      characteristicId: characteristicId,
      value: typedArray.buffer,
      success: function (res) {
        console.log('getSecret-suc', res)
        if (obj.flag) {
          self.getSecret(deviceId, serviceId, characteristicId, client, kBytes, pBytes, gBytes, obj.laveData);
        } else {
          // setTimeout(function(){
          //   self.getWifiList(deviceId, serviceId, characteristicId);
          // }, 3000)
          self.setSecret(deviceId, serviceId, characteristicId);
        }
      },
      fail: function (res) {
        console.log('getSecret-error', res)
        self.showFailToast();
      }
    })
  },
  setSecret (deviceId, serviceId, characteristicId) {
    const self = this;
    let value = 0;
    // value |= 1; // 数据包校验
    // value |= 0b10; //数据包加密
    let data = [value]
    app.data.sequenceControl = parseInt(app.data.sequenceControl) + 1;
    let frameControl = util.getFrameCTRLValue(false, false, util.DIRECTION_OUTPUT, false, false);
    value = util.writeData(util.PACKAGE_CONTROL_VALUE, util.SUBTYPE_SET_SEC_MODE, frameControl, app.data.sequenceControl, data.length, data);
    var typedArray = new Uint8Array(value);
    wx.writeBLECharacteristicValue({
      deviceId: deviceId,
      serviceId: serviceId,
      characteristicId: characteristicId,
      value: typedArray.buffer,
      success: function (res) {
        setTimeout(function(){
          self.getWifiList(deviceId, serviceId, characteristicId);
        }, 3000)
      },
      fail: function (res) {
        self.showFailToast();
      }
    })
  },
  getWifiList: function (deviceId, serviceId, characteristicId) {
    var self = this;
    var frameControl = util.getFrameCTRLValue(false, false, util.DIRECTION_OUTPUT, false, false);
    app.data.sequenceControl = parseInt(app.data.sequenceControl) + 1;
    var value = util.writeData(util.PACKAGE_CONTROL_VALUE, util.SUBTYPE_WIFI_NEG, frameControl, app.data.sequenceControl, 0, null);
    var typedArray = new Uint8Array(value);
    wx.writeBLECharacteristicValue({
      deviceId: deviceId,
      serviceId: serviceId,
      characteristicId: characteristicId,
      value: typedArray.buffer,
      success: function (res) {
      },
      fail: function (res) {
        self.showFailToast();
      }
    })
  },
  // 监听特征值变化
  onNotify: function () {
    var self = this;
    wx.onBLECharacteristicValueChange(function (res) {
      self.analysisWifi(util.ab2hex(res.value));
    })
  },
  // 启用特征值变化
  openNotify: function (deviceId, serviceId, characteristicId) {
    var self = this;
    wx.notifyBLECharacteristicValueChange({
      state: true, // 启用 notify 功能
      deviceId: deviceId,
      serviceId: serviceId,
      characteristicId: app.data.characteristic_read_uuid,
      success: function (res) {
        //通知设备交互方式（是否加密）
        // self.notifyDevice(deviceId, serviceId, characteristicId);
        self.onNotify();
        self.getWifiList(deviceId, serviceId, characteristicId);
      },
      fail: function (res) {
        self.showFailToast();
      }
    })
  },
  analysisWifi: function (list) {
    const self = this;
    var fragList = self.data.fragList;
    if (list.length < 4) {
      return false;
    }
    var val = list[0],
      type = val & 3,
      subType = val >> 2;
    if (type != parseInt(util.PACKAGE_VALUE)) {
      wx.hideLoading();
      return false;
    }
    var sequenceNum = parseInt(list[2], 16);
    if (sequenceNum - app.data.sequenceNumber  != 1) {
      wx.hideLoading();
      return false;
    }
    app.data.sequenceNumber  = sequenceNum;
    if (app.data.sequenceNumber  == 255) {
      app.data.sequenceNumber  = -1
    }
    var dataLength = parseInt(list[3], 16);
    if (dataLength == 0) {
      wx.hideLoading();
      return false;
    }
    console.log('fragNum', list)
    var fragNum = util.hexToBinArray(list[1]);
    list = util.isEncrypt(self, fragNum, list);
    fragList = fragList.concat(list);
    self.setData({
      fragList: fragList,
    })
    if (self.data.flagEnd) {
      if (subType == util.SUBTYPE_WIFI_LIST_NEG) {
        self.getList(fragList, fragList.length, 0);
        wx.hideLoading();
      } else if (subType == util.SUBTYPE_NEGOTIATION_NEG) { 
        var arr = util.hexByInt(fragList.join(""));
        var clientSecret = client.computeSecret(new Uint8Array(arr));
        var md5Key = md5.array(clientSecret);
        app.data.md5Key = md5Key;
        self.setData({
          fragList: [],
        })
      } else {
        wx.hideLoading();
      }
      self.setData({
        flagEnd: false,
      })
    }
  },
  getList: function (arr, totalLength, curLength) {
    var self = this;
    if (arr.length > 0) {
      var len = parseInt(arr[0], 16);
      curLength += (1 + len);
      if (len > 0 && curLength < totalLength) {
        var rssi = 0, name = "";
        let list = []
        for (var i = 1; i <= len; i++) {
          if (i == 1) {
            rssi = parseInt(arr[i], 16);
          } else {
            list.push(parseInt(arr[i], 16))
          }
        }
        name = decodeURIComponent(encodeURIComponent(String.fromCharCode(...list)))
        var wifiList = self.data.wifiList;
        wifiList.push({ "rssi": rssi, "SSID": name });
        self.setData({
          wifiList: wifiList.sort(util.sortBy("rssi", false))
        })
        arr = arr.splice(len + 1);
        this.getList(arr, totalLength, curLength);
      }
    }
  },
  
  showFailToast: function() {
    wx.hideLoading();
    wx.showToast({
      title: 'WiFi信息获取失败',
      icon: 'none',
      duration: 2000
    })
  },
  /**
   * 生命周期函数--监听页面加载
   */
  onLoad: function (options) {
    var self = this;
    wx.setNavigationBarTitle({
      title: '选择WiFi'
    });
    self.setData({
      deviceId: options.deviceId,
    })
    wx.showLoading({
      title: 'WiFi获取中...',
    })
    wx.stopBluetoothDevicesDiscovery({
      success: function (res) {
      }
    })
    self.blueConnect();
  },

  /**
   * 生命周期函数--监听页面初次渲染完成
   */
  onReady: function () {
  
  },

  /**
   * 生命周期函数--监听页面显示
   */
  onShow: function () {
  
  },

  /**
   * 生命周期函数--监听页面隐藏
   */
  onHide: function () {
  
  },

  /**
   * 生命周期函数--监听页面卸载
   */
  onUnload: function () {
  
  },

  /**
   * 页面相关事件处理函数--监听用户下拉动作
   */
  onPullDownRefresh: function () {
    var self = this;
    self.setData({
      wifiList: [],
      fragList: []
    });
    self.getWifiList(self.data.deviceId, self.data.serviceId, self.data.uuid);
    setTimeout(function () {
      wx.stopPullDownRefresh();
    }, 6000);
  },

  /**
   * 页面上拉触底事件的处理函数
   */
  onReachBottom: function () {
  
  },

  /**
   * 用户点击右上角分享
   */
  onShareAppMessage: function () {
  
  }
})