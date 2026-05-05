// AmbiSense — App shell

const TABS = [
  { id: "live", name: "Live", icon: "dashboard" },
  { id: "leds", name: "LEDs", icon: "led" },
  { id: "motion", name: "Motion", icon: "motion" },
  { id: "mesh", name: "Mesh", icon: "mesh" },
  { id: "hardware", name: "Hardware", icon: "chip" },
  { id: "network", name: "Network", icon: "wifi" },
  { id: "system", name: "System", icon: "settings" },
];

/* ============ LOGO ============ */
function Logo({ size = "md", mono = false }) {
  // size: sm (header mobile), md, lg (sidebar)
  const dims = { sm: { mark: 30, font: 13, sub: 9, gap: 9, pad: 0 },
                 md: { mark: 36, font: 16, sub: 10, gap: 10, pad: 0 },
                 lg: { mark: 42, font: 18, sub: 10, gap: 12, pad: 4 } }[size];
  return (
    <div style={{ display: "flex", alignItems: "center", gap: dims.gap, padding: `${dims.pad}px 6px ${dims.pad + 12}px` }}>
      <LogoMark size={dims.mark}/>
      {!mono && (
        <div style={{ display: "flex", flexDirection: "column", lineHeight: 1.05 }}>
          <span style={{
            fontSize: dims.font, fontWeight: 600, letterSpacing: "-0.025em",
            background: "linear-gradient(180deg, var(--text-0) 0%, var(--text-1) 100%)",
            WebkitBackgroundClip: "text", WebkitTextFillColor: "transparent", backgroundClip: "text",
          }}>
            Ambi<span style={{
              background: "var(--acc-grad)", WebkitBackgroundClip: "text",
              WebkitTextFillColor: "transparent", backgroundClip: "text", fontWeight: 600,
            }}>Sense</span>
          </span>
          <span className="mono" style={{
            fontSize: dims.sub, color: "var(--text-3)", letterSpacing: "0.08em",
            textTransform: "uppercase", marginTop: 2,
          }}>v6.2.1 <span style={{ color: "var(--text-4)" }}>·</span> esp32</span>
        </div>
      )}
    </div>
  );
}

function LogoMark({ size = 36 }) {
  const live = useLive();
  // animated pulse phase from live distance
  const pulseStrength = clamp((250 - live.distance) / 200, 0.2, 1);
  return (
    <div style={{
      position: "relative", width: size, height: size, flexShrink: 0,
    }}>
      {/* outer glow */}
      <div style={{
        position: "absolute", inset: -size * 0.25, borderRadius: "50%",
        background: "radial-gradient(circle, rgba(255,122,61,0.35) 0%, rgba(255,61,130,0.15) 35%, transparent 65%)",
        filter: "blur(6px)", opacity: 0.6 * pulseStrength,
        animation: "logo-breath 2.6s ease-in-out infinite",
        pointerEvents: "none",
      }}/>
      <svg viewBox="0 0 48 48" width={size} height={size} style={{ position: "relative", display: "block" }}>
        <defs>
          <linearGradient id="lm-grad" x1="0" y1="0" x2="1" y2="1">
            <stop offset="0%" stopColor="var(--acc-amber)"/>
            <stop offset="50%" stopColor="var(--acc-orange)"/>
            <stop offset="100%" stopColor="var(--acc-pink)"/>
          </linearGradient>
          <radialGradient id="lm-core" cx="0.5" cy="0.5" r="0.5">
            <stop offset="0%" stopColor="#FFE4B0" stopOpacity="1"/>
            <stop offset="60%" stopColor="var(--acc-orange)" stopOpacity="1"/>
            <stop offset="100%" stopColor="var(--acc-pink)" stopOpacity="1"/>
          </radialGradient>
          <mask id="lm-mask">
            <rect width="48" height="48" fill="white"/>
            <path d="M 24 4 L 4 44 L 44 44 Z" fill="black"/>
          </mask>
        </defs>

        {/* Squircle/rounded-square chip body */}
        <rect x="1" y="1" width="46" height="46" rx="12" fill="#0a0c0f"
          stroke="url(#lm-grad)" strokeWidth="1" opacity="0.55"/>

        {/* Concentric pulse rings, animated */}
        <g style={{ transformOrigin: "24px 32px" }}>
          {[0, 1, 2].map(i => (
            <circle key={i} cx="24" cy="32" r="6"
              fill="none" stroke="url(#lm-grad)" strokeWidth="1.2"
              opacity="0.7"
              style={{
                animation: `logo-pulse 2.2s ease-out infinite`,
                animationDelay: `${i * 0.6}s`,
                transformOrigin: "24px 32px",
              }}/>
          ))}
        </g>

        {/* Triangle "A" mark — pointing up, the radar emitter */}
        <path d="M 24 11 L 13 32 L 35 32 Z" fill="url(#lm-grad)" opacity="0.95"/>
        {/* inner cutout to form the "A" crossbar */}
        <path d="M 24 18 L 18 30 L 30 30 Z" fill="#0a0c0f"/>

        {/* core dot at base of A */}
        <circle cx="24" cy="32" r="2.6" fill="url(#lm-core)"/>
        <circle cx="24" cy="32" r="1.1" fill="white" opacity="0.9"/>

        {/* corner ticks — chip detail */}
        {[[6,6],[42,6],[6,42],[42,42]].map(([x,y],i) => (
          <circle key={i} cx={x} cy={y} r="0.8" fill="var(--text-3)" opacity="0.5"/>
        ))}
      </svg>
      <style>{`
        @keyframes logo-pulse {
          0%   { r: 4; opacity: 0.9; stroke-width: 1.4; }
          70%  { r: 18; opacity: 0; stroke-width: 0.4; }
          100% { r: 18; opacity: 0; }
        }
        @keyframes logo-breath {
          0%, 100% { opacity: ${0.45 * pulseStrength}; transform: scale(1); }
          50%      { opacity: ${0.75 * pulseStrength}; transform: scale(1.08); }
        }
      `}</style>
    </div>
  );
}

function Header({ tab, onTabName, theme, onTheme }) {
  const live = useLive();
  const { s } = useStore();
  return (
    <header style={{
      position: "sticky", top: 0, zIndex: 50,
      display: "flex", alignItems: "center", gap: 14,
      padding: "12px 24px",
      background: "color-mix(in srgb, var(--bg-0) 88%, transparent)",
      backdropFilter: "blur(12px)",
      borderBottom: "1px solid var(--line)",
    }}>
      <div className="show-mobile" style={{
        display: "flex", alignItems: "center", gap: 10,
      }}>
      <Logo size="sm" mono/>
      </div>
      <div style={{ flex: 1, display: "flex", alignItems: "center", gap: 14 }}>
        <div style={{ display: "flex", alignItems: "baseline", gap: 8 }}>
          <span style={{ fontSize: 14, fontWeight: 600, letterSpacing: "-0.01em" }}>{onTabName}</span>
          <span className="mono hide-mobile" style={{ fontSize: 11, color: "var(--text-3)" }}>{s.hostname}.local</span>
        </div>
      </div>
      <div className="hide-mobile" style={{ display: "flex", alignItems: "center", gap: 10 }}>
        <span className="chip" title="WebSocket /api/live">
          <span className={`dot ${live.connected ? "dot-ok" : "dot-err"}`}/>
          live · {Math.round(live.distance)} cm
        </span>
        <span className="chip mono">{live.rssi} dBm</span>
      </div>
      <button className="btn btn-icon btn-ghost" onClick={() => onTheme(theme === "dark" ? "light" : "dark")}
        title={theme === "dark" ? "Light mode" : "Dark mode"}>
        <Icon name={theme === "dark" ? "sun" : "moon"} size={15}/>
      </button>
    </header>
  );
}

function Sidebar({ tab, setTab }) {
  return (
    <aside className="hide-mobile" style={{
      width: 220, flexShrink: 0,
      borderRight: "1px solid var(--line)",
      padding: "20px 14px",
      display: "flex", flexDirection: "column", gap: 4,
      background: "var(--bg-0)",
    }}>
      <Logo size="lg"/>
      {TABS.map(t => (
        <button key={t.id} onClick={() => setTab(t.id)}
          style={{
            display: "flex", alignItems: "center", gap: 10,
            padding: "9px 12px", borderRadius: 8, textAlign: "left",
            color: tab === t.id ? "var(--text-0)" : "var(--text-2)",
            background: tab === t.id ? "var(--bg-2)" : "transparent",
            border: tab === t.id ? "1px solid var(--line)" : "1px solid transparent",
            fontSize: 13, fontWeight: tab === t.id ? 500 : 400,
            transition: "background .12s, color .12s",
            cursor: "pointer",
          }}
          onMouseEnter={(e) => { if (tab !== t.id) e.currentTarget.style.background = "var(--bg-1)"; }}
          onMouseLeave={(e) => { if (tab !== t.id) e.currentTarget.style.background = "transparent"; }}>
          <Icon name={t.icon} size={15}/>
          {t.name}
        </button>
      ))}
      <div style={{ flex: 1 }}/>
      <div style={{ padding: "10px 12px", fontSize: 10, color: "var(--text-3)", lineHeight: 1.5 }}>
        <span className="mono">10.0.0.42</span><br/>
        ESP32-C3 · LD2410C
      </div>
    </aside>
  );
}

function BottomTabs({ tab, setTab }) {
  return (
    <nav className="show-mobile" style={{
      position: "fixed", bottom: 0, left: 0, right: 0, zIndex: 50,
      display: "flex",
      background: "color-mix(in srgb, var(--bg-0) 92%, transparent)",
      backdropFilter: "blur(14px)",
      borderTop: "1px solid var(--line)",
      padding: "6px 4px env(safe-area-inset-bottom)",
    }}>
      {TABS.map(t => (
        <button key={t.id} onClick={() => setTab(t.id)}
          style={{
            flex: 1, padding: "8px 2px",
            display: "flex", flexDirection: "column", alignItems: "center", gap: 2,
            color: tab === t.id ? "var(--acc-orange)" : "var(--text-3)",
            fontSize: 9, fontWeight: 500,
            cursor: "pointer",
          }}>
          <Icon name={t.icon} size={18}/>
          {t.name}
        </button>
      ))}
    </nav>
  );
}

function RebootOverlay({ open, onDone }) {
  const [count, setCount] = React.useState(30);
  React.useEffect(() => {
    if (!open) return;
    setCount(30);
    const id = setInterval(() => setCount(c => {
      if (c <= 1) { clearInterval(id); onDone(); return 0; }
      return c - 1;
    }), 1000);
    return () => clearInterval(id);
  }, [open]);
  if (!open) return null;
  return (
    <div style={{
      position: "fixed", inset: 0, zIndex: 200,
      background: "color-mix(in srgb, var(--bg-0) 85%, transparent)",
      backdropFilter: "blur(20px)",
      display: "flex", alignItems: "center", justifyContent: "center",
      animation: "fade-up .25s ease-out",
    }}>
      <div style={{ textAlign: "center", maxWidth: 340 }}>
        <div style={{ width: 64, height: 64, margin: "0 auto 18px", borderRadius: 16,
          background: "var(--acc-grad)", display: "flex", alignItems: "center", justifyContent: "center",
          color: "#1A0F08", animation: "pulse-acc 1.4s ease-in-out infinite" }}>
          <Icon name="refresh" size={28} stroke={2}/>
        </div>
        <div style={{ fontSize: 18, fontWeight: 600, marginBottom: 6 }}>Rebooting…</div>
        <div style={{ fontSize: 13, color: "var(--text-2)", marginBottom: 18 }}>
          Polling <span className="mono">/api/version</span> · {count}s remaining
        </div>
        <div style={{ height: 4, background: "var(--bg-3)", borderRadius: 999, overflow: "hidden", maxWidth: 220, margin: "0 auto" }}>
          <div style={{ height: "100%", background: "var(--acc-grad)", width: `${((30 - count) / 30) * 100}%`, transition: "width 1s linear" }}/>
        </div>
      </div>
    </div>
  );
}

/* ============ TWEAKS ============ */
const TWEAK_DEFAULTS = /*EDITMODE-BEGIN*/{
  "density": "comfortable",
  "accentHue": 22,
  "accentBlend": 100,
  "showLiveSparkline": true,
  "monoNumbers": true
}/*EDITMODE-END*/;

function TweaksUI({ theme, setTheme }) {
  const [tweaks, setTweak] = useTweaks(TWEAK_DEFAULTS);
  const [open, setOpen] = React.useState(false);
  React.useEffect(() => {
    const onActivate = (e) => { if (e.data?.type === "__activate_edit_mode") setOpen(true); };
    const onDeactivate = (e) => { if (e.data?.type === "__deactivate_edit_mode") setOpen(false); };
    window.addEventListener("message", onActivate);
    window.addEventListener("message", onDeactivate);
    window.parent.postMessage({ type: "__edit_mode_available" }, "*");
    return () => {
      window.removeEventListener("message", onActivate);
      window.removeEventListener("message", onDeactivate);
    };
  }, []);

  // apply tweaks to root
  React.useEffect(() => {
    const root = document.documentElement;
    const h = tweaks.accentHue;
    const acc1 = `oklch(0.78 0.17 ${h + 60})`;
    const acc2 = `oklch(0.68 0.22 ${h + 30})`;
    const acc3 = `oklch(0.62 0.24 ${h})`;
    root.style.setProperty("--acc-amber", acc1);
    root.style.setProperty("--acc-orange", acc2);
    root.style.setProperty("--acc-pink", acc3);
    root.style.setProperty("--acc-grad", `linear-gradient(135deg, ${acc1} 0%, ${acc2} 45%, ${acc3} 100%)`);
    root.dataset.density = tweaks.density;
  }, [tweaks.accentHue, tweaks.density]);

  if (!open) return null;
  return (
    <TweaksPanel onClose={() => setOpen(false)}>
      <TweakSection title="Theme">
        <TweakRadio label="Mode" value={theme} options={[{value:"dark",label:"Dark"},{value:"light",label:"Light"}]} onChange={setTheme}/>
        <TweakRadio label="Density" value={tweaks.density}
          options={[{value:"compact",label:"Compact"},{value:"comfortable",label:"Comfy"},{value:"spacious",label:"Spacious"}]}
          onChange={v => setTweak("density", v)}/>
      </TweakSection>
      <TweakSection title="Accent">
        <TweakSlider label="Hue" value={tweaks.accentHue} min={0} max={360} step={1}
          onChange={v => setTweak("accentHue", v)}/>
        <div style={{ height: 24, borderRadius: 6, background: "var(--acc-grad)", marginTop: 8 }}/>
      </TweakSection>
      <TweakSection title="Display">
        <TweakToggle label="Live sparkline in header" value={tweaks.showLiveSparkline} onChange={v => setTweak("showLiveSparkline", v)}/>
        <TweakToggle label="Tabular numerals" value={tweaks.monoNumbers} onChange={v => setTweak("monoNumbers", v)}/>
      </TweakSection>
    </TweaksPanel>
  );
}

/* ============ APP ============ */
function App() {
  const [tab, setTab] = React.useState("live");
  const [theme, setTheme] = React.useState("dark");
  const [reboot, setReboot] = React.useState(false);

  React.useEffect(() => {
    document.documentElement.dataset.theme = theme;
  }, [theme]);

  React.useEffect(() => {
    const onReboot = () => setReboot(true);
    window.addEventListener("ambisense:reboot", onReboot);
    return () => window.removeEventListener("ambisense:reboot", onReboot);
  }, []);

  const screen = {
    live: <ScreenLive/>,
    leds: <ScreenLEDs/>,
    motion: <ScreenMotion/>,
    mesh: <ScreenMesh/>,
    hardware: <ScreenHardware/>,
    network: <ScreenNetwork/>,
    system: <ScreenSystem/>,
  }[tab];

  const tabName = TABS.find(t => t.id === tab)?.name;

  return (
    <div data-screen-label={tabName} style={{ display: "flex", minHeight: "100vh", background: "var(--bg-0)" }}>
      <Sidebar tab={tab} setTab={setTab}/>
      <div style={{ flex: 1, minWidth: 0, display: "flex", flexDirection: "column" }}>
        <Header tab={tab} onTabName={tabName} theme={theme} onTheme={setTheme}/>
        <main style={{ flex: 1, minHeight: 0 }}>{screen}</main>
      </div>
      <BottomTabs tab={tab} setTab={setTab}/>
      <RebootOverlay open={reboot} onDone={() => setReboot(false)}/>
      <TweaksUI theme={theme} setTheme={setTheme}/>
    </div>
  );
}

const root = ReactDOM.createRoot(document.getElementById("root"));
root.render(
  <ToastHost>
    <StoreProvider>
      <LiveProvider>
        <App/>
      </LiveProvider>
    </StoreProvider>
  </ToastHost>
);
