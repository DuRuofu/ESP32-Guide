import DefaultTheme from 'vitepress/theme'

// 首页自定义样式
import './custom.css'
// 代码块样式
import './vp-code.css'

// 图片查看器
import 'viewerjs/dist/viewer.min.css';
import imageViewer from 'vitepress-plugin-image-viewer';
import vImageViewer from 'vitepress-plugin-image-viewer/lib/vImageViewer.vue';
import { useRoute } from 'vitepress';

//导航布局
import NavMenu from './components/NavMenu.vue';

export default {
	...DefaultTheme,
	enhanceApp(ctx) {
		DefaultTheme.enhanceApp(ctx);
		// 点击显示图片组件
		ctx.app.component('vImageViewer', vImageViewer);
		// 导航卡片
		ctx.app.component('NavMenu', NavMenu);
	},
	setup() {
		// 获取路由
		const route = useRoute();
		// 使用
		imageViewer(route);
	}
};
