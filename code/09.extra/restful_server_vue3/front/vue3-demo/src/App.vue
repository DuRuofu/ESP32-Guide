<template>
  <div class="page">
    <h1>version</h1>
    <p>{{ version }}</p>
    <el-button type="primary" @click="getVersionButton">Primary</el-button>
  </div>
</template>

<script setup lang="ts">

import { ref } from 'vue'
import axios from 'axios';
const version = ref('')

const getVersionButton = () => {

  axios.get('/api/v1/system/info').then((res) => {
    version.value = res.data.version
    console.log("获取成功",res)
  }).catch((err) => {
    console.log(err)
  })
}
</script>

<style scoped lang="scss">
.page {
  text-align: center;
  margin: 20px;
}

.page h1 {
  font-size: 2em;
  margin-bottom: 10px;
}

.page p {
  font-size: 1.2em;
}
</style>
