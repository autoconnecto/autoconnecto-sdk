'use client';

import React, { useEffect, useRef } from 'react';
import Icon from '@/components/ui/AppIcon';
import {
  ARDUINO_SDK_GITHUB_URL,
  DEVICE_CONNECTIVITY_DOCS_URL,
  MOBILE_APP_DOWNLOAD_URL,
} from '@/config/links';

const SOLUTIONS = [
  {
    icon: 'Cog6ToothIcon' as const,
    accent: 'primary',
    badge: 'Manufacturing',
    title: 'Machine Fleet Runtime',
    desc: 'Monitor factory machines from current draw — Off, idle, and on-load states, productivity charts, downtime culprits, and optional tool-life interlocks. Worker sessions via RFID, NFC, or BLE mobile app.',
    features: [
      '250+ machines per dashboard',
      'Scheduled email reports (Brevo)',
      'Tool life tracking & SSR block',
      'ESP32 + PZEM hardware BOM',
    ],
    sdkPath: 'examples/Machine_Runtime_mqtt',
  },
  {
    icon: 'BoltIcon' as const,
    accent: 'orange',
    badge: 'Facilities & energy',
    title: 'Generator Monitoring',
    desc: 'Live diesel generator dashboards — run state, kW/kVA, RPM, fuel level, alarms, and fault cycles. Pair with LTE (Quectel EC200) for remote sites without WiFi.',
    features: [
      'gen_* telemetry contract',
      'Fleet-ready widget',
      'MQTT or HTTPS ingest',
      'Raspberry Pi mirror example',
    ],
    sdkPath: 'examples/Generator_Monitoring_mqtt',
  },
  {
    icon: 'ServerStackIcon' as const,
    accent: 'violet',
    badge: 'Brownfield',
    title: 'Edge gateways & integrations',
    desc: 'Connect legacy PLCs and plant equipment without replacing hardware. Modbus TCP/RTU, OPC-UA, MQTT bridge, and LoRaWAN (ChirpStack / TTN) webhooks forward to Autoconnecto telemetry.',
    features: [
      'Modbus & OPC-UA gateways',
      'Gateway child-device relay',
      'Linux SBC (Raspberry Pi) examples',
      'Generic JSON webhooks',
    ],
    href: `${ARDUINO_SDK_GITHUB_URL}/tree/main/examples/integrations`,
  },
];

const PLATFORM_UPDATES = [
  {
    icon: 'ArrowPathIcon' as const,
    title: 'OTA firmware updates',
    desc: 'ThingsBoard-style OTA via shared attributes and chunked HTTPS download. ESP32 example sketch included.',
  },
  {
    icon: 'DevicePhoneMobileIcon' as const,
    title: 'Mobile worker app (BLE)',
    desc: 'Android companion for machine fleet Rev 3 — worker identity over BLE GATT without RFID pods.',
    href: MOBILE_APP_DOWNLOAD_URL,
  },
  {
    icon: 'SignalIcon' as const,
    title: 'LTE PPP (EC200)',
    desc: 'Cellular bring-up for remote assets — MQTTS over Quectel EC200 on ESP32 core 3.x.',
  },
  {
    icon: 'CommandLineIcon' as const,
    title: 'STM32 & Linux targets',
    desc: 'Arduino-core STM32 guide, Raspberry Pi Python examples mirroring ESP32, and systemd unit templates.',
  },
];

function accentClasses(accent: string) {
  const map: Record<string, { bg: string; border: string; text: string; badge: string }> = {
    primary: {
      bg: 'bg-primary/15',
      border: 'border-primary/20',
      text: 'text-primary',
      badge: 'bg-primary/10 border-primary/25 text-primary',
    },
    orange: {
      bg: 'bg-orange-500/15',
      border: 'border-orange-500/20',
      text: 'text-orange-400',
      badge: 'bg-orange-500/10 border-orange-500/25 text-orange-300',
    },
    violet: {
      bg: 'bg-violet-500/15',
      border: 'border-violet-500/20',
      text: 'text-violet-400',
      badge: 'bg-violet-500/10 border-violet-500/25 text-violet-300',
    },
  };
  return map[accent] || map.primary;
}

export default function SolutionsSection() {
  const sectionRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    const observer = new IntersectionObserver(
      (entries) => {
        entries.forEach((entry) => {
          if (entry.isIntersecting) {
            entry.target.querySelectorAll<HTMLElement>('.scroll-reveal').forEach((el) => {
              el.classList.remove('hidden-init');
            });
          }
        });
      },
      { rootMargin: '0px 0px -60px 0px', threshold: 0.05 }
    );
    if (sectionRef.current) observer.observe(sectionRef.current);
    return () => observer.disconnect();
  }, []);

  return (
    <section id="solutions" ref={sectionRef} className="py-16 relative scroll-mt-24">
      <div className="max-w-7xl mx-auto px-4 sm:px-6">
        <div className="scroll-reveal hidden-init flex justify-center mb-6">
          <span className="inline-flex items-center gap-2 rounded-full border border-emerald-500/25 bg-emerald-500/10 px-4 py-1.5 text-xs font-semibold uppercase tracking-widest text-emerald-400">
            <Icon name="RocketLaunchIcon" size={12} />
            What&apos;s new
          </span>
        </div>

        <div className="scroll-reveal hidden-init scroll-reveal-delay-1 text-center mb-12 max-w-3xl mx-auto">
          <h2 className="font-display font-bold text-4xl sm:text-5xl tracking-tight text-foreground mb-4">
            Vertical solutions{' '}
            <span className="text-gradient-primary">ready to deploy</span>
          </h2>
          <p className="text-muted-foreground text-lg font-light leading-relaxed">
            Beyond generic charts — first-class dashboard widgets, hardware playbooks, and edge
            gateways for manufacturing, facilities, and brownfield industrial sites.
          </p>
        </div>

        <div className="grid grid-cols-1 lg:grid-cols-3 gap-4 mb-10">
          {SOLUTIONS.map((solution, i) => {
            const c = accentClasses(solution.accent);
            const sdkUrl = solution.href
              ? solution.href
              : `${ARDUINO_SDK_GITHUB_URL}/tree/main/${solution.sdkPath}`;
            return (
              <div
                key={solution.title}
                className={`scroll-reveal hidden-init ${
                  i === 1 ? 'scroll-reveal-delay-1' : i === 2 ? 'scroll-reveal-delay-2' : ''
                } relative overflow-hidden rounded-2xl border border-border bg-card p-6 card-glow card-glow-hover transition-all duration-300 flex flex-col`}
              >
                <span
                  className={`inline-flex self-start items-center px-2.5 py-0.5 rounded-full border text-[10px] font-semibold uppercase tracking-wider mb-4 ${c.badge}`}
                >
                  {solution.badge}
                </span>
                <div
                  className={`w-10 h-10 rounded-xl ${c.bg} border ${c.border} flex items-center justify-center mb-3`}
                >
                  <Icon name={solution.icon} size={20} className={c.text} />
                </div>
                <h3 className="font-semibold text-lg text-foreground mb-2">{solution.title}</h3>
                <p className="text-sm text-muted-foreground leading-relaxed mb-4 flex-1">
                  {solution.desc}
                </p>
                <ul className="space-y-1.5 mb-5">
                  {solution.features.map((f) => (
                    <li key={f} className="flex items-center gap-2 text-xs text-muted-foreground">
                      <Icon name="CheckCircleIcon" size={14} className={`${c.text} shrink-0`} />
                      {f}
                    </li>
                  ))}
                </ul>
                <a
                  href={sdkUrl}
                  target="_blank"
                  rel="noopener noreferrer"
                  className={`inline-flex items-center gap-1.5 text-sm font-semibold ${c.text} hover:underline mt-auto`}
                >
                  View SDK example
                  <Icon name="ArrowTopRightOnSquareIcon" size={14} />
                </a>
              </div>
            );
          })}
        </div>

        <div className="scroll-reveal hidden-init scroll-reveal-delay-2 rounded-2xl border border-border bg-card/80 p-6 sm:p-8">
          <div className="flex flex-col sm:flex-row sm:items-center sm:justify-between gap-4 mb-6">
            <div>
              <h3 className="font-semibold text-lg text-foreground">Latest platform &amp; SDK</h3>
              <p className="text-sm text-muted-foreground mt-1">
                SDK v1.3.7 — OTA, LTE, batch telemetry, watchdog reconnect, and integration
                gateways.
              </p>
            </div>
            <a
              href={DEVICE_CONNECTIVITY_DOCS_URL}
              target="_blank"
              rel="noopener noreferrer"
              className="inline-flex items-center gap-2 text-sm font-semibold text-primary hover:underline shrink-0"
            >
              Device connectivity docs
              <Icon name="ArrowTopRightOnSquareIcon" size={14} />
            </a>
          </div>
          <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-4 gap-4">
            {PLATFORM_UPDATES.map((item) => {
              const inner = (
                <>
                  <div className="w-8 h-8 rounded-lg bg-secondary/60 border border-border flex items-center justify-center mb-2">
                    <Icon name={item.icon} size={16} className="text-primary" />
                  </div>
                  <div className="font-medium text-sm text-foreground mb-1">{item.title}</div>
                  <div className="text-xs text-muted-foreground leading-relaxed">{item.desc}</div>
                </>
              );
              if (item.href) {
                return (
                  <a
                    key={item.title}
                    href={item.href}
                    target="_blank"
                    rel="noopener noreferrer"
                    className="rounded-xl border border-border bg-secondary/30 p-4 hover:border-primary/30 transition-colors"
                  >
                    {inner}
                  </a>
                );
              }
              return (
                <div
                  key={item.title}
                  className="rounded-xl border border-border bg-secondary/30 p-4"
                >
                  {inner}
                </div>
              );
            })}
          </div>
        </div>
      </div>
    </section>
  );
}
