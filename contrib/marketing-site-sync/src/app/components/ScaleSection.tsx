'use client';

import React, { useEffect, useRef } from 'react';
import AppImage from '@/components/ui/AppImage';
import Icon from '@/components/ui/AppIcon';

const METRICS = [
{ value: '10M+', label: 'Devices per Deployment', icon: 'CpuChipIcon' },
{ value: '99.9%', label: 'Uptime SLA', icon: 'CheckBadgeIcon' },
{ value: '< 50ms', label: 'Avg. Message Latency', icon: 'BoltIcon' },
{ value: 'Cloud &\nOn-Prem', label: 'Deployment Modes', icon: 'ServerStackIcon' }];


const TESTIMONIAL = {
  quote:
  "We evaluated ThingsBoard, AWS IoT, and Autoconnecto. Autoconnecto was the only platform that gave us white-label dashboards, RBAC, and a working multi-tenant setup in under 48 hours. Our clients think it's our own product.",
  author: 'Marcus Hoffmann',
  role: 'CTO, Nexus Industrial Systems',
  avatar: "https://img.rocket.new/generatedImages/rocket_gen_img_17c6ec630-1763296514678.png",
  avatarAlt: 'Professional man in his 40s with short hair, neutral background, corporate headshot'
};

const INDUSTRIES = [
'Machine OEE & uptime', 'Diesel generator (DG) AMC', 'Smart Energy',
'Brownfield PLC bridge', 'Smart Agriculture', 'Cold chain & logistics'];


export default function ScaleSection() {
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
    <section id="scale" ref={sectionRef} className="py-16 relative">
      <div className="max-w-7xl mx-auto px-4 sm:px-6">
        {/* Section label */}
        <div className="scroll-reveal hidden-init flex justify-center mb-6">
          <span className="inline-flex items-center gap-2 rounded-full border border-primary/20 bg-primary/10 px-4 py-1.5 text-xs font-semibold uppercase tracking-widest text-primary">
            <Icon name="ServerStackIcon" size={12} />
            Built for Enterprise Scale
          </span>
        </div>

        {/* Heading */}
        <div className="scroll-reveal hidden-init scroll-reveal-delay-1 text-center mb-14 max-w-2xl mx-auto">
          <h2 className="font-display font-bold text-4xl sm:text-5xl tracking-tight text-foreground mb-4">
            From Prototype to{' '}
            <span className="text-gradient-primary">Production at Scale</span>
          </h2>
          <p className="text-muted-foreground text-lg font-light">
            Cloud or on-premises. Horizontal scaling. Fault-tolerant by design.
          </p>
        </div>

        {/* Asymmetric 60/40 split */}
        <div className="grid grid-cols-1 lg:grid-cols-5 gap-6 mb-12">
          {/* Left: metrics (3 cols) */}
          <div className="lg:col-span-3 flex flex-col gap-4">
            {/* Metrics grid */}
            <div className="scroll-reveal hidden-init grid grid-cols-2 gap-4">
              {METRICS.map((m, i) =>
              <div key={i} className="bg-card border border-border rounded-2xl p-5 card-glow flex flex-col gap-2">
                  <div className="w-9 h-9 rounded-xl bg-primary/15 border border-primary/20 flex items-center justify-center">
                    <Icon name={m.icon as 'CpuChipIcon'} size={18} className="text-primary" />
                  </div>
                  <div className="font-display font-bold text-2xl sm:text-3xl text-foreground whitespace-pre-line">
                    {m.value}
                  </div>
                  <div className="text-xs text-muted-foreground">{m.label}</div>
                </div>
              )}
            </div>

            {/* Industry tags */}
            <div className="scroll-reveal hidden-init scroll-reveal-delay-2 bg-card border border-border rounded-2xl p-5 card-glow">
              <p className="text-sm font-medium text-foreground mb-3">Proven across industries</p>
              <div className="flex flex-wrap gap-2">
                {INDUSTRIES.map((ind) =>
                <span key={ind}
                className="px-3 py-1 rounded-full border border-border bg-secondary/40 text-xs font-medium text-muted-foreground hover:border-primary/30 hover:text-foreground transition-colors cursor-default">
                    {ind}
                  </span>
                )}
              </div>
            </div>
          </div>

          {/* Right: testimonial (2 cols) */}
          <div className="lg:col-span-2 flex flex-col gap-4">
            {/* Stacked testimonial */}
            <div className="scroll-reveal hidden-init scroll-reveal-delay-2 relative flex-1">
              {/* Back layers */}
              <div className="absolute inset-x-4 -top-3 h-full bg-secondary/30 border border-border rounded-2xl" />
              <div className="absolute inset-x-2 -top-1.5 h-full bg-secondary/50 border border-border rounded-2xl" />
              {/* Front */}
              <div className="relative z-10 bg-card border border-border rounded-2xl p-6 card-glow h-full flex flex-col justify-between">
                <div>
                  <svg width="32" height="32" viewBox="0 0 24 24" fill="currentColor" className="text-primary/30 mb-4">
                    <path d="M14.017 21v-3c0-1.105.895-2 2-2h3c.552 0 1-.448 1-1V9c0-.552-.448-1-1-1h-4c-.552 0-1 .448-1 1v2c0 .552-.448 1-1 1h-1V5h10v10c0 3.314-2.686 6-6 6h-2zm-9 0v-3c0-1.105.895-2 2-2h3c.552 0 1-.448 1-1V9c0-.552-.448-1-1-1H6c-.552 0-1 .448-1 1v2c0 .552-.448 1-1 1H3V5h10v10c0 3.314-2.686 6-6 6H5z" />
                  </svg>
                  <p className="text-foreground text-sm leading-relaxed font-light italic mb-6">
                    &ldquo;{TESTIMONIAL.quote}&rdquo;
                  </p>
                </div>
                <div className="flex items-center gap-3 pt-4 border-t border-border">
                  <div className="w-10 h-10 rounded-full overflow-hidden flex-shrink-0">
                    <AppImage
                      src={TESTIMONIAL.avatar}
                      alt={TESTIMONIAL.avatarAlt}
                      width={40}
                      height={40}
                      className="object-cover w-full h-full" />
                    
                  </div>
                  <div>
                    <div className="text-sm font-semibold text-foreground">{TESTIMONIAL.author}</div>
                    <div className="text-xs text-muted-foreground">{TESTIMONIAL.role}</div>
                  </div>
                </div>
              </div>
            </div>

            {/* Deployment modes card */}
            <div className="scroll-reveal hidden-init scroll-reveal-delay-3 bg-card border border-border rounded-2xl p-5 card-glow">
              <p className="text-sm font-medium text-foreground mb-3">Deployment flexibility</p>
              <div className="grid grid-cols-2 gap-2">
                {[
                { icon: 'CloudIcon', label: 'Cloud', desc: 'AWS, GCP, Azure' },
                { icon: 'ServerIcon', label: 'On-Premises', desc: 'Your infrastructure' }].
                map((d, i) =>
                <div key={i} className="bg-secondary/40 rounded-xl p-3 border border-border">
                    <Icon name={d.icon as 'CloudIcon'} size={16} className="text-primary mb-1.5" />
                    <div className="text-xs font-semibold text-foreground">{d.label}</div>
                    <div className="text-xs text-muted-foreground">{d.desc}</div>
                  </div>
                )}
              </div>
            </div>
          </div>
        </div>
      </div>
    </section>);

}