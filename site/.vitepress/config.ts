import { defineConfig } from 'vitepress'
import { withMermaid } from 'vitepress-plugin-mermaid'
import llmstxt from 'vitepress-plugin-llms'

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
    ['meta', { property: 'og:type', content: 'website' }],
    ['meta', { property: 'og:title', content: 'N-Body Simulation' }],
    ['meta', { property: 'og:description', content: 'Million-Particle GPU Physics Engine with CUDA Acceleration' }],
  ],

  locales: {
    en: {
      label: 'English',
      lang: 'en-US',
      link: '/en/',
      title: 'N-Body Simulation',
      description: 'Million-Particle GPU Physics Engine',
      themeConfig: {
        nav: [
          { text: 'Home', link: '/en/' },
          { text: 'Getting Started', link: '/en/getting-started/installation', activeMatch: '/en/getting-started/' },
          { text: 'User Guide', link: '/en/user-guide/configuration', activeMatch: '/en/user-guide/' },
          { text: 'API', link: '/en/api-reference/particle-system', activeMatch: '/en/api-reference/' },
          { text: 'Developer', link: '/en/developer-guide/architecture', activeMatch: '/en/developer-guide/' },
          { text: 'Benchmarks', link: '/en/benchmarks/performance', activeMatch: '/en/benchmarks/' },
        ],
        sidebar: {
          '/en/getting-started/': [
            {
              text: 'Getting Started',
              items: [
                { text: 'Installation', link: '/en/getting-started/installation' },
                { text: 'Quick Start', link: '/en/getting-started/quick-start' },
                { text: 'Examples', link: '/en/getting-started/examples' },
                { text: 'Testing', link: '/en/getting-started/testing' },
              ],
            },
          ],
          '/en/user-guide/': [
            {
              text: 'User Guide',
              items: [
                { text: 'Configuration', link: '/en/user-guide/configuration' },
                { text: 'Algorithms', link: '/en/user-guide/algorithms' },
                { text: 'Particle Distributions', link: '/en/user-guide/distributions' },
                { text: 'Serialization', link: '/en/user-guide/serialization' },
              ],
            },
          ],
          '/en/api-reference/': [
            {
              text: 'API Reference',
              items: [
                { text: 'ParticleSystem', link: '/en/api-reference/particle-system' },
                { text: 'ForceCalculator', link: '/en/api-reference/force-calculator' },
                { text: 'Integrator', link: '/en/api-reference/integrator' },
                { text: 'Renderer', link: '/en/api-reference/renderer' },
              ],
            },
          ],
          '/en/developer-guide/': [
            {
              text: 'Developer Guide',
              items: [
                { text: 'Architecture', link: '/en/developer-guide/architecture' },
                { text: 'CUDA Kernels', link: '/en/developer-guide/cuda-kernels' },
                { text: 'Memory Layout', link: '/en/developer-guide/memory-layout' },
                { text: 'Testing Guide', link: '/en/developer-guide/testing' },
                { text: 'Contributing', link: '/en/developer-guide/contributing' },
                { text: 'References', link: '/en/developer-guide/references' },
              ],
            },
          ],
          '/en/benchmarks/': [
            {
              text: 'Benchmarks',
              items: [
                { text: 'Performance Overview', link: '/en/benchmarks/performance' },
                { text: 'Algorithm Comparison', link: '/en/benchmarks/algorithm-comparison' },
                { text: 'Methodology', link: '/en/benchmarks/methodology' },
              ],
            },
          ],
        },
        editLink: {
          pattern: 'https://github.com/AICL-Lab/n-body/edit/main/site/:path',
          text: 'Edit this page on GitHub',
        },
        footer: {
          message: 'Released under the MIT License.',
          copyright: 'Copyright © 2024-2026 AICL-Lab',
        },
        docFooter: {
          prev: 'Previous',
          next: 'Next',
        },
        outline: {
          level: [2, 3],
          label: 'On this page',
        },
        lastUpdated: {
          text: 'Last updated',
          formatOptions: {
            dateStyle: 'medium',
            timeStyle: 'short',
          },
        },
      },
    },
    zhCN: {
      label: '简体中文',
      lang: 'zh-CN',
      link: '/zh-CN/',
      title: 'N-Body 粒子模拟',
      description: '百万粒子 GPU 物理引擎',
      themeConfig: {
        nav: [
          { text: '首页', link: '/zh-CN/' },
          { text: '入门指南', link: '/zh-CN/getting-started/installation', activeMatch: '/zh-CN/getting-started/' },
          { text: '用户手册', link: '/zh-CN/user-guide/configuration', activeMatch: '/zh-CN/user-guide/' },
          { text: 'API 参考', link: '/zh-CN/api-reference/particle-system', activeMatch: '/zh-CN/api-reference/' },
          { text: '开发指南', link: '/zh-CN/developer-guide/architecture', activeMatch: '/zh-CN/developer-guide/' },
          { text: '性能基准', link: '/zh-CN/benchmarks/performance', activeMatch: '/zh-CN/benchmarks/' },
        ],
        sidebar: {
          '/zh-CN/getting-started/': [
            {
              text: '入门指南',
              items: [
                { text: '安装', link: '/zh-CN/getting-started/installation' },
                { text: '快速开始', link: '/zh-CN/getting-started/quick-start' },
                { text: '示例', link: '/zh-CN/getting-started/examples' },
                { text: '测试', link: '/zh-CN/getting-started/testing' },
              ],
            },
          ],
          '/zh-CN/user-guide/': [
            {
              text: '用户手册',
              items: [
                { text: '配置', link: '/zh-CN/user-guide/configuration' },
                { text: '算法', link: '/zh-CN/user-guide/algorithms' },
                { text: '粒子分布', link: '/zh-CN/user-guide/distributions' },
                { text: '序列化', link: '/zh-CN/user-guide/serialization' },
              ],
            },
          ],
          '/zh-CN/api-reference/': [
            {
              text: 'API 参考',
              items: [
                { text: 'ParticleSystem', link: '/zh-CN/api-reference/particle-system' },
                { text: 'ForceCalculator', link: '/zh-CN/api-reference/force-calculator' },
                { text: 'Integrator', link: '/zh-CN/api-reference/integrator' },
                { text: 'Renderer', link: '/zh-CN/api-reference/renderer' },
              ],
            },
          ],
          '/zh-CN/developer-guide/': [
            {
              text: '开发指南',
              items: [
                { text: '架构', link: '/zh-CN/developer-guide/architecture' },
                { text: 'CUDA 内核', link: '/zh-CN/developer-guide/cuda-kernels' },
                { text: '内存布局', link: '/zh-CN/developer-guide/memory-layout' },
                { text: '测试指南', link: '/zh-CN/developer-guide/testing' },
                { text: '贡献指南', link: '/zh-CN/developer-guide/contributing' },
                { text: '参考文献', link: '/zh-CN/developer-guide/references' },
              ],
            },
          ],
          '/zh-CN/benchmarks/': [
            {
              text: '性能基准',
              items: [
                { text: '性能概览', link: '/zh-CN/benchmarks/performance' },
                { text: '算法对比', link: '/zh-CN/benchmarks/algorithm-comparison' },
                { text: '测试方法', link: '/zh-CN/benchmarks/methodology' },
              ],
            },
          ],
        },
        editLink: {
          pattern: 'https://github.com/AICL-Lab/n-body/edit/main/site/:path',
          text: '在 GitHub 上编辑此页',
        },
        footer: {
          message: '基于 MIT 许可证发布。',
          copyright: '版权所有 © 2024-2026 AICL-Lab',
        },
        docFooter: {
          prev: '上一页',
          next: '下一页',
        },
        outline: {
          level: [2, 3],
          label: '本页目录',
        },
        lastUpdated: {
          text: '最后更新',
          formatOptions: {
            dateStyle: 'medium',
            timeStyle: 'short',
          },
        },
      },
    },
  },

  // Shared theme config (applies to all locales)
  themeConfig: {
    socialLinks: [
      { icon: 'github', link: 'https://github.com/AICL-Lab/n-body' },
    ],
    search: {
      provider: 'local',
      options: {
        detailedView: true,
      },
    },
  },

  vite: {
    plugins: [llmstxt()],
  },

  markdown: {
    theme: {
      light: 'github-light',
      dark: 'github-dark',
    },
    codeTransformers: [
      {
        name: 'cuda-to-cpp',
        preprocess(code, options) {
          if (options?.lang === 'cuda') {
            options.lang = 'cpp'
          }
          return code
        }
      }
    ],
  },

  mermaid: {
    // refer https://mermaid-js.github.io/mermaid/#/Setup for options
  },
}))