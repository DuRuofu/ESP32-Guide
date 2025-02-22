//index.js
//获取应用实例
const app = getApp();
const util = require('../../utils/util.js');
Page({
  data: {
    
  },
  //事件处理函数
  bindViewBlue: function() {
    wx.closeBluetoothAdapter({
      success: function () {
      }
    });
    wx.openBluetoothAdapter({
      success: function (res) {
        wx.startBluetoothDevicesDiscovery({
          success: function (res) {
            wx.navigateTo({
              url: '/pages/blueDevices/blueDevices',
            })
          }
        })
      },
      fail: function (res) {
        wx.showToast({
          title: '请打开蓝牙',
          icon: 'none',
          duration: 2000
        })
      }
    })
    
  },
  bindViewWifi: function () {
    wx.navigateTo({
      url: '/pages/wifiDevices/wifiDevices',
    })
  },
  onLoad: function () {
    wx.setNavigationBarTitle({
      title: '配网方式'
    })
    wx.getSystemInfo({
      success (res) {
       try {
        app.data.platform = res.platform.toLocaleLowerCase()
       } catch(e) {
         console.log(e)
       }
      }
    })
  },
  getUserInfo: function(e) {
  }
    
})
