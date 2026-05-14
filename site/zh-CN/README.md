---
layout: home
---

<script setup>
import { onMounted } from 'vue'
import { useRouter } from 'vitepress'

const router = useRouter()

onMounted(() => {
  const lang = navigator.language.toLowerCase()
  if (lang.startsWith('zh')) {
    router.go('/zh-CN/')
  } else {
    router.go('/en/')
  }
})
</script>

<div class="redirect-notice">
  <p>Redirecting to your preferred language...</p>
  <p>正在跳转到您的首选语言...</p>
</div>

<style>
.redirect-notice {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  min-height: 60vh;
  text-align: center;
  color: var(--vp-c-text-2);
}
</style>
