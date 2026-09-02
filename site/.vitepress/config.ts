import { defineConfig } from 'vitepress'

const rawBase = process.env.VITEPRESS_BASE
const base = rawBase
  ? rawBase.startsWith('/')
    ? rawBase.endsWith('/') ? rawBase : `${rawBase}/`
    : `/${rawBase}/`
  : '/nbody-gpu-sim/'

export default defineConfig({
  base,
  lang: 'zh-CN',
  title: 'N-Body Simulation',
  description: '百万粒子 GPU 物理引擎',

  head: [
    // 内联 SVG favicon，无需从 public/ 服务二进制资源。
    ['link', { rel: 'icon', href: 'data:image/svg+xml,<svg xmlns=%22http://www.w3.org/2000/svg%22 viewBox=%220 0 100 100%22><text y=%22.9em%22 font-size=%2290%22>🪐</text></svg>' }],
    ['meta', { name: 'theme-color', content: '#10b981' }],
  ],

  themeConfig: {
    socialLinks: [
      { icon: 'github', link: 'https://github.com/build-workbench/nbody-gpu-sim' },
    ],
    footer: {
      message: '基于 MIT 许可证发布。',
      copyright: '版权所有 © 2024-2026 Vibe Knight',
    },
  },

  markdown: {
    theme: { light: 'github-light', dark: 'github-dark' },
    codeTransformers: [
      {
        name: 'cuda-to-cpp',
        preprocess(code, options) {
          if (options?.lang === 'cuda') options.lang = 'cpp'
          return code
        },
      },
    ],
  },
})
