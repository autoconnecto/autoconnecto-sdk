'use client';

import React from 'react';
import Icon from '@/components/ui/AppIcon';
import { ARDUINO_SDK_GITHUB_URL } from '@/config/links';

const VERTICALS = [
  {
    widget: 'machineFleetRuntime',
    title: 'Machine Fleet Runtime',
    industry: 'Discrete manufacturing',
    summary:
      'Classify machines as Off, idle, or on load from `machine_current_a`. Productivity charts, worst culprits, operator sessions (RFID / NFC / BLE), tool-life interlocks, and scheduled factory email reports.',
    hardware: 'ESP32 + PZEM-004T + Fotek SSR (~₹2,800/machine)',
    docs: `${ARDUINO_SDK_GITHUB_URL}/blob/main/MACHINE_RUNTIME.md`,
  },
  {
    widget: 'generatorMonitoring',
    title: 'Generator Monitoring',
    industry: 'Facilities & DG AMC',
    summary:
      'Diesel generator widget with `gen_*` telemetry — run state, kW, kVA, RPM, fuel level, and alarm counts. MQTT sketch and Raspberry Pi mirror for fleet rollouts.',
    hardware: 'ESP32 + Modbus RS485 or LTE (EC200)',
    docs: `${ARDUINO_SDK_GITHUB_URL}/tree/main/examples/Generator_Monitoring_mqtt`,
  },
  {
    widget: 'edgeGateway',
    title: 'Edge gateways',
    industry: 'Brownfield industrial',
    summary:
      'Modbus TCP/RTU, OPC-UA, and MQTT bridge scripts publish plant tags to device-token telemetry. Gateway child-device relay for multi-node hubs.',
    hardware: 'Raspberry Pi 4 + RS485 HAT',
    docs: `${ARDUINO_SDK_GITHUB_URL}/tree/main/examples/integrations`,
  },
];

export default function ProductSolutionsSection() {
  return (
    <section id="solutions" className="px-4 sm:px-6 mb-24 scroll-mt-28">
      <div className="max-w-6xl mx-auto">
        <div className="text-center mb-12 max-w-2xl mx-auto">
          <span className="inline-flex items-center gap-2 rounded-full border border-emerald-500/25 bg-emerald-500/10 px-3 py-1 text-xs font-semibold uppercase tracking-widest text-emerald-400 mb-4">
            Vertical widgets
          </span>
          <h2 className="font-display font-bold text-3xl sm:text-4xl text-foreground mb-3">
            Ship solutions, not just dashboards
          </h2>
          <p className="text-muted-foreground">
            First-class widgets with telemetry contracts, SDK examples, and hardware BOMs — so
            integrators go from pilot to production without rebuilding the UI.
          </p>
        </div>

        <div className="space-y-4">
          {VERTICALS.map((v) => (
            <div
              key={v.widget}
              className="rounded-2xl border border-border bg-card p-6 sm:p-8 card-glow flex flex-col lg:flex-row gap-6 lg:items-start"
            >
              <div className="flex-1">
                <div className="flex flex-wrap items-center gap-2 mb-2">
                  <h3 className="font-semibold text-xl text-foreground">{v.title}</h3>
                  <span className="text-[10px] font-mono px-2 py-0.5 rounded border border-border bg-secondary/50 text-muted-foreground">
                    {v.widget}
                  </span>
                </div>
                <p className="text-xs text-primary/80 font-medium mb-3">{v.industry}</p>
                <p className="text-sm text-muted-foreground leading-relaxed mb-3">{v.summary}</p>
                <p className="text-xs text-muted-foreground">
                  <span className="text-foreground font-medium">Hardware: </span>
                  {v.hardware}
                </p>
              </div>
              <a
                href={v.docs}
                target="_blank"
                rel="noopener noreferrer"
                className="inline-flex items-center gap-2 px-5 py-2.5 rounded-full border border-primary/30 bg-primary/10 text-primary text-sm font-semibold hover:bg-primary/20 transition-colors shrink-0 self-start"
              >
                SDK &amp; contract
                <Icon name="ArrowTopRightOnSquareIcon" size={14} />
              </a>
            </div>
          ))}
        </div>
      </div>
    </section>
  );
}
