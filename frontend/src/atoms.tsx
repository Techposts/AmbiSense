/* Faithful ports of the Sparkline, Icon, Toggle from the Claude Design
 * handoff (frontend/design-source/project/core.jsx). Kept in their own
 * file so they're easy to lift verbatim. */
import { JSX } from 'preact';
import { useState, useEffect, useRef } from 'preact/hooks';

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

/* Number+Slider field used across LEDs and Motion screens. */
export function NumberAndSlider({ label, value, onChange, min, max, step = 1, suffix = '' }: {
  label: string; value: number; onChange: (v: number) => void; min: number; max: number; step?: number; suffix?: string;
}) {
  return (
    <div>
      <div style="display: flex; justify-content: space-between; margin-bottom: 6px;">
        <span class="field-label" style="margin-bottom: 0;">{label}</span>
        <input type="number" value={value} min={min} max={max} step={step}
          onChange={(e) => onChange(Math.max(min, Math.min(max, Number((e.target as HTMLInputElement).value))))}
          style="width: 80px; background: var(--bg-1); border: 1px solid var(--line); border-radius: 6px; padding: 2px 6px; font-family: var(--font-mono); font-size: 12px; color: var(--text-0); text-align: right; outline: none;"/>
      </div>
      <input type="range" class="range" value={value} min={min} max={max} step={step}
        onInput={(e) => onChange(Number((e.target as HTMLInputElement).value))}/>
      {suffix && <div style="font-size: 10px; color: var(--text-3); text-align: right; margin-top: 2px;">{suffix}</div>}
    </div>
  );
}

/* Dual-handle range slider for the distance window. */
export function DualHandleRange({ minVal, maxVal, onChange, min = 0, max = 500 }: {
  minVal: number; maxVal: number; onChange: (v: { minVal: number; maxVal: number }) => void; min?: number; max?: number;
}) {
  const ref = useRef<HTMLDivElement>(null);
  const [drag, setDrag] = useState<'min' | 'max' | null>(null);
  const handle = (e: PointerEvent) => {
    if (!drag || !ref.current) return;
    const rect = ref.current.getBoundingClientRect();
    const t = Math.max(0, Math.min(1, (e.clientX - rect.left) / rect.width));
    const v = Math.round(min + t * (max - min));
    if (drag === 'min') onChange({ minVal: Math.min(v, maxVal - 5), maxVal });
    else onChange({ minVal, maxVal: Math.max(v, minVal + 5) });
  };
  useEffect(() => {
    if (!drag) return;
    const up = () => setDrag(null);
    window.addEventListener('pointermove', handle);
    window.addEventListener('pointerup', up);
    return () => { window.removeEventListener('pointermove', handle); window.removeEventListener('pointerup', up); };
  }, [drag, minVal, maxVal]);
  const tMin = (minVal - min) / (max - min);
  const tMax = (maxVal - min) / (max - min);
  return (
    <div ref={ref} style="position: relative; height: 28px; padding: 12px 0; cursor: pointer; touch-action: none;">
      <div style="position: absolute; left: 0; right: 0; top: 50%; height: 4px; background: var(--bg-3); border-radius: 999px;"/>
      <div style={`position: absolute; top: 50%; height: 4px; transform: translateY(-2px); left: ${tMin*100}%; width: ${(tMax-tMin)*100}%; background: var(--acc-grad); border-radius: 999px;`}/>
      <div onPointerDown={() => setDrag('min')} style={`position: absolute; top: 50%; left: calc(${tMin*100}% - 9px); transform: translateY(-50%); width: 18px; height: 18px; border-radius: 50%; background: var(--text-0); border: 3px solid var(--acc-orange); cursor: grab; box-shadow: var(--shadow-1);`}/>
      <div onPointerDown={() => setDrag('max')} style={`position: absolute; top: 50%; left: calc(${tMax*100}% - 9px); transform: translateY(-50%); width: 18px; height: 18px; border-radius: 50%; background: var(--text-0); border: 3px solid var(--acc-orange); cursor: grab; box-shadow: var(--shadow-1);`}/>
    </div>
  );
}

/* SVG topology diagrams used by Mesh screen. */
export function TopologyDiagram({ kind, size = 96 }: { kind: 'straight'|'l_shape'|'u_shape'|'custom'; size?: number }) {
  const stroke = 'var(--text-2)';
  const acc = 'var(--acc-orange)';
  if (kind === 'straight') return (
    <svg viewBox="0 0 100 60" width="100%" style={`height: ${size*0.6}px;`}>
      <line x1="10" y1="30" x2="90" y2="30" stroke={stroke} stroke-width="2"/>
      {[10, 50, 90].map((x, i) => <circle key={i} cx={x} cy="30" r="4" fill={acc}/>)}
    </svg>
  );
  if (kind === 'l_shape') return (
    <svg viewBox="0 0 100 100" width="100%" style={`height: ${size}px;`}>
      <polyline points="20,20 20,80 80,80" stroke={stroke} stroke-width="2" fill="none"/>
      {[[20,20],[20,50],[20,80],[50,80],[80,80]].map(([x,y], i) => <circle key={i} cx={x} cy={y} r="4" fill={acc}/>)}
    </svg>
  );
  if (kind === 'u_shape') return (
    <svg viewBox="0 0 100 100" width="100%" style={`height: ${size}px;`}>
      <polyline points="15,20 15,80 85,80 85,20" stroke={stroke} stroke-width="2" fill="none"/>
      {[[15,20],[15,50],[15,80],[50,80],[85,80],[85,50],[85,20]].map(([x,y], i) => <circle key={i} cx={x} cy={y} r="4" fill={acc}/>)}
    </svg>
  );
  return (
    <svg viewBox="0 0 100 100" width="100%" style={`height: ${size}px;`}>
      <path d="M 15 30 Q 40 10 55 40 T 85 70" stroke={stroke} stroke-width="2" fill="none" stroke-dasharray="3 3"/>
      {[[15,30],[40,22],[55,40],[70,52],[85,70]].map(([x,y], i) => <circle key={i} cx={x} cy={y} r="4" fill={acc}/>)}
    </svg>
  );
}

/* Two-line chart for Motion screen — raw vs smoothed distance. */
export function LineChart({ raw, smooth, width = 600, height = 180 }: {
  raw: number[]; smooth: number[]; width?: number; height?: number;
}) {
  const lo = 0, hi = 300;
  const range = hi - lo;
  const toPath = (data: number[]) => data.map((v, i) => {
    const x = (i / Math.max(1, data.length - 1)) * width;
    const y = height - ((v - lo) / range) * height;
    return `${i === 0 ? 'M' : 'L'}${x.toFixed(1)},${y.toFixed(1)}`;
  }).join(' ');
  return (
    <svg width="100%" viewBox={`0 0 ${width} ${height}`} preserveAspectRatio="none" style={`display: block; height: ${height}px;`}>
      {[0, 0.25, 0.5, 0.75, 1].map(t => (
        <line key={t} x1="0" y1={height * t} x2={width} y2={height * t} stroke="var(--line-soft)" stroke-dasharray="2 4" stroke-width="1"/>
      ))}
      <path d={toPath(raw)} stroke="var(--text-3)" stroke-width="1" fill="none" opacity="0.7"/>
      <path d={toPath(smooth)} stroke="var(--acc-orange)" stroke-width="2" fill="none" stroke-linecap="round"/>
    </svg>
  );
}

/* HSV ↔ RGB conversion for the color wheel. */
export function hsv2rgb(h: number, s: number, v: number): [number, number, number] {
  h = ((h % 360) + 360) % 360;
  const c = v * s, x = c * (1 - Math.abs(((h / 60) % 2) - 1)), m = v - c;
  let r = 0, g = 0, b = 0;
  if (h < 60) [r, g, b] = [c, x, 0];
  else if (h < 120) [r, g, b] = [x, c, 0];
  else if (h < 180) [r, g, b] = [0, c, x];
  else if (h < 240) [r, g, b] = [0, x, c];
  else if (h < 300) [r, g, b] = [x, 0, c];
  else [r, g, b] = [c, 0, x];
  return [Math.round((r + m) * 255), Math.round((g + m) * 255), Math.round((b + m) * 255)];
}
export function rgb2hex(r: number, g: number, b: number) {
  return '#' + [r, g, b].map(n => Math.max(0, Math.min(255, n)).toString(16).padStart(2, '0')).join('').toUpperCase();
}
export function hex2rgb(hex: string): [number, number, number] {
  const h = hex.replace('#', '').padEnd(6, '0').slice(0, 6);
  return [parseInt(h.slice(0,2), 16), parseInt(h.slice(2,4), 16), parseInt(h.slice(4,6), 16)];
}

/* Animated logo mark — triangle "A" with concentric pulse rings.
 * Pulse intensity scales with proximity (closer target = brighter).
 * Faithful port of frontend/design-source/project/app.jsx LogoMark. */
export function LogoMark({ size = 36, distance = 0 }: { size?: number; distance?: number }) {
  const pulseStrength = Math.max(0.2, Math.min(1, (250 - distance) / 200));
  return (
    <div style={`position: relative; width: ${size}px; height: ${size}px; flex-shrink: 0;`}>
      <div style={`position: absolute; inset: -${size * 0.25}px; border-radius: 50%; background: radial-gradient(circle, rgba(255,122,61,0.35) 0%, rgba(255,61,130,0.15) 35%, transparent 65%); filter: blur(6px); opacity: ${0.6 * pulseStrength}; pointer-events: none; animation: logo-breath 2.6s ease-in-out infinite;`}/>
      <svg viewBox="0 0 48 48" width={size} height={size} style="position: relative; display: block;">
        <defs>
          <linearGradient id="lm-grad" x1="0" y1="0" x2="1" y2="1">
            <stop offset="0%" stop-color="var(--acc-amber)"/>
            <stop offset="50%" stop-color="var(--acc-orange)"/>
            <stop offset="100%" stop-color="var(--acc-pink)"/>
          </linearGradient>
          <radialGradient id="lm-core" cx="0.5" cy="0.5" r="0.5">
            <stop offset="0%" stop-color="#FFE4B0"/>
            <stop offset="60%" stop-color="var(--acc-orange)"/>
            <stop offset="100%" stop-color="var(--acc-pink)"/>
          </radialGradient>
        </defs>
        <rect x="1" y="1" width="46" height="46" rx="12" fill="#0a0c0f" stroke="url(#lm-grad)" stroke-width="1" opacity="0.55"/>
        <g style="transform-origin: 24px 32px;">
          {[0, 1, 2].map(i => (
            <circle cx="24" cy="32" r="6" fill="none" stroke="url(#lm-grad)" stroke-width="1.2" opacity="0.7"
              style={`animation: logo-pulse 2.2s ease-out infinite; animation-delay: ${i * 0.6}s; transform-origin: 24px 32px;`}/>
          ))}
        </g>
        <path d="M 24 11 L 13 32 L 35 32 Z" fill="url(#lm-grad)" opacity="0.95"/>
        <path d="M 24 18 L 18 30 L 30 30 Z" fill="#0a0c0f"/>
        <circle cx="24" cy="32" r="2.6" fill="url(#lm-core)"/>
        <circle cx="24" cy="32" r="1.1" fill="white" opacity="0.9"/>
        {[[6,6],[42,6],[6,42],[42,42]].map(([x,y]) => (
          <circle cx={x} cy={y} r="0.8" fill="var(--text-3)" opacity="0.5"/>
        ))}
      </svg>
    </div>
  );
}

/* Wordmark with gradient "Sense" — paired with LogoMark in sidebar/header. */
export function Wordmark({ font = 16, sub = 10, mono = false, version, target }: {
  font?: number; sub?: number; mono?: boolean; version?: string; target?: string;
}) {
  if (mono) return null;
  return (
    <div style="display: flex; flex-direction: column; line-height: 1.05;">
      <span style={`font-size: ${font}px; font-weight: 600; letter-spacing: -0.025em; background: linear-gradient(180deg, var(--text-0) 0%, var(--text-1) 100%); -webkit-background-clip: text; -webkit-text-fill-color: transparent; background-clip: text;`}>
        Ambi<span style="background: var(--acc-grad); -webkit-background-clip: text; -webkit-text-fill-color: transparent; background-clip: text; font-weight: 600;">Sense</span>
      </span>
      <span class="mono" style={`font-size: ${sub}px; color: var(--text-3); letter-spacing: 0.08em; text-transform: uppercase; margin-top: 2px;`}>
        {version || ''} <span style="color: var(--text-4);">·</span> {target || ''}
      </span>
    </div>
  );
}
