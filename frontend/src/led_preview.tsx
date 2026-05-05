/** LED strip canvas preview. Mirrors the firmware's mode logic so the
    on-screen animation matches what a real strip would show. */
import { useEffect, useRef } from 'preact/hooks';

export const LED_MODE_NAMES = [
  'Standard','Rainbow','Color Wave','Breathing','Solid',
  'Comet','Pulse','Fire','Theater Chase','Dual Scan','Motion Particles',
];

interface PreviewProps {
  mode: number;
  rgb: [number, number, number];
  count: number;
  brightness: number;
  span: number;
  distance?: number;
  minD?: number;
  maxD?: number;
  height?: number;
  speed?: number;
  intensity?: number;
}

interface RGB { r: number; g: number; b: number; }

const wheel = (p: number): RGB => {
  if (p < 85)  return { r: p*3, g: 255-p*3, b: 0 };
  if (p < 170) { p -= 85;  return { r: 255-p*3, g: 0, b: p*3 }; }
                 p -= 170; return { r: 0, g: p*3, b: 255-p*3 };
};
const dim = (c: RGB, k: number): RGB => ({ r: (c.r*k)|0, g: (c.g*k)|0, b: (c.b*k)|0 });

export function LedPreview(p: PreviewProps) {
  const ref = useRef<HTMLCanvasElement>(null);
  const stateRef = useRef<{ step: number; particles: any[]; heat: number[]; prev: RGB[] }>({
    step: 0, particles: [], heat: [], prev: [],
  });

  useEffect(() => {
    const canvas = ref.current;
    if (!canvas) return;
    const ctx = canvas.getContext('2d');
    if (!ctx) return;

    let raf = 0;
    let stop = false;

    const draw = () => {
      if (stop) return;
      const w = canvas.width = canvas.clientWidth * window.devicePixelRatio;
      const h = canvas.height = (p.height || 80) * window.devicePixelRatio;
      const n = Math.max(1, Math.min(p.count, 300));   /* preview cap */
      const pxW = w / n;
      ctx.clearRect(0, 0, w, h);

      const s = stateRef.current;
      if (s.heat.length !== n) s.heat = new Array(n).fill(0);
      if (s.prev.length !== n) s.prev = new Array(n).fill({r:0,g:0,b:0});

      const base: RGB = { r: p.rgb[0], g: p.rgb[1], b: p.rgb[2] };
      const br = (p.brightness ?? 255) / 255;
      const minD = p.minD ?? 30, maxD = p.maxD ?? 300;
      const span = Math.max(1, p.span);
      const avail = Math.max(0, n - span);
      const dist = p.distance ?? minD + Math.abs(Math.sin(s.step * 0.01)) * (maxD - minD);
      const start = ((dist - minD) / (maxD - minD) * avail) | 0;
      const espd = (p.speed ?? 50) / 50;
      const eint = (p.intensity ?? 50) / 100;

      const paint = (i: number, c: RGB) => {
        if (i < 0 || i >= n) return;
        ctx.fillStyle = `rgb(${(c.r*br)|0},${(c.g*br)|0},${(c.b*br)|0})`;
        ctx.fillRect(i * pxW, 0, pxW + 1, h);
      };

      switch (p.mode) {
        case 0: { /* standard */
          for (let i = 0; i < n; i++) paint(i, dim(base, 0.05));
          for (let i = start; i < Math.min(start + span, n); i++) paint(i, base);
          break;
        }
        case 1: { /* rainbow */
          const off = (s.step * espd) & 0xFF;
          for (let i = 0; i < n; i++) paint(i, wheel(((i*256/n + off) | 0) & 0xFF));
          break;
        }
        case 2: { /* color_wave */
          for (let i = 0; i < n; i++) {
            const phase = i / n * 6.28 + s.step * 0.05 * espd;
            let k = (Math.sin(phase) + 1) * 0.5;
            k = k * eint + (1 - eint) * 0.4;
            paint(i, dim(wheel(((i*256/n + s.step) | 0) & 0xFF), k));
          }
          break;
        }
        case 3: { /* breathing */
          let k = (Math.sin(s.step * 0.05 * espd) + 1) * 0.5;
          k = 0.1 + 0.9 * k * eint;
          for (let i = 0; i < n; i++) paint(i, dim(base, k));
          break;
        }
        case 4: { /* solid */
          for (let i = 0; i < n; i++) paint(i, base);
          break;
        }
        case 5: { /* comet */
          const fade = 0.85;
          for (let i = 0; i < n; i++) {
            s.prev[i] = dim(s.prev[i], fade);
            paint(i, s.prev[i]);
          }
          for (let i = 0; i < 3; i++) {
            const px = start + i;
            if (px < n) { s.prev[px] = base; paint(px, base); }
          }
          break;
        }
        case 6: { /* pulse */
          for (let i = 0; i < n; i++) paint(i, {r:0,g:0,b:0});
          const max_r = n / 4;
          for (let p2 = 0; p2 < 3; p2++) {
            const phase = p2 * 2;
            const r = ((s.step * 0.2 + phase) % max_r);
            for (let off = -((r)|0); off <= ((r)|0); off++) {
              const k = (1 - Math.abs(off / Math.max(0.1, r)) ** 2) * eint;
              const idx = start + off;
              if (idx >= 0 && idx < n && k > 0) paint(idx, dim(base, k));
            }
          }
          break;
        }
        case 7: { /* fire */
          for (let i = 0; i < n; i++) s.heat[i] = Math.max(0, s.heat[i] - (Math.random()*55 + 2)|0);
          for (let i = n-1; i >= 2; i--) s.heat[i] = ((s.heat[i-1] + s.heat[i-2]*2) / 3)|0;
          if (Math.random() < 0.5) { const y = (Math.random()*7)|0; s.heat[y] = Math.min(255, s.heat[y] + 160 + ((Math.random()*96)|0)); }
          for (let i = 0; i < n; i++) {
            const t = ((s.heat[i] * 191) / 255)|0;
            let c: RGB;
            if (t < 64)        c = {r: t*4, g: 0, b: 0};
            else if (t < 128)  c = {r: 255, g: (t-64)*4, b: 0};
            else               c = {r: 255, g: 255, b: (t-128)*4};
            paint(i, c);
          }
          break;
        }
        case 8: { /* theater_chase */
          const gap = 3, ph = s.step % gap;
          for (let i = 0; i < n; i++) paint(i, ((i+ph) % gap === 0) ? base : {r:0,g:0,b:0});
          break;
        }
        case 9: { /* dual_scan */
          for (let i = 0; i < n; i++) paint(i, {r:0,g:0,b:0});
          const sw = 4, p1 = (s.step|0) % n, p2 = n - 1 - ((s.step|0) % n);
          const inv: RGB = { r: 255-base.r, g: 255-base.g, b: 255-base.b };
          for (let off = -sw; off <= sw; off++) {
            const k = 1 - Math.abs(off/sw);
            if (p1+off >= 0 && p1+off < n) paint(p1+off, dim(base, k));
            if (p2+off >= 0 && p2+off < n) paint(p2+off, dim(inv, k));
          }
          if (start >= 0 && start < n) paint(start, {r:255,g:255,b:255});
          break;
        }
        case 10: { /* motion_particles */
          for (const part of s.particles) {
            if (!part.active) continue;
            part.pos += part.vel; part.bright -= 0.02;
            if (part.bright <= 0 || part.pos < 0 || part.pos >= n) part.active = false;
          }
          const spawn = 1 + (eint * 4)|0;
          for (let i = 0; i < spawn; i++) {
            if (s.particles.length < 50) s.particles.push({ active: true, pos: start + (Math.random()*7-3), vel: (Math.random()*2-1)*espd*0.6, bright: 1 });
            else for (const part of s.particles) if (!part.active) { part.active = true; part.pos = start + (Math.random()*7-3); part.vel = (Math.random()*2-1)*espd*0.6; part.bright = 1; break; }
          }
          for (let i = 0; i < n; i++) paint(i, {r:0,g:0,b:0});
          for (const part of s.particles) {
            if (!part.active) continue;
            const px = part.pos|0;
            if (px >= 0 && px < n) paint(px, dim(base, part.bright));
          }
          break;
        }
      }

      s.step++;
      raf = requestAnimationFrame(draw);
    };
    raf = requestAnimationFrame(draw);
    return () => { stop = true; cancelAnimationFrame(raf); };
  }, [p.mode, p.rgb[0], p.rgb[1], p.rgb[2], p.count, p.brightness, p.span, p.distance, p.speed, p.intensity]);

  return <canvas ref={ref} class="led-canvas" style={`height: ${p.height||80}px;`} />;
}
