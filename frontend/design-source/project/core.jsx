// AmbiSense — shared atoms, icons, store, and live data sim
const { useState, useEffect, useRef, useCallback, useMemo, createContext, useContext } = React;

/* ============ ICONS ============ */
const Icon = ({ name, size = 16, stroke = 1.6, className = "", style = {} }) => {
  const s = size;
  const common = {
    width: s, height: s, viewBox: "0 0 24 24",
    fill: "none", stroke: "currentColor",
    strokeWidth: stroke, strokeLinecap: "round", strokeLinejoin: "round",
    className, style,
  };
  const paths = {
    dashboard: <><rect x="3" y="3" width="7" height="9" rx="1.5"/><rect x="14" y="3" width="7" height="5" rx="1.5"/><rect x="14" y="12" width="7" height="9" rx="1.5"/><rect x="3" y="16" width="7" height="5" rx="1.5"/></>,
    led: <><path d="M12 3v3M12 18v3M5 12H2M22 12h-3M5.6 5.6l2.1 2.1M16.3 16.3l2.1 2.1M5.6 18.4l2.1-2.1M16.3 7.7l2.1-2.1"/><circle cx="12" cy="12" r="4"/></>,
    motion: <><path d="M3 12h3l3-7 4 14 3-7h5"/></>,
    mesh: <><circle cx="12" cy="5" r="2"/><circle cx="5" cy="19" r="2"/><circle cx="19" cy="19" r="2"/><path d="M12 7v10M10.5 6.5l-5 11M13.5 6.5l5 11"/></>,
    chip: <><rect x="6" y="6" width="12" height="12" rx="1.5"/><path d="M9 1v3M12 1v3M15 1v3M9 20v3M12 20v3M15 20v3M1 9h3M1 12h3M1 15h3M20 9h3M20 12h3M20 15h3"/></>,
    wifi: <><path d="M2 8.5C5 6 8.5 4.5 12 4.5s7 1.5 10 4M5 12c2-1.7 4.5-2.5 7-2.5s5 .8 7 2.5M8.5 15.5c1-.8 2.2-1.2 3.5-1.2s2.5.4 3.5 1.2"/><circle cx="12" cy="19" r="1" fill="currentColor"/></>,
    settings: <><circle cx="12" cy="12" r="3"/><path d="M19.4 15a1.7 1.7 0 0 0 .3 1.8l.1.1a2 2 0 1 1-2.8 2.8l-.1-.1a1.7 1.7 0 0 0-1.8-.3 1.7 1.7 0 0 0-1 1.5V21a2 2 0 1 1-4 0v-.1a1.7 1.7 0 0 0-1-1.5 1.7 1.7 0 0 0-1.8.3l-.1.1a2 2 0 1 1-2.8-2.8l.1-.1a1.7 1.7 0 0 0 .3-1.8 1.7 1.7 0 0 0-1.5-1H3a2 2 0 1 1 0-4h.1a1.7 1.7 0 0 0 1.5-1 1.7 1.7 0 0 0-.3-1.8l-.1-.1a2 2 0 1 1 2.8-2.8l.1.1a1.7 1.7 0 0 0 1.8.3h0a1.7 1.7 0 0 0 1-1.5V3a2 2 0 1 1 4 0v.1a1.7 1.7 0 0 0 1 1.5 1.7 1.7 0 0 0 1.8-.3l.1-.1a2 2 0 1 1 2.8 2.8l-.1.1a1.7 1.7 0 0 0-.3 1.8v0a1.7 1.7 0 0 0 1.5 1H21a2 2 0 1 1 0 4h-.1a1.7 1.7 0 0 0-1.5 1z"/></>,
    sun: <><circle cx="12" cy="12" r="4"/><path d="M12 2v2M12 20v2M4.93 4.93l1.41 1.41M17.66 17.66l1.41 1.41M2 12h2M20 12h2M4.93 19.07l1.41-1.41M17.66 6.34l1.41-1.41"/></>,
    moon: <><path d="M21 12.79A9 9 0 1 1 11.21 3 7 7 0 0 0 21 12.79z"/></>,
    check: <><path d="M5 12l5 5L20 7"/></>,
    x: <><path d="M6 6l12 12M6 18L18 6"/></>,
    info: <><circle cx="12" cy="12" r="9"/><path d="M12 8h.01M11 12h1v4h1"/></>,
    warn: <><path d="M12 3 2 21h20L12 3zM12 10v5M12 18h.01"/></>,
    chevron: <><path d="M9 6l6 6-6 6"/></>,
    chevronDown: <><path d="M6 9l6 6 6-6"/></>,
    plus: <><path d="M12 5v14M5 12h14"/></>,
    minus: <><path d="M5 12h14"/></>,
    refresh: <><path d="M3 12a9 9 0 1 0 3-6.7L3 8"/><path d="M3 3v5h5"/></>,
    upload: <><path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4M17 8l-5-5-5 5M12 3v12"/></>,
    download: <><path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4M7 10l5 5 5-5M12 15V3"/></>,
    trash: <><path d="M3 6h18M8 6V4a2 2 0 0 1 2-2h4a2 2 0 0 1 2 2v2M19 6l-1 14a2 2 0 0 1-2 2H8a2 2 0 0 1-2-2L5 6"/></>,
    eye: <><path d="M1 12s4-7 11-7 11 7 11 7-4 7-11 7-11-7-11-7z"/><circle cx="12" cy="12" r="3"/></>,
    eyeOff: <><path d="M17.94 17.94A10.94 10.94 0 0 1 12 19c-7 0-11-7-11-7a19.7 19.7 0 0 1 5.06-5.94M9.9 4.24A10.9 10.9 0 0 1 12 4c7 0 11 7 11 7a19.6 19.6 0 0 1-2.16 3.19M14.12 14.12a3 3 0 1 1-4.24-4.24M1 1l22 22"/></>,
    bolt: <><path d="M13 2L3 14h7l-1 8 10-12h-7z"/></>,
    cpu: <><rect x="4" y="4" width="16" height="16" rx="2"/><rect x="9" y="9" width="6" height="6"/><path d="M9 1v3M15 1v3M9 20v3M15 20v3M20 9h3M20 14h3M1 9h3M1 14h3"/></>,
    radar: <><circle cx="12" cy="12" r="9"/><path d="M12 12L19 7"/><path d="M12 12a4 4 0 1 1-4 4"/></>,
    pin: <><path d="M12 21l-7-7a4 4 0 1 1 7-5 4 4 0 1 1 7 5l-7 7z"/></>,
    grid: <><rect x="3" y="3" width="7" height="7"/><rect x="14" y="3" width="7" height="7"/><rect x="3" y="14" width="7" height="7"/><rect x="14" y="14" width="7" height="7"/></>,
    palette: <><circle cx="13.5" cy="6.5" r="1.5" fill="currentColor"/><circle cx="17.5" cy="10.5" r="1.5" fill="currentColor"/><circle cx="8.5" cy="7.5" r="1.5" fill="currentColor"/><circle cx="6.5" cy="12.5" r="1.5" fill="currentColor"/><path d="M12 22a10 10 0 1 1 10-10c0 2.76-2.24 4-5 4h-2a2 2 0 0 0-1 3.74A2 2 0 0 1 12 22z"/></>,
    sliders: <><path d="M4 21v-7M4 10V3M12 21v-9M12 8V3M20 21v-5M20 12V3M1 14h6M9 8h6M17 16h6"/></>,
    play: <><path d="M5 3l14 9-14 9V3z" fill="currentColor"/></>,
    pause: <><rect x="6" y="4" width="4" height="16" fill="currentColor"/><rect x="14" y="4" width="4" height="16" fill="currentColor"/></>,
    copy: <><rect x="9" y="9" width="13" height="13" rx="2"/><path d="M5 15H4a2 2 0 0 1-2-2V4a2 2 0 0 1 2-2h9a2 2 0 0 1 2 2v1"/></>,
    link: <><path d="M10 13a5 5 0 0 0 7.54.54l3-3a5 5 0 0 0-7.07-7.07l-1.72 1.71"/><path d="M14 11a5 5 0 0 0-7.54-.54l-3 3a5 5 0 0 0 7.07 7.07l1.71-1.71"/></>,
    flash: <><path d="M13 2L3 14h7l-1 8 10-12h-7l1-8z"/></>,
    box: <><path d="M21 16V8a2 2 0 0 0-1-1.73l-7-4a2 2 0 0 0-2 0l-7 4A2 2 0 0 0 3 8v8a2 2 0 0 0 1 1.73l7 4a2 2 0 0 0 2 0l7-4A2 2 0 0 0 21 16z"/><path d="M3.27 6.96L12 12.01l8.73-5.05M12 22.08V12"/></>,
    arrowRight: <><path d="M5 12h14M13 5l7 7-7 7"/></>,
  };
  return <svg {...common}>{paths[name] || null}</svg>;
};

/* ============ TOAST ============ */
const ToastCtx = createContext({ push: () => {} });
const useToast = () => useContext(ToastCtx);

function ToastHost({ children }) {
  const [items, setItems] = useState([]);
  const push = useCallback((msg, kind = "ok") => {
    const id = Math.random().toString(36).slice(2);
    setItems(s => [...s, { id, msg, kind }]);
    setTimeout(() => setItems(s => s.filter(i => i.id !== id)), 1800);
  }, []);
  return (
    <ToastCtx.Provider value={{ push }}>
      {children}
      <div style={{
        position: "fixed", bottom: 24, left: "50%", transform: "translateX(-50%)",
        display: "flex", flexDirection: "column", gap: 8, zIndex: 1000, pointerEvents: "none",
      }}>
        {items.map(t => (
          <div key={t.id} style={{
            display: "flex", alignItems: "center", gap: 8,
            padding: "8px 14px",
            background: "var(--bg-3)",
            border: "1px solid var(--line)",
            borderRadius: 999,
            fontSize: 12,
            boxShadow: "var(--shadow-2)",
            animation: "fade-up .18s ease-out",
            color: t.kind === "err" ? "var(--err)" : "var(--text-0)",
          }}>
            <Icon name={t.kind === "err" ? "warn" : "check"} size={13}
              style={{ color: t.kind === "err" ? "var(--err)" : "var(--ok)" }} />
            {t.msg}
          </div>
        ))}
      </div>
    </ToastCtx.Provider>
  );
}

/* ============ SETTINGS STORE (mocks /api/settings) ============ */
const defaultSettings = {
  systemEnabled: true,
  deviceName: "ambisense-hall",
  hostname: "ambisense-hall",
  // LED
  ledMode: "standard",
  brightness: 180,
  color: { h: 22, s: 0.95, v: 1 }, // amber
  numLeds: 60,
  minDistance: 40,
  maxDistance: 220,
  lightSpan: 18,
  centerShift: 0,
  backgroundMode: false,
  trailLength: 4,
  directionalLight: true,
  effectSpeed: 60,
  effectIntensity: 70,
  // Motion
  motionSmoothing: true,
  positionSmoothing: 65,
  velocitySmoothing: 50,
  predictionFactor: 35,
  pGain: 60,
  iGain: 22,
  // Mesh
  role: "master",
  topology: "u-shape",
  sensorPriority: "most-recent",
  // Hardware
  boardProfile: "esp32-c3-supermini",
  radar: "ld2410c",
  pins: { ledData: 4, radarRx: 20, radarTx: 21, button: 9, statusLed: 8 },
  // Network
  ssid: "Loft 5GHz",
  staticIp: false,
  ip: "10.0.0.42",
  gateway: "10.0.0.1",
  netmask: "255.255.255.0",
  // System
  authRequired: false,
  authPassword: "",
  presets: ["#FFB54A", "#FF7A3D", "#FF3D82", "#5BC7FF", "#4ADE80", "#9D5BFF", "#FF5470", "#FFFFFF"],
};

const StoreCtx = createContext(null);
const useStore = () => useContext(StoreCtx);

function StoreProvider({ children }) {
  const [s, setS] = useState(defaultSettings);
  const toast = useToast();
  // optimistic write
  const set = useCallback((patch, opts = {}) => {
    const silent = opts.silent;
    setS(prev => {
      const next = typeof patch === "function" ? patch(prev) : { ...prev, ...patch };
      return next;
    });
    if (!silent) {
      // simulate /api/settings POST 200 OK
      setTimeout(() => toast.push("Saved"), 80);
    }
  }, [toast]);
  return <StoreCtx.Provider value={{ s, set }}>{children}</StoreCtx.Provider>;
}

/* ============ LIVE WS SIM ============ */
const LiveCtx = createContext(null);
const useLive = () => useContext(LiveCtx);

function LiveProvider({ children }) {
  const [live, setLive] = useState({
    distance: 120,
    rssi: -52,
    heap: 142000,
    minHeap: 118000,
    fragmentation: 14,
    uptime: 78921,
    connected: true,
    distanceHistory: Array.from({ length: 80 }, (_, i) => 100 + Math.sin(i / 6) * 20),
    rawHistory: Array.from({ length: 80 }, () => 0),
    smoothHistory: Array.from({ length: 80 }, () => 0),
    slaveHealth: [
      { mac: "A8:42:E3:9C:11:F2", name: "stair-mid", rssi: -64, lost: 0.6, lastSeen: 1, online: true },
      { mac: "A8:42:E3:9C:14:88", name: "stair-top", rssi: -71, lost: 1.2, lastSeen: 2, online: true },
    ],
  });
  const tRef = useRef(0);
  useEffect(() => {
    const id = setInterval(() => {
      tRef.current += 1;
      setLive(prev => {
        const t = tRef.current;
        // simulate person walking around
        const target = 80 + Math.sin(t / 14) * 70 + Math.sin(t / 5) * 8;
        const noisy = target + (Math.random() - 0.5) * 22;
        const distance = Math.max(5, Math.min(300, noisy));
        const smoothPrev = prev.smoothHistory[prev.smoothHistory.length - 1] || target;
        const smoothed = smoothPrev + (target - smoothPrev) * 0.18;
        return {
          ...prev,
          distance,
          rssi: -50 + Math.round((Math.random() - 0.5) * 6),
          heap: 142000 + Math.round((Math.random() - 0.5) * 800),
          uptime: prev.uptime + 1,
          distanceHistory: [...prev.distanceHistory.slice(1), distance],
          rawHistory: [...prev.rawHistory.slice(1), noisy],
          smoothHistory: [...prev.smoothHistory.slice(1), smoothed],
        };
      });
    }, 200);
    return () => clearInterval(id);
  }, []);
  return <LiveCtx.Provider value={live}>{children}</LiveCtx.Provider>;
}

/* ============ Helpers ============ */
function clamp(n, a, b) { return Math.max(a, Math.min(b, n)); }
function lerp(a, b, t) { return a + (b - a) * t; }
function hsv2rgb(h, s, v) {
  h = ((h % 360) + 360) % 360;
  const c = v * s;
  const x = c * (1 - Math.abs(((h / 60) % 2) - 1));
  const m = v - c;
  let r = 0, g = 0, b = 0;
  if (h < 60) [r, g, b] = [c, x, 0];
  else if (h < 120) [r, g, b] = [x, c, 0];
  else if (h < 180) [r, g, b] = [0, c, x];
  else if (h < 240) [r, g, b] = [0, x, c];
  else if (h < 300) [r, g, b] = [x, 0, c];
  else [r, g, b] = [c, 0, x];
  return [Math.round((r + m) * 255), Math.round((g + m) * 255), Math.round((b + m) * 255)];
}
function rgb2hex(r, g, b) {
  return "#" + [r, g, b].map(n => n.toString(16).padStart(2, "0")).join("").toUpperCase();
}
function hsv2hex(h, s, v) { const [r, g, b] = hsv2rgb(h, s, v); return rgb2hex(r, g, b); }
function hex2rgb(hex) {
  const h = hex.replace("#", "");
  return [parseInt(h.slice(0, 2), 16), parseInt(h.slice(2, 4), 16), parseInt(h.slice(4, 6), 16)];
}
function rgb2hsv(r, g, b) {
  r /= 255; g /= 255; b /= 255;
  const max = Math.max(r, g, b), min = Math.min(r, g, b);
  const d = max - min;
  let h;
  if (d === 0) h = 0;
  else if (max === r) h = ((g - b) / d) % 6;
  else if (max === g) h = (b - r) / d + 2;
  else h = (r - g) / d + 4;
  h *= 60;
  if (h < 0) h += 360;
  const s = max === 0 ? 0 : d / max;
  return { h, s, v: max };
}
function fmtUptime(s) {
  const d = Math.floor(s / 86400);
  const h = Math.floor((s % 86400) / 3600);
  const m = Math.floor((s % 3600) / 60);
  const sec = s % 60;
  if (d) return `${d}d ${h}h ${m}m`;
  if (h) return `${h}h ${m}m ${sec}s`;
  return `${m}m ${sec}s`;
}

/* ============ Sub-components ============ */
function Toggle({ on, onChange, large }) {
  return (
    <button
      role="switch"
      aria-checked={on}
      className={`toggle ${on ? "on" : ""} ${large ? "toggle-lg" : ""}`}
      onClick={() => onChange(!on)}
    />
  );
}

function Slider({ value, onChange, min = 0, max = 100, step = 1, suffix = "", showValue = true, label }) {
  return (
    <div>
      {label && (
        <div style={{ display: "flex", justifyContent: "space-between", marginBottom: 6 }}>
          <span className="field-label" style={{ marginBottom: 0 }}>{label}</span>
          {showValue && <span className="mono" style={{ fontSize: 12, color: "var(--text-1)" }}>{value}{suffix}</span>}
        </div>
      )}
      <input
        type="range" className="range"
        min={min} max={max} step={step} value={value}
        onChange={e => onChange(Number(e.target.value))}
      />
    </div>
  );
}

function StatusDot({ status = "ok" }) {
  return <span className={`dot dot-${status}`} />;
}

function Sparkline({ data, width = 200, height = 40, color = "var(--acc-orange)", fill = true, min, max }) {
  const padded = data.length > 1 ? data : [...data, ...data];
  const lo = min !== undefined ? min : Math.min(...padded);
  const hi = max !== undefined ? max : Math.max(...padded);
  const range = hi - lo || 1;
  const pts = padded.map((v, i) => {
    const x = (i / (padded.length - 1)) * width;
    const y = height - ((v - lo) / range) * height;
    return [x, y];
  });
  const pathD = pts.map((p, i) => `${i === 0 ? "M" : "L"}${p[0].toFixed(1)},${p[1].toFixed(1)}`).join(" ");
  const fillD = pathD + ` L${width},${height} L0,${height} Z`;
  return (
    <svg width={width} height={height} style={{ display: "block", overflow: "visible" }}>
      {fill && (
        <>
          <defs>
            <linearGradient id="sparkfill" x1="0" x2="0" y1="0" y2="1">
              <stop offset="0%" stopColor={color} stopOpacity="0.35"/>
              <stop offset="100%" stopColor={color} stopOpacity="0"/>
            </linearGradient>
          </defs>
          <path d={fillD} fill="url(#sparkfill)"/>
        </>
      )}
      <path d={pathD} stroke={color} strokeWidth="1.5" fill="none" strokeLinecap="round" strokeLinejoin="round"/>
    </svg>
  );
}

/* expose globals */
Object.assign(window, {
  Icon, ToastHost, useToast, StoreProvider, useStore, LiveProvider, useLive,
  Toggle, Slider, StatusDot, Sparkline,
  hsv2rgb, hsv2hex, hex2rgb, rgb2hsv, clamp, lerp, fmtUptime,
});
