// AmbiSense — Hardware (E) + Network (F) + System (G) screens

const BOARD_PROFILES = {
  "esp32-c3-supermini": {
    name: "ESP32-C3 SuperMini",
    sub: "RISC-V · 4 MB flash",
    pins: { ledData: 4, radarRx: 20, radarTx: 21, button: 9, statusLed: 8 },
    valid: [0,1,2,3,4,5,6,7,8,9,10,20,21],
    unsafe: { 8: "Strapping pin (boot mode select)", 9: "Strapping pin (boot mode select)" },
  },
  "esp32-s3-zero": {
    name: "ESP32-S3 Zero",
    sub: "Xtensa LX7 · USB-CDC",
    pins: { ledData: 21, radarRx: 17, radarTx: 18, button: 0, statusLed: 48 },
    valid: [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,21,33,34,35,36,37,38,39,40,41,42,47,48],
    unsafe: { 0: "Strapping pin", 45: "Strapping pin", 46: "Strapping pin" },
  },
  "esp32-devkit": {
    name: "ESP32 DevKit",
    sub: "Original ESP32 · 30-pin",
    pins: { ledData: 5, radarRx: 16, radarTx: 17, button: 0, statusLed: 2 },
    valid: [0,2,4,5,12,13,14,15,16,17,18,19,21,22,23,25,26,27,32,33],
    unsafe: { 0: "Strapping pin", 12: "Strapping pin (MTDI)", 15: "Strapping pin" },
  },
  "custom": { name: "Custom", sub: "Pick your own pins",
    pins: { ledData: 4, radarRx: 20, radarTx: 21, button: 9, statusLed: 8 },
    valid: Array.from({ length: 49 }, (_, i) => i), unsafe: {},
  },
};

function PinSelect({ value, onChange, profile }) {
  return (
    <div style={{ position: "relative" }}>
      <select className="select mono" value={value} onChange={e => onChange(Number(e.target.value))}>
        {profile.valid.map(p => (
          <option key={p} value={p} disabled={!!profile.unsafe[p]}>
            GPIO {p}{profile.unsafe[p] ? " · unsafe" : ""}
          </option>
        ))}
      </select>
    </div>
  );
}

function ScreenHardware() {
  const { s, set } = useStore();
  const profile = BOARD_PROFILES[s.boardProfile];
  const [needsReboot, setNeedsReboot] = React.useState(false);
  const onProfileChange = (id) => {
    set({ boardProfile: id, pins: BOARD_PROFILES[id].pins });
    setNeedsReboot(true);
  };

  return (
    <div className="page">
      <div className="page-header">
        <div>
          <h1>Hardware</h1>
          <div className="sub">Board profile, radar, and pin assignments</div>
        </div>
        {needsReboot && (
          <button className="btn btn-primary" onClick={() => { setNeedsReboot(false); window.dispatchEvent(new CustomEvent("ambisense:reboot")); }}>
            <Icon name="refresh" size={13}/> Reboot to apply
          </button>
        )}
      </div>

      <div style={{ display: "grid", gridTemplateColumns: "1fr 1fr", gap: 14, marginBottom: 14 }} className="hw-grid">
        <div className="card">
          <div className="card-title"><span className="smallcaps">Board profile</span><Icon name="chip" size={13} style={{ color: "var(--text-3)" }}/></div>
          <div className="card-body">
            <div style={{ display: "flex", flexDirection: "column", gap: 6 }}>
              {Object.entries(BOARD_PROFILES).map(([id, p]) => (
                <button key={id} onClick={() => onProfileChange(id)}
                  style={{
                    display: "flex", alignItems: "center", gap: 12,
                    padding: "12px 14px", borderRadius: 10, textAlign: "left",
                    background: s.boardProfile === id ? "linear-gradient(135deg, rgba(255,181,74,0.06), rgba(255,61,130,0.06))" : "var(--bg-1)",
                    border: s.boardProfile === id ? "1px solid rgba(255,122,61,0.55)" : "1px solid var(--line)",
                    cursor: "pointer", color: "inherit",
                  }}>
                  <div style={{ width: 32, height: 32, borderRadius: 8, background: "var(--bg-3)", display: "flex", alignItems: "center", justifyContent: "center", color: "var(--text-2)" }}>
                    <Icon name="cpu" size={16}/>
                  </div>
                  <div style={{ flex: 1 }}>
                    <div style={{ fontSize: 13, fontWeight: 500 }}>{p.name}</div>
                    <div style={{ fontSize: 11, color: "var(--text-3)" }}>{p.sub}</div>
                  </div>
                  {s.boardProfile === id && <Icon name="check" size={14} style={{ color: "var(--acc-orange)" }}/>}
                </button>
              ))}
            </div>
          </div>
        </div>

        <div className="card">
          <div className="card-title"><span className="smallcaps">Radar</span><Icon name="radar" size={13} style={{ color: "var(--text-3)" }}/></div>
          <div className="card-body" style={{ display: "flex", flexDirection: "column", gap: 10 }}>
            {[
              { id: "ld2410c", name: "LD2410C", desc: "1-D distance · low cost · 6 m range" },
              { id: "ld2450", name: "LD2450", desc: "2-D zone tracking · up to 3 targets" },
            ].map(r => (
              <button key={r.id} onClick={() => { set({ radar: r.id }); setNeedsReboot(true); }}
                style={{
                  display: "flex", alignItems: "center", gap: 12,
                  padding: "14px", borderRadius: 10, textAlign: "left",
                  background: s.radar === r.id ? "linear-gradient(135deg, rgba(255,181,74,0.06), rgba(255,61,130,0.06))" : "var(--bg-1)",
                  border: s.radar === r.id ? "1px solid rgba(255,122,61,0.55)" : "1px solid var(--line)",
                  cursor: "pointer", color: "inherit",
                }}>
                <div style={{ flex: 1 }}>
                  <div style={{ fontSize: 13, fontWeight: 600 }}>{r.name}</div>
                  <div style={{ fontSize: 11, color: "var(--text-3)" }}>{r.desc}</div>
                </div>
                {s.radar === r.id && <Icon name="check" size={14} style={{ color: "var(--acc-orange)" }}/>}
              </button>
            ))}
          </div>
        </div>
      </div>

      <div className="card">
        <div className="card-title"><span className="smallcaps">Pin map</span><span className="chip mono">{profile.name}</span></div>
        <div className="card-body">
          <div style={{ display: "grid", gridTemplateColumns: "repeat(auto-fit, minmax(180px, 1fr))", gap: 14 }}>
            {[
              ["ledData", "LED data", "led"],
              ["radarRx", "Radar RX", "arrowRight"],
              ["radarTx", "Radar TX", "arrowRight"],
              ["button", "Button", "pin"],
              ["statusLed", "Status LED", "bolt"],
            ].map(([k, label, icon]) => (
              <div key={k}>
                <div style={{ display: "flex", alignItems: "center", gap: 6, marginBottom: 6 }}>
                  <Icon name={icon} size={12} style={{ color: "var(--text-3)" }}/>
                  <span className="field-label" style={{ marginBottom: 0 }}>{label}</span>
                </div>
                <PinSelect value={s.pins[k]} profile={profile}
                  onChange={v => { set({ pins: { ...s.pins, [k]: v } }); setNeedsReboot(true); }}/>
              </div>
            ))}
          </div>
          <div style={{ marginTop: 14, padding: 10, background: "var(--bg-1)", borderRadius: 8, fontSize: 11, color: "var(--text-2)", display: "flex", gap: 8 }}>
            <Icon name="info" size={13} style={{ color: "var(--info)", flexShrink: 0, marginTop: 1 }}/>
            <span>Strapping pins are disabled — they affect boot mode and shouldn't drive an LED strip or radar UART.</span>
          </div>
        </div>
      </div>

      <style>{`@media (max-width: 760px) { .hw-grid { grid-template-columns: 1fr !important; } }`}</style>
    </div>
  );
}

/* ============ NETWORK ============ */
function ScreenNetwork() {
  const { s, set } = useStore();
  const live = useLive();
  const networks = [
    { ssid: "Loft 5GHz", rssi: -52, secured: true, current: true },
    { ssid: "Loft 2.4GHz", rssi: -58, secured: true },
    { ssid: "FRITZ!Box 7530", rssi: -71, secured: true },
    { ssid: "neighbor-iot", rssi: -78, secured: false },
    { ssid: "JOSE-WIFI-EXT", rssi: -82, secured: true },
  ];
  const [confirm, setConfirm] = React.useState(false);
  const toast = useToast();

  return (
    <div className="page">
      <div className="page-header">
        <div>
          <h1>Network</h1>
          <div className="sub">Wi-Fi, mDNS, and static IP</div>
        </div>
      </div>

      <div className="card" style={{ padding: 18, marginBottom: 14, background: "linear-gradient(135deg, rgba(91,199,255,0.04), transparent)" }}>
        <div style={{ display: "flex", alignItems: "center", gap: 16 }}>
          <div style={{ width: 44, height: 44, borderRadius: 12, background: "var(--bg-3)", display: "flex", alignItems: "center", justifyContent: "center", color: "var(--info)" }}>
            <Icon name="wifi" size={22}/>
          </div>
          <div style={{ flex: 1 }}>
            <div style={{ display: "flex", alignItems: "center", gap: 8 }}>
              <span style={{ fontSize: 15, fontWeight: 600 }}>{s.ssid}</span>
              <span className="chip"><span className="dot dot-ok"/> connected</span>
            </div>
            <div className="mono" style={{ fontSize: 11, color: "var(--text-3)", marginTop: 2 }}>
              {s.ip} · gw {s.gateway} · {live.rssi} dBm
            </div>
          </div>
          <button className="btn">Disconnect</button>
        </div>
      </div>

      <div className="card" style={{ marginBottom: 14 }}>
        <div className="card-title"><span className="smallcaps">Available networks</span>
          <button className="btn btn-sm"><Icon name="refresh" size={11}/> Scan</button>
        </div>
        <div className="card-body" style={{ display: "flex", flexDirection: "column", gap: 4 }}>
          {networks.map(n => {
            const bars = n.rssi > -60 ? 4 : n.rssi > -68 ? 3 : n.rssi > -76 ? 2 : 1;
            return (
              <div key={n.ssid} style={{
                display: "flex", alignItems: "center", gap: 12,
                padding: "10px 12px", borderRadius: 8,
                background: n.current ? "var(--bg-1)" : "transparent",
                border: n.current ? "1px solid var(--line)" : "1px solid transparent",
              }}>
                <div style={{ display: "flex", alignItems: "end", gap: 1.5, height: 16 }}>
                  {[1,2,3,4].map(i => (
                    <div key={i} style={{
                      width: 3, height: i * 4,
                      background: i <= bars ? "var(--text-1)" : "var(--bg-4)",
                      borderRadius: 1,
                    }}/>
                  ))}
                </div>
                <div style={{ flex: 1 }}>
                  <div style={{ fontSize: 13 }}>{n.ssid}</div>
                  <div className="mono" style={{ fontSize: 11, color: "var(--text-3)" }}>
                    {n.rssi} dBm · {n.secured ? "WPA2" : "open"}
                  </div>
                </div>
                {n.current ? <span className="chip">current</span>
                  : <button className="btn btn-sm">Join</button>}
              </div>
            );
          })}
        </div>
      </div>

      <div className="hw-grid" style={{ display: "grid", gridTemplateColumns: "1fr 1fr", gap: 14 }}>
        <div className="card">
          <div className="card-title"><span className="smallcaps">Hostname</span></div>
          <div className="card-body">
            <span className="field-label">mDNS name</span>
            <div style={{ display: "flex", gap: 6, alignItems: "center" }}>
              <input className="input mono" value={s.hostname}
                onChange={e => set({ hostname: e.target.value.replace(/[^a-z0-9-]/g, "") }, { silent: true })}
                onBlur={() => set({ hostname: s.hostname })}/>
              <span className="mono" style={{ fontSize: 12, color: "var(--text-3)" }}>.local</span>
            </div>
          </div>
        </div>

        <div className="card">
          <div className="card-title">
            <span className="smallcaps">Static IP</span>
            <Toggle on={s.staticIp} onChange={v => set({ staticIp: v })}/>
          </div>
          <div className="card-body" style={{ opacity: s.staticIp ? 1 : 0.4, pointerEvents: s.staticIp ? "auto" : "none" }}>
            <div style={{ display: "grid", gridTemplateColumns: "1fr 1fr", gap: 10 }}>
              <div>
                <span className="field-label">Address</span>
                <input className="input mono" value={s.ip} onChange={e => set({ ip: e.target.value }, { silent: true })}/>
              </div>
              <div>
                <span className="field-label">Gateway</span>
                <input className="input mono" value={s.gateway} onChange={e => set({ gateway: e.target.value }, { silent: true })}/>
              </div>
              <div style={{ gridColumn: "1 / -1" }}>
                <span className="field-label">Netmask</span>
                <input className="input mono" value={s.netmask} onChange={e => set({ netmask: e.target.value }, { silent: true })}/>
              </div>
            </div>
          </div>
        </div>
      </div>

      <div className="card" style={{ marginTop: 14, padding: 16 }}>
        <div style={{ display: "flex", alignItems: "center", gap: 14 }}>
          <Icon name="warn" size={18} style={{ color: "var(--err)" }}/>
          <div style={{ flex: 1 }}>
            <div style={{ fontSize: 13, fontWeight: 600 }}>Reset Wi-Fi</div>
            <div style={{ fontSize: 12, color: "var(--text-2)" }}>Forgets credentials and reboots into setup AP</div>
          </div>
          {confirm
            ? <div style={{ display: "flex", gap: 6 }}>
                <button className="btn btn-sm" onClick={() => setConfirm(false)}>Cancel</button>
                <button className="btn btn-sm btn-danger" onClick={() => { setConfirm(false); toast.push("Resetting…"); }}>Confirm reset</button>
              </div>
            : <button className="btn btn-danger" onClick={() => setConfirm(true)}>Reset Wi-Fi</button>}
        </div>
      </div>

      <style>{`@media (max-width: 760px) { .hw-grid { grid-template-columns: 1fr !important; } }`}</style>
    </div>
  );
}

/* ============ SYSTEM ============ */
function ScreenSystem() {
  const { s, set } = useStore();
  const live = useLive();
  const toast = useToast();
  const [showPwd, setShowPwd] = React.useState(false);
  const [confirmText, setConfirmText] = React.useState("");
  const [otaName, setOtaName] = React.useState(null);
  const [checking, setChecking] = React.useState(false);

  const checkUpdates = () => {
    setChecking(true);
    setTimeout(() => { setChecking(false); toast.push("Up to date"); }, 1200);
  };

  return (
    <div className="page">
      <div className="page-header">
        <div>
          <h1>System</h1>
          <div className="sub">Firmware, auth, and diagnostics</div>
        </div>
      </div>

      <div className="hw-grid" style={{ display: "grid", gridTemplateColumns: "1.2fr 1fr", gap: 14, marginBottom: 14 }}>
        <div className="card">
          <div className="card-title"><span className="smallcaps">Firmware</span><span className="chip mono">v6.2.1</span></div>
          <div className="card-body" style={{ display: "flex", flexDirection: "column", gap: 12 }}>
            <div style={{ display: "flex", alignItems: "center", gap: 12, padding: "12px 14px", background: "var(--bg-1)", borderRadius: 10 }}>
              <Icon name="box" size={18} style={{ color: "var(--text-2)" }}/>
              <div style={{ flex: 1 }}>
                <div className="mono" style={{ fontSize: 13 }}>v6.2.1 <span style={{ color: "var(--text-3)" }}>· build a8f3</span></div>
                <div style={{ fontSize: 11, color: "var(--text-3)" }}>ESP-IDF 5.1.4 · checked 2 min ago</div>
              </div>
              <button className="btn btn-sm" disabled={checking} onClick={checkUpdates}>
                {checking ? <Icon name="refresh" size={12} style={{ animation: "spin-slow 1s linear infinite" }}/> : <Icon name="refresh" size={12}/>}
                {checking ? "Checking…" : "Check"}
              </button>
            </div>

            <label
              onDrop={(e) => { e.preventDefault(); const f = e.dataTransfer.files[0]; if (f) setOtaName(f.name); }}
              onDragOver={e => e.preventDefault()}
              style={{
                display: "flex", flexDirection: "column", alignItems: "center", justifyContent: "center",
                gap: 6, padding: 24, border: "1.5px dashed var(--line)", borderRadius: 10,
                background: "var(--bg-1)", cursor: "pointer", textAlign: "center",
              }}>
              <Icon name="upload" size={20} style={{ color: "var(--text-2)" }}/>
              <div style={{ fontSize: 13 }}>{otaName || "Drop firmware .bin or click to select"}</div>
              <div style={{ fontSize: 11, color: "var(--text-3)" }}>Signed builds verified before flashing</div>
              <input type="file" accept=".bin" style={{ display: "none" }} onChange={e => setOtaName(e.target.files[0]?.name)}/>
            </label>
            {otaName && (
              <button className="btn btn-primary" style={{ alignSelf: "stretch", justifyContent: "center" }}
                onClick={() => { window.dispatchEvent(new CustomEvent("ambisense:reboot")); setOtaName(null); }}>
                Flash & reboot
              </button>
            )}
          </div>
        </div>

        <div className="card">
          <div className="card-title"><span className="smallcaps">Diagnostics</span></div>
          <div className="card-body">
            <div style={{ display: "grid", gridTemplateColumns: "1fr 1fr", gap: "12px 18px" }}>
              {[
                ["Free heap", `${(live.heap/1024).toFixed(1)} kB`],
                ["Min free heap", `${(118).toFixed(1)} kB`],
                ["Fragmentation", "14%"],
                ["Uptime", fmtUptime(live.uptime)],
                ["Reset reason", "POWERON_RESET"],
                ["CPU temp", "47.2 °C"],
              ].map(([k, v]) => (
                <div key={k}>
                  <div style={{ fontSize: 11, color: "var(--text-3)" }}>{k}</div>
                  <div className="mono" style={{ fontSize: 13 }}>{v}</div>
                </div>
              ))}
            </div>
          </div>
        </div>
      </div>

      <div className="card" style={{ marginBottom: 14 }}>
        <div className="card-title">
          <span className="smallcaps">Auth</span>
          <Toggle on={s.authRequired} onChange={v => set({ authRequired: v })}/>
        </div>
        <div className="card-body" style={{ opacity: s.authRequired ? 1 : 0.4, pointerEvents: s.authRequired ? "auto" : "none" }}>
          <span className="field-label">Password</span>
          <div style={{ display: "flex", gap: 6 }}>
            <input className="input mono" type={showPwd ? "text" : "password"} value={s.authPassword}
              placeholder="Set a password"
              onChange={e => set({ authPassword: e.target.value }, { silent: true })}/>
            <button className="btn btn-icon" onClick={() => setShowPwd(x => !x)}>
              <Icon name={showPwd ? "eyeOff" : "eye"} size={14}/>
            </button>
          </div>
          <div style={{ fontSize: 11, color: "var(--text-3)", marginTop: 6 }}>
            Required only on this network. Local mDNS access is always password-protected.
          </div>
        </div>
      </div>

      <div className="hw-grid" style={{ display: "grid", gridTemplateColumns: "1fr 1fr", gap: 14 }}>
        <div className="card">
          <div className="card-title"><span className="smallcaps">JSON config</span></div>
          <div className="card-body" style={{ display: "flex", gap: 8 }}>
            <button className="btn" onClick={() => toast.push("Exported")}>
              <Icon name="download" size={13}/> Export
            </button>
            <button className="btn">
              <Icon name="upload" size={13}/> Import
            </button>
          </div>
        </div>

        <div className="card" style={{ borderColor: "rgba(255,84,112,0.25)" }}>
          <div className="card-title"><span className="smallcaps" style={{ color: "var(--err)" }}>Factory reset</span></div>
          <div className="card-body">
            <div style={{ fontSize: 12, color: "var(--text-2)", marginBottom: 10 }}>
              Type <span className="mono" style={{ color: "var(--err)" }}>{s.deviceName}</span> to confirm
            </div>
            <div style={{ display: "flex", gap: 6 }}>
              <input className="input mono" value={confirmText}
                placeholder={s.deviceName}
                onChange={e => setConfirmText(e.target.value)}/>
              <button className="btn btn-danger" disabled={confirmText !== s.deviceName}
                style={{ opacity: confirmText !== s.deviceName ? 0.4 : 1 }}
                onClick={() => { setConfirmText(""); window.dispatchEvent(new CustomEvent("ambisense:reboot")); }}>
                Erase
              </button>
            </div>
          </div>
        </div>
      </div>

      <style>{`@media (max-width: 760px) { .hw-grid { grid-template-columns: 1fr !important; } }`}</style>
    </div>
  );
}

Object.assign(window, { ScreenHardware, ScreenNetwork, ScreenSystem });
