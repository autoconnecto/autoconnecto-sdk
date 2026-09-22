import React from 'react';
import type { Metadata } from 'next';
import Header from '@/components/Header';
import Footer from '@/components/Footer';
import HeroSection from '@/app/components/HeroSection';
import DashboardSection from '@/app/components/DashboardSection';
import FeaturesSection from '@/app/components/FeaturesSection';
import SolutionsSection from '@/app/components/SolutionsSection';
import ScaleSection from '@/app/components/ScaleSection';
import DevelopersSection from '@/app/components/DevelopersSection';
import ContactSection from '@/app/components/ContactSection';
import TestimonialsSection from '@/app/components/TestimonialsSection';
import { siteOrigin } from '@/lib/siteOrigin';

const baseUrl = siteOrigin();

export const metadata: Metadata = {
  title: 'Autoconnecto — Enterprise IoT Platform',
  description:
    'Autoconnecto is a full-stack IoT platform with Machine Fleet and Generator Monitoring widgets, live dashboards, 60+ widgets, OTA firmware, edge gateways, MQTT/HTTPS/LTE, pipelines, RBAC, and white-label for enterprise scale.',
  alternates: { canonical: `${baseUrl}/` },
  openGraph: {
    type: 'website',
    url: `${baseUrl}/`,
    title: 'Autoconnecto — Enterprise IoT Platform',
    description:
      'Full-stack IoT platform: Machine Fleet, Generator Monitoring, dashboards, OTA, edge gateways, pipelines, MQTT/HTTPS/LTE, multi-tenant SaaS from India.',
    siteName: 'Autoconnecto',
    images: [
      {
        url: `${baseUrl}/assets/images/app_logo.png`,
        width: 1200,
        height: 630,
        alt: 'Autoconnecto Enterprise IoT Platform',
        type: 'image/png',
      },
    ],
    locale: 'en_US',
  },
  twitter: {
    card: 'summary_large_image',
    title: 'Autoconnecto — Enterprise IoT Platform',
    description:
      'Full-stack IoT platform: Machine Fleet, Generator Monitoring, dashboards, OTA, edge gateways, pipelines, MQTT/HTTPS/LTE, multi-tenant SaaS from India.',
    images: [`${baseUrl}/assets/images/app_logo.png`],
  },
};

export default function LandingPage() {
  return (
    <main className="relative overflow-x-hidden bg-background">
      {/* Structured Data - WebPage Schema */}
      <script
        type="application/ld+json"
        dangerouslySetInnerHTML={{
          __html: JSON.stringify({
            '@context': 'https://schema.org',
            '@type': 'WebPage',
            name: 'Autoconnecto — Enterprise IoT Platform',
            description:
              'Autoconnecto is a full-stack IoT platform with Machine Fleet and Generator Monitoring widgets, live dashboards, OTA firmware, edge gateways, MQTT/HTTPS/LTE, pipelines, RBAC, alarms, and white-label capability.',
            url: `${baseUrl}/`,
            image: `${baseUrl}/assets/images/app_logo.png`,
            isPartOf: {
              '@type': 'WebSite',
              name: 'Autoconnecto',
              url: baseUrl,
            },
          }),
        }}
      />

      {/* Structured Data - SoftwareApplication Schema */}
      <script
        type="application/ld+json"
        dangerouslySetInnerHTML={{
          __html: JSON.stringify({
            '@context': 'https://schema.org',
            '@type': 'SoftwareApplication',
            name: 'Autoconnecto',
            description: 'Enterprise IoT Platform for device management and visualization',
            url: baseUrl,
            applicationCategory: 'BusinessApplication',
            operatingSystem: 'Cloud',
            offers: {
              '@type': 'Offer',
              price: '0',
              priceCurrency: 'INR',
              url: `${baseUrl}/pricing`,
            },
            author: {
              '@type': 'Organization',
              name: 'Autoconnecto',
              url: baseUrl,
            },
          }),
        }}
      />

      {/* Background atmosphere */}
      <div className="fixed inset-0 -z-10 pointer-events-none">
        <div className="absolute inset-0 bg-gradient-to-br from-[#0B1120] via-[#0D1829] to-[#0B1120]" />
        <div className="absolute top-0 left-1/4 w-[600px] h-[600px] rounded-full blur-[120px] opacity-20"
          style={{ background: 'radial-gradient(circle, #0EA5E9 0%, transparent 70%)' }} />
        <div className="absolute top-1/3 right-0 w-[400px] h-[400px] rounded-full blur-[100px] opacity-10"
          style={{ background: 'radial-gradient(circle, #06B6D4 0%, transparent 70%)' }} />
        <div className="noise-overlay absolute inset-0 opacity-30" />
      </div>

      <Header />
      <HeroSection />
      <DashboardSection />
      <FeaturesSection />
      <SolutionsSection />
      <ScaleSection />
      <DevelopersSection />
      <TestimonialsSection />
      <ContactSection />
      <Footer />
    </main>
  );
}