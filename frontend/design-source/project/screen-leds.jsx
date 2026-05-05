// AmbiSense — LEDs screen (Screen B)

function ColorWheel({ color, onChange, size = 200 }) {
  const cnvRef = React.useRef(null);
  const draggingRef = React.useRef(false);
  React.useEffect(() => {
    const c = cnvRef.current;
    if (!c) return;
    const ctx = c.getContext("2d");
    const dpr = window.devicePixelRatio || 1;
    c.width = size * dpr; c.height = size * dpr;
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    const r = size / 2;
    const img = ctx.createImageData(size, size);
    for (let y = 0; y < size; y++) {
      for (let x = 0; x < size; x++) {
        const dx = x - r, dy = y - r;
        const d = Math.sqrt(dx*dx + dy*dy);
        const idx = (y * size + x) * 4;
        if (d > r) { img.data[idx+3] = 0; continue; }
        const h = (Math.atan2(dy, dx) * 180 / Math.PI + 360) % 360;
        const s = Math.min(1, d / r);
        const [rr, gg, bb] = hsv2rgb(h, s, 1);
        img.data[idx] = rr; img.data[idx+1] = gg; img.data[idx+2] = bb; img.data[idx+3] = 255;
      }
    }
    ctx.putImageData(img, 0, 0);
  }, [size]);
  const r = size / 2;
  const px = r + Math.cos(color.h * Math.PI/180) * color.s * r;
  const py = r + Math.sin(color.h * Math.PI/180) * color.s * r;
  const handle = (e) => {
    const rect = cnvRef.current.getBoundingClientRect();
    const cx = e.clientX - rect.left;
    const cy = e.clientY - rect.top;
    const dx = cx - r, dy = cy - r;
    const d = Math.sqrt(dx*dx + dy*dy);
    const h = (Math.atan2(dy, dx) * 180 / Math.PI + 360) % 360;
    const s = Math.min(1, d / r);
    onChange({ ...color, h, s });
  };
  return (
    <div style={{ position: "relative", width: size, height: size }}
      onMouseDown={(e) => { draggingRef.current = true; handle(e); }}
      onMouseMove={(e) => { if (draggingRef.current) handle(e); }}
      onMouseUp={() => { draggingRef.current = false; }}
      onMouseLeave={() => { draggingRef.current = false; }}>
      <canvas ref={cnvRef} style={{ width: size, height: size, borderRadius: "50%", display: "block" }}/>
      <div style={{
        position: "absolute", left: px - 9, top: py - 9, width: 18, height: 18,
        borderRadius: "50%", border: "3px solid white", boxShadow: "0 0 0 1px rgba(0,0,0,0.4)",
        background: hsv2hex(color.h, color.s, 1), pointerEvents: "none",
      }}/>
    </div>
  );
}

function ColorPicker() {
  const { s, set } = useStore();
  const hex = hsv2hex(s.color.h, s.color.s, s.color.v);
  const [hexInput, setHexInput] = React.useState(hex);
  React.useEffect(() => setHexInput(hex), [hex]);
  const submitHex = (val) => {
    const m = val.replace(/[^0-9a-f]/gi, "").slice(0, 6).padEnd(6, "0");
    const [r, g, b] = hex2rgb("#" + m);
    const hsv = rgb2hsv(r, g, b);
    set({ color: hsv });
  };
  return (
    <div style={{ display: "flex", gap: 22, flexWrap: "wrap" }}>
      <ColorWheel color={s.color} onChange={c => set({ color: c }, { silent: true })} size={180}/>
      <div style={{ flex: 1, minWidth: 180, display: "flex", flexDirection: "column", gap: 12 }}>
        <div>
          <span className="field-label">Brightness</span>
          <Slider value={s.color.v * 100} onChange={v => set({ color: { ...s.color, v: v/100 } }, { silent: true })} min={0} max={100} suffix="%"/>
        </div>
        <div>
          <span className="field-label">Hex</span>
          <div style={{ display: "flex", gap: 8 }}>
            <div style={{ width: 36, height: 36, borderRadius: 8, background: hex, border: "1px solid var(--line)" }}/>
            <input className="input mono" value={hexInput}
              onChange={e => setHexInput(e.target.value)}
              onBlur={() => { submitHex(hexInput); }}
              onKeyDown={e => { if (e.key === "Enter") e.target.blur(); }}/>
          </div>
        </div>
        <div>
          <span className="field-label">Presets</span>
          <div style={{ display: "grid", gridTemplateColumns: "repeat(8, 1fr)", gap: 6 }}>
            {s.presets.map((p, i) => (
              <button key={i} onClick={() => submitHex(p)}
                style={{
                  aspectRatio: "1 / 1", borderRadius: 6, background: p,
                  border: hex.toLowerCase() === p.toLowerCase() ? "2px solid var(--text-0)" : "1px solid var(--line)",
                  cursor: "pointer", padding: 0,
                }} title={p}/>
            ))}
          </div>
        </div>
      </div>
    </div>
  );
}

function ModeCard({ mode, active, onClick }) {
  const { s } = useStore();
  return (
    <button onClick={onClick}
      style={{
        textAlign: "left", padding: 0, width: "100%",
        background: active ? "linear-gradient(135deg, rgba(255,181,74,0.08), rgba(255,61,130,0.08))" : "var(--bg-2)",
        border: active ? "1px solid rgba(255,122,61,0.55)" : "1px solid var(--line)",
        borderRadius: "var(--r-md)",
        overflow: "hidden",
        transition: "border-color .15s, transform .08s",
        cursor: "pointer",
      }}
      onMouseEnter={(e) => { if (!active) e.currentTarget.style.borderColor = "var(--text-4)"; }}
      onMouseLeave={(e) => { if (!active) e.currentTarget.style.borderColor = "var(--line)"; }}>
      <div style={{ position: "relative" }}>
        <LEDStrip mode={mode.id} n={28} distance={0.55} color={s.color} brightness={0.85}
          speed={s.effectSpeed} intensity={s.effectIntensity} trail={s.trailLength} height={42}/>
        {active && (
          <div style={{ position: "absolute", top: 6, right: 6, width: 20, height: 20, borderRadius: 999,
            background: "var(--acc-grad)", display: "flex", alignItems: "center", justifyContent: "center", color: "#1A0F08" }}>
            <Icon name="check" size={12} stroke={3}/>
          </div>
        )}
      </div>
      <div style={{ padding: "10px 12px 12px" }}>
        <div style={{ fontSize: 13, fontWeight: 600, color: active ? "var(--text-0)" : "var(--text-1)" }}>{mode.name}</div>
        <div style={{ fontSize: 11, color: "var(--text-3)", marginTop: 2, lineHeight: 1.4 }}>{mode.desc}</div>
      </div>
    </button>
  );
}

function DualHandleRange({ minVal, maxVal, onChange, min = 0, max = 300 }) {
  const ref = React.useRef(null);
  const [drag, setDrag] = React.useState(null);
  const handle = (e) => {
    if (!drag || !ref.current) return;
    const rect = ref.current.getBoundingClientRect();
    const t = clamp((e.clientX - rect.left) / rect.width, 0, 1);
    const v = Math.round(min + t * (max - min));
    if (drag === "min") onChange({ minVal: Math.min(v, maxVal - 5), maxVal });
    else onChange({ minVal, maxVal: Math.max(v, minVal + 5) });
  };
  React.useEffect(() => {
    if (!drag) return;
    const up = () => setDrag(null);
    window.addEventListener("mousemove", handle);
    window.addEventListener("mouseup", up);
    return () => {
      window.removeEventListener("mousemove", handle);
      window.removeEventListener("mouseup", up);
    };
  }, [drag, minVal, maxVal]);
  const tMin = (minVal - min) / (max - min);
  const tMax = (maxVal - min) / (max - min);
  return (
    <div ref={ref} style={{ position: "relative", height: 28, padding: "12px 0", cursor: "pointer" }}>
      <div style={{ position: "absolute", left: 0, right: 0, top: "50%", height: 4, background: "var(--bg-3)", borderRadius: 999 }}/>
      <div style={{ position: "absolute", top: "50%", height: 4, transform: "translateY(-2px)",
        left: `${tMin * 100}%`, width: `${(tMax - tMin) * 100}%`,
        background: "var(--acc-grad)", borderRadius: 999 }}/>
      {[["min", tMin, minVal], ["max", tMax, maxVal]].map(([k, t, v]) => (
        <div key={k}
          onMouseDown={() => setDrag(k)}
          style={{
            position: "absolute", top: "50%", left: `calc(${t * 100}% - 9px)`, transform: "translateY(-50%)",
            width: 18, height: 18, borderRadius: "50%",
            background: "var(--text-0)", border: "3px solid var(--acc-orange)",
            cursor: "grab", boxShadow: "var(--shadow-1)",
            display: "flex", alignItems: "center", justifyContent: "center",
          }}>
          <span style={{
            position: "absolute", top: -28, left: "50%", transform: "translateX(-50%)",
            background: "var(--bg-3)", border: "1px solid var(--line)", borderRadius: 4,
            padding: "1px 6px", fontSize: 10, fontFamily: "var(--font-mono)", color: "var(--text-1)",
            whiteSpace: "nowrap", opacity: drag === k ? 1 : 0, transition: "opacity .12s",
          }}>{v} cm</span>
        </div>
      ))}
    </div>
  );
}

function NumberAndSlider({ label, value, onChange, min, max, suffix = "" }) {
  return (
    <div>
      <div style={{ display: "flex", justifyContent: "space-between", marginBottom: 6 }}>
        <span className="field-label" style={{ marginBottom: 0 }}>{label}</span>
        <input
          type="number" value={value} min={min} max={max}
          onChange={e => onChange(clamp(Number(e.target.value), min, max))}
          style={{
            width: 70, background: "var(--bg-1)", border: "1px solid var(--line)",
            borderRadius: 6, padding: "2px 6px", fontFamily: "var(--font-mono)", fontSize: 12,
            color: "var(--text-0)", textAlign: "right", outline: "none",
          }}/>
      </div>
      <input type="range" className="range" value={value} min={min} max={max}
        onChange={e => onChange(Number(e.target.value))}/>
    </div>
  );
}

function ScreenLEDs() {
  const { s, set } = useStore();
  const live = useLive();
  const dNorm = clamp((live.distance - s.minDistance) / (s.maxDistance - s.minDistance), 0, 1);
  const currentMode = LED_MODES.find(m => m.id === s.ledMode);
  const showColorPicker = ["standard", "solid", "color-wave", "comet", "pulse", "breathing", "dual-scan", "particles"].includes(s.ledMode);
  const showSpeed = !["solid"].includes(s.ledMode);
  const showTrail = ["standard", "comet"].includes(s.ledMode);
  const showDirection = s.ledMode === "standard";

  return (
    <div className="page">
      <div className="page-header">
        <div>
          <h1>LEDs</h1>
          <div className="sub">{currentMode?.name} · {s.numLeds} pixels · {s.minDistance}–{s.maxDistance} cm</div>
        </div>
        <div style={{ display: "flex", gap: 10 }}>
          <button className="btn"><Icon name="copy" size={13}/> Save preset</button>
        </div>
      </div>

      <div className="card" style={{ padding: 18, marginBottom: 14 }}>
        <div style={{ display: "flex", justifyContent: "space-between", alignItems: "center", marginBottom: 10 }}>
          <span className="smallcaps">Live preview · {currentMode?.name}</span>
          <span className="chip mono">distance {Math.round(live.distance)} cm</span>
        </div>
        <LEDStrip mode={s.ledMode} n={s.numLeds} distance={dNorm}
          color={s.color} brightness={s.brightness/255}
          speed={s.effectSpeed} intensity={s.effectIntensity} trail={s.trailLength}
          height={72}/>
      </div>

      <div className="led-grid" style={{ display: "grid", gridTemplateColumns: "minmax(0, 1.4fr) minmax(0, 1fr)", gap: 14 }}>
        <div className="card">
          <div className="card-title"><span className="smallcaps">Mode</span></div>
          <div className="card-body">
            <div style={{ display: "grid", gridTemplateColumns: "repeat(auto-fill, minmax(155px, 1fr))", gap: 10 }}>
              {LED_MODES.map(m => (
                <ModeCard key={m.id} mode={m} active={s.ledMode === m.id} onClick={() => set({ ledMode: m.id })}/>
              ))}
            </div>
          </div>
        </div>

        <div style={{ display: "grid", gap: 14, alignContent: "start" }}>
          {showColorPicker && (
            <div className="card">
              <div className="card-title"><span className="smallcaps">Color</span><Icon name="palette" size={13} style={{ color: "var(--text-3)" }}/></div>
              <div className="card-body"><ColorPicker/></div>
            </div>
          )}

          <div className="card">
            <div className="card-title"><span className="smallcaps">{currentMode?.name} parameters</span></div>
            <div className="card-body" style={{ display: "flex", flexDirection: "column", gap: 14 }}>
              <NumberAndSlider label="Brightness" value={s.brightness}
                onChange={v => set({ brightness: v }, { silent: true })} min={0} max={255}/>
              {showSpeed && (
                <NumberAndSlider label="Effect speed" value={s.effectSpeed}
                  onChange={v => set({ effectSpeed: v }, { silent: true })} min={0} max={100} suffix="%"/>
              )}
              {!["solid","standard"].includes(s.ledMode) && (
                <NumberAndSlider label="Effect intensity" value={s.effectIntensity}
                  onChange={v => set({ effectIntensity: v }, { silent: true })} min={0} max={100} suffix="%"/>
              )}
              {showTrail && (
                <NumberAndSlider label="Trail length" value={s.trailLength}
                  onChange={v => set({ trailLength: v }, { silent: true })} min={1} max={20} suffix=" px"/>
              )}
              {showDirection && (
                <div style={{ display: "flex", justifyContent: "space-between", alignItems: "center" }}>
                  <div>
                    <div style={{ fontSize: 13 }}>Directional light</div>
                    <div style={{ fontSize: 11, color: "var(--text-3)" }}>Brighter side leads movement</div>
                  </div>
                  <Toggle on={s.directionalLight} onChange={v => set({ directionalLight: v })}/>
                </div>
              )}
              <div style={{ display: "flex", justifyContent: "space-between", alignItems: "center" }}>
                <div>
                  <div style={{ fontSize: 13 }}>Background mode</div>
                  <div style={{ fontSize: 11, color: "var(--text-3)" }}>Faint always-on color when idle</div>
                </div>
                <Toggle on={s.backgroundMode} onChange={v => set({ backgroundMode: v })}/>
              </div>
            </div>
          </div>

          <div className="card">
            <div className="card-title"><span className="smallcaps">Layout</span></div>
            <div className="card-body" style={{ display: "flex", flexDirection: "column", gap: 14 }}>
              <div>
                <div style={{ display: "flex", justifyContent: "space-between", marginBottom: 6 }}>
                  <span className="field-label" style={{ marginBottom: 0 }}>Distance window</span>
                  <span className="mono" style={{ fontSize: 12, color: "var(--text-1)" }}>{s.minDistance}–{s.maxDistance} cm</span>
                </div>
                <DualHandleRange minVal={s.minDistance} maxVal={s.maxDistance}
                  onChange={({ minVal, maxVal }) => set({ minDistance: minVal, maxDistance: maxVal }, { silent: true })}/>
              </div>
              <NumberAndSlider label="Light span" value={s.lightSpan}
                onChange={v => set({ lightSpan: v }, { silent: true })} min={1} max={60} suffix=" px"/>
              <NumberAndSlider label="Center shift" value={s.centerShift}
                onChange={v => set({ centerShift: v }, { silent: true })} min={-30} max={30} suffix=" px"/>
              <NumberAndSlider label="Strip length" value={s.numLeds}
                onChange={v => set({ numLeds: v }, { silent: true })} min={10} max={300} suffix=" px"/>
            </div>
          </div>
        </div>
      </div>

      <style>{`
        @media (max-width: 980px) {
          .led-grid { grid-template-columns: 1fr !important; }
        }
      `}</style>
    </div>
  );
}

Object.assign(window, { ScreenLEDs });
