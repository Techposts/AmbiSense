/* Faithful ports of the Sparkline, Icon, Toggle from the Claude Design
 * handoff (frontend/design-source/project/core.jsx). Kept in their own
 * file so they're easy to lift verbatim. */
import { JSX } from 'preact';
import { useState } from 'preact/hooks';

export const Icon = ({ name, size = 16, stroke = 1.6, style }: {
  name: string; size?: number; stroke?: number; style?: any;
}) => {
  const s = size;
  const common: any = {
    width: s, height: s, viewBox: '0 0 24 24',
    fill: 'none', stroke: 'currentColor',
    'stroke-width': stroke, 'stroke-linecap': 'round', 'stroke-linejoin': 'round',
    style,
  };
  const paths: Record<string, JSX.Element> = {
    dashboard: <><rect x="3" y="3" width="7" height="9" rx="1.5"/><rect x="14" y="3" width="7" height="5" rx="1.5"/><rect x="14" y="12" width="7" height="9" rx="1.5"/><rect x="3" y="16" width="7" height="5" rx="1.5"/></>,
    led: <><path d="M12 3v3M12 18v3M5 12H2M22 12h-3M5.6 5.6l2.1 2.1M16.3 16.3l2.1 2.1M5.6 18.4l2.1-2.1M16.3 7.7l2.1-2.1"/><circle cx="12" cy="12" r="4"/></>,
    motion: <><path d="M3 12h3l3-7 4 14 3-7h5"/></>,
    mesh: <><circle cx="12" cy="5" r="2"/><circle cx="5" cy="19" r="2"/><circle cx="19" cy="19" r="2"/><path d="M12 7v10M10.5 6.5l-5 11M13.5 6.5l5 11"/></>,
    chip: <><rect x="6" y="6" width="12" height="12" rx="1.5"/><path d="M9 1v3M12 1v3M15 1v3M9 20v3M12 20v3M15 20v3M1 9h3M1 12h3M1 15h3M20 9h3M20 12h3M20 15h3"/></>,
    wifi: <><path d="M2 8.5C5 6 8.5 4.5 12 4.5s7 1.5 10 4M5 12c2-1.7 4.5-2.5 7-2.5s5 .8 7 2.5M8.5 15.5c1-.8 2.2-1.2 3.5-1.2s2.5.4 3.5 1.2"/><circle cx="12" cy="19" r="1" fill="currentColor"/></>,
    settings: <><circle cx="12" cy="12" r="3"/><path d="M19.4 15a1.7 1.7 0 0 0 .3 1.8a2 2 0 1 1-2.8 2.8a1.7 1.7 0 0 0-1.8-.3 1.7 1.7 0 0 0-1 1.5V21a2 2 0 1 1-4 0a1.7 1.7 0 0 0-1-1.5 1.7 1.7 0 0 0-1.8.3a2 2 0 1 1-2.8-2.8a1.7 1.7 0 0 0 .3-1.8 1.7 1.7 0 0 0-1.5-1H3a2 2 0 1 1 0-4a1.7 1.7 0 0 0 1.5-1 1.7 1.7 0 0 0-.3-1.8a2 2 0 1 1 2.8-2.8a1.7 1.7 0 0 0 1.8.3a1.7 1.7 0 0 0 1-1.5V3a2 2 0 1 1 4 0a1.7 1.7 0 0 0 1 1.5 1.7 1.7 0 0 0 1.8-.3a2 2 0 1 1 2.8 2.8a1.7 1.7 0 0 0-.3 1.8a1.7 1.7 0 0 0 1.5 1H21a2 2 0 1 1 0 4z"/></>,
    sun: <><circle cx="12" cy="12" r="4"/><path d="M12 2v2M12 20v2M4.93 4.93l1.41 1.41M17.66 17.66l1.41 1.41M2 12h2M20 12h2M4.93 19.07l1.41-1.41M17.66 6.34l1.41-1.41"/></>,
    moon: <><path d="M21 12.79A9 9 0 1 1 11.21 3 7 7 0 0 0 21 12.79z"/></>,
    check: <><path d="M5 12l5 5L20 7"/></>,
    x: <><path d="M6 6l12 12M6 18L18 6"/></>,
    info: <><circle cx="12" cy="12" r="9"/><path d="M12 8h.01M11 12h1v4h1"/></>,
    warn: <><path d="M12 3 2 21h20L12 3zM12 10v5M12 18h.01"/></>,
    refresh: <><path d="M3 12a9 9 0 1 0 3-6.7L3 8"/><path d="M3 3v5h5"/></>,
    upload: <><path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4M17 8l-5-5-5 5M12 3v12"/></>,
    bolt: <><path d="M13 2L3 14h7l-1 8 10-12h-7z"/></>,
    cpu: <><rect x="4" y="4" width="16" height="16" rx="2"/><rect x="9" y="9" width="6" height="6"/><path d="M9 1v3M15 1v3M9 20v3M15 20v3M20 9h3M20 14h3M1 9h3M1 14h3"/></>,
    radar: <><circle cx="12" cy="12" r="9"/><path d="M12 12L19 7"/><path d="M12 12a4 4 0 1 1-4 4"/></>,
    palette: <><circle cx="13.5" cy="6.5" r="1.5" fill="currentColor"/><circle cx="17.5" cy="10.5" r="1.5" fill="currentColor"/><circle cx="8.5" cy="7.5" r="1.5" fill="currentColor"/><circle cx="6.5" cy="12.5" r="1.5" fill="currentColor"/><path d="M12 22a10 10 0 1 1 10-10c0 2.76-2.24 4-5 4h-2a2 2 0 0 0-1 3.74A2 2 0 0 1 12 22z"/></>,
    play: <><path d="M5 3l14 9-14 9V3z" fill="currentColor"/></>,
    pause: <><rect x="6" y="4" width="4" height="16" fill="currentColor"/><rect x="14" y="4" width="4" height="16" fill="currentColor"/></>,
  };
  return <svg {...common}>{paths[name] || null}</svg>;
};

/* Sparkline — exact port from design source/core.jsx */
export function Sparkline({ data, width = 200, height = 40, color = 'var(--acc-orange)', fill = true, min, max }: {
  data: number[]; width?: number; height?: number; color?: string; fill?: boolean; min?: number; max?: number;
}) {
  if (!data || data.length === 0) return <svg width={width} height={height} />;
  const padded = data.length > 1 ? data : [...data, ...data];
  const lo = min !== undefined ? min : Math.min(...padded);
  const hi = max !== undefined ? max : Math.max(...padded);
  const range = hi - lo || 1;
  const pts = padded.map((v, i) => {
    const x = (i / (padded.length - 1)) * width;
    const y = height - ((v - lo) / range) * height;
    return [x, y];
  });
  const pathD = pts.map((p, i) => `${i === 0 ? 'M' : 'L'}${p[0].toFixed(1)},${p[1].toFixed(1)}`).join(' ');
  const fillD = pathD + ` L${width},${height} L0,${height} Z`;
  const id = 'spark-' + Math.abs(color.split('').reduce((a, c) => a * 31 + c.charCodeAt(0), 0));
  return (
    <svg width={width} height={height} style="display: block; overflow: visible;">
      {fill && (
        <>
          <defs>
            <linearGradient id={id} x1="0" x2="0" y1="0" y2="1">
              <stop offset="0%" stop-color={color} stop-opacity="0.35"/>
              <stop offset="100%" stop-color={color} stop-opacity="0"/>
            </linearGradient>
          </defs>
          <path d={fillD} fill={`url(#${id})`}/>
        </>
      )}
      <path d={pathD} stroke={color} stroke-width="1.5" fill="none" stroke-linecap="round" stroke-linejoin="round"/>
    </svg>
  );
}

export function fmtUptime(s: number): string {
  if (!s) return '—';
  const d = Math.floor(s / 86400);
  const h = Math.floor((s % 86400) / 3600);
  const m = Math.floor((s % 3600) / 60);
  const sec = s % 60;
  if (d) return `${d}d ${h}h ${m}m`;
  if (h) return `${h}h ${m}m ${sec}s`;
  return `${m}m ${sec}s`;
}

/* Generic ring buffer hook for client-side history charts. Avoids growing
 * an array unbounded — caps at `size` samples, oldest pushed out. */
export function useRing(size: number, seed: number = 0) {
  const [buf, setBuf] = useState<number[]>(() => Array(size).fill(seed));
  const push = (v: number) => setBuf(b => [...b.slice(1), v]);
  return [buf, push] as const;
}
