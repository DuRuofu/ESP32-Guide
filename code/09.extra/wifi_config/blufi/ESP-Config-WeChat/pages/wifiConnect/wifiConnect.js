// pages/wifiConnect/wifiConnect.js

Page({

  /**
   * 页面的初始数据
   */
  data: {
    failStatus: false,
    sucStatus: false,
    progress: 0,
  },
  getStatus: function() {
    var self = this;
    
    wx.request({
      url: 'https://www.alavening.com/wifi/connstatus.cgi',
      data: {
      },
      header: {
        'content-type': 'application/json' // 默认值
      },
      success: function (res) {
        console.log(res.data);
        var data = res.data;
        console.log(data.status);
        if (data.status == "success") {
          self.setData({
            progress: 100,
            failStatus: false,
            sucStatus: true,         
          })
          self.setResult("success");
        } else if (data.status == "fail") {
          self.setData({
            failStatus: true,
            sucStatus: false,
          })
          self.setResult("fail");
        } else {
          if (self.data.progress <= 90) {
            self.setData({
              progress: (self.data.progress + 5),
              failStatus: false,
              sucStatus: false,
            })
          }
          self.getStatus();
        }
      },
      fail: function (res) {
        self.setData({
          failStatus: true,
          sucStatus: false,
        })
        
      }
    })
    
  },
  setResult: function(result) {
    var self = this;
    wx.request({
      url: 'https://www.alavening.com/wifi/configsuccess.cgi',
      method: "post",
      data: {
        configstatus: result
      },
      header: {
        'content-type': 'application/x-www-form-urlencoded'
      },
      success: function (res) {
      },
      fail: function () {
      }
    })
  },
  setConnect: function(ssid, password) {
    var self = this;
    wx.request({
      url: 'https://alavening.com/wifi/connect.cgi',
      method: "post",
      data: {
        essid: ssid,
        password: password
      },
      header: {
        'content-type': 'application/x-www-form-urlencoded'
      },
      success: function (res) {
                                   
      },
      fail: function () {
      }
    })
    self.getStatus();
  },
  successwifi: function() {
    wx.reLaunch({
      url: '/pages/index/index'
    })
  },
  failwifi: function() {
    wx.navigateBack({
      delta: 2
    })
  },
  /**
   * 生命周期函数--监听页面加载
   */
  onLoad: function (options) {
    var self = this;
    wx.setNavigationBarTitle({
      title: 'WiFi配网'
    });
    self.setConnect(options.ssid, options.password);
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

  /**
   * 用户点击右上角分享
   */
  onShareAppMessage: function () {
  
  }
})