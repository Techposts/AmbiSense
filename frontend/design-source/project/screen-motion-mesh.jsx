// AmbiSense — Motion (C) + Mesh (D) screens

function LineChart({ raw, smooth, width = 600, height = 180 }) {
  const lo = 0, hi = 300;
  const range = hi - lo;
  const toPath = (data) => data.map((v, i) => {
    const x = (i / (data.length - 1)) * width;
    const y = height - ((v - lo) / range) * height;
    return `${i === 0 ? "M" : "L"}${x.toFixed(1)},${y.toFixed(1)}`;
  }).join(" ");
  return (
    <svg width="100%" viewBox={`0 0 ${width} ${height}`} preserveAspectRatio="none" style={{ display: "block", height }}>
      {[0, 0.25, 0.5, 0.75, 1].map(t => (
        <line key={t} x1="0" y1={height * t} x2={width} y2={height * t}
          stroke="var(--line-soft)" strokeDasharray="2 4" strokeWidth="1"/>
      ))}
      <path d={toPath(raw)} stroke="var(--text-3)" strokeWidth="1" fill="none" opacity="0.7"/>
      <path d={toPath(smooth)} stroke="var(--acc-orange)" strokeWidth="2" fill="none" strokeLinecap="round"/>
    </svg>
  );
}

function ScreenMotion() {
  const { s, set } = useStore();
  const live = useLive();
  return (
    <div className="page">
      <div className="page-header">
        <div>
          <h1>Motion</h1>
          <div className="sub">Smoothing, prediction, and PI gains</div>
        </div>
      </div>

      <div className="card" style={{ padding: 18, marginBottom: 14 }}>
        <div style={{ display: "flex", justifyContent: "space-between", alignItems: "center", marginBottom: 14 }}>
          <div>
            <div style={{ fontSize: 14, fontWeight: 600 }}>Motion smoothing</div>
            <div style={{ fontSize: 12, color: "var(--text-2)" }}>Filters jitter and predicts velocity</div>
          </div>
          <Toggle large on={s.motionSmoothing} onChange={v => set({ motionSmoothing: v })}/>
        </div>

        <div style={{ position: "relative", opacity: s.motionSmoothing ? 1 : 0.45 }}>
          <div style={{ display: "flex", gap: 16, alignItems: "center", marginBottom: 8 }}>
            <span className="smallcaps">Raw vs smoothed · last 5 s</span>
            <span style={{ display: "inline-flex", alignItems: "center", gap: 6, fontSize: 11, color: "var(--text-2)" }}>
              <span style={{ width: 14, height: 1.5, background: "var(--text-3)" }}/> raw
            </span>
            <span style={{ display: "inline-flex", alignItems: "center", gap: 6, fontSize: 11, color: "var(--text-2)" }}>
              <span style={{ width: 14, height: 2, background: "var(--acc-orange)" }}/> smoothed
            </span>
            <span style={{ marginLeft: "auto" }} className="chip mono">{Math.round(live.distance)} cm</span>
          </div>
          <LineChart raw={live.rawHistory} smooth={live.smoothHistory}/>
        </div>
      </div>

      <div style={{ display: "grid", gridTemplateColumns: "repeat(auto-fit, minmax(280px, 1fr))", gap: 14 }}>
        <div className="card"><div className="card-title"><span className="smallcaps">Filter</span></div>
          <div className="card-body" style={{ display: "flex", flexDirection: "column", gap: 14 }}>
            <NumberAndSlider label="Position smoothing" value={s.positionSmoothing}
              onChange={v => set({ positionSmoothing: v }, { silent: true })} min={0} max={100} suffix="%"/>
            <NumberAndSlider label="Velocity smoothing" value={s.velocitySmoothing}
              onChange={v => set({ velocitySmoothing: v }, { silent: true })} min={0} max={100} suffix="%"/>
            <NumberAndSlider label="Prediction factor" value={s.predictionFactor}
              onChange={v => set({ predictionFactor: v }, { silent: true })} min={0} max={100} suffix="%"/>
          </div>
        </div>
        <div className="card"><div className="card-title"><span className="smallcaps">PI gains</span></div>
          <div className="card-body" style={{ display: "flex", flexDirection: "column", gap: 14 }}>
            <NumberAndSlider label="P gain" value={s.pGain}
              onChange={v => set({ pGain: v }, { silent: true })} min={0} max={100}/>
            <NumberAndSlider label="I gain" value={s.iGain}
              onChange={v => set({ iGain: v }, { silent: true })} min={0} max={100}/>
            <div style={{ padding: 10, background: "var(--bg-1)", borderRadius: 8, fontSize: 11, color: "var(--text-2)", lineHeight: 1.5 }}>
              <Icon name="info" size={12} style={{ display: "inline", verticalAlign: "-2px", marginRight: 6, color: "var(--info)" }}/>
              Higher P responds faster but overshoots. Higher I corrects steady-state offset over time.
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}

/* ============ MESH ============ */
function TopologyDiagram({ kind, size = 96 }) {
  const stroke = "var(--text-2)";
  const acc = "var(--acc-orange)";
  if (kind === "straight") {
    return (
      <svg viewBox="0 0 100 60" width="100%" style={{ height: size * 0.6 }}>
        <line x1="10" y1="30" x2="90" y2="30" stroke={stroke} strokeWidth="2"/>
        {[10, 50, 90].map((x,i) => <circle key={i} cx={x} cy="30" r="4" fill={acc}/>)}
      </svg>
    );
  }
  if (kind === "l-shape") {
    return (
      <svg viewBox="0 0 100 100" width="100%" style={{ height: size }}>
        <polyline points="20,20 20,80 80,80" stroke={stroke} strokeWidth="2" fill="none"/>
        {[[20,20],[20,50],[20,80],[50,80],[80,80]].map(([x,y],i) => <circle key={i} cx={x} cy={y} r="4" fill={acc}/>)}
      </svg>
    );
  }
  if (kind === "u-shape") {
    return (
      <svg viewBox="0 0 100 100" width="100%" style={{ height: size }}>
        <polyline points="15,20 15,80 85,80 85,20" stroke={stroke} strokeWidth="2" fill="none"/>
        {[[15,20],[15,50],[15,80],[50,80],[85,80],[85,50],[85,20]].map(([x,y],i) => <circle key={i} cx={x} cy={y} r="4" fill={acc}/>)}
      </svg>
    );
  }
  if (kind === "custom") {
    return (
      <svg viewBox="0 0 100 100" width="100%" style={{ height: size }}>
        <path d="M 15 30 Q 40 10 55 40 T 85 70" stroke={stroke} strokeWidth="2" fill="none" strokeDasharray="3 3"/>
        {[[15,30],[40,22],[55,40],[70,52],[85,70]].map(([x,y],i) => <circle key={i} cx={x} cy={y} r="4" fill={acc}/>)}
      </svg>
    );
  }
  return null;
}

function SegmentEditor() {
  const { s, set } = useStore();
  const segments = [
    { id: "master", label: "master · hall", start: 0, end: 22, color: "#FFB54A" },
    { id: "stair-mid", label: "stair-mid", start: 22, end: 42, color: "#FF7A3D" },
    { id: "stair-top", label: "stair-top", start: 42, end: 60, color: "#FF3D82" },
  ];
  const total = s.numLeds;
  return (
    <div>
      <div style={{ position: "relative", height: 56, background: "var(--bg-1)", border: "1px solid var(--line)",
        borderRadius: 10, overflow: "hidden", display: "flex" }}>
        {segments.map((seg, i) => {
          const w = ((seg.end - seg.start) / total) * 100;
          return (
            <div key={seg.id} style={{
              width: `${w}%`, position: "relative",
              background: `linear-gradient(180deg, ${seg.color}30, ${seg.color}10)`,
              borderRight: i < segments.length - 1 ? "1px solid var(--line)" : "none",
              display: "flex", flexDirection: "column", justifyContent: "center", padding: "0 10px",
            }}>
              <div style={{ fontSize: 11, color: "var(--text-1)", fontWeight: 500 }}>{seg.label}</div>
              <div className="mono" style={{ fontSize: 10, color: "var(--text-3)" }}>{seg.start}–{seg.end - 1}</div>
              {i < segments.length - 1 && (
                <div style={{
                  position: "absolute", right: -5, top: "50%", transform: "translateY(-50%)",
                  width: 10, height: 28, background: "var(--text-0)", border: "1px solid var(--text-3)",
                  borderRadius: 3, cursor: "ew-resize", zIndex: 2,
                }}/>
              )}
            </div>
          );
        })}
      </div>
      <div className="mono" style={{ display: "flex", justifyContent: "space-between", fontSize: 10, color: "var(--text-3)", marginTop: 4 }}>
        <span>px 0</span><span>px {total - 1}</span>
      </div>
    </div>
  );
}

function ScreenMesh() {
  const { s, set } = useStore();
  const live = useLive();
  const [pairing, setPairing] = React.useState(false);
  const [pairTime, setPairTime] = React.useState(0);
  const toast = useToast();
  React.useEffect(() => {
    if (!pairing) return;
    setPairTime(30);
    const t = setInterval(() => setPairTime(x => {
      if (x <= 1) { setPairing(false); return 0; }
      return x - 1;
    }), 1000);
    return () => clearInterval(t);
  }, [pairing]);

  const topologies = [
    { id: "straight", name: "Straight", desc: "Single hallway run" },
    { id: "l-shape", name: "L-shape", desc: "One corner, two flights" },
    { id: "u-shape", name: "U-shape", desc: "Two corners, three flights" },
    { id: "custom", name: "Custom", desc: "Position pixels manually" },
  ];

  const priorities = [
    { id: "most-recent", name: "Most recent", desc: "Whichever device just saw motion" },
    { id: "slave-first", name: "Slave first", desc: "Slaves win unless silent for 2 s" },
    { id: "master-first", name: "Master first", desc: "Master wins unless silent for 2 s" },
    { id: "zone-based", name: "Zone based", desc: "Each device owns its segment range" },
  ];

  return (
    <div className="page">
      <div className="page-header">
        <div>
          <h1>Mesh & Topology</h1>
          <div className="sub">{live.slaveHealth.length + 1} devices · ESP-NOW · {s.topology}</div>
        </div>
        <button className="btn btn-primary" onClick={() => setPairing(true)}>
          {pairing ? <><Icon name="link" size={13}/> Listening · {pairTime}s</> : <><Icon name="plus" size={13}/> Pair new device</>}
        </button>
      </div>

      {pairing && (
        <div className="card" style={{ padding: 14, marginBottom: 14, background: "linear-gradient(135deg, rgba(255,181,74,0.06), rgba(255,61,130,0.06))", borderColor: "rgba(255,122,61,0.35)" }}>
          <div style={{ display: "flex", alignItems: "center", gap: 14 }}>
            <div style={{ width: 32, height: 32, borderRadius: 999, background: "var(--acc-grad)", display: "flex", alignItems: "center", justifyContent: "center", color: "#1A0F08", animation: "pulse-acc 1.4s infinite" }}>
              <Icon name="link" size={16}/>
            </div>
            <div style={{ flex: 1 }}>
              <div style={{ fontSize: 13, fontWeight: 600 }}>Pairing window open · {pairTime}s</div>
              <div style={{ fontSize: 12, color: "var(--text-2)" }}>Press the button on the new device until its status LED blinks twice</div>
            </div>
            <button className="btn btn-sm" onClick={() => setPairing(false)}>Cancel</button>
          </div>
        </div>
      )}

      <div className="card" style={{ marginBottom: 14 }}>
        <div className="card-title"><span className="smallcaps">Topology</span></div>
        <div className="card-body">
          <div style={{ display: "grid", gridTemplateColumns: "repeat(auto-fit, minmax(170px, 1fr))", gap: 10 }}>
            {topologies.map(t => (
              <button key={t.id}
                onClick={() => set({ topology: t.id })}
                style={{
                  padding: 14, borderRadius: 10, textAlign: "left",
                  background: s.topology === t.id ? "linear-gradient(135deg, rgba(255,181,74,0.06), rgba(255,61,130,0.06))" : "var(--bg-1)",
                  border: s.topology === t.id ? "1px solid rgba(255,122,61,0.55)" : "1px solid var(--line)",
                  cursor: "pointer", color: "inherit",
                }}>
                <div style={{ height: 70, marginBottom: 8 }}>
                  <TopologyDiagram kind={t.id}/>
                </div>
                <div style={{ fontSize: 13, fontWeight: 600 }}>{t.name}</div>
                <div style={{ fontSize: 11, color: "var(--text-3)" }}>{t.desc}</div>
              </button>
            ))}
          </div>
        </div>
      </div>

      <div className="card" style={{ marginBottom: 14 }}>
        <div className="card-title"><span className="smallcaps">LED segment editor</span><span className="chip mono">drag handles</span></div>
        <div className="card-body"><SegmentEditor/></div>
      </div>

      <div className="card" style={{ marginBottom: 14 }}>
        <div className="card-title"><span className="smallcaps">Devices</span></div>
        <div className="card-body" style={{ display: "flex", flexDirection: "column", gap: 8 }}>
          {[
            { name: s.deviceName, mac: "A8:42:E3:9C:0F:4A", role: "master", rssi: -42, lost: 0.0, online: true },
            ...live.slaveHealth.map(sl => ({ ...sl, role: "slave" })),
          ].map(d => (
            <div key={d.mac} style={{
              display: "flex", alignItems: "center", gap: 12,
              padding: "12px 14px", background: "var(--bg-1)",
              border: "1px solid var(--line-soft)", borderRadius: 10,
            }}>
              <span className={`dot ${d.online ? "dot-ok" : "dot-err"}`}/>
              <div style={{ flex: 1, minWidth: 0 }}>
                <div style={{ display: "flex", alignItems: "center", gap: 8 }}>
                  <span style={{ fontSize: 13, fontWeight: 500 }}>{d.name}</span>
                  <span className="chip" style={{ textTransform: "capitalize" }}>{d.role}</span>
                </div>
                <div className="mono" style={{ fontSize: 11, color: "var(--text-3)" }}>{d.mac}</div>
              </div>
              <div className="mono" style={{ fontSize: 11, color: "var(--text-2)", textAlign: "right" }}>
                <div>{d.rssi} dBm</div>
                <div style={{ color: d.lost > 5 ? "var(--err)" : "var(--text-3)" }}>{d.lost?.toFixed(1) ?? "0.0"}% lost</div>
              </div>
              {d.role === "slave" && (
                <button className="btn btn-sm" onClick={() => toast.push("Blinking…")}>
                  <Icon name="flash" size={12}/> Identify
                </button>
              )}
            </div>
          ))}
        </div>
      </div>

      <div className="card">
        <div className="card-title"><span className="smallcaps">Sensor priority</span></div>
        <div className="card-body">
          <div style={{ display: "grid", gridTemplateColumns: "repeat(auto-fit, minmax(220px, 1fr))", gap: 10 }}>
            {priorities.map(p => (
              <button key={p.id}
                onClick={() => set({ sensorPriority: p.id })}
                style={{
                  padding: 14, borderRadius: 10, textAlign: "left",
                  background: s.sensorPriority === p.id ? "linear-gradient(135deg, rgba(255,181,74,0.06), rgba(255,61,130,0.06))" : "var(--bg-1)",
                  border: s.sensorPriority === p.id ? "1px solid rgba(255,122,61,0.55)" : "1px solid var(--line)",
                  cursor: "pointer", color: "inherit",
                }}>
                <div style={{ fontSize: 13, fontWeight: 600 }}>{p.name}</div>
                <div style={{ fontSize: 11, color: "var(--text-3)", marginTop: 2 }}>{p.desc}</div>
              </button>
            ))}
          </div>
        </div>
      </div>
    </div>
  );
}

Object.assign(window, { ScreenMotion, ScreenMesh });
