// AmbiSense — Live Dashboard (Screen A)

function DistanceMeter() {
  const live = useLive();
  const { s } = useStore();
  const d = live.distance;
  const inWindow = d >= s.minDistance && d <= s.maxDistance;
  return (
    <div className="card" style={{ padding: 22, position: "relative", overflow: "hidden" }}>
      <div style={{ display: "flex", justifyContent: "space-between", alignItems: "start" }}>
        <div>
          <div className="smallcaps">Distance</div>
          <div style={{ display: "flex", alignItems: "baseline", gap: 8, marginTop: 8 }}>
            <span className="mono" style={{
              fontSize: 64, fontWeight: 500, letterSpacing: "-0.04em",
              background: "var(--acc-grad)", WebkitBackgroundClip: "text",
              WebkitTextFillColor: "transparent", backgroundClip: "text",
              lineHeight: 1,
            }}>{Math.round(d)}</span>
            <span style={{ color: "var(--text-2)", fontSize: 16 }}>cm</span>
          </div>
          <div style={{ display: "flex", gap: 10, marginTop: 14, alignItems: "center" }}>
            <span className="chip" style={{ color: inWindow ? "var(--ok)" : "var(--text-2)" }}>
              <span className={`dot ${inWindow ? "dot-ok" : "dot-off"}`} />
              {inWindow ? "in window" : "outside"}
            </span>
            <span className="chip">min {s.minDistance}</span>
            <span className="chip">max {s.maxDistance}</span>
          </div>
        </div>
        <div style={{ flex: 1, marginLeft: 24, alignSelf: "end" }}>
          <div style={{ position: "relative" }}>
            <Sparkline data={live.distanceHistory} width={420} height={80} min={0} max={300}/>
            {/* min/max guides */}
            <div style={{ position: "absolute", inset: 0, pointerEvents: "none" }}>
              <div style={{
                position: "absolute", left: 0, right: 0,
                top: `${(1 - s.maxDistance / 300) * 100}%`,
                borderTop: "1px dashed var(--text-4)",
              }}/>
              <div style={{
                position: "absolute", left: 0, right: 0,
                top: `${(1 - s.minDistance / 300) * 100}%`,
                borderTop: "1px dashed var(--text-4)",
              }}/>
            </div>
          </div>
          <div className="mono" style={{ fontSize: 10, color: "var(--text-3)", textAlign: "right", marginTop: 4 }}>
            last 16 s · 5 hz
          </div>
        </div>
      </div>
    </div>
  );
}

function StripPreviewCard() {
  const { s } = useStore();
  const live = useLive();
  const dNorm = clamp((live.distance - s.minDistance) / (s.maxDistance - s.minDistance), 0, 1);
  return (
    <div className="card" style={{ padding: 18 }}>
      <div style={{ display: "flex", justifyContent: "space-between", alignItems: "center", marginBottom: 10 }}>
        <div className="smallcaps">Live LED preview</div>
        <span className="chip mono">{s.numLeds} px · {LED_MODES.find(m => m.id === s.ledMode)?.name}</span>
      </div>
      <LEDStrip mode={s.ledMode} n={s.numLeds} distance={dNorm}
        color={s.color} brightness={s.brightness / 255}
        speed={s.effectSpeed} intensity={s.effectIntensity} trail={s.trailLength}
        height={64}/>
      <div style={{ display: "flex", justifyContent: "space-between", marginTop: 8, fontSize: 11, color: "var(--text-3)" }}>
        <span className="mono">px 0</span>
        <span>{Math.round(dNorm * 100)}% along</span>
        <span className="mono">px {s.numLeds - 1}</span>
      </div>
    </div>
  );
}

function DeviceCard() {
  const { s } = useStore();
  const live = useLive();
  const fields = [
    ["Name", s.deviceName],
    ["IP", "10.0.0.42"],
    ["mDNS", `${s.hostname}.local`],
    ["RSSI", `${live.rssi} dBm`],
    ["Free heap", `${(live.heap / 1024).toFixed(1)} kB`],
    ["Uptime", fmtUptime(live.uptime)],
    ["Firmware", "v6.2.1"],
    ["Board", "ESP32-C3"],
  ];
  return (
    <div className="card">
      <div className="card-title"><span className="smallcaps">Device</span><Icon name="cpu" size={14} style={{ color: "var(--text-3)" }}/></div>
      <div className="card-body">
        <div style={{ display: "grid", gridTemplateColumns: "1fr 1fr", gap: "12px 18px" }}>
          {fields.map(([k, v]) => (
            <div key={k}>
              <div style={{ fontSize: 11, color: "var(--text-3)" }}>{k}</div>
              <div className="mono" style={{ fontSize: 13, color: "var(--text-0)" }}>{v}</div>
            </div>
          ))}
        </div>
      </div>
    </div>
  );
}

function MeshCard() {
  const { s } = useStore();
  const live = useLive();
  return (
    <div className="card">
      <div className="card-title">
        <span className="smallcaps">Mesh</span>
        <span className="chip" style={{ textTransform: "capitalize" }}>{s.role}</span>
      </div>
      <div className="card-body">
        <div style={{ fontSize: 12, color: "var(--text-2)", marginBottom: 10 }}>
          {live.slaveHealth.length} slave{live.slaveHealth.length === 1 ? "" : "s"} · topology {s.topology}
        </div>
        <div style={{ display: "flex", flexDirection: "column", gap: 6 }}>
          {live.slaveHealth.map(sl => (
            <div key={sl.mac} style={{
              display: "flex", alignItems: "center", gap: 10,
              padding: "8px 10px",
              background: "var(--bg-1)", borderRadius: "var(--r-sm)",
              border: "1px solid var(--line-soft)",
            }}>
              <span className={`dot ${sl.online ? "dot-ok" : "dot-err"}`} />
              <div style={{ flex: 1 }}>
                <div style={{ fontSize: 13 }}>{sl.name}</div>
                <div className="mono" style={{ fontSize: 10, color: "var(--text-3)" }}>{sl.mac}</div>
              </div>
              <span className="mono" style={{ fontSize: 11, color: "var(--text-2)" }}>{sl.rssi} dBm</span>
            </div>
          ))}
        </div>
      </div>
    </div>
  );
}

function SystemEnableCard() {
  const { s, set } = useStore();
  return (
    <div className="card" style={{ padding: 18, display: "flex", alignItems: "center", gap: 16,
      background: s.systemEnabled
        ? "linear-gradient(135deg, rgba(255,181,74,0.06), rgba(255,61,130,0.06))"
        : "var(--bg-2)",
      borderColor: s.systemEnabled ? "rgba(255,122,61,0.25)" : "var(--line)",
    }}>
      <div style={{
        width: 44, height: 44, borderRadius: 12,
        background: s.systemEnabled ? "var(--acc-grad)" : "var(--bg-3)",
        display: "flex", alignItems: "center", justifyContent: "center",
        color: s.systemEnabled ? "#1A0F08" : "var(--text-3)",
      }}>
        <Icon name="bolt" size={22} stroke={2}/>
      </div>
      <div style={{ flex: 1 }}>
        <div style={{ fontSize: 15, fontWeight: 600 }}>System {s.systemEnabled ? "active" : "paused"}</div>
        <div style={{ fontSize: 12, color: "var(--text-2)" }}>
          {s.systemEnabled ? "Radar, mesh, and LED output running" : "All output muted, mesh idle"}
        </div>
      </div>
      <Toggle large on={s.systemEnabled} onChange={v => set({ systemEnabled: v })}/>
    </div>
  );
}

function StatTile({ label, value, sub, accent }) {
  return (
    <div className="card" style={{ padding: 16 }}>
      <div className="smallcaps">{label}</div>
      <div className="mono" style={{
        fontSize: 22, fontWeight: 500, marginTop: 6, letterSpacing: "-0.02em",
        color: accent ? "var(--acc-orange)" : "var(--text-0)",
      }}>{value}</div>
      {sub && <div style={{ fontSize: 11, color: "var(--text-3)", marginTop: 2 }}>{sub}</div>}
    </div>
  );
}

function ScreenLive() {
  const live = useLive();
  return (
    <div className="page">
      <div className="page-header">
        <div>
          <h1>Live</h1>
          <div className="sub">Real-time radar, mesh, and LED output</div>
        </div>
        <div style={{ display: "flex", gap: 10, alignItems: "center" }}>
          <span className="chip"><span className="dot dot-ok" /> WS connected · 5 Hz</span>
        </div>
      </div>

      <div style={{ display: "grid", gridTemplateColumns: "1fr", gap: 14, marginBottom: 14 }}>
        <SystemEnableCard/>
      </div>

      <div style={{ display: "grid", gridTemplateColumns: "minmax(0, 2fr) minmax(0, 1fr)", gap: 14 }} className="dash-grid">
        <div style={{ display: "grid", gap: 14 }}>
          <DistanceMeter/>
          <StripPreviewCard/>
          <div style={{ display: "grid", gridTemplateColumns: "repeat(4, 1fr)", gap: 10 }} className="stat-row">
            <StatTile label="Free heap" value={`${(live.heap/1024).toFixed(0)}`} sub="kB · stable"/>
            <StatTile label="RSSI" value={`${live.rssi}`} sub="dBm · excellent"/>
            <StatTile label="Uptime" value={fmtUptime(live.uptime).split(" ")[0]} sub={fmtUptime(live.uptime)}/>
            <StatTile label="Cycle" value="5.04" sub="ms · radar→led" accent/>
          </div>
        </div>
        <div style={{ display: "grid", gap: 14, alignContent: "start" }}>
          <DeviceCard/>
          <MeshCard/>
        </div>
      </div>

      <style>{`
        @media (max-width: 900px) {
          .dash-grid { grid-template-columns: 1fr !important; }
          .stat-row { grid-template-columns: repeat(2, 1fr) !important; }
        }
      `}</style>
    </div>
  );
}

Object.assign(window, { ScreenLive });
