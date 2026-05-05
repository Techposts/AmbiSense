/** All seven screens: Live, LEDs, Motion, Mesh, Hardware, Network, System. */
import { useEffect, useRef, useState } from 'preact/hooks';
import { Card, Toggle, Field, Slider, Row, Dot, ColorPicker, useToaster } from './components';
import { LedPreview, LED_MODE_NAMES } from './led_preview';
import { Icon, Sparkline, fmtUptime } from './atoms';
import { getJSON, postJSON, postBinary } from './api';

/* Static CSS-gradient thumbnails for the 11 mode cards. One animated
 * canvas (the hero) is enough; per-card animations crushed phone GPUs. */
const THUMB_CLASSES = [
  'thumb-standard',
  'thumb-rainbow',
  'thumb-color-wave',
  'thumb-breathing',
  'thumb-solid',
  'thumb-comet',
  'thumb-pulse',
  'thumb-fire',
  'thumb-theater-chase',
  'thumb-dual-scan',
  'thumb-particles',
];

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
export function ScreenLive({ live, version, settings, setToast }: AppState) {
  const dist = Math.round(live.distance || 0);
  const minD = settings.min_distance ?? 30;
  const maxD = settings.max_distance ?? 300;
  const inWindow = dist >= minD && dist <= maxD;

  /* Client-side ring buffer of last 80 samples (~16 s @ 5 Hz). Faithful
   * to the design — gives the sparkline its rolling shape. */
  const histRef = useRef<number[]>(Array(80).fill(0));
  const [hist, setHist] = useState<number[]>(histRef.current);
  useEffect(() => {
    histRef.current = [...histRef.current.slice(1), dist];
    setHist(histRef.current);
  }, [dist]);

  /* System enable toggle (real /api/system) */
  const [sysEn, setSysEn] = useState<boolean>(true);
  useEffect(() => {
    getJSON('/api/system').then(r => setSysEn(!!r.enabled)).catch(() => {});
  }, []);
  const toggleSys = async () => {
    const next = !sysEn;
    setSysEn(next);
    try { await postJSON('/api/system', { enabled: next }); }
    catch (e: any) { setSysEn(!next); setToast(e.message || 'Toggle failed', 'err'); }
  };

  return (
    <>
      <div class="page-head">
        <div>
          <h1>Live</h1>
          <div class="sub">Real-time radar, mesh, and LED output</div>
        </div>
        <div style="display: flex; gap: 10px; align-items: center;">
          <span class="chip"><span class="dot dot-ok"/> WS connected · 5 Hz</span>
        </div>
      </div>

      {/* System enable hero — gradient when active */}
      <div class="card" style={`padding: 18px; display: flex; align-items: center; gap: 16px; margin-bottom: 14px; ${sysEn ? 'background: linear-gradient(135deg, rgba(255,181,74,0.06), rgba(255,61,130,0.06)); border-color: rgba(255,122,61,0.25);' : ''}`}>
        <div style={`width: 44px; height: 44px; border-radius: 12px; display: flex; align-items: center; justify-content: center; background: ${sysEn ? 'var(--acc-grad)' : 'var(--bg-3)'}; color: ${sysEn ? '#1A0F08' : 'var(--text-3)'};`}>
          <Icon name="bolt" size={22} stroke={2}/>
        </div>
        <div style="flex: 1;">
          <div style="font-size: 15px; font-weight: 600;">System {sysEn ? 'active' : 'paused'}</div>
          <div style="font-size: 12px; color: var(--text-2);">
            {sysEn ? 'Radar, mesh, and LED output running' : 'All output muted, mesh idle'}
          </div>
        </div>
        <Toggle large value={sysEn} onChange={toggleSys}/>
      </div>

      <div class="dash-grid" style="display: grid; grid-template-columns: minmax(0, 2fr) minmax(0, 1fr); gap: 14px;">
        {/* Left column */}
        <div style="display: grid; gap: 14px;">
          {/* Distance meter — big gradient number + sparkline */}
          <div class="card" style="padding: 22px; position: relative; overflow: hidden;">
            <div class="distance-row">
              <div>
                <div class="smallcaps">Distance</div>
                <div style="display: flex; align-items: baseline; gap: 8px; margin-top: 8px;">
                  <span class="mono dist-big">{dist}</span>
                  <span style="color: var(--text-2); font-size: 16px;">cm</span>
                </div>
                <div style="display: flex; gap: 10px; margin-top: 14px; align-items: center; flex-wrap: wrap;">
                  <span class="chip" style={`color: ${inWindow ? 'var(--ok)' : 'var(--text-2)'};`}>
                    <span class={`dot ${inWindow ? 'dot-ok' : 'dot-off'}`}/>
                    {inWindow ? 'in window' : 'outside'}
                  </span>
                  <span class="chip">min {minD}</span>
                  <span class="chip">max {maxD}</span>
                  <span class="chip">{live.direction === 0 ? 'still' : live.direction < 0 ? 'closer →' : 'away →'}</span>
                </div>
              </div>
              <div class="dist-spark">
                <div style="position: relative;">
                  <Sparkline data={hist} width={420} height={80} min={0} max={Math.max(300, maxD)}/>
                  <div style="position: absolute; inset: 0; pointer-events: none;">
                    <div style={`position: absolute; left: 0; right: 0; top: ${(1 - maxD/Math.max(300,maxD))*100}%; border-top: 1px dashed var(--text-4);`}/>
                    <div style={`position: absolute; left: 0; right: 0; top: ${(1 - minD/Math.max(300,maxD))*100}%; border-top: 1px dashed var(--text-4);`}/>
                  </div>
                </div>
                <div class="mono" style="font-size: 10px; color: var(--text-3); text-align: right; margin-top: 4px;">last 16 s · 5 Hz</div>
              </div>
            </div>
          </div>

          {/* Strip preview */}
          <div class="card" style="padding: 18px;">
            <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 10px;">
              <div class="smallcaps">Live LED preview</div>
              <span class="chip mono">{settings.led_count ?? 30} px · {LED_MODE_NAMES[settings.light_mode ?? 0]}</span>
            </div>
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
              height={64}
            />
            <div style="display: flex; justify-content: space-between; margin-top: 8px; font-size: 11px; color: var(--text-3);">
              <span class="mono">px 0</span>
              <span>{Math.round(((dist - minD) / Math.max(1, maxD - minD)) * 100)}% along</span>
              <span class="mono">px {(settings.led_count ?? 30) - 1}</span>
            </div>
          </div>

          {/* Stat tiles */}
          <div class="stat-row" style="display: grid; grid-template-columns: repeat(4, 1fr); gap: 10px;">
            <StatTile label="Free heap" value={`${Math.round((live.heap||0)/1024)}`} sub="kB · stable"/>
            <StatTile label="RSSI"      value={`${live.rssi||0}`} sub={(live.rssi||0) > -65 ? 'dBm · excellent' : (live.rssi||0) > -75 ? 'dBm · good' : 'dBm · weak'}/>
            <StatTile label="Uptime"    value={fmtUptime(live.uptime||0).split(' ')[0]} sub={fmtUptime(live.uptime||0)}/>
            <StatTile label="Cycle"     value="200" sub="ms · radar→led" accent/>
          </div>
        </div>

        {/* Right column */}
        <div style="display: grid; gap: 14px; align-content: start;">
          {/* Device card */}
          <div class="card">
            <div style="display: flex; align-items: center; justify-content: space-between; padding: 16px 18px 0;">
              <span class="smallcaps">Device</span>
              <Icon name="cpu" size={14} style={{ color: 'var(--text-3)' }}/>
            </div>
            <div style="padding: 14px 18px 18px;">
              <div style="display: grid; grid-template-columns: 1fr 1fr; gap: 12px 18px;">
                <DevField k="Name"     v={version.hostname || '—'}/>
                <DevField k="IP"       v={version.ip || 'AP only'}/>
                <DevField k="mDNS"     v={version.hostname ? `${version.hostname}.local` : '—'}/>
                <DevField k="RSSI"     v={`${live.rssi||0} dBm`}/>
                <DevField k="Free heap" v={`${((live.heap||0)/1024).toFixed(1)} kB`}/>
                <DevField k="Uptime"   v={fmtUptime(live.uptime||0)}/>
                <DevField k="Firmware" v={version.version || '—'}/>
                <DevField k="Board"    v={version.board || '—'}/>
              </div>
            </div>
          </div>

          {/* Mesh card */}
          <div class="card">
            <div style="display: flex; align-items: center; justify-content: space-between; padding: 16px 18px 0;">
              <span class="smallcaps">Mesh</span>
              <span class="chip" style="text-transform: capitalize;">{live.peers > 0 ? 'peer' : 'standalone'}</span>
            </div>
            <div style="padding: 14px 18px 18px;">
              <div style="font-size: 12px; color: var(--text-2); margin-bottom: 10px;">
                {live.peers || 0} peer{(live.peers||0) === 1 ? '' : 's'} · {live.healthy||0} healthy
              </div>
              {(live.peers || 0) === 0 && (
                <div style="font-size: 12px; color: var(--text-3); padding: 12px; background: var(--bg-1); border: 1px solid var(--line); border-radius: 8px;">
                  No peers discovered. Open Mesh tab to start a 30 s pairing window.
                </div>
              )}
            </div>
          </div>
        </div>
      </div>
    </>
  );
}

function StatTile({ label, value, sub, accent }: { label: string; value: string; sub?: string; accent?: boolean }) {
  return (
    <div class="card" style="padding: 16px;">
      <div class="smallcaps">{label}</div>
      <div class="mono" style={`font-size: 22px; font-weight: 500; margin-top: 6px; letter-spacing: -0.02em; color: ${accent ? 'var(--acc-orange)' : 'var(--text-0)'};`}>
        {value}
      </div>
      {sub && <div style="font-size: 11px; color: var(--text-3); margin-top: 2px;">{sub}</div>}
    </div>
  );
}

function DevField({ k, v }: { k: string; v: string }) {
  return (
    <div>
      <div style="font-size: 11px; color: var(--text-3);">{k}</div>
      <div class="mono" style="font-size: 13px; color: var(--text-0); word-break: break-all;">{v}</div>
    </div>
  );
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
              <div class={`preview ${THUMB_CLASSES[i]}`} />
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
export function ScreenHardware({ setToast, settings, reload }: AppState) {
  const [profiles, setProfiles] = useState<any>(null);
  const [kinds, setKinds] = useState<any>(null);
  const [activeBoard, setActiveBoard] = useState('');
  const [activeRadar, setActiveRadar] = useState('');
  /* Pin state initializes from saved settings (so the dropdowns show
   * what's actually persisted, not just the profile defaults). */
  const [pins, setPins] = useState<any>({});
  const [busy, setBusy] = useState(false);

  useEffect(() => {
    Promise.all([
      getJSON('/api/board/profiles'),
      getJSON('/api/radar/kinds'),
      getJSON('/api/settings'),
    ]).then(([p, k, s]) => {
      setProfiles(p);
      setKinds(k);
      setActiveBoard(p.active);
      setActiveRadar(k.active);
      /* Initialize pin state from saved values, falling back to the
       * profile defaults for any pins the user hasn't customised yet. */
      const profile = p.profiles.find((x: any) => x.id === p.active) || p.profiles[0];
      setPins({
        led_pin:        s.led_pin        ?? profile.led_pin,
        radar_rx:       s.radar_rx       ?? profile.radar_rx,
        radar_tx:       s.radar_tx       ?? profile.radar_tx,
        button_pin:     s.button_pin     ?? profile.button,
        status_led_pin: s.status_led_pin ?? profile.status_led,
      });
    }).catch((e) => setToast(e.message || 'Load failed', 'err'));
  }, []);

  if (!profiles || !kinds) return <Section title="Hardware"><Card title="Loading">Fetching board profiles…</Card></Section>;
  const profile = profiles.profiles.find((p: any) => p.id === activeBoard) || profiles.profiles[0];
  const unsafe: number[] = profile.unsafe || [];

  /* When the user changes the board profile, snap pins to the new defaults
   * so the dropdowns aren't pointing at GPIOs that don't exist on the new
   * MCU (e.g. ESP32-C3 has GPIO ≤21; ESP32-S3 has 0..48). */
  const onBoardChange = (id: string) => {
    setActiveBoard(id);
    const np = profiles.profiles.find((p: any) => p.id === id);
    if (np) setPins({
      led_pin:        np.led_pin,
      radar_rx:       np.radar_rx,
      radar_tx:       np.radar_tx,
      button_pin:     np.button,
      status_led_pin: np.status_led,
    });
  };

  const PIN_FIELDS: [keyof typeof pins, string, string][] = [
    ['led_pin',        'LED data pin',       'led_pin'],
    ['radar_rx',       'Radar RX (MCU side)', 'radar_rx'],
    ['radar_tx',       'Radar TX (MCU side)', 'radar_tx'],
    ['button_pin',     'Button',              'button'],
    ['status_led_pin', 'Status LED',          'status_led'],
  ];

  const buildPayload = () => ({
    id: activeBoard,
    radar_kind: activeRadar,
    led_pin:        pins.led_pin,
    radar_rx:       pins.radar_rx,
    radar_tx:       pins.radar_tx,
    button_pin:     pins.button_pin,
    status_led_pin: pins.status_led_pin,
  });

  const saveOnly = async () => {
    setBusy(true);
    try {
      await postJSON('/api/board', buildPayload());
      setToast('Saved. Reboot to apply pin/radar changes.');
      reload();
    } catch (e: any) { setToast(e.message || 'Save failed', 'err'); }
    finally { setBusy(false); }
  };

  const saveAndReboot = async () => {
    if (!confirm('Save and reboot device now? You will lose connection for ~10 seconds.')) return;
    setBusy(true);
    try {
      await postJSON('/api/board', buildPayload());
      await postJSON('/api/reboot', {});
      setToast('Rebooting — refresh the page in 10 seconds.');
    } catch (e: any) { setToast(e.message || 'Save failed', 'err'); }
    finally { setBusy(false); }
  };

  /* Show whether the current pin differs from the profile default — small
   * "(default)" / "(custom)" hint helps the user reason about state. */
  const pinHint = (cur: number, def: number) =>
    cur === def ? ' (default)' : ' (custom)';

  return (
    <Section title="Hardware" sub="Board profile, radar driver, and pin assignments. Changes take effect on reboot.">
      <div class="grid-cards">
        <Card title="Board profile">
          <Field label="Board">
            <select class="select" value={activeBoard} onChange={(e) => onBoardChange((e.target as HTMLSelectElement).value)}>
              {profiles.profiles.map((p: any) => (
                <option value={p.id}>{p.display}{p.validated ? '' : ' — untested'}</option>
              ))}
            </select>
          </Field>
          <Row k="MCU"          v={<span class="mono">{profile.mcu}</span>} />
          <Row k="max GPIO"     v={profile.max_gpio} />
          <Row k="status"       v={profile.validated ? <span style="color: var(--ok)">validated</span> : <span style="color: var(--warn)">untested</span>} />
          <Row k="active radar" v={<span class="mono">{activeRadar}</span>} />
        </Card>

        <Card title="Radar driver">
          <Field label="Sensor">
            <select class="select" value={activeRadar} onChange={(e) => setActiveRadar((e.target as HTMLSelectElement).value)}>
              {kinds.kinds.map((k: any) => <option value={k.id}>{k.display}</option>)}
            </select>
          </Field>
          {kinds.kinds.find((k: any) => k.id === activeRadar) && (
            <div style="font-size: 12px; color: var(--text-2); margin-top: 6px;">
              {kinds.kinds.find((k: any) => k.id === activeRadar).note}
            </div>
          )}
          <div style="margin-top: 14px; padding: 10px 12px; background: var(--bg-1); border: 1px solid var(--line); border-radius: 8px; font-size: 11px; color: var(--text-3);">
            All five drivers ship in the firmware. Switching does NOT require a reflash — just save and reboot. The active driver is exclusive (one radar per board).
          </div>
        </Card>

        <Card title="Pin assignments">
          {PIN_FIELDS.map(([key, label, profKey]) => {
            const cur = pins[key] ?? (profile as any)[profKey];
            const def = (profile as any)[profKey];
            return (
              <Field label={`${label}${pinHint(cur, def)} — default GPIO ${def}`}>
                <select class="select" value={cur} onChange={(e) => {
                  const v = parseInt((e.target as HTMLSelectElement).value);
                  setPins({ ...pins, [key]: v });
                }}>
                  {Array.from({ length: profile.max_gpio + 1 }, (_, i) => i)
                    .filter(p => !unsafe.includes(p))
                    .map(p => (
                      <option value={p}>GPIO {p}{p === def ? ' — default' : ''}</option>
                    ))}
                </select>
              </Field>
            );
          })}

          <div style="display: flex; gap: 8px; flex-wrap: wrap; margin-top: 14px;">
            <button class="btn" onClick={saveOnly} disabled={busy}>
              {busy ? 'Saving…' : 'Save'}
            </button>
            <button class="btn btn-primary" onClick={saveAndReboot} disabled={busy}>
              {busy ? '…' : 'Save & reboot'}
            </button>
          </div>

          <div style="font-size: 11px; color: var(--text-3); margin-top: 10px;">
            {unsafe.length} unsafe GPIO{unsafe.length > 1 ? 's' : ''} hidden ({unsafe.join(', ')}). These are strapping/USB-JTAG/flash pins on this MCU and will brick boot if used as I/O.
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
  /* Lazy: fetch wifi state on mount, but DON'T auto-scan (a 1-second wifi
   * scan stalls the page on slow devices). User clicks "Scan" when ready. */
  useEffect(() => { refresh(); }, []);

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

  const doReboot = async () => {
    if (!confirm('Reboot device? You will lose connection for ~10 seconds.')) return;
    try { await postJSON('/api/reboot', {}); setToast('Rebooting — refresh the page in 10 s.'); }
    catch (e: any) { setToast(e.message || 'Reboot failed', 'err'); }
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
          <button class="btn btn-danger" onClick={doReboot} style="margin-top: 14px;">Reboot device</button>
        </Card>
      </div>
    </Section>
  );
}
