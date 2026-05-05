// AmbiSense — LED rendering engine and mode definitions
const { useEffect: useEffectLED, useRef: useRefLED } = React;

const LED_MODES = [
  { id: "standard", name: "Standard", desc: "Distance-driven cluster with directional fade", anim: "standard" },
  { id: "rainbow", name: "Rainbow", desc: "Full-strip hue cycle", anim: "rainbow" },
  { id: "color-wave", name: "Color Wave", desc: "Sine wave of color across strip", anim: "wave" },
  { id: "breathing", name: "Breathing", desc: "Strip gently inhales and exhales", anim: "breathing" },
  { id: "solid", name: "Solid", desc: "All pixels one color", anim: "solid" },
  { id: "comet", name: "Comet", desc: "Tail chasing across the strip", anim: "comet" },
  { id: "pulse", name: "Pulse", desc: "Center pulse expands outward", anim: "pulse" },
  { id: "fire", name: "Fire", desc: "Flickering ember simulation", anim: "fire" },
  { id: "theater-chase", name: "Theater Chase", desc: "Marquee dot pattern", anim: "chase" },
  { id: "dual-scan", name: "Dual Scan", desc: "Two scanners meet in the middle", anim: "dualscan" },
  { id: "motion-particles", name: "Motion Particles", desc: "Particles spawn from your position", anim: "particles" },
];

/* Render N pixels for a given mode at time t (seconds), distance 0..1 (along strip),
   color {h,s,v}, brightness 0..1.
   Returns array of [r,g,b] of length N. */
function renderLEDFrame(mode, n, t, distance, color, brightness, opts = {}) {
  const out = new Array(n);
  const speed = (opts.speed ?? 60) / 100;
  const intensity = (opts.intensity ?? 70) / 100;
  const trail = opts.trail ?? 4;
  const baseHue = color.h;

  const set = (i, r, g, b) => {
    out[i] = [r * brightness, g * brightness, b * brightness];
  };

  switch (mode) {
    case "standard": {
      const center = distance * (n - 1);
      const span = Math.max(2, trail * 2.5);
      for (let i = 0; i < n; i++) {
        const d = Math.abs(i - center);
        const f = Math.max(0, 1 - d / span);
        const v = f * f;
        const [r, g, b] = hsv2rgb(baseHue, color.s, color.v * v);
        set(i, r, g, b);
      }
      break;
    }
    case "rainbow": {
      for (let i = 0; i < n; i++) {
        const h = (i / n) * 360 + t * 60 * speed;
        const [r, g, b] = hsv2rgb(h, 1, 1);
        set(i, r, g, b);
      }
      break;
    }
    case "wave": {
      for (let i = 0; i < n; i++) {
        const phase = (i / n) * Math.PI * 2 + t * speed * 3;
        const v = (Math.sin(phase) + 1) / 2;
        const h = baseHue + Math.sin(phase) * 40;
        const [r, g, b] = hsv2rgb(h, color.s, v * color.v);
        set(i, r, g, b);
      }
      break;
    }
    case "breathing": {
      const v = (Math.sin(t * speed * 1.6) + 1) / 2;
      const [r, g, b] = hsv2rgb(baseHue, color.s, v * color.v);
      for (let i = 0; i < n; i++) set(i, r, g, b);
      break;
    }
    case "solid": {
      const [r, g, b] = hsv2rgb(baseHue, color.s, color.v);
      for (let i = 0; i < n; i++) set(i, r, g, b);
      break;
    }
    case "comet": {
      const head = (t * speed * n * 0.6) % (n + trail * 4);
      const tailLen = 6 + trail * 2;
      for (let i = 0; i < n; i++) {
        const d = head - i;
        let v = 0;
        if (d >= 0 && d < tailLen) v = Math.pow(1 - d / tailLen, 2);
        const [r, g, b] = hsv2rgb(baseHue, color.s, v * color.v);
        set(i, r, g, b);
      }
      break;
    }
    case "pulse": {
      const center = (n - 1) / 2;
      const radius = ((t * speed * 1.5) % 1.4) * (n / 2);
      for (let i = 0; i < n; i++) {
        const d = Math.abs(i - center);
        const v = Math.max(0, 1 - Math.abs(d - radius) / 3);
        const [r, g, b] = hsv2rgb(baseHue, color.s, v * v * color.v);
        set(i, r, g, b);
      }
      break;
    }
    case "fire": {
      for (let i = 0; i < n; i++) {
        const flick = (Math.sin(i * 1.7 + t * 6 * speed) + Math.sin(i * 0.6 + t * 9 * speed)) / 2;
        const v = clamp(0.4 + flick * 0.6 * intensity, 0, 1);
        const h = lerp(8, 38, (Math.sin(i * 0.3 + t * 2) + 1) / 2);
        const [r, g, b] = hsv2rgb(h, 1, v);
        set(i, r, g, b);
      }
      break;
    }
    case "chase": {
      const off = Math.floor(t * speed * 8) % 3;
      for (let i = 0; i < n; i++) {
        const v = ((i + off) % 3 === 0) ? 1 : 0;
        const [r, g, b] = hsv2rgb(baseHue, color.s, v * color.v);
        set(i, r, g, b);
      }
      break;
    }
    case "dualscan": {
      const pos = (Math.sin(t * speed * 2) + 1) / 2 * (n - 1);
      const pos2 = n - 1 - pos;
      for (let i = 0; i < n; i++) {
        const v = Math.max(
          Math.max(0, 1 - Math.abs(i - pos) / 3),
          Math.max(0, 1 - Math.abs(i - pos2) / 3),
        );
        const [r, g, b] = hsv2rgb(baseHue, color.s, v * v * color.v);
        set(i, r, g, b);
      }
      break;
    }
    case "particles": {
      // simulate based on time + distance
      const center = distance * (n - 1);
      for (let i = 0; i < n; i++) {
        const noise = Math.sin(i * 3.1 + t * 4 * speed) * Math.cos(i * 1.7 - t * 3.5);
        const proximity = Math.max(0, 1 - Math.abs(i - center) / (n * 0.4));
        const v = Math.max(0, noise * 0.5 + 0.4) * proximity * intensity;
        const h = baseHue + i * 1.5;
        const [r, g, b] = hsv2rgb(h, color.s, v * color.v);
        set(i, r, g, b);
      }
      break;
    }
    default:
      for (let i = 0; i < n; i++) set(i, 0, 0, 0);
  }
  return out;
}

/* Canvas LED strip preview. Renders horizontal pixels with bloom. */
function LEDStrip({ mode, n = 60, distance = 0.5, color, brightness = 1, height = 56, speed = 60, intensity = 70, trail = 4, paused = false }) {
  const ref = useRefLED(null);
  const tRef = useRefLED(0);
  const lastRef = useRefLED(performance.now());
  useEffectLED(() => {
    const c = ref.current;
    if (!c) return;
    const ctx = c.getContext("2d");
    let raf;
    const reduced = window.matchMedia("(prefers-reduced-motion: reduce)").matches;
    const draw = (now) => {
      const dt = (now - lastRef.current) / 1000;
      lastRef.current = now;
      if (!paused && !reduced) tRef.current += dt;
      const dpr = window.devicePixelRatio || 1;
      const w = c.clientWidth;
      const h = c.clientHeight;
      if (c.width !== w * dpr || c.height !== h * dpr) {
        c.width = w * dpr; c.height = h * dpr;
      }
      ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
      // background
      ctx.fillStyle = "#0a0c0f";
      ctx.fillRect(0, 0, w, h);

      const frame = renderLEDFrame(mode, n, tRef.current, distance, color, brightness, { speed, intensity, trail });
      const px = w / n;
      const cy = h / 2;
      const r = Math.min(px * 0.4, h * 0.25);

      // glow pass
      ctx.globalCompositeOperation = "lighter";
      for (let i = 0; i < n; i++) {
        const [rr, gg, bb] = frame[i];
        if (rr + gg + bb < 6) continue;
        const cx = i * px + px / 2;
        const grad = ctx.createRadialGradient(cx, cy, 0, cx, cy, r * 4);
        grad.addColorStop(0, `rgba(${rr|0},${gg|0},${bb|0},0.55)`);
        grad.addColorStop(1, `rgba(${rr|0},${gg|0},${bb|0},0)`);
        ctx.fillStyle = grad;
        ctx.fillRect(cx - r * 4, 0, r * 8, h);
      }
      // dot pass
      ctx.globalCompositeOperation = "source-over";
      for (let i = 0; i < n; i++) {
        const [rr, gg, bb] = frame[i];
        const cx = i * px + px / 2;
        ctx.fillStyle = `rgb(${rr|0},${gg|0},${bb|0})`;
        ctx.beginPath();
        ctx.arc(cx, cy, r, 0, Math.PI * 2);
        ctx.fill();
      }

      raf = requestAnimationFrame(draw);
    };
    raf = requestAnimationFrame(draw);
    return () => cancelAnimationFrame(raf);
  }, [mode, n, distance, color.h, color.s, color.v, brightness, speed, intensity, trail, paused]);
  return (
    <canvas
      ref={ref}
      style={{
        width: "100%", height,
        borderRadius: "var(--r-md)",
        background: "#0a0c0f",
        border: "1px solid var(--line)",
        display: "block",
      }}
    />
  );
}

Object.assign(window, { LED_MODES, renderLEDFrame, LEDStrip });
