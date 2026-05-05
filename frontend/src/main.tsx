/** App shell: sidebar + tab routing + global state. */
import { render } from 'preact';
import { useEffect, useState } from 'preact/hooks';
import { getJSON, liveSocket } from './api';
import { Toast, useToaster } from './components';
import { ScreenLive, ScreenLeds, ScreenMotion, ScreenMesh, ScreenHardware, ScreenNetwork, ScreenSystem } from './screens';
import './styles.css';

type Tab = 'live'|'leds'|'motion'|'mesh'|'hardware'|'network'|'system';

const TABS: { id: Tab; name: string; icon: string }[] = [
  { id: 'live',     name: 'Live',     icon: '◉' },
  { id: 'leds',     name: 'LEDs',     icon: '✦' },
  { id: 'motion',   name: 'Motion',   icon: '∿' },
  { id: 'mesh',     name: 'Mesh',     icon: '⌬' },
  { id: 'hardware', name: 'Hardware', icon: '⚙' },
  { id: 'network',  name: 'Network',  icon: '⌁' },
  { id: 'system',   name: 'System',   icon: '☼' },
];

function App() {
  const [tab, setTab] = useState<Tab>(localStorage.getItem('ambitab') as Tab || 'live');
  const [version, setVersion] = useState<any>({});
  const [settings, setSettings] = useState<any>({});
  const [live, setLive] = useState<any>({ distance: 0, direction: 0, rssi: 0, heap: 0, uptime: 0, peers: 0, healthy: 0 });
  const [theme, setTheme] = useState<'dark'|'light'>(localStorage.getItem('ambitheme') as any || 'dark');
  const t = useToaster();

  const reload = () => {
    getJSON('/api/version').then(setVersion).catch(() => {});
    getJSON('/api/settings').then(setSettings).catch(() => {});
  };

  useEffect(() => { reload(); }, []);
  useEffect(() => liveSocket(setLive), []);
  useEffect(() => { document.documentElement.setAttribute('data-theme', theme); localStorage.setItem('ambitheme', theme); }, [theme]);
  useEffect(() => { localStorage.setItem('ambitab', tab); }, [tab]);

  const state = { live, settings, version, toast: t.toast, setToast: t.set, reload };

  const switcher: Record<Tab, any> = {
    live: ScreenLive, leds: ScreenLeds, motion: ScreenMotion, mesh: ScreenMesh,
    hardware: ScreenHardware, network: ScreenNetwork, system: ScreenSystem,
  };
  const Screen = switcher[tab];

  return (
    <div class="app">
      <nav class="sidebar">
        <div class="brand">
          <div class="brand-mark">A</div>
          <div>
            <div class="brand-name">AmbiSense<span class="brand-ver"> v6</span></div>
            <div style="font-size: 11px; color: var(--text-3); margin-top: 2px;">
              {version.hostname || 'configuring…'}
            </div>
          </div>
        </div>
        {TABS.map(({ id, name, icon }) => (
          <a class={`navlink ${tab === id ? 'on' : ''}`} onClick={() => setTab(id)}>
            <span class="navlink-icon">{icon}</span>
            <span>{name}</span>
          </a>
        ))}
        <div style="flex: 1;" />
        <a class="navlink" onClick={() => setTheme(theme === 'dark' ? 'light' : 'dark')}>
          <span class="navlink-icon">{theme === 'dark' ? '☀' : '☾'}</span>
          <span>{theme === 'dark' ? 'Light' : 'Dark'}</span>
        </a>
      </nav>
      <main class="main">
        <Screen {...state} />
      </main>
      {t.toast && <Toast msg={t.toast.msg} kind={t.toast.kind} onDone={t.clear} />}
    </div>
  );
}

render(<App />, document.getElementById('root')!);
