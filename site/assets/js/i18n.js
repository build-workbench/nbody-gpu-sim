/**
 * i18n.js - Language switching and auto-detection for N-Body Simulation site
 * =========================================================================
 */

(function() {
  'use strict';

  // Configuration
  const CONFIG = {
    storageKey: 'nbody-lang',
    defaultLang: 'en',
    supportedLangs: ['en', 'zh-CN'],
    zhPatterns: ['zh', 'zh-CN', 'zh-CN', 'zh-Hans', 'zh-Hans-CN']
  };

  // DOM Elements
  const langToggle = document.getElementById('lang-toggle');
  const langDropdown = document.getElementById('lang-dropdown');

  /**
   * Get user's preferred language from browser
   */
  function getBrowserLanguage() {
    const browserLang = navigator.language || navigator.userLanguage;
    // Check if browser language is Chinese
    if (CONFIG.zhPatterns.some(pattern => browserLang.startsWith(pattern))) {
      return 'zh-CN';
    }
    return 'en';
  }

  /**
   * Get stored language preference
   */
  function getStoredLanguage() {
    try {
      return localStorage.getItem(CONFIG.storageKey);
    } catch (e) {
      return null;
    }
  }

  /**
   * Store language preference
   */
  function setStoredLanguage(lang) {
    try {
      localStorage.setItem(CONFIG.storageKey, lang);
    } catch (e) {
      // localStorage not available
    }
  }

  /**
   * Check if this is the first visit (no language preference stored)
   */
  function isFirstVisit() {
    return getStoredLanguage() === null;
  }

  /**
   * Get current page language from URL or page data
   */
  function getCurrentLanguage() {
    const path = window.location.pathname;
    if (path.includes('/zh-CN')) {
      return 'zh-CN';
    }
    return 'en';
  }

  /**
   * Get alternate language URL
   */
  function getAlternateUrl(targetLang) {
    const currentPath = window.location.pathname;
    const currentLang = getCurrentLanguage();

    // Remove any base path (like /n-body)
    let path = currentPath;

    if (targetLang === 'zh-CN') {
      // Switch to Chinese: add /zh-CN prefix
      if (!path.includes('/zh-CN')) {
        // Remove leading slash, add /zh-CN/
        path = '/zh-CN' + path;
        // Handle double slashes
        path = path.replace(/\/+/g, '/');
      }
    } else {
      // Switch to English: remove /zh-CN prefix
      path = path.replace('/zh-CN', '');
      if (path === '' || path === '/') {
        path = '/';
      }
    }

    return path;
  }

  /**
   * Redirect to appropriate language version based on browser preference
   */
  function autoRedirect() {
    if (!isFirstVisit()) {
      return; // User already has a preference
    }

    const browserLang = getBrowserLanguage();
    const currentLang = getCurrentLanguage();

    // Only redirect if there's a mismatch
    if (browserLang !== currentLang) {
      const targetUrl = getAlternateUrl(browserLang);
      console.log(`Auto-redirecting to ${browserLang}: ${targetUrl}`);

      // Store the preference before redirecting
      setStoredLanguage(browserLang);

      // Redirect
      window.location.href = targetUrl;
    }
  }

  /**
   * Handle language toggle click
   */
  function toggleDropdown() {
    const isOpen = langDropdown.classList.contains('open');
    langDropdown.classList.toggle('open');
    langToggle.setAttribute('aria-expanded', !isOpen);
  }

  /**
   * Close dropdown when clicking outside
   */
  function handleClickOutside(event) {
    if (!event.target.closest('.lang-switch')) {
      langDropdown.classList.remove('open');
      langToggle.setAttribute('aria-expanded', 'false');
    }
  }

  /**
   * Handle language selection
   */
  function handleLanguageSelect(event) {
    const link = event.target.closest('a[data-lang]');
    if (!link) return;

    event.preventDefault();
    const targetLang = link.getAttribute('data-lang');
    const currentLang = getCurrentLanguage();

    // Store preference
    setStoredLanguage(targetLang);

    // Navigate if different language
    if (targetLang !== currentLang) {
      const targetUrl = link.getAttribute('href');
      window.location.href = targetUrl;
    } else {
      // Just close dropdown if same language
      langDropdown.classList.remove('open');
      langToggle.setAttribute('aria-expanded', 'false');
    }
  }

  /**
   * Handle keyboard navigation
   */
  function handleKeyboard(event) {
    if (event.key === 'Escape') {
      langDropdown.classList.remove('open');
      langToggle.setAttribute('aria-expanded', 'false');
      langToggle.focus();
    }
  }

  /**
   * Initialize the language switcher
   */
  function init() {
    // Auto-redirect on first visit
    autoRedirect();

    // Set up event listeners
    if (langToggle) {
      langToggle.addEventListener('click', toggleDropdown);
    }

    if (langDropdown) {
      langDropdown.addEventListener('click', handleLanguageSelect);
    }

    // Global event listeners
    document.addEventListener('click', handleClickOutside);
    document.addEventListener('keydown', handleKeyboard);

    // Log current language
    console.log(`Current language: ${getCurrentLanguage()}`);
  }

  // Run on DOM ready
  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', init);
  } else {
    init();
  }

})();
