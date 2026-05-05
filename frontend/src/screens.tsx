/** All seven screens: Live, LEDs, Motion, Mesh, Hardware, Network, System. */
import { useEffect, useState } from 'preact/hooks';
import { Card, Toggle, Field, Slider, Row, Dot, ColorPicker, useToaster } from './components';
import { LedPreview, LED_MODE_NAMES } from './led_preview';
import { getJSON, postJSON, postBinary } from './api';

interface Live { distance: number; direction: number; rssi: number; heap: number; uptime: number; peers: number; healthy: number; }
interface AppState {
  live: Live;
  settings: any;
  version: any;
  toast: any;
  setToast: (m: string, k?: 'ok'|'err') => void;
  reload: () => void;
}

function Section({ title, sub, right, children }: any) {
  return (
    <>
      <div class="page-head">
        <div>
          <h1>{title}</h1>
          {sub && <div class="sub">{sub}</div>}
        </div>
        {right}
      </div>
      {children}
    </>
  );
}

/* ----------------------------------------------------------------- */
/*                          A. Live dashboard                        */
/* ----------------------------------------------------------------- */
export function ScreenLive({ live, version, settings }: AppState) {
  const dist = live.distance || 0;
  const minD = settings.min_distance ?? 30;
  const maxD = settings.max_distance ?? 300;
  const pct = Math.max(0, Math.min(100, ((dist - minD) / Math.max(1, maxD - minD)) * 100));
  return (
    <Section title="Live" sub={`Real-time view of ${version.hostname || 'device'}`}>
      <div class="grid-cards">
        <Card title="Distance">
          <div class="distance-meter">
            <div>
              <span class="distance-num">{dist}</span>
              <span class="distance-unit">cm</span>
            </div>
            <div class="bar"><div class="bar-fill" style={`width: ${pct}%`} /></div>
            <div style="display: flex; justify-content: space-between; margin-top: 6px; font-size: 11px; color: var(--text-3);">
              <span>{minD} cm</span>
              <span>direction: {live.direction === 0 ? '—' : live.direction < 0 ? 'closer' : 'away'}</span>
              <span>{maxD} cm</span>
            </div>
          </div>
        </Card>
        <Card title="LED preview">
          <LedPreview
            mode={settings.light_mode ?? 0}
            rgb={[settings.r ?? 255, settings.g ?? 255, settings.b ?? 255]}
            count={settings.led_count ?? 30}
            brightness={settings.brightness ?? 80}
            span={settings.span ?? 30}
            distance={dist}
            minD={minD}
            maxD={maxD}
            speed={settings.effect_speed}
            intensity={settings.effect_intensity}
          />
          <div style="margin-top: 10px; font-size: 12px; color: var(--text-2);">
            Mode: <b style="color: var(--text-0);">{LED_MODE_NAMES[settings.light_mode ?? 0]}</b>
            {' · '}{settings.led_count ?? 30} LEDs
          </div>
        </Card>
        <Card title="Device">
          <Row k="firmware" v={<span class="mono">{version.version || '—'}</span>} />
          <Row k="board" v={version.board || '—'} />
          <Row k="ip" v={<span class="mono">{version.ip || 'AP only'}</span>} />
          <Row k="hostname" v={<span class="mono">{version.hostname || '—'}</span>} />
          <Row k="rssi" v={live.rssi ? `${live.rssi} dBm` : 'AP'} />
          <Row k="free heap" v={`${Math.round(live.heap/1024)} KB`} />
          <Row k="uptime" v={fmtUptime(live.uptime)} />
        </Card>
        <Card title="Mesh">
          <Row k="peers" v={`${live.peers || 0}`} />
          <Row k="healthy" v={`${live.healthy || 0}`} />
          <div style="margin-top: 10px; font-size: 12px; color: var(--text-3);">
            ESP-NOW peer mesh activates in PR #4. Each device drives its own strip; readings broadcast at 5 Hz.
          </div>
        </Card>
      </div>
    </Section>
  );
}

function fmtUptime(s: number) {
  if (!s) return '—';
  const d = Math.floor(s / 86400);
  const h = Math.floor((s % 86400) / 3600);
  const m = Math.floor((s % 3600) / 60);
  return d ? `${d}d ${h}h` : h ? `${h}h ${m}m` : `${m}m ${s%60}s`;
}

/* ----------------------------------------------------------------- */
/*                              B. LEDs                              */
/* ----------------------------------------------------------------- */
export function ScreenLeds({ settings, live, reload, setToast }: AppState) {
  const [s, setS] = useState(settings);
  useEffect(() => setS(settings), [settings.light_mode, settings.r, settings.g, settings.b, settings.led_count, settings.brightness]);

  const save = async (patch: any) => {
    const next = { ...s, ...patch };
    setS(next);
    try {
      await postJSON('/api/settings', patch);
      reload();
    } catch (e: any) {
      setToast(e.message || 'Save failed', 'err');
    }
  };

  return (
    <Section title="LEDs" sub="Visual modes, color, brightness, and the distance window.">
      <Card title="Live preview">
        <LedPreview
          mode={s.light_mode ?? 0}
          rgb={[s.r ?? 255, s.g ?? 255, s.b ?? 255]}
          count={s.led_count ?? 30}
          brightness={s.brightness ?? 80}
          span={s.span ?? 30}
          distance={live.distance}
          minD={s.min_distance ?? 30}
          maxD={s.max_distance ?? 300}
          height={120}
          speed={s.effect_speed}
          intensity={s.effect_intensity}
        />
      </Card>

      <div style="height: 14px;" />
      <Card title="Mode">
        <div class="tab-grid">
          {LED_MODE_NAMES.map((name, i) => (
            <div class={`mode-card ${s.light_mode === i ? 'on' : ''}`} onClick={() => save({ light_mode: i })}>
              <div class="preview" style="height: 32px;">
                <LedPreview
                  mode={i}
                  rgb={[s.r ?? 255, s.g ?? 255, s.b ?? 255]}
                  count={Math.min(40, s.led_count ?? 30)}
                  brightness={s.brightness ?? 80}
                  span={Math.min(8, s.span ?? 8)}
                  distance={s.min_distance + ((s.max_distance - s.min_distance) * 0.5)}
                  minD={s.min_distance}
                  maxD={s.max_distance}
                  height={32}
                  speed={s.effect_speed}
                  intensity={s.effect_intensity}
                />
              </div>
              <div class="name">{name}</div>
            </div>
          ))}
        </div>
      </Card>

      <div style="height: 14px;" />
      <div class="grid-cards">
        <Card title="Color & brightness">
          <Field label="Base color">
            <ColorPicker rgb={[s.r ?? 255, s.g ?? 255, s.b ?? 255]} onChange={(r,g,b) => save({ r, g, b })} />
          </Field>
          <Field label={`Brightness — ${s.brightness ?? 80}`}>
            <Slider value={s.brightness ?? 80} min={1} max={255} onChange={(v) => save({ brightness: v })} />
          </Field>
          <Field label={`LED count — ${s.led_count ?? 30}`}>
            <Slider value={s.led_count ?? 30} min={1} max={1500} onChange={(v) => save({ led_count: v })} />
          </Field>
        </Card>
        <Card title="Distance window">
          <Field label={`Min distance — ${s.min_distance ?? 30} cm`}>
            <Slider value={s.min_distance ?? 30} min={0} max={500} onChange={(v) => save({ min_distance: v })} suffix=" cm" />
          </Field>
          <Field label={`Max distance — ${s.max_distance ?? 300} cm`}>
            <Slider value={s.max_distance ?? 300} min={50} max={800} onChange={(v) => save({ max_distance: v })} suffix=" cm" />
          </Field>
          <Field label={`Light span — ${s.span ?? 30}`}>
            <Slider value={s.span ?? 30} min={1} max={150} onChange={(v) => save({ span: v })} />
          </Field>
          <Field label={`Center shift — ${s.center_shift ?? 0}`}>
            <Slider value={s.center_shift ?? 0} min={-100} max={100} onChange={(v) => save({ center_shift: v })} />
          </Field>
        </Card>
        <Card title="Effects">
          <Field label={`Effect speed — ${s.effect_speed ?? 50}`}>
            <Slider value={s.effect_speed ?? 50} min={1} max={100} onChange={(v) => save({ effect_speed: v })} />
          </Field>
          <Field label={`Effect intensity — ${s.effect_intensity ?? 50}`}>
            <Slider value={s.effect_intensity ?? 50} min={1} max={100} onChange={(v) => save({ effect_intensity: v })} />
          </Field>
          <Field label={`Trail length — ${s.trail ?? 0}`}>
            <Slider value={s.trail ?? 0} min={0} max={100} onChange={(v) => save({ trail: v })} />
          </Field>
          <div style="display: flex; justify-content: space-between; padding: 8px 0;">
            <span class="lbl">Direction trail</span>
            <Toggle value={!!s.dir_light} onChange={(v) => save({ dir_light: v ? 1 : 0 })} />
          </div>
          <div style="display: flex; justify-content: space-between; padding: 8px 0;">
            <span class="lbl">Background dim</span>
            <Toggle value={!!s.bg_mode} onChange={(v) => save({ bg_mode: v ? 1 : 0 })} />
          </div>
        </Card>
      </div>
    </Section>
  );
}

/* ----------------------------------------------------------------- */
/*                             C. Motion                             */
/* ----------------------------------------------------------------- */
export function ScreenMotion({ settings, reload, setToast }: AppState) {
  const [s, setS] = useState(settings);
  useEffect(() => setS(settings), [settings.motion_enabled]);
  const save = async (patch: any) => {
    setS({ ...s, ...patch });
    try { await postJSON('/api/settings', patch); reload(); }
    catch (e: any) { setToast(e.message || 'Save failed', 'err'); }
  };
  return (
    <Section title="Motion" sub="PI smoother on the radar reading. Higher smoothing = calmer; higher prediction = snappier.">
      <Card title="Smoothing">
        <div style="display: flex; justify-content: space-between; padding: 8px 0;">
          <span class="lbl">Enable smoothing</span>
          <Toggle value={!!s.motion_enabled} onChange={(v) => save({ motion_enabled: v ? 1 : 0 })} />
        </div>
        <div class="divider" style="margin: 8px 0;" />
        <Field label={`Position smooth — ${((s.pos_smooth_x1k ?? 200)/1000).toFixed(2)}`}>
          <Slider value={s.pos_smooth_x1k ?? 200} min={10} max={800} onChange={(v) => save({ pos_smooth_x1k: v })} />
        </Field>
        <Field label={`Velocity smooth — ${((s.vel_smooth_x1k ?? 100)/1000).toFixed(2)}`}>
          <Slider value={s.vel_smooth_x1k ?? 100} min={10} max={500} onChange={(v) => save({ vel_smooth_x1k: v })} />
        </Field>
        <Field label={`Prediction factor — ${((s.predict_x1k ?? 500)/1000).toFixed(2)}`}>
          <Slider value={s.predict_x1k ?? 500} min={0} max={2000} onChange={(v) => save({ predict_x1k: v })} />
        </Field>
        <Field label={`P gain — ${((s.p_gain_x1k ?? 100)/1000).toFixed(3)}`}>
          <Slider value={s.p_gain_x1k ?? 100} min={0} max={1000} onChange={(v) => save({ p_gain_x1k: v })} />
        </Field>
        <Field label={`I gain — ${((s.i_gain_x1k ?? 10)/1000).toFixed(3)}`}>
          <Slider value={s.i_gain_x1k ?? 10} min={0} max={200} onChange={(v) => save({ i_gain_x1k: v })} />
        </Field>
      </Card>
    </Section>
  );
}

/* ----------------------------------------------------------------- */
/*                              D. Mesh                              */
/* ----------------------------------------------------------------- */
export function ScreenMesh({ live }: AppState) {
  return (
    <Section title="Mesh & Topology" sub="ESP-NOW peer mesh for U/L/asymmetric stair installs. (PR #4 wires this up.)">
      <Card title="Topology">
        <div class="tab-grid">
          {['Straight','L-shape','U-shape','Custom'].map(t => (
            <div class="mode-card" style="opacity:.6">
              <div class="preview" />
              <div class="name">{t}</div>
            </div>
          ))}
        </div>
      </Card>
      <div style="height: 14px;" />
      <Card title="Peers">
        <Row k="discovered" v={`${live.peers ?? 0}`} />
        <Row k="healthy" v={`${live.healthy ?? 0}`} />
        <div style="margin-top: 14px; padding: 12px; background: var(--bg-1); border: 1px solid var(--line); border-radius: 8px; color: var(--text-2); font-size: 12px;">
          When PR #4 lands: each device broadcasts its smoothed reading at 5&nbsp;Hz, every device fuses locally and renders only its own segment. Lowest-MAC device serves the web UI. Pair new devices with a 3-second BOOT-button hold during a pairing window.
        </div>
      </Card>
    </Section>
  );
}

/* ----------------------------------------------------------------- */
/*                            E. Hardware                            */
/* ----------------------------------------------------------------- */
export function ScreenHardware({ setToast }: AppState) {
  const [profiles, setProfiles] = useState<any>(null);
  const [kinds, setKinds] = useState<any>(null);
  const [activeBoard, setActiveBoard] = useState('');
  const [activeRadar, setActiveRadar] = useState('');
  const [overrides, setOverrides] = useState<any>({});

  useEffect(() => {
    getJSON('/api/board/profiles').then(r => { setProfiles(r); setActiveBoard(r.active); });
    getJSON('/api/radar/kinds').then(r => { setKinds(r); setActiveRadar(r.active); });
  }, []);

  if (!profiles || !kinds) return <Section title="Hardware"><Card>Loading…</Card></Section>;
  const profile = profiles.profiles.find((p: any) => p.id === activeBoard) || profiles.profiles[0];
  const unsafe: number[] = profile.unsafe || [];

  const PIN_FIELDS: [string, string][] = [
    ['led_pin', 'LED data'],
    ['radar_rx', 'Radar RX'],
    ['radar_tx', 'Radar TX'],
    ['button', 'Button'],
    ['status_led', 'Status LED'],
  ];

  const save = async () => {
    try {
      await postJSON('/api/board', { id: activeBoard, radar_kind: activeRadar, ...overrides });
      setToast('Saved. Reboot to apply pin changes.');
    } catch (e: any) { setToast(e.message || 'Save failed', 'err'); }
  };

  return (
    <Section title="Hardware" sub="Board profile, radar kind, and per-pin overrides.">
      <div class="grid-cards">
        <Card title="Board profile">
          <Field label="Board">
            <select class="select" value={activeBoard} onChange={(e) => setActiveBoard((e.target as HTMLSelectElement).value)}>
              {profiles.profiles.map((p: any) => (
                <option value={p.id}>{p.display}{p.validated ? '' : ' — untested'}</option>
              ))}
            </select>
          </Field>
          <Row k="MCU" v={profile.mcu} />
          <Row k="max GPIO" v={profile.max_gpio} />
          <Row k="status" v={profile.validated ? <span style="color: var(--ok)">validated</span> : <span style="color: var(--warn)">untested</span>} />
        </Card>

        <Card title="Radar sensor">
          <Field label="Driver">
            <select class="select" value={activeRadar} onChange={(e) => setActiveRadar((e.target as HTMLSelectElement).value)}>
              {kinds.kinds.map((k: any) => <option value={k.id}>{k.display}</option>)}
            </select>
          </Field>
          {kinds.kinds.find((k: any) => k.id === activeRadar) && (
            <div style="font-size: 12px; color: var(--text-2); margin-top: 6px;">
              {kinds.kinds.find((k: any) => k.id === activeRadar).note}
            </div>
          )}
          <div style="margin-top: 10px; padding: 10px; background: var(--bg-1); border: 1px solid var(--line); border-radius: 8px; font-size: 11px; color: var(--text-3);">
            Radar driver swap takes effect after reboot. No reflash needed — all drivers ship in the firmware.
          </div>
        </Card>

        <Card title="Pin overrides">
          {PIN_FIELDS.map(([key, label]) => {
            const def = profile[key];
            const cur = overrides[key] ?? def;
            return (
              <Field label={`${label} (default GPIO ${def})`}>
                <select class="select" value={cur} onChange={(e) => {
                  const v = parseInt((e.target as HTMLSelectElement).value);
                  setOverrides({ ...overrides, [key]: v });
                }}>
                  {Array.from({ length: profile.max_gpio + 1 }, (_, i) => i)
                    .filter(p => !unsafe.includes(p))
                    .map(p => <option value={p}>GPIO {p}{p === def ? ' (default)' : ''}</option>)}
                </select>
              </Field>
            );
          })}
          <button class="btn btn-primary" onClick={save}>Save & note reboot</button>
          <div style="font-size: 11px; color: var(--text-3); margin-top: 8px;">
            {unsafe.length} unsafe GPIO{unsafe.length > 1 ? 's' : ''} hidden ({unsafe.join(', ')}). These are strapping/USB-JTAG/flash pins.
          </div>
        </Card>
      </div>
    </Section>
  );
}

/* ----------------------------------------------------------------- */
/*                             F. Network                            */
/* ----------------------------------------------------------------- */
export function ScreenNetwork({ setToast, version }: AppState) {
  const [wifi, setWifi] = useState<any>(null);
  const [scan, setScan] = useState<any[] | null>(null);
  const [ssid, setSsid] = useState('');
  const [pass, setPass] = useState('');
  const [host, setHost] = useState('');
  const [apMode, setApMode] = useState('auto');
  const [apPass, setApPass] = useState('');

  const refresh = () => getJSON('/api/wifi').then(w => {
    setWifi(w);
    setApMode(w.ap_mode);
    setHost(w.hostname || '');
  });
  const doScan = () => {
    setScan(null);
    getJSON('/api/wifi/scan').then(r => setScan(r.networks)).catch(e => { setScan([]); setToast(e.message, 'err'); });
  };
  useEffect(() => { refresh(); doScan(); }, []);

  const saveWifi = async () => {
    if (!ssid) { setToast('Pick a network', 'err'); return; }
    try {
      await postJSON('/api/wifi', { ssid, pass, hostname: host || undefined });
      setToast('Saved — reconnecting');
      setTimeout(refresh, 4000);
    } catch (e: any) { setToast(e.message, 'err'); }
  };
  const saveApMode = async (mode: string) => {
    setApMode(mode);
    try { await postJSON('/api/wifi', { ap_mode: mode }); setToast(`AP mode → ${mode}`); refresh(); }
    catch (e: any) { setToast(e.message, 'err'); }
  };
  const saveApPass = async () => {
    try { await postJSON('/api/wifi', { ap_password: apPass }); setToast('AP password updated'); }
    catch (e: any) { setToast(e.message, 'err'); }
  };
  const forgetSta = async () => {
    if (!confirm('Forget Wi-Fi credentials? Device will return to AP-only mode.')) return;
    try { await postJSON('/api/wifi', { forget_sta: true }); setToast('STA cleared'); refresh(); }
    catch (e: any) { setToast(e.message, 'err'); }
  };

  if (!wifi) return <Section title="Network"><Card>Loading…</Card></Section>;

  return (
    <Section title="Network" sub="Wi-Fi setup, captive portal, and access-point preferences.">
      <div class="grid-cards">
        <Card title="Status">
          <div style="display: flex; gap: 8px; align-items: center; padding: 8px 0;">
            <Dot kind={wifi.sta_connected ? 'ok' : (wifi.sta_configured ? 'warn' : 'off')} />
            <span style="font-size: 13px;">
              STA: <b>{wifi.sta_connected ? `connected to ${wifi.ssid}` : (wifi.sta_configured ? `joining ${wifi.ssid}…` : 'not configured')}</b>
            </span>
          </div>
          <div style="display: flex; gap: 8px; align-items: center; padding: 8px 0;">
            <Dot kind={wifi.ap_active ? 'ok' : 'off'} />
            <span style="font-size: 13px;">AP: <b>{wifi.ap_active ? `up — AmbiSense-XXXX (chan 6)` : 'down'}</b></span>
          </div>
          <Row k="ip" v={<span class="mono">{wifi.ip || '—'}</span>} />
          <Row k="hostname" v={<span class="mono">{wifi.hostname}</span>} />
          <Row k="rssi" v={wifi.rssi ? `${wifi.rssi} dBm` : '—'} />
        </Card>

        <Card title="AP behaviour">
          <Field label="When STA is connected">
            <select class="select" value={apMode} onChange={(e) => saveApMode((e.target as HTMLSelectElement).value)}>
              <option value="auto">Auto — AP off when STA connected (default)</option>
              <option value="always">Always on — AP up at all times</option>
              <option value="sta_only">STA only — AP off, ESP-NOW uses STA channel</option>
            </select>
          </Field>
          <Field label="AP password (≥ 8 chars enables WPA2; empty = open)">
            <input class="input mono" type="text" value={apPass} onInput={(e) => setApPass((e.target as HTMLInputElement).value)} placeholder="leave blank for open AP" />
          </Field>
          <button class="btn" onClick={saveApPass}>Update AP password</button>
        </Card>

        <Card title="Connect to Wi-Fi" right={<button class="btn btn-sm btn-ghost" onClick={doScan}>Rescan</button>}>
          <Field label="Network">
            <select class="select" value={ssid} onChange={(e) => setSsid((e.target as HTMLSelectElement).value)}>
              <option value="">{scan ? 'Pick a network…' : 'Scanning…'}</option>
              {(scan || []).map(n => (
                <option value={n.ssid}>{n.ssid}  ({n.rssi} dBm{n.secure ? ', 🔒' : ''})</option>
              ))}
            </select>
          </Field>
          <Field label="Password">
            <input class="input" type="password" value={pass} onInput={(e) => setPass((e.target as HTMLInputElement).value)} />
          </Field>
          <Field label="Device hostname">
            <input class="input mono" value={host} onInput={(e) => setHost((e.target as HTMLInputElement).value)} placeholder="ambisense-living" />
          </Field>
          <button class="btn btn-primary" onClick={saveWifi}>Save & connect</button>
          {wifi.sta_configured && (
            <button class="btn btn-danger" style="margin-left: 8px;" onClick={forgetSta}>Forget STA</button>
          )}
        </Card>
      </div>
    </Section>
  );
}

/* ----------------------------------------------------------------- */
/*                              G. System                            */
/* ----------------------------------------------------------------- */
export function ScreenSystem({ version, setToast }: AppState) {
  const [pw, setPw] = useState('');
  const [otaProg, setOtaProg] = useState(-1);
  const fileInput = (() => { let r: HTMLInputElement | null = null; return { set: (e: any) => r = e, get: () => r }; })();

  const setPassword = async () => {
    if (pw && pw.length < 8) { setToast('Min 8 chars', 'err'); return; }
    try { await postJSON('/api/auth/password', { password: pw }); setToast(pw ? 'Password set' : 'Password cleared'); setPw(''); }
    catch (e: any) { setToast(e.message, 'err'); }
  };

  const doOta = async () => {
    const inp = fileInput.get();
    if (!inp || !inp.files || inp.files.length === 0) { setToast('Pick a .bin file', 'err'); return; }
    const f = inp.files[0];
    setOtaProg(0);
    try {
      await postBinary('/api/ota', f, p => setOtaProg(p));
      setToast('Flashed. Device rebooting in 1 s; refresh in 30 s.');
    } catch (e: any) {
      setOtaProg(-1);
      setToast(e.message || 'OTA failed', 'err');
    }
  };

  return (
    <Section title="System" sub="Authentication, firmware update, diagnostics.">
      <div class="grid-cards">
        <Card title="Authentication">
          {!version.auth_enabled && (
            <div style="background: rgba(255,84,112,.08); border: 1px solid rgba(255,84,112,.35); border-radius: 8px; padding: 10px 12px; font-size: 12px; color: var(--err); margin-bottom: 12px;">
              Authentication is OFF. Anyone on this network can change settings. Set a password.
            </div>
          )}
          <Field label="Admin password (≥ 8 chars; leave blank to disable auth)">
            <input class="input" type="password" value={pw} onInput={(e) => setPw((e.target as HTMLInputElement).value)} />
          </Field>
          <button class="btn btn-primary" onClick={setPassword}>{pw ? 'Set password' : 'Disable auth'}</button>
        </Card>

        <Card title="Firmware update (OTA)">
          <Field label="Firmware (.bin from idf.py build)">
            <input ref={fileInput.set} type="file" accept=".bin" class="input" />
          </Field>
          {otaProg >= 0 && (
            <div class="bar" style="margin-bottom: 10px;"><div class="bar-fill" style={`width: ${(otaProg*100).toFixed(1)}%`} /></div>
          )}
          <button class="btn btn-primary" onClick={doOta} disabled={otaProg >= 0 && otaProg < 1}>
            {otaProg >= 0 && otaProg < 1 ? `Uploading ${(otaProg*100).toFixed(0)}%` : 'Upload firmware'}
          </button>
          <div style="font-size: 11px; color: var(--text-3); margin-top: 8px;">
            Bootloader rollback is armed; if a bad image hangs, the previous firmware boots automatically.
          </div>
        </Card>

        <Card title="Diagnostics">
          <Row k="version" v={<span class="mono">{version.version}</span>} />
          <Row k="idf" v={<span class="mono">{version.idf_version}</span>} />
          <Row k="built" v={`${version.build_date} ${version.build_time}`} />
          <Row k="target" v={version.target} />
          <Row k="MAC" v={<span class="mono">{version.mac}</span>} />
          <Row k="free heap" v={`${Math.round((version.free_heap||0)/1024)} KB`} />
          <Row k="min free heap" v={`${Math.round((version.min_free_heap||0)/1024)} KB`} />
        </Card>
      </div>
    </Section>
  );
}
