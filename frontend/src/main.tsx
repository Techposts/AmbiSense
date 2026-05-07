/** App shell — sidebar (logo + nav + IP/board footer) + sticky header
 *  (page name · hostname.local + live distance chip + RSSI + theme).
 *  Faithful port of frontend/design-source/project/app.jsx. */
import { render } from 'preact';
import { useEffect, useState } from 'preact/hooks';
import { getJSON, liveSocket } from './api';
import { Toast, useToaster } from './components';
import { Icon, LogoMark, Wordmark } from './atoms';
import { ScreenLive, ScreenLeds, ScreenMotion, ScreenHardware, ScreenNetwork, ScreenSystem } from './screens';
import './styles.css';

type Tab = 'live'|'leds'|'motion'|'hardware'|'network'|'system';

const TABS: { id: Tab; name: string; icon: string }[] = [
  { id: 'live',     name: 'Live',     icon: 'dashboard' },
  { id: 'leds',     name: 'LEDs',     icon: 'led' },
  { id: 'motion',   name: 'Motion',   icon: 'motion' },
  { id: 'hardware', name: 'Hardware', icon: 'chip' },
  { id: 'network',  name: 'Network',  icon: 'wifi' },
  { id: 'system',   name: 'System',   icon: 'settings' },
];

function App() {
  const [tab, setTab] = useState<Tab>(localStorage.getItem('ambitab') as Tab || 'live');
  const [version, setVersion] = useState<any>({});
  const [settings, setSettings] = useState<any>({});
  const [live, setLive] = useState<any>({ distance: 0, raw: 0, direction: 0, rssi: 0, heap: 0, uptime: 0 });
  const [theme, setTheme] = useState<'dark'|'light'>(localStorage.getItem('ambitheme') as any || 'dark');
  const [wsConnected, setWsConnected] = useState(false);
  const t = useToaster();

  const reload = () => {
    getJSON('/api/version').then(setVersion).catch(() => {});
    getJSON('/api/settings').then(setSettings).catch(() => {});
  };

  useEffect(() => { reload(); }, []);
  useEffect(() => {
    const id = setInterval(() => { if (!document.hidden) getJSON('/api/version').then(setVersion).catch(() => {}); }, 30000);
    return () => clearInterval(id);
  }, []);
  useEffect(() => {
    const close = liveSocket((data) => { setLive(data); setWsConnected(true); });
    return () => { close(); setWsConnected(false); };
  }, []);
  useEffect(() => { document.documentElement.setAttribute('data-theme', theme); localStorage.setItem('ambitheme', theme); }, [theme]);
  useEffect(() => { localStorage.setItem('ambitab', tab); }, [tab]);

  const state = { live, settings, version, toast: t.toast, setToast: t.set, reload };
  const switcher: Record<Tab, any> = { live: ScreenLive, leds: ScreenLeds, motion: ScreenMotion, hardware: ScreenHardware, network: ScreenNetwork, system: ScreenSystem };
  const Screen = switcher[tab];
  const tabName = TABS.find(x => x.id === tab)?.name || '';
  const dist = Math.round(live.distance || 0);
  const ipShort = version.ip || '—';
  const radarKind = settings.radar_kind ? settings.radar_kind.toUpperCase() : '';
  const boardSub = `${(version.target || 'esp32').toUpperCase()}${radarKind ? ` · ${radarKind}` : ''}`;
  const fwVer = version.version || 'v6.x';
  const fwTarget = (version.target || 'esp32').toUpperCase();

  return (
    <div style="display: flex; min-height: 100vh; background: var(--bg-0);">
      {/* Sidebar — desktop only */}
      <aside class="hide-mobile" style="width: 220px; flex-shrink: 0; border-right: 1px solid var(--line); padding: 20px 14px; display: flex; flex-direction: column; gap: 4px; background: var(--bg-0);">
        <div class="brand-block">
          <LogoMark size={42} distance={dist}/>
          <Wordmark font={18} sub={10} version={fwVer} target={fwTarget}/>
        </div>
        {TABS.map(x => {
          const on = tab === x.id;
          return (
            <button onClick={() => setTab(x.id)} style={`display: flex; align-items: center; gap: 10px; padding: 9px 12px; border-radius: 8px; text-align: left; color: ${on ? 'var(--text-0)' : 'var(--text-2)'}; background: ${on ? 'var(--bg-2)' : 'transparent'}; border: ${on ? '1px solid var(--line)' : '1px solid transparent'}; font-size: 13px; font-weight: ${on ? 500 : 400}; cursor: pointer; transition: background .12s, color .12s;`}>
              <Icon name={x.icon} size={15}/>
              {x.name}
            </button>
          );
        })}
        <div style="flex: 1;"/>
        <div class="sidebar-foot">
          <span class="mono">{ipShort}</span><br/>
          {boardSub}
        </div>
      </aside>

      {/* Main column */}
      <div style="flex: 1; min-width: 0; display: flex; flex-direction: column;">
        {/* Sticky header */}
        <header class="app-header">
          <div class="show-mobile" style="display: flex; align-items: center; gap: 10px;">
            <LogoMark size={30} distance={dist}/>
          </div>
          <div style="flex: 1; display: flex; align-items: center; gap: 14px;">
            <div style="display: flex; align-items: baseline; gap: 8px;">
              <span style="font-size: 14px; font-weight: 600; letter-spacing: -0.01em;">{tabName}</span>
              <span class="mono hide-mobile" style="font-size: 11px; color: var(--text-3);">
                {version.hostname ? `${version.hostname}.local` : 'configuring…'}
              </span>
            </div>
          </div>
          <div class="hide-mobile" style="display: flex; align-items: center; gap: 10px;">
            <span class="chip" title="WebSocket /api/live">
              <span class={`dot ${wsConnected ? 'dot-ok' : 'dot-err'}`}/> live · {dist} cm
            </span>
            <span class="chip mono">{live.rssi || 0} dBm</span>
          </div>
          <button class="btn btn-ghost btn-icon" onClick={() => setTheme(theme === 'dark' ? 'light' : 'dark')} title={theme === 'dark' ? 'Light mode' : 'Dark mode'}>
            <Icon name={theme === 'dark' ? 'sun' : 'moon'} size={15}/>
          </button>
        </header>

        {/* Page content */}
        <main class="main" style="padding: 28px 32px 96px; max-width: 1280px;">
          <Screen {...state} />
        </main>

        {/* Mobile bottom-tabs */}
        <nav class="show-mobile" style="position: fixed; bottom: 0; left: 0; right: 0; display: flex; justify-content: space-around; padding: 8px 6px; background: var(--bg-1); border-top: 1px solid var(--line); z-index: 10; overflow-x: auto;">
          {TABS.map(x => {
            const on = tab === x.id;
            return (
              <button onClick={() => setTab(x.id)} style={`display: flex; flex-direction: column; align-items: center; gap: 3px; padding: 6px 8px; font-size: 10px; color: ${on ? 'var(--text-0)' : 'var(--text-2)'}; background: ${on ? 'var(--bg-3)' : 'transparent'}; border-radius: 8px; border: 0; cursor: pointer;`}>
                <Icon name={x.icon} size={16}/>
                {x.name}
              </button>
            );
          })}
        </nav>
      </div>
      {t.toast && <Toast msg={t.toast.msg} kind={t.toast.kind} onDone={t.clear} />}
    </div>
  );
}

render(<App />, document.getElementById('root')!);
