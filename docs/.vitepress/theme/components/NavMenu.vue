<template>
	<div class="list">
		<a v-for="(item, index) in list" :key="index" class="item" :href="item.link" target="_blank">
			<article class="box">
				<div class="box-header">
					<div class="icon">
						<img :src="item.icon" class="icon-img" alt="" />
					</div>
					<div class="title" :title="item.title">{{ item.title }}</div>
				</div>
				<p class="desc" :title="item.desc">{{ item.desc }}</p>
			</article>
		</a>
	</div>
</template>
<script setup lang="ts">
type Item = {
	icon: string;
	title: string;
	desc: string;
	link: string;
};
defineProps<{
	list: Item[];
}>();
</script>
<style scoped lang="scss">
.list {
	display: flex;
	flex-wrap: wrap;

	.item {
		flex: 1;
		min-width: 200px;
		margin: 10px;
		display: block;
		border: 1px solid var(--vp-c-bg-soft);
		border-radius: 8px;
		text-decoration: inherit;
		background-color: var(--vp-c-bg-alt);
		transition: all 0.25s;

		&:last-child {
			margin-right: 0;
		}

		&:nth-child(3n) {
			margin-right: 0;
		}

		&:nth-child(3n + 1) {
			margin-left: 0;
		}

		&:hover {
			box-shadow: var(--vp-shadow-2);
			border-color: var(--vp-c-brand);
			text-decoration: initial;
			background-color: var(--vp-c-bg);
		}
	}

	.box {
		display: flex;
		flex-direction: column;
		padding: 12px;
		height: 100%;
		color: var(--vp-c-text-1);

		&-header {
			display: flex;
			align-items: center;
		}
	}

	.icon {
		display: flex;
		justify-content: center;
		align-items: center;
		margin-right: 10px;
		border-radius: 6px;
		width: 40px;
		height: 40px;
		font-size: 24px;
		background-color: var(--vp-c-default-soft);
		transition: background-color 0.25s;

		.icon-img {
			border-radius: 4px;
			width: 24px;
			pointer-events: none;
		}
	}

	.title {
		overflow: hidden;
		flex-grow: 1;
		white-space: nowrap;
		text-overflow: ellipsis;
		line-height: 40px;
		font-size: 16px;
		font-weight: 600;
	}

	.desc {
		display: -webkit-box;
		-webkit-line-clamp: 2;
		-webkit-box-orient: vertical;
		overflow: hidden;
		text-overflow: ellipsis;
		flex-grow: 1;
		margin: 10px 0 0;
		line-height: 1.5;
		font-size: 12px;
		color: var(--vp-c-text-2);
	}
}
</style>
