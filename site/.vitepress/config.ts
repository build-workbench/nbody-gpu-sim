import { defineConfig } from 'vitepress'
import { withMermaid } from 'vitepress-plugin-mermaid'

const rawBase = process.env.VITEPRESS_BASE
const base = rawBase
  ? rawBase.startsWith('/')
    ? rawBase.endsWith('/') ? rawBase : `${rawBase}/`
    : `/${rawBase}/`
  : '/n-body/'

export default withMermaid(defineConfig({
  base,
  title: 'N-Body Simulation',
  description: 'Million-Particle GPU Physics Engine',

  head: [
    ['link', { rel: 'icon', href: '/n-body/favicon.ico' }],
    ['meta', { name: 'theme-color', content: '#10b981' }],
  ],

  locales: {
    en: {
      label: 'English',
      lang: 'en-US',
      link: '/en/',
      themeConfig: {
        nav: [{ text: 'Home', link: '/en/' }],
        footer: {
          message: 'Released under the MIT License.',
          copyright: 'Copyright © 2024-2026 AICL-Lab',
        },
      },
    },
    zhCN: {
      label: '简体中文',
      lang: 'zh-CN',
      link: '/zh-CN/',
      themeConfig: {
        nav: [{ text: '首页', link: '/zh-CN/' }],
        footer: {
          message: '基于 MIT 许可证发布。',
          copyright: '版权所有 © 2024-2026 AICL-Lab',
        },
      },
    },
  },

  themeConfig: {
    socialLinks: [
      { icon: 'github', link: 'https://github.com/AICL-Lab/n-body' },
    ],
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
}))
