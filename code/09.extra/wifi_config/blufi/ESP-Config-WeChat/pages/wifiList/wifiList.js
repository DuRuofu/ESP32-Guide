// pages/wifiList/wifiList.js
Page({

  /**
   * 页面的初始数据
   */
  data: {
    password: '',
    hiddenmodal: true,//可以通过hidden是否掩藏弹出框的属性，来指定那个弹出框 
    wifiList: [],
    ssid: '',
  },
  //点击按钮弹出指定的hiddenmodalput弹出框  
  bindViewWifi: function (e) {
    var list = this.data.wifiList;
    var index = e.currentTarget.dataset.index;
    //console.log(list[index])
    this.setData({
      ssid: list[index].essid,
      hiddenmodal: !this.data.hiddenmodal,

    })

  },
  bindViewInput: function (e) {
    this.setData({ password: e.detail.value })
  },
  getWifiList: function() {
    var self = this;
    wx.request({
      url: 'https://www.alavening.com/wifi/wifiscan.cgi',
      data: {
      },
      header: {
        'content-type': 'application/json' // 默认值
      },
      success: function (res) {
        console.log(res);
        var data = res.data.result;
        console.log(data);
        if (data.inProgress == "0") {
          wx.hideLoading();
          self.setData({
            wifiList: data.APs
          })
        } else {
          self.getWifiList();
        }
      },
      fail: function(res) {
        console.log(res);
        wx.hideLoading();
      }
    })
  },
  //取消按钮  
  cancel: function () {
    this.setData({
      hiddenmodal: true
    });
  },
  //确认  
  confirm: function () {
    var self = this;
    // this.setData({
    //   hiddenmodal: true
    // })
    if (!self.data.password.length < 8) {
      // 这里修改成跳转的页面
      wx.redirectTo({
        url: '/pages/wifiConnect/wifiConnect?password=' + self.data.password + "&ssid=" + self.data.ssid
      })

    }
  },
  /**
   * 生命周期函数--监听页面加载
   */
  onLoad: function (options) {
    var self = this;
    wx.setNavigationBarTitle({
      title: 'WiFi配网'
    });
    setTimeout(function() {
      wx.showLoading({
        title: 'WiFi加载中...',
      });
      self.getWifiList();
      console.log("WiFi加载中...");
    }, 500);
   
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