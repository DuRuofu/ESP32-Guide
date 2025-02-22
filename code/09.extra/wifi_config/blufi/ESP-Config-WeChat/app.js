//app.js
App({
  data: {
    service_uuid: "0000FFFF-0000-1000-8000-00805F9B34FB",
    characteristic_write_uuid: "0000FF01-0000-1000-8000-00805F9B34FB",
    characteristic_read_uuid: "0000FF02-0000-1000-8000-00805F9B34FB",
    name: "BLUFI",
    mtu: 19,
    constMtu: 128,
    md5Key: "",
    platform: '',
    sequenceControl: -1,
    sequenceNumber: -1
  },
  
  onLaunch: function () {
   
  },
  globalData: {
    userInfo: null
  }
})