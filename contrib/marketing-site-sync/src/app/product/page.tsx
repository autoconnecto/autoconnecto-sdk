import type { Metadata } from 'next';
import Header from '@/components/Header';
import Footer from '@/components/Footer';
import ProductPageContent from '@/app/components/ProductPageContent';
import { siteOrigin } from '@/lib/siteOrigin';

const baseUrl = siteOrigin();

export const metadata: Metadata = {
  title: 'Product — Autoconnecto IoT Platform',
  description:
    'Multi-tenant IoT SaaS: Machine Fleet & Generator Monitoring widgets, device connectivity, OTA firmware, edge gateways, pipelines, live dashboards, alarms, reboot-safe control, and ESP32 SDK. Built in India.',
  alternates: { canonical: `${baseUrl}/product` },
  openGraph: {
    title: 'Product — Autoconnecto IoT Platform',
    description:
      'Connect devices, process telemetry and attributes with pipelines, visualize in realtime, and control equipment reliably.',
    url: `${baseUrl}/product`,
    images: [
      {
        url: '/assets/images/app_logo.png',
        width: 1200,
        height: 630,
        alt: 'Autoconnecto IoT Platform',
      },
    ],
  },
  twitter: {
    card: 'summary_large_image',
    title: 'Product — Autoconnecto IoT Platform',
    description:
      'Connect devices, process telemetry and attributes with pipelines, visualize in realtime, and control equipment reliably.',
    images: [`${baseUrl}/assets/images/app_logo.png`],
  },
};

export default function ProductPage() {
  return (
    <main className="relative min-h-screen overflow-x-hidden bg-background">
      <script
        type="application/ld+json"
        dangerouslySetInnerHTML={{
          __html: JSON.stringify({
            '@context': 'https://schema.org',
            '@type': 'WebPage',
            name: 'Autoconnecto Product Overview',
            description:
              'IoT platform with data pipelines, attribute pipelines, live dashboards, and reboot-safe device control.',
            url: `${baseUrl}/product`,
            isPartOf: { '@type': 'WebSite', name: 'Autoconnecto', url: baseUrl },
          }),
        }}
      />
      <div className="fixed inset-0 -z-10 pointer-events-none">
        <div className="absolute inset-0 bg-gradient-to-br from-[#0B1120] via-[#0D1829] to-[#0B1120]" />
        <div
          className="absolute top-0 right-1/4 w-[500px] h-[500px] rounded-full blur-[120px] opacity-15"
          style={{
            background: 'radial-gradient(circle, #0EA5E9 0%, transparent 70%)',
          }}
        />
        <div className="noise-overlay absolute inset-0 opacity-30" />
      </div>

      <Header />
      <ProductPageContent />
      <Footer />
    </main>
  );
}
