//获取应用实例
const app = getApp()
const util = require('../../utils/util.js');

const timeOut = 20;//超时时间
var timeId = "";
Page({
  data: {
    failure: false,
    value: 0,
    desc: "Device connecting...",
    isChecksum: false,
    isEncrypt: false,
    flagEnd: false,
    defaultData: 1,
    ssidType: 2,
    passwordType: 3,
    meshIdType: 3,
    deviceId: "",
    ssid: "",
    uuid: "",
    serviceId: "",
    password: "",
    meshId: "",
    processList: [],
    result: [],
  },
  blueConnect: function (event) {
    var self = this;
    self.setProcess(0, util.descSucList[0]);
    self.setProcess(10, util.descSucList[1]);
    self.setProcess(20, util.descSucList[2]);
    self.setProcess(35, util.descSucList[3]);
    self.openNotify(self.data.deviceId, self.data.serviceId, self.data.uuid);
  },
  // 告知设备数据开始写入
  writeDeviceStart: function (deviceId, serviceId, characteristicId, data) {
    var self = this, obj = {}, frameControl = 0;
    self.setProcess(40, util.descSucList[4]);
    app.data.sequenceControl = parseInt(app.data.sequenceControl) + 1;
    if (!util._isEmpty(data)) {
      obj = util.isSubcontractor(data, self.data.isChecksum, app.data.sequenceControl, self.data.isEncrypt);
      frameControl = util.getFrameCTRLValue(self.data.isEncrypt, self.data.isChecksum, util.DIRECTION_OUTPUT, false, obj.flag);
    } else {
      obj = util.isSubcontractor([self.data.defaultData], self.data.isChecksum, app.data.sequenceControl, true);
      frameControl = util.getFrameCTRLValue(self.data.isEncrypt, self.data.isChecksum, util.DIRECTION_OUTPUT, false, obj.flag);
    }
    // var defaultData = util.encrypt(app.data.sequenceControl, obj.lenData, true);
    var value = util.writeData(util.PACKAGE_CONTROL_VALUE, util.SUBTYPE_WIFI_MODEl, frameControl, app.data.sequenceControl, obj.len, obj.lenData);
    var typedArray = new Uint8Array(value)
    wx.writeBLECharacteristicValue({
      deviceId: deviceId,
      serviceId: serviceId,
      characteristicId: characteristicId,
      value: typedArray.buffer,
      success: function (res) {
        if (obj.flag) { 
          self.writeDeviceStart(deviceId, serviceId, characteristicId, obj.laveData);
        } else {
          self.setProcess(60, util.descSucList[5]);
          self.writeRouterSsid(deviceId, serviceId, characteristicId, null);
        }
      },
      fail: function (res) {
        self.setFailProcess(true, util.descFailList[3]);
      }
    })
  },
  //写入路由ssid
  writeRouterSsid: function (deviceId, serviceId, characteristicId, data) {
    var self = this, obj = {}, frameControl = 0;
    app.data.sequenceControl = parseInt(app.data.sequenceControl) + 1;
    if (!util._isEmpty(data)) {
      obj = util.isSubcontractor(data, self.data.isChecksum, app.data.sequenceControl, self.data.isEncrypt);
      frameControl = util.getFrameCTRLValue(self.data.isEncrypt, self.data.isChecksum, util.DIRECTION_OUTPUT, false, obj.flag);
    } else {
      var ssidData = self.getCharCodeat(self.data.ssid);
      obj = util.isSubcontractor(ssidData, self.data.isChecksum, app.data.sequenceControl, self.data.isEncrypt);
      frameControl = util.getFrameCTRLValue(self.data.isEncrypt, self.data.isChecksum, util.DIRECTION_OUTPUT, false, obj.flag);
    }
    // var defaultData = util.encrypt(app.data.sequenceControl, obj.lenData, true);
    var value = util.writeData(util.PACKAGE_VALUE, util.SUBTYPE_SET_SSID, frameControl, app.data.sequenceControl, obj.len, obj.lenData);
    var typedArray = new Uint8Array(value)
    wx.writeBLECharacteristicValue({
      deviceId: deviceId,
      serviceId: serviceId,
      characteristicId: characteristicId,
      value: typedArray.buffer,
      success: function (res) {
        if (obj.flag) {
          self.writeRouterSsid(deviceId, serviceId, characteristicId, obj.laveData);
        } else {
          self.writeDevicePwd(deviceId, serviceId, characteristicId, null);
        }
      },
      fail: function (res) {
        self.setFailProcess(true, util.descFailList[4]);
      }
    })
  },
  //写入路由密码
  writeDevicePwd: function (deviceId, serviceId, characteristicId, data) {
    var self = this, obj = {}, frameControl = 0;
    app.data.sequenceControl = parseInt(app.data.sequenceControl) + 1;
    if (!util._isEmpty(data)) {
      obj = util.isSubcontractor(data, self.data.isChecksum, app.data.sequenceControl, self.data.isEncrypt);
      frameControl = util.getFrameCTRLValue(self.data.isEncrypt, self.data.isChecksum, util.DIRECTION_OUTPUT, false, obj.flag);
    } else {
      var pwdData = self.getCharCodeat(self.data.password);
      obj = util.isSubcontractor(pwdData, self.data.isChecksum, app.data.sequenceControl, self.data.isEncrypt);
      frameControl = util.getFrameCTRLValue(self.data.isEncrypt, self.data.isChecksum, util.DIRECTION_OUTPUT, false, obj.flag);
    }
    // var defaultData = util.encrypt(app.data.sequenceControl, obj.lenData, true);
    var value = util.writeData(util.PACKAGE_VALUE, util.SUBTYPE_SET_PWD, frameControl, app.data.sequenceControl, obj.len, obj.lenData);
    var typedArray = new Uint8Array(value)
    wx.writeBLECharacteristicValue({
      deviceId: deviceId,
      serviceId: serviceId,
      characteristicId: characteristicId,
      value: typedArray.buffer,
      success: function (res) {
        if (obj.flag) {
          self.writeDevicePwd(deviceId, serviceId, characteristicId, obj.laveData);
        } else {
          self.writeDeviceEnd(deviceId, serviceId, characteristicId);
        }
      },
      fail: function (res) {
        self.setFailProcess(true, util.descFailList[4]);
      }
    })
  },
  //告知设备写入结束
  writeDeviceEnd: function (deviceId, serviceId, characteristicId) {
    var self = this;
    app.data.sequenceControl = parseInt(app.data.sequenceControl) + 1;
    var frameControl = util.getFrameCTRLValue(self.data.isEncrypt, false, util.DIRECTION_OUTPUT, false, false);
    var value = util.writeData(self.data.PACKAGE_CONTROL_VALUE, util.SUBTYPE_END, frameControl, app.data.sequenceControl, 0, null);
    var typedArray = new Uint8Array(value)
    wx.writeBLECharacteristicValue({
      deviceId: deviceId,
      serviceId: serviceId,
      characteristicId: characteristicId,
      value: typedArray.buffer,
      success: function (res) {
        self.onTimeout(0);
      },
      fail: function (res) {
        self.setFailProcess(true, util.descFailList[4]);
      }
    })
  }, 
  //连接超时
  onTimeout: function(num) {
    const self = this;
    timeId = setInterval(function() {
      if (num < timeOut) {
        num++;
      } else {
        clearInterval(timeId);
        self.setFailProcess(true, util.descFailList[4]);
      }
    }, 1000)
  },
  //监听通知
  onNotify: function () {
    var self = this;
    wx.onBLECharacteristicValueChange(function (res) {
      self.getResultType(util.ab2hex(res.value));
    })
  },
  //启用通知
  openNotify: function (deviceId, serviceId, characteristicId) {
    var self = this;
    wx.notifyBLECharacteristicValueChange({
      state: true, // 启用 notify 功能
      deviceId: deviceId,
      serviceId: serviceId,
      characteristicId: app.data.characteristic_read_uuid,
      success: function (res) {
        self.writeDeviceStart(deviceId, serviceId, characteristicId);
        self.onNotify();
      },
      fail: function (res) {
      }
    })
  },
  getSsids: function (str) {
    var list = [],
      strs = str.split(":");
    for (var i = 0; i < strs.length; i++) {
      list.push(parseInt(strs[i], 16));
    }
    return list;
  },
  getCharCodeat: function (str) {
    var list = [];
    for (var i = 0; i < str.length; i++) {
      list.push(str.charCodeAt(i));
    }
    return list;
  },
  setProcess: function(value, desc) {
    var self = this, list = [];
    list = self.data.processList;
    list.push(desc);
    self.setData({
      value: value,
      processList: list
    });
    if (value == 100) {
      self.closeConnect();
      self.setData({
        desc: util.descSucList[6]
      });
      clearInterval(timeId);
      app.data.sequenceControl = 0;
      setTimeout(function () {
        wx.reLaunch({
          url: '/pages/index/index'
        })
      }, 3000)
    }
  },
  setFailProcess: function (flag, desc) {
    var self = this, list = [];
    list = self.data.processList;
    list.push(desc);
    self.setFailBg();
    self.setData({
      failure: flag,
      processList: list
    });
  },
  getResultType: function(list) {
    var self = this;
    var result = self.data.result;
    console.log(list)
    if (list.length < 4) {
      self.setFailProcess(true, util.descFailList[4]);
      return false;
    }
    var val = parseInt(list[0], 16),
      type = val & 3,
      subType = val >> 2;
      console.log(type, subType, self.data.flagEnd)
    if (type != parseInt(util.PACKAGE_VALUE)) {
      self.setFailProcess(true, util.descFailList[4]);
      return false;
    }
    var sequenceNum = parseInt(list[2], 16);
    if (sequenceNum - app.data.sequenceNumber  != 1) {
      self.setFailProcess(true, util.descFailList[4]);
      return false;
    }
    app.data.sequenceNumber  = sequenceNum;
    if (app.data.sequenceNumber  == 255) {
      app.data.sequenceNumber  = -1
    }
    var dataLength = parseInt(list[3], 16);
    if (dataLength == 0) {
      self.setFailProcess(true, util.descFailList[4]);
      return false;
    }
    var fragNum = util.hexToBinArray(list[1]);
    list = util.isEncrypt(self, fragNum, list);
    result = result.concat(list);
    self.setData({
      result: result,
    })
    if (self.data.flagEnd) {
      self.setData({
        flagEnd: false,
      })
      if (type == 1) {
        if (subType == 15) {
          for (var i = 0; i <= result.length; i++) {
            var num = parseInt(result[i], 16) + "";
            if (i == 0) {
              self.setProcess(85, "Connected: " + util.successList[num]);
            } else if (i == 1) {
              if (num == 0) {
                self.setProcess(100, util.descSucList[6]);
              }
            }
          }
        } else if (subType == 18) {
          for (var i = 0; i <= result.length; i++) {
            var num = parseInt(result[i], 16) + "";
            if (i == 0) {
              self.setProcess(85, util.successList[num]);
            } else if (i == 1) {
              self.setFailProcess(true, util.failList[num]);
            }
          }
        } else {
          self.setFailProcess(true, util.descFailList[4])
        }
      } else {
        self.setFailProcess(true, util.descFailList[4])
      }
    }
    
    
  },
  closeConnect: function () {
    var self = this;
    wx.closeBLEConnection({
        deviceId: self.data.deviceId,
        success: function (res) {
        }
      })
      wx.closeBluetoothAdapter({
        success: function() {
        }
      });
  },
  //设置配网失败背景色
  setFailBg: function() {
    wx.setNavigationBarColor({
      frontColor: "#ffffff",
      backgroundColor: '#737d89',
    })
  },
  //设置配网成功背景色
  setSucBg: function() {
    wx.setNavigationBarColor({
      frontColor: "#ffffff",
      backgroundColor: '#4d9efb',
    })
  },
  onLoad: function (options) {
    var self = this;
    self.setSucBg();
    wx.setNavigationBarTitle({
      title: '配网'
    });
    self.setData({
      deviceId: options.deviceId,
      ssid: unescape(options.ssid),
      password: unescape(options.password),
      meshId: options.bssid,
      uuid: options.uuid,
      serviceId: options.serviceId,
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

  },

  /**
   * 页面上拉触底事件的处理函数
   */
  onReachBottom: function () {

  },

})
