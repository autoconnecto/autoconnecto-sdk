'use client';

import React, { useState, useEffect } from 'react';
import Image from 'next/image';
import Icon from '@/components/ui/AppIcon';
import Link from 'next/link';
import {
  ARDUINO_SDK_GITHUB_URL,
  MOBILE_APP_RELEASES_URL,
} from '@/config/links';

const navLinks = [
  { label: 'Product', href: '/product' },
  { label: 'Platform', href: '/#platform' },
  { label: 'Pricing', href: '/pricing' },
  { label: 'Features', href: '/#features' },
  { label: 'Solutions', href: '/#solutions' },
  { label: 'Scale', href: '/#scale' },
  { label: 'Developers', href: '/#developers' },
  { label: 'Arduino SDK', href: ARDUINO_SDK_GITHUB_URL, external: true },
  { label: 'Mobile app', href: MOBILE_APP_RELEASES_URL, external: true },
  { label: 'Docs', href: 'https://docs.autoconnecto.in', external: true },
  { label: 'Contact', href: '/#contact' },
];

export default function Header() {
  const [scrolled, setScrolled] = useState(false);
  const [mobileOpen, setMobileOpen] = useState(false);

  useEffect(() => {
    const onScroll = () => setScrolled(window.scrollY > 20);
    window.addEventListener('scroll', onScroll, { passive: true });
    return () => window.removeEventListener('scroll', onScroll);
  }, []);

  useEffect(() => {
    if (mobileOpen) {
      const onScroll = () => setMobileOpen(false);
      window.addEventListener('scroll', onScroll, { passive: true, once: true });
    }
  }, [mobileOpen]);

  return (
    <header
      className={`fixed top-0 inset-x-0 z-50 transition-all duration-300 ${
        scrolled
          ? 'bg-[#0B1120]/90 backdrop-blur-xl border-b border-[#1E2D4A]'
          : 'bg-transparent'
      }`}
    >
      <div className="max-w-7xl mx-auto px-4 sm:px-6">
        <div className="flex items-center justify-between h-16 sm:h-18">
          {/* Logo */}
          <Link href="/" className="flex items-center gap-2.5 group">
            <Image
              src="/assets/images/autoconnecto-logo-1777694039224.png"
              alt="Autoconnecto Logo"
              width={140}
              height={36}
              className="h-9 w-auto object-contain mix-blend-lighten"
              priority
            />
          </Link>

          {/* Desktop Nav */}
          <nav className="hidden md:flex items-center gap-1 bg-secondary/40 border border-border rounded-full px-2 py-1">
            {navLinks?.map((link) => (
              <a
                key={link?.href}
                href={link?.href}
                target={link?.external ? '_blank' : undefined}
                rel={link?.external ? 'noopener noreferrer' : undefined}
                className="px-4 py-1.5 rounded-full text-sm font-medium text-muted-foreground hover:text-foreground hover:bg-white/5 transition-all duration-200"
              >
                {link?.label}
              </a>
            ))}
          </nav>

          {/* CTA */}
          <div className="flex items-center gap-3">
            <a
              href="/#contact"
              className="hidden sm:inline-flex items-center gap-2 px-5 py-2 rounded-full bg-primary text-primary-foreground text-sm font-semibold hover:bg-primary/90 transition-all duration-200 shadow-[0_0_20px_rgba(14,165,233,0.3)]"
            >
              Get Started
              <Icon name="ArrowRightIcon" size={14} />
            </a>

            {/* Mobile hamburger */}
            <button
              className="md:hidden flex items-center justify-center w-10 h-10 rounded-full border border-border text-muted-foreground hover:text-foreground transition-colors"
              onClick={() => setMobileOpen((v) => !v)}
              aria-label="Toggle menu"
            >
              <Icon name={mobileOpen ? 'XMarkIcon' : 'Bars3Icon'} size={20} />
            </button>
          </div>
        </div>
      </div>
      {/* Mobile menu */}
      {mobileOpen && (
        <div className="md:hidden bg-[#0B1120]/95 backdrop-blur-xl border-b border-border px-6 py-6 flex flex-col gap-3">
          {navLinks?.map((link) => (
            <a
              key={link?.href}
              href={link?.href}
              target={link?.external ? '_blank' : undefined}
              rel={link?.external ? 'noopener noreferrer' : undefined}
              onClick={() => setMobileOpen(false)}
              className="py-2.5 text-base font-medium text-muted-foreground hover:text-foreground transition-colors border-b border-border/50 last:border-0"
            >
              {link?.label}
            </a>
          ))}
          <a
            href="#contact"
            onClick={() => setMobileOpen(false)}
            className="mt-2 flex items-center justify-center gap-2 px-5 py-3 rounded-full bg-primary text-primary-foreground text-sm font-semibold"
          >
            Get Started
            <Icon name="ArrowRightIcon" size={14} />
          </a>
        </div>
      )}
    </header>
  );
}