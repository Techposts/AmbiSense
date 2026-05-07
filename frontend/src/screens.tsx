/** Six screens: Live, LEDs, Motion, Hardware, Network, System.
 *  Single-device architecture (v6.x): every control wires to a local
 *  /api/* endpoint with optimistic updates + toast confirmation. */
import { useEffect, useRef, useState } from 'preact/hooks';
import { Card, Toggle, Field, Slider, Row, Dot, ColorPicker as PaletteColorPicker, useToaster } from './components';
import { LedPreview, LED_MODE_NAMES } from './led_preview';
import { Icon, Sparkline, fmtUptime, NumberAndSlider, DualHandleRange, LineChart, hsv2rgb, rgb2hex, hex2rgb } from './atoms';
import { getJSON, postJSON, postBinary } from './api';

interface Live { distance: number; direction: number; rssi: number; heap: number; uptime: number; }
export interface AppState {
  live: Live;
  settings: any;
  version: any;
  toast: any;
  setToast: (m: string, k?: 'ok'|'err') => void;
  reload: () => void;
}

const MODE_DESCRIPTIONS = [
  'Distance-driven cluster with directional fade',
  'Full-strip hue cycle',
  'Sine wave of color across strip',
  'Strip gently inhales and exhales',
  'All pixels one color',
  'Tail chasing across the strip',
  'Center pulse expands outward',
  'Flickering ember simulation',
  'Marquee dot pattern',
  'Two scanners meet in the middle',
  'Particles spawn from your position',
];

function PageHead({ title, sub, right }: any) {
  return (
    <div class="page-head">
      <div>
        <h1>{title}</h1>
        {sub && <div class="sub">{sub}</div>}
      </div>
      {right}
    </div>
  );
}

/* Shared debounced save — every screen with sliders uses this. Sliders fire
 * onChange ~30×/s while dragging; without debouncing each tick fires a POST
 * /api/settings + 2× GET (reload), which overwhelms the C3's single-core
 * httpd (max_open_sockets=7) and triggers ERR_CONNECTION_RESET. With a
 * 300 ms tail, only the *final* slider value POSTs once the user stops
 * moving. Multiple keys touched within the window get coalesced into a
 * single JSON body — the firmware /api/settings POST handler already
 * iterates the whole object looking for known keys, so one batched call
 * is identical in effect to N individual calls. */
function useDebouncedSave(reload: () => void, setToast: (m: string, k?: 'ok'|'err') => void, delay = 300) {
  const pending = useRef<any>({});
  const timer = useRef<any>(null);
  return (patch: any) => {
    pending.current = { ...pending.current, ...patch };
    if (timer.current) clearTimeout(timer.current);
    timer.current = setTimeout(async () => {
      const body = pending.current;
      pending.current = {};
      timer.current = null;
      try { await postJSON('/api/settings', body); reload(); }
      catch (e: any) { setToast(e.message || 'Save failed', 'err'); }
    }, delay);
  };
}

/* ================================================================= */
/*                          A. LIVE                                  */
/* ================================================================= */
export function ScreenLive({ live, version, settings, setToast }: AppState) {
  const dist = Math.round(live.distance || 0);
  const minD = settings.min_distance ?? 30;
  const maxD = settings.max_distance ?? 300;
  const inWindow = dist >= minD && dist <= maxD;
  /* Push a sample on EVERY live update (now 20 Hz) instead of only when
   * the integer cm value changes. Otherwise the sparkline freezes during
   * stationary-but-real-time periods, which looks broken. */
  const histRef = useRef<number[]>(Array(80).fill(0));
  const [hist, setHist] = useState<number[]>(histRef.current);
  useEffect(() => {
    histRef.current = [...histRef.current.slice(1), dist];
    setHist(histRef.current);
  }, [live]);

  const [sysEn, setSysEn] = useState<boolean>(true);
  useEffect(() => { getJSON('/api/system').then(r => setSysEn(!!r.enabled)).catch(() => {}); }, []);
  const toggleSys = async () => {
    const next = !sysEn;
    setSysEn(next);
    try { await postJSON('/api/system', { enabled: next }); }
    catch (e: any) { setSysEn(!next); setToast(e.message || 'Toggle failed', 'err'); }
  };

  return (
    <>
      <PageHead title="Live" sub="Real-time radar and LED output"
        right={<span class="chip"><span class="dot dot-ok"/> WS connected · 20 Hz</span>}/>

      <div class="card" style={`padding: 18px; display: flex; align-items: center; gap: 16px; margin-bottom: 14px; ${sysEn ? 'background: linear-gradient(135deg, rgba(255,181,74,0.06), rgba(255,61,130,0.06)); border-color: rgba(255,122,61,0.25);' : ''}`}>
        <div style={`width: 44px; height: 44px; border-radius: 12px; display: flex; align-items: center; justify-content: center; background: ${sysEn ? 'var(--acc-grad)' : 'var(--bg-3)'}; color: ${sysEn ? '#1A0F08' : 'var(--text-3)'};`}>
          <Icon name="bolt" size={22} stroke={2}/>
        </div>
        <div style="flex: 1;">
          <div style="font-size: 15px; font-weight: 600;">System {sysEn ? 'active' : 'paused'}</div>
          <div style="font-size: 12px; color: var(--text-2);">{sysEn ? 'Radar and LED output running' : 'All output muted'}</div>
        </div>
        <Toggle large value={sysEn} onChange={toggleSys}/>
      </div>

      <div class="dash-grid" style="display: grid; grid-template-columns: minmax(0, 2fr) minmax(0, 1fr); gap: 14px;">
        <div style="display: grid; gap: 14px;">
          <div class="card" style="padding: 22px; position: relative; overflow: hidden;">
            <div class="distance-row">
              <div>
                <div class="smallcaps">Distance</div>
                <div style="display: flex; align-items: baseline; gap: 8px; margin-top: 8px;">
                  <span class="mono dist-big">{dist}</span><span style="color: var(--text-2); font-size: 16px;">cm</span>
                </div>
                <div style="display: flex; gap: 10px; margin-top: 14px; align-items: center; flex-wrap: wrap;">
                  <span class="chip" style={`color: ${inWindow ? 'var(--ok)' : 'var(--text-2)'};`}>
                    <span class={`dot ${inWindow ? 'dot-ok' : 'dot-off'}`}/>{inWindow ? 'in window' : 'outside'}
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

          <div class="card" style="padding: 18px;">
            <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 10px;">
              <div class="smallcaps">Live LED preview · {LED_MODE_NAMES[settings.light_mode ?? 0]}</div>
              <span class="chip mono">distance {dist} cm</span>
            </div>
            <LedPreview mode={settings.light_mode ?? 0} rgb={[settings.r ?? 255, settings.g ?? 255, settings.b ?? 255]}
              count={settings.led_count ?? 30} brightness={settings.brightness ?? 80} span={settings.span ?? 30}
              distance={dist} minD={minD} maxD={maxD} speed={settings.effect_speed} intensity={settings.effect_intensity} height={72}/>
          </div>

          <div class="stat-row" style="display: grid; grid-template-columns: repeat(4, 1fr); gap: 10px;">
            <StatTile label="Free heap" value={`${Math.round((live.heap||0)/1024)}`} sub="kB · stable"/>
            <StatTile label="RSSI"      value={`${live.rssi||0}`} sub={(live.rssi||0) > -65 ? 'dBm · excellent' : (live.rssi||0) > -75 ? 'dBm · good' : 'dBm · weak'}/>
            <StatTile label="Uptime"    value={fmtUptime(live.uptime||0).split(' ')[0]} sub={fmtUptime(live.uptime||0)}/>
            <StatTile label="Cycle"     value="200" sub="ms · radar→led" accent/>
          </div>
        </div>

        <div style="display: grid; gap: 14px; align-content: start;">
          <div class="card">
            <div class="card-head"><span class="smallcaps">Device</span><Icon name="cpu" size={14} style={{ color: 'var(--text-3)' }}/></div>
            <div class="card-body">
              <div style="display: grid; grid-template-columns: 1fr 1fr; gap: 12px 18px;">
                <DevField k="Name" v={version.hostname || '—'}/>
                <DevField k="IP" v={version.ip || 'AP only'}/>
                <DevField k="mDNS" v={version.hostname ? `${version.hostname}.local` : '—'}/>
                <DevField k="RSSI" v={`${live.rssi||0} dBm`}/>
                <DevField k="Free heap" v={`${((live.heap||0)/1024).toFixed(1)} kB`}/>
                <DevField k="Uptime" v={fmtUptime(live.uptime||0)}/>
                <DevField k="Firmware" v={version.version || '—'}/>
                <DevField k="Board" v={version.board || '—'}/>
              </div>
            </div>
          </div>
        </div>
      </div>
    </>
  );
}

function StatTile({ label, value, sub, accent }: any) {
  return (
    <div class="card" style="padding: 16px;">
      <div class="smallcaps">{label}</div>
      <div class="mono" style={`font-size: 22px; font-weight: 500; margin-top: 6px; letter-spacing: -0.02em; color: ${accent ? 'var(--acc-orange)' : 'var(--text-0)'};`}>{value}</div>
      {sub && <div style="font-size: 11px; color: var(--text-3); margin-top: 2px;">{sub}</div>}
    </div>
  );
}

function DevField({ k, v }: any) {
  return (
    <div>
      <div style="font-size: 11px; color: var(--text-3);">{k}</div>
      <div class="mono" style="font-size: 13px; color: var(--text-0); word-break: break-all;">{v}</div>
    </div>
  );
}

/* ================================================================= */
/*                          B. LEDs                                  */
/* ================================================================= */
export function ScreenLeds({ settings, live, reload, setToast }: AppState) {
  const [s, setS] = useState(settings);
  useEffect(() => setS(settings), [JSON.stringify(settings)]);
  const debouncedSave = useDebouncedSave(reload, setToast);
  /* `save` updates local state immediately so the slider/preview feel
   * instantaneous, then queues the network write under the debouncer. */
  const save = (patch: any) => {
    setS((prev: any) => ({ ...prev, ...patch }));
    debouncedSave(patch);
  };
  const mode = s.light_mode ?? 0;
  const showColor = [0,2,3,4,5,6,9,10].includes(mode);
  const showSpeed = mode !== 4;
  const showTrail = mode === 0 || mode === 5;
  const showDirection = mode === 0;
  const showIntensity = ![0,4].includes(mode);

  const hex = rgb2hex(s.r ?? 255, s.g ?? 255, s.b ?? 255);
  const presets = ['#FFB54A', '#FF7A3D', '#FF3D82', '#5BC7FF', '#4ADE80', '#9D5BFF', '#FF5470', '#FFFFFF'];

  return (
    <>
      <PageHead title="LEDs" sub={`${LED_MODE_NAMES[mode]} · ${s.led_count ?? 30} pixels · ${s.min_distance ?? 30}–${s.max_distance ?? 300} cm`}/>

      <div class="card" style="padding: 18px; margin-bottom: 14px;">
        <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 10px;">
          <span class="smallcaps">Live preview · {LED_MODE_NAMES[mode]}</span>
          <span class="chip mono">distance {Math.round(live.distance||0)} cm</span>
        </div>
        <LedPreview mode={mode} rgb={[s.r??255, s.g??255, s.b??255]} count={s.led_count??30}
          brightness={s.brightness??80} span={s.span??30} distance={live.distance}
          minD={s.min_distance??30} maxD={s.max_distance??300} speed={s.effect_speed} intensity={s.effect_intensity} height={72}/>
      </div>

      <div class="led-grid" style="display: grid; grid-template-columns: minmax(0, 1.4fr) minmax(0, 1fr); gap: 14px;">
        <div class="card">
          <div class="card-head"><span class="smallcaps">Mode</span></div>
          <div class="card-body">
            <div style="display: grid; grid-template-columns: repeat(auto-fill, minmax(155px, 1fr)); gap: 10px;">
              {LED_MODE_NAMES.map((name, i) => (
                <button onClick={() => save({ light_mode: i })} style={`text-align: left; padding: 0; width: 100%; background: ${mode === i ? 'linear-gradient(135deg, rgba(255,181,74,0.08), rgba(255,61,130,0.08))' : 'var(--bg-2)'}; border: ${mode === i ? '1px solid rgba(255,122,61,0.55)' : '1px solid var(--line)'}; border-radius: 12px; overflow: hidden; cursor: pointer; color: inherit; transition: border-color .15s;`}>
                  <div style="position: relative; height: 42px;">
                    <LedPreview mode={i} rgb={[s.r??255, s.g??255, s.b??255]} count={28} brightness={(s.brightness??80)} span={Math.min(8, s.span??8)} distance={(s.min_distance??30) + ((s.max_distance??300) - (s.min_distance??30)) * 0.55} minD={s.min_distance??30} maxD={s.max_distance??300} speed={s.effect_speed} intensity={s.effect_intensity} height={42}/>
                    {mode === i && <div style="position: absolute; top: 6px; right: 6px; width: 20px; height: 20px; border-radius: 999px; background: var(--acc-grad); display: flex; align-items: center; justify-content: center; color: #1A0F08;"><Icon name="check" size={12} stroke={3}/></div>}
                  </div>
                  <div style="padding: 10px 12px 12px;">
                    <div style="font-size: 13px; font-weight: 600;">{name}</div>
                    <div style="font-size: 11px; color: var(--text-3); margin-top: 2px; line-height: 1.4;">{MODE_DESCRIPTIONS[i]}</div>
                  </div>
                </button>
              ))}
            </div>
          </div>
        </div>

        <div style="display: grid; gap: 14px; align-content: start;">
          {showColor && (
            <div class="card">
              <div class="card-head"><span class="smallcaps">Color</span><Icon name="palette" size={13} style={{ color: 'var(--text-3)' }}/></div>
              <div class="card-body">
                <div style="display: flex; gap: 8px; align-items: center; margin-bottom: 12px;">
                  <div style={`width: 36px; height: 36px; border-radius: 8px; background: ${hex}; border: 1px solid var(--line);`}/>
                  <input class="input mono" value={hex} onChange={(e) => {
                    const h = (e.target as HTMLInputElement).value;
                    const [r, g, b] = hex2rgb(h);
                    save({ r, g, b });
                  }}/>
                </div>
                <div class="field-label">Presets</div>
                <div style="display: grid; grid-template-columns: repeat(8, 1fr); gap: 6px;">
                  {presets.map(p => {
                    const [r, g, b] = hex2rgb(p);
                    const on = hex.toUpperCase() === p.toUpperCase();
                    return (
                      <button onClick={() => save({ r, g, b })} title={p} style={`aspect-ratio: 1 / 1; border-radius: 6px; background: ${p}; border: ${on ? '2px solid var(--text-0)' : '1px solid var(--line)'}; cursor: pointer; padding: 0;`}/>
                    );
                  })}
                </div>
              </div>
            </div>
          )}

          <div class="card">
            <div class="card-head"><span class="smallcaps">{LED_MODE_NAMES[mode]} parameters</span></div>
            <div class="card-body" style="display: flex; flex-direction: column; gap: 14px;">
              <NumberAndSlider label="Brightness" value={s.brightness ?? 80} onChange={(v) => save({ brightness: v })} min={0} max={255}/>
              {showSpeed && <NumberAndSlider label="Effect speed" value={s.effect_speed ?? 50} onChange={(v) => save({ effect_speed: v })} min={0} max={100}/>}
              {showIntensity && <NumberAndSlider label="Effect intensity" value={s.effect_intensity ?? 50} onChange={(v) => save({ effect_intensity: v })} min={0} max={100}/>}
              {showTrail && <NumberAndSlider label="Trail length" value={s.trail ?? 0} onChange={(v) => save({ trail: v })} min={0} max={20}/>}
              {showDirection && (
                <div style="display: flex; justify-content: space-between; align-items: center;">
                  <div>
                    <div style="font-size: 13px;">Directional light</div>
                    <div style="font-size: 11px; color: var(--text-3);">Brighter side leads movement</div>
                  </div>
                  <Toggle value={!!s.dir_light} onChange={(v) => save({ dir_light: v ? 1 : 0 })}/>
                </div>
              )}
              <div style="display: flex; justify-content: space-between; align-items: center;">
                <div>
                  <div style="font-size: 13px;">Background mode</div>
                  <div style="font-size: 11px; color: var(--text-3);">Faint always-on color when idle</div>
                </div>
                <Toggle value={!!s.bg_mode} onChange={(v) => save({ bg_mode: v ? 1 : 0 })}/>
              </div>
            </div>
          </div>

          <div class="card">
            <div class="card-head"><span class="smallcaps">Layout</span></div>
            <div class="card-body" style="display: flex; flex-direction: column; gap: 14px;">
              <div>
                <div style="display: flex; justify-content: space-between; margin-bottom: 6px;">
                  <span class="field-label" style="margin-bottom: 0;">Distance window</span>
                  <span class="mono" style="font-size: 12px; color: var(--text-1);">{s.min_distance ?? 30}–{s.max_distance ?? 300} cm</span>
                </div>
                <DualHandleRange minVal={s.min_distance ?? 30} maxVal={s.max_distance ?? 300}
                  onChange={({ minVal, maxVal }) => save({ min_distance: minVal, max_distance: maxVal })} min={0} max={500}/>
              </div>
              <NumberAndSlider label="Light span" value={s.span ?? 30} onChange={(v) => save({ span: v })} min={1} max={150}/>
              <NumberAndSlider label="Center shift" value={s.center_shift ?? 0} onChange={(v) => save({ center_shift: v })} min={-100} max={100}/>
              <NumberAndSlider label="Strip length" value={s.led_count ?? 30} onChange={(v) => save({ led_count: v })} min={1} max={1500}/>
            </div>
          </div>
        </div>
      </div>
    </>
  );
}

/* ================================================================= */
/*                          C. MOTION                                */
/* ================================================================= */
export function ScreenMotion({ settings, live, reload, setToast }: AppState) {
  const [s, setS] = useState(settings);
  const [showAdvanced, setShowAdvanced] = useState(false);
  useEffect(() => setS(settings), [JSON.stringify(settings)]);
  const debouncedSave = useDebouncedSave(reload, setToast);
  const save = (patch: any) => {
    setS((prev: any) => ({ ...prev, ...patch }));
    debouncedSave(patch);
  };

  /* Both buffers come straight from the firmware (raw_cm + distance_cm over
   * WS at 20 Hz). No client-side alpha simulation — the chart shows what
   * the firmware actually feeds the LED engine. */
  const rawRef = useRef<number[]>(Array(80).fill(0));
  const smoothRef = useRef<number[]>(Array(80).fill(0));
  const [raw, setRaw] = useState(rawRef.current);
  const [smooth, setSmooth] = useState(smoothRef.current);
  useEffect(() => {
    const r = (live as any).raw ?? live.distance;
    const sm = live.distance;
    rawRef.current = [...rawRef.current.slice(1), r];
    smoothRef.current = [...smoothRef.current.slice(1), sm];
    setRaw([...rawRef.current]);
    setSmooth([...smoothRef.current]);
  }, [live]);

  const enabled = !!s.motion_enabled;
  const mode: 'kalman'|'pi' = (s.motion_mode === 'pi') ? 'pi' : 'kalman';
  const response = s.response ?? 50;
  const lookAhead = s.look_ahead_ms ?? 0;
  const outlier = s.outlier_strength ?? 1;

  /* Tooltip text under each main slider — explains *what* the knob does
   * physically, not just its numeric value. Helps users without filtering
   * theory background pick a setting that matches their installation. */
  const responseHint =
    response < 25 ? 'Heavy filtering. Drift is invisible, but fast walk-throughs lag noticeably.' :
    response < 65 ? 'Balanced. Walks render smoothly; jitter is suppressed.' :
                    'Snappy. The strip tracks subtle motion but radar noise leaks through.';
  const lookHint =
    lookAhead === 0    ? 'No predictive lead. The strip lights where the radar last saw you.' :
    lookAhead < 200    ? 'Slight predictive lead — masks ~50 ms render latency.' :
                         'Aggressive prediction. Great for fast stairs, may overshoot near corners.';
  const outlierLabel = ['Off', 'Soft (3-sample)', 'Strong (7-sample)'][outlier] || 'Soft';

  return (
    <>
      <PageHead title="Motion" sub="Smoothing, prediction, outlier rejection"
        right={<button class="btn btn-ghost btn-sm" onClick={() => setShowAdvanced(!showAdvanced)}>
          <Icon name="settings" size={13}/> {showAdvanced ? 'Hide' : 'Show'} advanced
        </button>}/>

      <div class="card" style="padding: 18px; margin-bottom: 14px;">
        <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 14px;">
          <div>
            <div style="font-size: 14px; font-weight: 600;">Motion smoothing</div>
            <div style="font-size: 12px; color: var(--text-2);">{enabled ? 'Filters jitter and predicts velocity' : 'Disabled — strip follows raw radar'}</div>
          </div>
          <Toggle large value={enabled} onChange={(v) => save({ motion_enabled: v ? 1 : 0 })}/>
        </div>

        <div style={`position: relative; opacity: ${enabled ? 1 : 0.45};`}>
          <div style="display: flex; gap: 16px; align-items: center; margin-bottom: 8px; flex-wrap: wrap;">
            <span class="smallcaps">Raw vs smoothed · last 16 s</span>
            <span style="display: inline-flex; align-items: center; gap: 6px; font-size: 11px; color: var(--text-2);">
              <span style="width: 14px; height: 1.5px; background: var(--text-3);"/> raw
            </span>
            <span style="display: inline-flex; align-items: center; gap: 6px; font-size: 11px; color: var(--text-2);">
              <span style="width: 14px; height: 2px; background: var(--acc-orange);"/> smoothed
            </span>
            <span style="margin-left: auto;" class="chip mono">{Math.round(live.distance||0)} cm</span>
          </div>
          <LineChart raw={raw} smooth={smooth}/>
        </div>
      </div>

      {/* Algorithm picker */}
      <div class="card" style="margin-bottom: 14px;">
        <div class="card-head"><span class="smallcaps">Algorithm</span></div>
        <div class="card-body">
          <div style="display: grid; grid-template-columns: 1fr 1fr; gap: 10px;">
            {[
              { id: 'kalman', name: 'Kalman', desc: 'Default. Estimates position + velocity together; energy-aware noise model. Best for stairs.' },
              { id: 'pi',     name: 'Legacy PI', desc: 'EMA + PI controller from v5. Five tunables; familiar if you tuned the Arduino build.' },
            ].map(a => {
              const active = mode === a.id;
              return (
                <button onClick={() => save({ motion_mode: a.id })} style={`padding: 14px; border-radius: 10px; text-align: left; background: ${active ? 'linear-gradient(135deg, rgba(255,181,74,0.06), rgba(255,61,130,0.06))' : 'var(--bg-1)'}; border: ${active ? '1px solid rgba(255,122,61,0.55)' : '1px solid var(--line)'}; cursor: pointer; color: inherit;`}>
                  <div style="font-size: 13px; font-weight: 600;">{a.name}</div>
                  <div style="font-size: 11px; color: var(--text-3); margin-top: 2px;">{a.desc}</div>
                </button>
              );
            })}
          </div>
        </div>
      </div>

      {/* Two main sliders: Response + Look-ahead. These map onto either
        * Kalman process noise (Q_pos, Q_vel) or PI alpha+predict in the
        * firmware — the user shouldn't need to know which. */}
      <div class="card" style="margin-bottom: 14px;">
        <div class="card-head"><span class="smallcaps">Tuning</span></div>
        <div class="card-body" style="display: flex; flex-direction: column; gap: 18px;">
          <div>
            <div style="display: flex; justify-content: space-between; align-items: baseline; margin-bottom: 4px;">
              <label style="font-size: 12px; color: var(--text-2);">Response</label>
              <span style="font-size: 11px; color: var(--text-3);">Calm ⇆ Snappy · {response}</span>
            </div>
            <input type="range" min={0} max={100} value={response} class="range"
              onChange={(e: any) => save({ response: +e.target.value })}/>
            <div style="font-size: 11px; color: var(--text-3); margin-top: 4px;">{responseHint}</div>
          </div>
          <div>
            <div style="display: flex; justify-content: space-between; align-items: baseline; margin-bottom: 4px;">
              <label style="font-size: 12px; color: var(--text-2);">Look-ahead</label>
              <span style="font-size: 11px; color: var(--text-3);">{lookAhead} ms</span>
            </div>
            <input type="range" min={0} max={500} step={10} value={lookAhead} class="range"
              onChange={(e: any) => save({ look_ahead_ms: +e.target.value })}/>
            <div style="font-size: 11px; color: var(--text-3); margin-top: 4px;">{lookHint}</div>
          </div>
          <div>
            <div style="display: flex; justify-content: space-between; align-items: baseline; margin-bottom: 6px;">
              <label style="font-size: 12px; color: var(--text-2);">Outlier rejection</label>
              <span style="font-size: 11px; color: var(--text-3);">{outlierLabel}</span>
            </div>
            <div style="display: grid; grid-template-columns: repeat(3, 1fr); gap: 6px;">
              {[
                { v: 0, name: 'Off',    note: 'Trust radar 1:1' },
                { v: 1, name: 'Soft',   note: 'Median of last 3' },
                { v: 2, name: 'Strong', note: 'Median of last 7' },
              ].map(o => {
                const active = outlier === o.v;
                return (
                  <button onClick={() => save({ outlier_strength: o.v })} style={`padding: 10px; border-radius: 8px; background: ${active ? 'var(--bg-2)' : 'var(--bg-1)'}; border: ${active ? '1px solid rgba(255,122,61,0.55)' : '1px solid var(--line)'}; cursor: pointer; color: inherit; text-align: center;`}>
                    <div style="font-size: 12px; font-weight: 500;">{o.name}</div>
                    <div style="font-size: 10px; color: var(--text-3);">{o.note}</div>
                  </button>
                );
              })}
            </div>
            <div style="font-size: 11px; color: var(--text-3); margin-top: 6px;">Stronger rejection masks single-sample radar glitches but adds 1-2 frames of lag.</div>
          </div>
        </div>
      </div>

      {/* Advanced (collapsed by default) — exposes the v5 PI knobs for power
        * users. Has visible effect only when Algorithm = Legacy PI. */}
      {showAdvanced && (
        <div class="card" style="margin-bottom: 14px; border-color: var(--line-soft);">
          <div class="card-head" style="display: flex; justify-content: space-between; align-items: center;">
            <span class="smallcaps">Advanced — Legacy PI gains</span>
            <span class="chip" style={`background: ${mode === 'pi' ? 'var(--bg-2)' : 'transparent'}; color: ${mode === 'pi' ? 'var(--text-1)' : 'var(--text-3)'};`}>
              {mode === 'pi' ? 'Active' : 'Inactive in Kalman mode'}
            </span>
          </div>
          <div class="card-body" style="display: flex; flex-direction: column; gap: 14px;">
            <NumberAndSlider label="Position smoothing α" value={s.pos_smooth_x1k ?? 200} onChange={(v) => save({ pos_smooth_x1k: v })} min={10} max={800} suffix="× 1/1000"/>
            <NumberAndSlider label="Velocity smoothing α" value={s.vel_smooth_x1k ?? 100} onChange={(v) => save({ vel_smooth_x1k: v })} min={10} max={500} suffix="× 1/1000"/>
            <NumberAndSlider label="Prediction factor"   value={s.predict_x1k    ?? 500} onChange={(v) => save({ predict_x1k:    v })} min={0}  max={2000} suffix="× 1/1000"/>
            <NumberAndSlider label="P gain" value={s.p_gain_x1k ?? 100} onChange={(v) => save({ p_gain_x1k: v })} min={0} max={1000} suffix="× 1/1000"/>
            <NumberAndSlider label="I gain" value={s.i_gain_x1k ?? 10}  onChange={(v) => save({ i_gain_x1k: v })} min={0} max={200}  suffix="× 1/1000"/>
            <div style="padding: 10px; background: var(--bg-1); border-radius: 8px; font-size: 11px; color: var(--text-2); line-height: 1.5; display: flex; gap: 8px;">
              <Icon name="info" size={13} style={{ color: 'var(--info)', flexShrink: 0, marginTop: 1 }}/>
              <span>These are the v5 firmware knobs. In Kalman mode the Response slider above replaces them. Higher P responds faster but overshoots; higher I corrects steady-state drift over time.</span>
            </div>
          </div>
        </div>
      )}
    </>
  );
}

/* ================================================================= */
/*                          D. HARDWARE                              */
/* ================================================================= */
export function ScreenHardware({ setToast, reload }: AppState) {
  const [profiles, setProfiles] = useState<any>(null);
  const [kinds, setKinds] = useState<any>(null);
  const [activeBoard, setActiveBoard] = useState('');
  const [activeRadar, setActiveRadar] = useState('');
  const [pins, setPins] = useState<any>({});
  const [busy, setBusy] = useState(false);
  const [needsReboot, setNeedsReboot] = useState(false);

  useEffect(() => {
    Promise.all([getJSON('/api/board/profiles'), getJSON('/api/radar/kinds'), getJSON('/api/settings')])
      .then(([p, k, st]) => {
        setProfiles(p); setKinds(k); setActiveBoard(p.active); setActiveRadar(k.active);
        const prof = p.profiles.find((x: any) => x.id === p.active) || p.profiles[0];
        setPins({
          led_pin: st.led_pin ?? prof.led_pin,
          radar_rx: st.radar_rx ?? prof.radar_rx,
          radar_tx: st.radar_tx ?? prof.radar_tx,
          button_pin: st.button_pin ?? prof.button,
          status_led_pin: st.status_led_pin ?? prof.status_led,
        });
      }).catch((e) => setToast(e.message || 'Load failed', 'err'));
  }, []);

  if (!profiles || !kinds) return <><PageHead title="Hardware"/><div class="card"><div class="card-body">Loading…</div></div></>;
  const profile = profiles.profiles.find((p: any) => p.id === activeBoard) || profiles.profiles[0];
  const unsafe: number[] = profile.unsafe || [];

  const onBoardChange = (id: string) => {
    setActiveBoard(id);
    const np = profiles.profiles.find((p: any) => p.id === id);
    if (np) setPins({ led_pin: np.led_pin, radar_rx: np.radar_rx, radar_tx: np.radar_tx, button_pin: np.button, status_led_pin: np.status_led });
    setNeedsReboot(true);
  };

  const onRadarChange = (id: string) => { setActiveRadar(id); setNeedsReboot(true); };
  const onPinChange = (k: string, v: number) => { setPins({ ...pins, [k]: v }); setNeedsReboot(true); };

  const save = async () => {
    setBusy(true);
    try {
      await postJSON('/api/board', { id: activeBoard, radar_kind: activeRadar, ...pins });
      setToast('Saved');
    } catch (e: any) { setToast(e.message || 'Save failed', 'err'); }
    finally { setBusy(false); }
  };

  const saveReboot = async () => {
    if (!confirm('Save and reboot device now? You will lose connection for ~10 seconds.')) return;
    setBusy(true);
    try {
      await postJSON('/api/board', { id: activeBoard, radar_kind: activeRadar, ...pins });
      await postJSON('/api/reboot', {});
      setToast('Rebooting — refresh in 10 s');
    } catch (e: any) { setToast(e.message || 'Save failed', 'err'); }
    finally { setBusy(false); setNeedsReboot(false); }
  };

  const PINS: [string, string, string, string][] = [
    ['led_pin', 'LED data', 'led', 'led_pin'],
    ['radar_rx', 'Radar RX', 'radar', 'radar_rx'],
    ['radar_tx', 'Radar TX', 'radar', 'radar_tx'],
    ['button_pin', 'Button', 'pin', 'button'],
    ['status_led_pin', 'Status LED', 'bolt', 'status_led'],
  ];

  return (
    <>
      <PageHead title="Hardware" sub="Board profile, radar, and pin assignments"
        right={needsReboot ? <button class="btn btn-primary" onClick={saveReboot} disabled={busy}><Icon name="refresh" size={13}/> Save & reboot</button> : <button class="btn" onClick={save} disabled={busy}>Save</button>}/>

      <div class="hw-grid" style="display: grid; grid-template-columns: 1fr 1fr; gap: 14px; margin-bottom: 14px;">
        <div class="card">
          <div class="card-head"><span class="smallcaps">Board profile</span><Icon name="chip" size={13} style={{ color: 'var(--text-3)' }}/></div>
          <div class="card-body">
            <div style="display: flex; flex-direction: column; gap: 6px;">
              {profiles.profiles.map((p: any) => {
                const on = activeBoard === p.id;
                return (
                  <button onClick={() => onBoardChange(p.id)} style={`display: flex; align-items: center; gap: 12px; padding: 12px 14px; border-radius: 10px; text-align: left; background: ${on ? 'linear-gradient(135deg, rgba(255,181,74,0.06), rgba(255,61,130,0.06))' : 'var(--bg-1)'}; border: ${on ? '1px solid rgba(255,122,61,0.55)' : '1px solid var(--line)'}; cursor: pointer; color: inherit;`}>
                    <div style="width: 32px; height: 32px; border-radius: 8px; background: var(--bg-3); display: flex; align-items: center; justify-content: center; color: var(--text-2);"><Icon name="cpu" size={16}/></div>
                    <div style="flex: 1;">
                      <div style="font-size: 13px; font-weight: 500;">{p.display}{p.validated ? '' : ' · untested'}</div>
                      <div style="font-size: 11px; color: var(--text-3);">{p.mcu} · max GPIO {p.max_gpio}</div>
                    </div>
                    {on && <Icon name="check" size={14} style={{ color: 'var(--acc-orange)' }}/>}
                  </button>
                );
              })}
            </div>
          </div>
        </div>

        <div class="card">
          <div class="card-head"><span class="smallcaps">Radar</span><Icon name="radar" size={13} style={{ color: 'var(--text-3)' }}/></div>
          <div class="card-body" style="display: flex; flex-direction: column; gap: 10px;">
            {kinds.kinds.map((r: any) => {
              const on = activeRadar === r.id;
              return (
                <button onClick={() => onRadarChange(r.id)} style={`display: flex; align-items: center; gap: 12px; padding: 14px; border-radius: 10px; text-align: left; background: ${on ? 'linear-gradient(135deg, rgba(255,181,74,0.06), rgba(255,61,130,0.06))' : 'var(--bg-1)'}; border: ${on ? '1px solid rgba(255,122,61,0.55)' : '1px solid var(--line)'}; cursor: pointer; color: inherit;`}>
                  <div style="flex: 1;">
                    <div style="font-size: 13px; font-weight: 600;">{r.display}</div>
                    <div style="font-size: 11px; color: var(--text-3);">{r.note}</div>
                  </div>
                  {on && <Icon name="check" size={14} style={{ color: 'var(--acc-orange)' }}/>}
                </button>
              );
            })}
          </div>
        </div>
      </div>

      <div class="card">
        <div class="card-head"><span class="smallcaps">Pin map</span><span class="chip mono">{profile.display}</span></div>
        <div class="card-body">
          <div style="display: grid; grid-template-columns: repeat(auto-fit, minmax(180px, 1fr)); gap: 14px;">
            {PINS.map(([key, label, icon, profKey]) => {
              const cur = pins[key] ?? (profile as any)[profKey];
              return (
                <div>
                  <div style="display: flex; align-items: center; gap: 6px; margin-bottom: 6px;">
                    <Icon name={icon} size={12} style={{ color: 'var(--text-3)' }}/>
                    <span class="field-label" style="margin-bottom: 0;">{label}</span>
                  </div>
                  <select class="select mono" value={cur} onChange={(e) => onPinChange(key, parseInt((e.target as HTMLSelectElement).value))}>
                    {Array.from({ length: profile.max_gpio + 1 }, (_, i) => i)
                      .filter(p => !unsafe.includes(p))
                      .map(p => <option value={p}>GPIO {p}{p === (profile as any)[profKey] ? ' · default' : ''}</option>)}
                  </select>
                </div>
              );
            })}
          </div>
          <div style="margin-top: 14px; padding: 10px; background: var(--bg-1); border-radius: 8px; font-size: 11px; color: var(--text-2); display: flex; gap: 8px;">
            <Icon name="info" size={13} style={{ color: 'var(--info)', flexShrink: 0, marginTop: 1 }}/>
            <span>Strapping pins are disabled — they affect boot mode and shouldn't drive an LED strip or radar UART. {unsafe.length} hidden ({unsafe.join(', ')}).</span>
          </div>
        </div>
      </div>
    </>
  );
}

/* ================================================================= */
/*                          F. NETWORK                               */
/* ================================================================= */
export function ScreenNetwork({ setToast }: AppState) {
  const [wifi, setWifi] = useState<any>(null);
  const [scan, setScan] = useState<any[] | null>(null);
  const [pwd, setPwd] = useState('');
  const [host, setHost] = useState('');
  const [apMode, setApMode] = useState('auto');
  const [confirm, setConfirm] = useState(false);
  const [scanning, setScanning] = useState(false);
  const [joinSsid, setJoinSsid] = useState<string | null>(null);

  const refresh = () => getJSON('/api/wifi').then(w => { setWifi(w); setApMode(w.ap_mode); setHost(w.hostname || ''); });
  const doScan = async () => {
    setScanning(true);
    try { const r = await getJSON('/api/wifi/scan'); setScan(r.networks); }
    catch (e: any) { setToast(e.message || 'Scan failed', 'err'); }
    finally { setScanning(false); }
  };
  useEffect(() => { refresh(); }, []);

  const join = async () => {
    if (!joinSsid) return;
    try { await postJSON('/api/wifi', { ssid: joinSsid, pass: pwd, hostname: host || undefined }); setToast('Saved · reconnecting'); setJoinSsid(null); setPwd(''); setTimeout(refresh, 4000); }
    catch (e: any) { setToast(e.message || 'Join failed', 'err'); }
  };
  const saveApMode = async (m: string) => { setApMode(m); try { await postJSON('/api/wifi', { ap_mode: m }); setToast('AP mode saved'); refresh(); } catch (e: any) { setToast(e.message, 'err'); } };
  const saveHost = async () => { try { await postJSON('/api/wifi', { hostname: host }); setToast('Hostname saved'); } catch (e: any) { setToast(e.message, 'err'); } };
  const forget = async () => {
    if (!confirm) { setConfirm(true); return; }
    try { await postJSON('/api/wifi', { forget_sta: true }); setToast('Reset · device returns to AP mode'); setConfirm(false); refresh(); }
    catch (e: any) { setToast(e.message, 'err'); }
  };

  if (!wifi) return <><PageHead title="Network"/><div class="card"><div class="card-body">Loading…</div></div></>;

  return (
    <>
      <PageHead title="Network" sub="Wi-Fi, mDNS, and AP behaviour"/>

      {/* connected card */}
      <div class="card" style="padding: 18px; margin-bottom: 14px; background: linear-gradient(135deg, rgba(91,199,255,0.04), transparent);">
        <div style="display: flex; align-items: center; gap: 16px; flex-wrap: wrap;">
          <div style="width: 44px; height: 44px; border-radius: 12px; background: var(--bg-3); display: flex; align-items: center; justify-content: center; color: var(--info);">
            <Icon name="wifi" size={22}/>
          </div>
          <div style="flex: 1; min-width: 0;">
            <div style="display: flex; align-items: center; gap: 8px;">
              <span style="font-size: 15px; font-weight: 600;">{wifi.sta_connected ? wifi.ssid : 'Not connected'}</span>
              {wifi.sta_connected && <span class="chip"><span class="dot dot-ok"/> connected</span>}
              {wifi.ap_active && <span class="chip"><span class="dot dot-ok"/> AP up</span>}
            </div>
            <div class="mono" style="font-size: 11px; color: var(--text-3); margin-top: 2px;">
              {wifi.ip || '—'} · {wifi.hostname}.local · {wifi.rssi} dBm
            </div>
          </div>
          {wifi.sta_configured && <button class="btn btn-danger" onClick={forget}>{confirm ? 'Confirm reset' : 'Reset Wi-Fi'}</button>}
        </div>
      </div>

      <div class="card" style="margin-bottom: 14px;">
        <div class="card-head"><span class="smallcaps">Available networks</span>
          <button class="btn btn-sm" onClick={doScan} disabled={scanning}><Icon name="refresh" size={11}/> {scanning ? 'Scanning…' : 'Scan'}</button>
        </div>
        <div class="card-body" style="display: flex; flex-direction: column; gap: 4px;">
          {scan === null && <div style="font-size: 12px; color: var(--text-3); padding: 12px;">Click Scan to discover networks.</div>}
          {scan?.length === 0 && <div style="font-size: 12px; color: var(--text-3); padding: 12px;">No networks found.</div>}
          {scan?.map(n => {
            const isCurrent = wifi.sta_connected && n.ssid === wifi.ssid;
            const bars = n.rssi > -60 ? 4 : n.rssi > -68 ? 3 : n.rssi > -76 ? 2 : 1;
            return (
              <div style={`display: flex; align-items: center; gap: 12px; padding: 10px 12px; border-radius: 8px; background: ${isCurrent ? 'var(--bg-1)' : 'transparent'}; border: ${isCurrent ? '1px solid var(--line)' : '1px solid transparent'};`}>
                <div style="display: flex; align-items: end; gap: 1.5px; height: 16px;">
                  {[1,2,3,4].map(i => <div style={`width: 3px; height: ${i*4}px; background: ${i <= bars ? 'var(--text-1)' : 'var(--bg-4)'}; border-radius: 1px;`}/>)}
                </div>
                <div style="flex: 1; min-width: 0;">
                  <div style="font-size: 13px;">{n.ssid}</div>
                  <div class="mono" style="font-size: 11px; color: var(--text-3);">{n.rssi} dBm · {n.secure ? 'WPA2' : 'open'}</div>
                </div>
                {isCurrent ? <span class="chip">current</span> : <button class="btn btn-sm" onClick={() => setJoinSsid(n.ssid)}>Join</button>}
              </div>
            );
          })}
        </div>
      </div>

      {joinSsid && (
        <div class="card" style="margin-bottom: 14px; padding: 14px; border-color: var(--acc-orange);">
          <div class="smallcaps" style="margin-bottom: 8px;">Join "{joinSsid}"</div>
          <input class="input" type="password" value={pwd} placeholder="Password (leave blank for open)" onInput={(e) => setPwd((e.target as HTMLInputElement).value)} style="margin-bottom: 8px;"/>
          <div style="display: flex; gap: 8px;">
            <button class="btn btn-primary" onClick={join}>Connect</button>
            <button class="btn" onClick={() => { setJoinSsid(null); setPwd(''); }}>Cancel</button>
          </div>
        </div>
      )}

      <div class="hw-grid" style="display: grid; grid-template-columns: 1fr 1fr; gap: 14px;">
        <div class="card">
          <div class="card-head"><span class="smallcaps">Hostname</span></div>
          <div class="card-body">
            <span class="field-label">mDNS name</span>
            <div style="display: flex; gap: 6px; align-items: center;">
              <input class="input mono" value={host} onInput={(e) => setHost((e.target as HTMLInputElement).value.replace(/[^a-z0-9-]/g, ''))}/>
              <span class="mono" style="font-size: 12px; color: var(--text-3);">.local</span>
              <button class="btn btn-sm" onClick={saveHost}>Save</button>
            </div>
          </div>
        </div>

        <div class="card">
          <div class="card-head"><span class="smallcaps">AP behaviour</span></div>
          <div class="card-body" style="display: flex; flex-direction: column; gap: 8px;">
            {[
              { id: 'auto', name: 'Auto', desc: 'AP off when STA connected' },
              { id: 'always', name: 'Always on', desc: 'AP up at all times — local fallback' },
              { id: 'sta_only', name: 'STA only', desc: 'AP off, ESP-NOW uses STA channel' },
            ].map(m => {
              const on = apMode === m.id;
              return (
                <button onClick={() => saveApMode(m.id)} style={`text-align: left; padding: 10px 12px; border-radius: 8px; background: ${on ? 'rgba(255,122,61,0.08)' : 'var(--bg-1)'}; border: ${on ? '1px solid rgba(255,122,61,0.55)' : '1px solid var(--line)'}; cursor: pointer; color: inherit;`}>
                  <div style="font-size: 13px; font-weight: 500;">{m.name}</div>
                  <div style="font-size: 11px; color: var(--text-3);">{m.desc}</div>
                </button>
              );
            })}
          </div>
        </div>
      </div>
    </>
  );
}

/* ================================================================= */
/*                          G. SYSTEM                                */
/* ================================================================= */
export function ScreenSystem({ version, setToast }: AppState) {
  const [pwd, setPwd] = useState('');
  const [showPwd, setShowPwd] = useState(false);
  const [authReq, setAuthReq] = useState(false);
  const [otaName, setOtaName] = useState<string | null>(null);
  const [otaFile, setOtaFile] = useState<File | null>(null);
  const [otaProg, setOtaProg] = useState(-1);
  const [confirmText, setConfirmText] = useState('');
  const fileRef = useRef<HTMLInputElement>(null);

  useEffect(() => setAuthReq(!!version.auth_enabled), [version.auth_enabled]);

  const setPassword = async () => {
    if (pwd && pwd.length < 8) { setToast('Password must be ≥ 8 characters', 'err'); return; }
    try {
      const r = await postJSON('/api/auth/password', { password: pwd });
      setToast(pwd ? 'Password set · auth enabled' : 'Password cleared · auth disabled');
      setPwd('');
      setAuthReq(!!r.auth_enabled);
    } catch (e: any) { setToast(e.message || 'Failed', 'err'); }
  };

  const onFile = (f: File | null | undefined) => {
    if (!f) return;
    if (!f.name.endsWith('.bin')) { setToast('Pick a .bin file', 'err'); return; }
    setOtaFile(f); setOtaName(f.name); setOtaProg(-1);
  };

  const flash = async () => {
    if (!otaFile) return;
    setOtaProg(0);
    try {
      await postBinary('/api/ota', otaFile, p => setOtaProg(p));
      setToast('Flashed · device rebooting');
      setOtaProg(1);
      setTimeout(() => { setOtaName(null); setOtaFile(null); setOtaProg(-1); }, 3000);
    } catch (e: any) { setToast(e.message || 'OTA failed', 'err'); setOtaProg(-1); }
  };

  const reboot = async () => {
    if (!confirm('Reboot device? You will lose connection for ~10 seconds.')) return;
    try { await postJSON('/api/reboot', {}); setToast('Rebooting'); }
    catch (e: any) { setToast(e.message, 'err'); }
  };

  const exportConfig = async () => {
    try {
      const r = await getJSON('/api/settings');
      const blob = new Blob([JSON.stringify(r, null, 2)], { type: 'application/json' });
      const url = URL.createObjectURL(blob);
      const a = document.createElement('a');
      a.href = url; a.download = `${version.hostname || 'ambisense'}-config.json`;
      a.click();
      URL.revokeObjectURL(url);
      setToast('Exported');
    } catch (e: any) { setToast(e.message, 'err'); }
  };

  return (
    <>
      <PageHead title="System" sub="Firmware, auth, and diagnostics"/>

      <div class="hw-grid" style="display: grid; grid-template-columns: 1.2fr 1fr; gap: 14px; margin-bottom: 14px;">
        <div class="card">
          <div class="card-head"><span class="smallcaps">Firmware</span><span class="chip mono">{version.version || '—'}</span></div>
          <div class="card-body" style="display: flex; flex-direction: column; gap: 12px;">
            <div style="display: flex; align-items: center; gap: 12px; padding: 12px 14px; background: var(--bg-1); border-radius: 10px;">
              <Icon name="cpu" size={18} style={{ color: 'var(--text-2)' }}/>
              <div style="flex: 1;">
                <div class="mono" style="font-size: 13px;">{version.version} <span style="color: var(--text-3);">· {version.target}</span></div>
                <div style="font-size: 11px; color: var(--text-3);">ESP-IDF {version.idf_version} · built {version.build_date}</div>
              </div>
              <button class="btn btn-sm" onClick={reboot}><Icon name="refresh" size={12}/> Reboot</button>
            </div>

            <label
              onDrop={(e: any) => { e.preventDefault(); onFile(e.dataTransfer.files[0]); }}
              onDragOver={(e) => e.preventDefault()}
              onClick={() => fileRef.current?.click()}
              style="display: flex; flex-direction: column; align-items: center; justify-content: center; gap: 6px; padding: 24px; border: 1.5px dashed var(--line); border-radius: 10px; background: var(--bg-1); cursor: pointer; text-align: center;">
              <Icon name="upload" size={20} style={{ color: 'var(--text-2)' }}/>
              <div style="font-size: 13px;">{otaName || 'Drop firmware .bin or click to select'}</div>
              <div style="font-size: 11px; color: var(--text-3);">Bootloader rollback armed — failed flashes auto-revert</div>
              <input ref={fileRef} type="file" accept=".bin" style="display: none;" onChange={(e: any) => onFile(e.target.files?.[0])}/>
            </label>

            {otaProg >= 0 && otaProg < 1 && (
              <div>
                <div class="bar"><div class="bar-fill" style={`width: ${(otaProg*100).toFixed(1)}%`}/></div>
                <div style="font-size: 11px; color: var(--text-3); text-align: right; margin-top: 4px;">Uploading {(otaProg*100).toFixed(0)}%</div>
              </div>
            )}

            {otaName && otaProg < 0 && (
              <button class="btn btn-primary" style="align-self: stretch; justify-content: center;" onClick={flash}>Flash & reboot</button>
            )}
          </div>
        </div>

        <div class="card">
          <div class="card-head"><span class="smallcaps">Diagnostics</span></div>
          <div class="card-body">
            <div style="display: grid; grid-template-columns: 1fr 1fr; gap: 12px 18px;">
              <DevField k="Free heap" v={`${((version.free_heap||0)/1024).toFixed(1)} kB`}/>
              <DevField k="Min free heap" v={`${((version.min_free_heap||0)/1024).toFixed(1)} kB`}/>
              <DevField k="Uptime" v={fmtUptime(version.uptime_s||0)}/>
              <DevField k="MAC" v={version.mac || '—'}/>
              <DevField k="Board" v={version.board || '—'}/>
              <DevField k="RSSI" v={`${version.rssi||0} dBm`}/>
            </div>
          </div>
        </div>
      </div>

      <div class="card" style="margin-bottom: 14px;">
        <div class="card-head"><span class="smallcaps">Auth</span>
          <span class="chip" style={`color: ${authReq ? 'var(--ok)' : 'var(--text-2)'};`}>
            <span class={`dot ${authReq ? 'dot-ok' : 'dot-off'}`}/> {authReq ? 'enabled' : 'disabled'}
          </span>
        </div>
        <div class="card-body">
          <span class="field-label">Password (≥ 8 chars · empty disables auth)</span>
          <div style="display: flex; gap: 6px;">
            <input class="input mono" type={showPwd ? 'text' : 'password'} value={pwd} placeholder="Set a password" onInput={(e) => setPwd((e.target as HTMLInputElement).value)}/>
            <button class="btn" onClick={() => setShowPwd(x => !x)}><Icon name={showPwd ? 'eyeOff' : 'eye'} size={14}/></button>
            <button class="btn btn-primary" onClick={setPassword}>{pwd ? 'Set password' : 'Disable auth'}</button>
          </div>
          <div style="font-size: 11px; color: var(--text-3); margin-top: 6px;">
            Required only on this network. Local mDNS access uses cookie sessions.
          </div>
        </div>
      </div>

      <div class="hw-grid" style="display: grid; grid-template-columns: 1fr 1fr; gap: 14px;">
        <div class="card">
          <div class="card-head"><span class="smallcaps">JSON config</span></div>
          <div class="card-body" style="display: flex; gap: 8px;">
            <button class="btn" onClick={exportConfig}><Icon name="download" size={13}/> Export</button>
            <button class="btn" onClick={() => setToast('Import: drag JSON → /api/settings (coming next)')}>
              <Icon name="upload" size={13}/> Import
            </button>
          </div>
        </div>

        <div class="card" style="border-color: rgba(255,84,112,0.25);">
          <div class="card-head"><span class="smallcaps" style="color: var(--err);">Factory reset</span></div>
          <div class="card-body">
            <div style="font-size: 12px; color: var(--text-2); margin-bottom: 10px;">
              Type <span class="mono" style="color: var(--err);">{version.hostname}</span> to confirm
            </div>
            <div style="display: flex; gap: 6px;">
              <input class="input mono" value={confirmText} placeholder={version.hostname} onInput={(e) => setConfirmText((e.target as HTMLInputElement).value)}/>
              <button class="btn btn-danger" disabled={confirmText !== version.hostname}
                style={`opacity: ${confirmText !== version.hostname ? 0.4 : 1};`}
                onClick={async () => {
                  if (confirmText !== version.hostname) return;
                  try { await postJSON('/api/factory_reset', {}); setToast('Erasing · device reboots'); setConfirmText(''); }
                  catch (e: any) { setToast(e.message || 'Reset failed', 'err'); }
                }}>
                Erase
              </button>
            </div>
          </div>
        </div>
      </div>
    </>
  );
}
