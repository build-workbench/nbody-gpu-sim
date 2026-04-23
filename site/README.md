# N-Body Simulation GitHub Pages

This directory contains the GitHub Pages site for the N-Body Simulation project.

## Development

### Prerequisites

- Ruby 3.0+
- Bundler

### Setup

```bash
cd site
bundle install
```

### Local Development

```bash
bundle exec jekyll serve
```

The site will be available at `http://localhost:4000/n-body/`

## Structure

```
site/
├── _config.yml          # Jekyll configuration
├── _layouts/            # HTML layouts
├── _includes/           # Reusable components
├── _sass/               # SCSS stylesheets
├── assets/              # CSS, JS, images
├── docs/                # Documentation pages
├── changelog/           # Changelog pages
└── index.md             # Homepage
```

## Deployment

The site is automatically deployed via GitHub Actions when changes are pushed to the main branch.

See `.github/workflows/pages.yml` for the deployment configuration.
