'use client';

import React, { useEffect, useRef } from 'react';
import Icon from '@/components/ui/AppIcon';

/*
BENTO GRID AUDIT:
Array has 8 cards: [ProtocolCard, DashboardCard, WidgetsCard, RBACCard, AlarmsCard, HelpWizardCard, WhiteLabelCard, MultiTenantCard]

Desktop (grid-cols-3):
Row 1: [col-1-2: ProtocolCard cs-2 rs-1] [col-3: DashboardCard cs-1 rs-2]
Row 2: [col-1: WidgetsCard cs-1 rs-1] [col-2: RBACCard cs-1 rs-1] [col-3: (filled by DashboardCard)]
Row 3: [col-1: AlarmsCard cs-1 rs-1] [col-2: HelpWizardCard cs-1 rs-1] [col-3: WhiteLabelCard cs-1 rs-1]
Row 4: [col-1-3: MultiTenantCard cs-3 rs-1]

Placed 8/8 cards ✓
*/

const ALARM_EVENTS = [
  { time: '14:32:01', device: 'Sensor-0042', type: 'High Temp', severity: 'critical' },
  { time: '14:31:44', device: 'Gateway-007', type: 'Offline',   severity: 'warning' },
  { time: '14:30:18', device: 'Meter-1191',  type: 'Low Volt',  severity: 'minor' },
];

const RBAC_ROLES = [
  { role: 'Tenant Admin', perms: ['Read', 'Write', 'Delete', 'Manage'] },
  { role: 'Operator',     perms: ['Read', 'Write'] },
  { role: 'Viewer',       perms: ['Read'] },
];

export default function FeaturesSection() {
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
    if (sectionRef?.current) observer?.observe(sectionRef?.current);
    return () => observer?.disconnect();
  }, []);

  return (
    <section id="features" ref={sectionRef} className="py-16 relative">
      <div className="max-w-7xl mx-auto px-4 sm:px-6">
        {/* Section label */}
        <div className="scroll-reveal hidden-init flex justify-center mb-6">
          <span className="inline-flex items-center gap-2 rounded-full border border-primary/20 bg-primary/10 px-4 py-1.5 text-xs font-semibold uppercase tracking-widest text-primary">
            <Icon name="CpuChipIcon" size={12} />
            Platform Capabilities
          </span>
        </div>

        {/* Heading */}
        <div className="scroll-reveal hidden-init scroll-reveal-delay-1 text-center mb-12 max-w-2xl mx-auto">
          <h2 className="font-display font-bold text-4xl sm:text-5xl tracking-tight text-foreground mb-4">
            Everything Your IoT Product{' '}
            <span className="text-gradient-primary">Needs to Ship</span>
          </h2>
          <p className="text-muted-foreground text-lg font-light">
            A complete platform — not a collection of disconnected tools.
          </p>
        </div>

        {/* Bento grid */}
        <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">

          {/* Card 1: Protocol — col-span-2 */}
          {/* BENTO: Row 1, col-1-2, cs-2 */}
          <div className="scroll-reveal hidden-init lg:col-span-2 relative overflow-hidden rounded-2xl border border-border bg-card card-glow card-glow-hover transition-all duration-300 p-6 min-h-[220px] flex flex-col justify-between">
            <div className="absolute inset-0 opacity-20 pointer-events-none"
              style={{ background: 'radial-gradient(ellipse at 80% 50%, rgba(14,165,233,0.2), transparent 60%)' }} />
            <div>
              <div className="flex items-center gap-3 mb-3">
                <div className="w-10 h-10 rounded-xl bg-primary/15 border border-primary/20 flex items-center justify-center">
                  <Icon name="WifiIcon" size={20} className="text-primary" />
                </div>
                <div>
                  <h3 className="font-semibold text-lg text-foreground">Universal device connectivity</h3>
                  <p className="text-xs text-muted-foreground">HTTP, MQTT, WebSocket, gateways, LoRa webhooks</p>
                </div>
              </div>
              <p className="text-muted-foreground text-sm leading-relaxed max-w-lg">
                Connect ESP32 firmware with the Arduino SDK, Raspberry Pi gateways, STM32 (Arduino core), LTE (Quectel EC200), any device over HTTPS or MQTT, edge gateways with child relay, Modbus/OPC-UA bridges, and LoRaWAN via ChirpStack or TTN webhooks.
              </p>
            </div>
            {/* Protocol badges */}
            <div className="flex gap-2 mt-4 flex-wrap">
              {['HTTPS / REST', 'MQTT (TLS)', 'WebSocket', 'Gateway relay', 'Modbus / OPC-UA', 'LoRa webhooks', 'LTE (EC200)', 'OTA firmware']?.map((p) => (
                <span key={p} className="px-3 py-1 rounded-full border border-primary/20 bg-primary/10 text-xs font-medium text-primary">
                  {p}
                </span>
              ))}
            </div>
          </div>

          {/* Card 2: Dashboard — col-span-1, row-span-2 */}
          {/* BENTO: Row 1-2, col-3, cs-1 rs-2 */}
          <div className="scroll-reveal hidden-init scroll-reveal-delay-1 relative overflow-hidden rounded-2xl border border-border bg-card card-glow card-glow-hover transition-all duration-300 p-6 flex flex-col h-full min-h-[460px]">
            <div className="flex items-center gap-2 mb-3">
              <div className="w-8 h-8 rounded-lg bg-accent/15 border border-accent/20 flex items-center justify-center">
                <Icon name="PresentationChartLineIcon" size={16} className="text-accent" />
              </div>
              <h3 className="font-semibold text-base text-foreground">Live Dashboards</h3>
            </div>
            <p className="text-muted-foreground text-sm leading-relaxed mb-4">
              Drag-and-drop dashboard builder with real-time data streaming. Share with customers or embed in your app.
            </p>
            {/* Mini dashboard preview */}
            <div className="flex-1 bg-secondary/40 rounded-xl border border-border p-3 flex flex-col gap-2">
              <div className="flex items-center justify-between mb-1">
                <span className="text-xs text-muted-foreground">Humidity Sensor</span>
                <span className="flex items-center gap-1 text-xs text-emerald-400">
                  <span className="w-1 h-1 rounded-full bg-emerald-400 animate-pulse" /> Live
                </span>
              </div>
              {/* Tiny sparkline */}
              <svg className="w-full" height="40" viewBox="0 0 120 40">
                <polyline points="0,30 15,22 30,28 45,15 60,20 75,10 90,16 105,8 120,12"
                  fill="none" stroke="#0EA5E9" strokeWidth="2" strokeLinecap="round" />
              </svg>
              <div className="grid grid-cols-2 gap-2 mt-1">
                <div className="bg-secondary/50 rounded-lg p-2 text-center">
                  <div className="font-bold text-base text-foreground">68%</div>
                  <div className="text-xs text-muted-foreground">Humidity</div>
                </div>
                <div className="bg-secondary/50 rounded-lg p-2 text-center">
                  <div className="font-bold text-base text-foreground">23°C</div>
                  <div className="text-xs text-muted-foreground">Temp</div>
                </div>
              </div>
            </div>
          </div>

          {/* Card 3: 60+ Widgets */}
          {/* BENTO: Row 2, col-1 */}
          <div className="scroll-reveal hidden-init scroll-reveal-delay-1 relative overflow-hidden rounded-2xl border border-border bg-card card-glow card-glow-hover transition-all duration-300 p-6">
            <div className="w-8 h-8 rounded-lg bg-violet-500/15 border border-violet-500/20 flex items-center justify-center mb-3">
              <Icon name="Squares2X2Icon" size={16} className="text-violet-400" />
            </div>
            <h3 className="font-semibold text-base text-foreground mb-1">60+ Widget Library</h3>
            <p className="text-muted-foreground text-sm leading-relaxed mb-3">
              Charts, gauges, maps, tables, alarms — plus vertical widgets like Machine Fleet Runtime and Generator Monitoring with SDK contracts and hardware playbooks.
            </p>
            <div className="grid grid-cols-4 gap-1.5">
              {['📈','🗺️','⏱️','🔢','📊','🌡️','💧','⚡']?.map((emoji, i) => (
                <div key={i} className="aspect-square rounded-lg bg-secondary/50 border border-border flex items-center justify-center text-lg">
                  {emoji}
                </div>
              ))}
            </div>
          </div>

          {/* Card 4: RBAC */}
          {/* BENTO: Row 2, col-2 */}
          <div className="scroll-reveal hidden-init scroll-reveal-delay-2 relative overflow-hidden rounded-2xl border border-border bg-card card-glow card-glow-hover transition-all duration-300 p-6">
            <div className="w-8 h-8 rounded-lg bg-emerald-500/15 border border-emerald-500/20 flex items-center justify-center mb-3">
              <Icon name="LockClosedIcon" size={16} className="text-emerald-400" />
            </div>
            <h3 className="font-semibold text-base text-foreground mb-1">Role-Based Access Control</h3>
            <p className="text-muted-foreground text-sm leading-relaxed mb-3">
              Granular permissions per user, role, and resource. Viewer to Admin — you control every operation.
            </p>
            <div className="space-y-1.5">
              {RBAC_ROLES?.map((r, i) => (
                <div key={i} className="flex items-center justify-between bg-secondary/40 rounded-lg px-3 py-1.5">
                  <span className="text-xs font-medium text-foreground">{r?.role}</span>
                  <div className="flex gap-1">
                    {['Read', 'Write', 'Delete', 'Manage']?.map((p) => (
                      <span key={p}
                        className={`text-[10px] px-1.5 py-0.5 rounded ${r?.perms?.includes(p) ? 'bg-emerald-500/20 text-emerald-400' : 'bg-secondary/60 text-muted-foreground/40'}`}>
                        {p?.[0]}
                      </span>
                    ))}
                  </div>
                </div>
              ))}
            </div>
          </div>

          {/* Card 5: Alarms Engine */}
          {/* BENTO: Row 3, col-1 */}
          <div className="scroll-reveal hidden-init scroll-reveal-delay-1 relative overflow-hidden rounded-2xl border border-border bg-card card-glow card-glow-hover transition-all duration-300 p-6">
            <div className="w-8 h-8 rounded-lg bg-orange-500/15 border border-orange-500/20 flex items-center justify-center mb-3">
              <Icon name="BellAlertIcon" size={16} className="text-orange-400" />
            </div>
            <h3 className="font-semibold text-base text-foreground mb-1">Full-Scale Alarms Engine</h3>
            <p className="text-muted-foreground text-sm leading-relaxed mb-3">
              Rule-chain driven alarms on telemetry, inactivity, or user actions. Route to email, Slack, or webhooks.
            </p>
            <div className="space-y-1.5">
              {ALARM_EVENTS?.map((ev, i) => (
                <div key={i} className="flex items-center gap-2 bg-secondary/40 rounded-lg px-2.5 py-1.5">
                  <span className={`w-1.5 h-1.5 rounded-full flex-shrink-0 ${
                    ev?.severity === 'critical' ? 'bg-red-400' :
                    ev?.severity === 'warning'  ? 'bg-orange-400' : 'bg-yellow-400'
                  }`} />
                  <span className="text-xs text-muted-foreground font-mono">{ev?.time}</span>
                  <span className="text-xs text-foreground truncate">{ev?.device}</span>
                  <span className="text-xs text-muted-foreground ml-auto">{ev?.type}</span>
                </div>
              ))}
            </div>
          </div>

          {/* Card 6: Help Wizard */}
          {/* BENTO: Row 3, col-2 */}
          <div className="scroll-reveal hidden-init scroll-reveal-delay-2 relative overflow-hidden rounded-2xl border border-border bg-card card-glow card-glow-hover transition-all duration-300 p-6">
            <div className="w-8 h-8 rounded-lg bg-sky-500/15 border border-sky-500/20 flex items-center justify-center mb-3">
              <Icon name="QuestionMarkCircleIcon" size={16} className="text-sky-400" />
            </div>
            <h3 className="font-semibold text-base text-foreground mb-1">Guided Help Wizard</h3>
            <p className="text-muted-foreground text-sm leading-relaxed mb-4">
              Step-by-step onboarding wizard gets teams productive in under an hour — no professional services required.
            </p>
            <div className="space-y-2">
              {['Connect your first device', 'Create a dashboard', 'Set up RBAC roles', 'Configure alarms']?.map((step, i) => (
                <div key={i} className="flex items-center gap-2.5">
                  <div className={`w-5 h-5 rounded-full flex items-center justify-center text-xs font-bold flex-shrink-0 ${
                    i < 2 ? 'bg-primary text-primary-foreground' : 'bg-secondary/60 text-muted-foreground border border-border'
                  }`}>
                    {i < 2 ? '✓' : i + 1}
                  </div>
                  <span className={`text-xs ${i < 2 ? 'text-foreground' : 'text-muted-foreground'}`}>{step}</span>
                </div>
              ))}
            </div>
          </div>

          {/* Card 7: White-Label */}
          {/* BENTO: Row 3, col-3 */}
          <div className="scroll-reveal hidden-init scroll-reveal-delay-3 relative overflow-hidden rounded-2xl border border-border bg-card card-glow card-glow-hover transition-all duration-300 p-6">
            <div
              className="absolute inset-0 opacity-10 pointer-events-none"
              style={{ background: 'radial-gradient(circle at 30% 70%, #0EA5E9, transparent 60%)' }}
            />
            <div className="w-8 h-8 rounded-lg bg-primary/15 border border-primary/20 flex items-center justify-center mb-3">
              <Icon name="SwatchIcon" size={16} className="text-primary" />
            </div>
            <h3 className="font-semibold text-base text-foreground mb-1">White-Label Ready</h3>
            <p className="text-muted-foreground text-sm leading-relaxed mb-3">
              Your brand, your domain, your colors. Deliver a fully branded IoT platform to each client.
            </p>
            <div className="flex gap-2">
              {['#0EA5E9', '#8B5CF6', '#10B981', '#F59E0B']?.map((color, i) => (
                <div key={i} className="w-7 h-7 rounded-full border-2 border-background ring-1 ring-border"
                  style={{ background: color }} />
              ))}
              <span className="text-xs text-muted-foreground self-center ml-1">Custom themes</span>
            </div>
          </div>

          {/* Card 8: Data & attribute pipelines — col-span-3 */}
          <div className="scroll-reveal hidden-init scroll-reveal-delay-2 lg:col-span-3 relative overflow-hidden rounded-2xl border border-border bg-card card-glow card-glow-hover transition-all duration-300 p-6 flex flex-col md:flex-row items-start md:items-center gap-6">
            <div
              className="absolute inset-0 opacity-15 pointer-events-none"
              style={{ background: 'linear-gradient(135deg, rgba(139,92,246,0.12) 0%, transparent 55%)' }}
            />
            <div className="flex-shrink-0 w-12 h-12 rounded-xl bg-violet-500/15 border border-violet-500/20 flex items-center justify-center">
              <Icon name="Squares2X2Icon" size={24} className="text-violet-400" />
            </div>
            <div className="flex-1">
              <h3 className="font-semibold text-lg text-foreground mb-1">Data &amp; attribute pipelines</h3>
              <p className="text-muted-foreground text-sm leading-relaxed max-w-2xl">
                Optional per device profile: transform telemetry and client attributes before storage — rename keys, filter bad readings, flatten nested JSON, compute fields, and bulk key scope changes. Blank profile binding means passthrough (store as received).{' '}
                <a href="/product#pipelines" className="text-violet-300 font-medium hover:underline">
                  See product overview →
                </a>
              </p>
              <div className="flex gap-2 mt-3 flex-wrap">
                {['Rename & scope', 'Filter / clamp', 'Scale & derive', 'Flatten JSON', 'Pick keys', 'Webhooks']?.map((p) => (
                  <span key={p} className="px-3 py-1 rounded-full border border-violet-500/20 bg-violet-500/10 text-xs font-medium text-violet-300">
                    {p}
                  </span>
                ))}
              </div>
            </div>
            <div className="flex-shrink-0 grid grid-cols-2 gap-3 text-center min-w-[200px]">
              {[
                { v: '2', l: 'Pipeline types' },
                { v: '14+', l: 'Step kinds' },
              ]?.map((s, i) => (
                <div key={i} className="bg-secondary/40 rounded-xl px-4 py-3">
                  <div className="font-display font-bold text-xl text-violet-400">{s?.v}</div>
                  <div className="text-xs text-muted-foreground mt-0.5">{s?.l}</div>
                </div>
              ))}
            </div>
          </div>

          {/* Card 9: Multi-Tenant — col-span-3 */}
          <div className="scroll-reveal hidden-init scroll-reveal-delay-2 lg:col-span-3 relative overflow-hidden rounded-2xl border border-border bg-card card-glow card-glow-hover transition-all duration-300 p-6 flex flex-col md:flex-row items-start md:items-center gap-6">
            <div
              className="absolute inset-0 opacity-15 pointer-events-none"
              style={{ background: 'linear-gradient(135deg, rgba(14,165,233,0.15) 0%, transparent 50%)' }}
            />
            <div className="flex-shrink-0 w-12 h-12 rounded-xl bg-primary/15 border border-primary/20 flex items-center justify-center">
              <Icon name="BuildingOffice2Icon" size={24} className="text-primary" />
            </div>
            <div className="flex-1">
              <h3 className="font-semibold text-lg text-foreground mb-1">Multi-Tenant &amp; Multi-User Architecture</h3>
              <p className="text-muted-foreground text-sm leading-relaxed max-w-2xl">
                A single installation serves unlimited tenants, each with their own administrators, customers, devices, and assets. One tenant — millions of devices. Full isolation, zero cross-tenant data leakage.
              </p>
            </div>
            <div className="flex-shrink-0 grid grid-cols-3 gap-3 text-center">
              {[
                { v: '∞', l: 'Tenants' },
                { v: 'M+', l: 'Devices/Tenant' },
                { v: '100%', l: 'Isolated' },
              ]?.map((s, i) => (
                <div key={i} className="bg-secondary/40 rounded-xl px-4 py-3">
                  <div className="font-display font-bold text-xl text-primary">{s?.v}</div>
                  <div className="text-xs text-muted-foreground mt-0.5">{s?.l}</div>
                </div>
              ))}
            </div>
          </div>

        </div>
      </div>
    </section>
  );
}