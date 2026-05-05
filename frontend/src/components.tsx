/** Shared UI atoms. */
import { ComponentChildren, JSX } from 'preact';
import { useEffect, useState } from 'preact/hooks';

export function Card({ title, right, children }: {
  title?: string; right?: ComponentChildren; children: ComponentChildren;
}) {
  return (
    <div class="card">
      {title && (
        <div class="card-head">
          <h2 class="card-title">{title}</h2>
          {right}
        </div>
      )}
      <div class="card-body">{children}</div>
    </div>
  );
}

export function Toggle({ value, onChange, large }: {
  value: boolean; onChange: (v: boolean) => void; large?: boolean;
}) {
  return (
    <div
      class={`toggle ${value ? 'on' : ''} ${large ? 'toggle-lg' : ''}`}
      onClick={() => onChange(!value)}
      role="switch"
      aria-checked={value}
    />
  );
}

export function Field({ label, children }: { label: string; children: ComponentChildren }) {
  return (
    <div style="margin-bottom: 12px;">
      <label class="field-label">{label}</label>
      {children}
    </div>
  );
}

export function Slider({ value, min, max, onChange, suffix }: {
  value: number; min: number; max: number; onChange: (v: number) => void; suffix?: string;
}) {
  return (
    <div style="display: flex; gap: 12px; align-items: center;">
      <input
        type="range"
        class="range"
        min={min}
        max={max}
        value={value}
        onInput={(e) => onChange(parseInt((e.target as HTMLInputElement).value))}
      />
      <span class="mono" style="min-width: 50px; text-align: right; color: var(--text-1); font-size: 12px;">
        {value}{suffix || ''}
      </span>
    </div>
  );
}

export function Toast({ msg, kind, onDone }: { msg: string; kind: 'ok'|'err'; onDone: () => void }) {
  useEffect(() => {
    const t = setTimeout(onDone, kind === 'err' ? 4500 : 2200);
    return () => clearTimeout(t);
  }, []);
  return <div class={`toast ${kind}`}>{msg}</div>;
}

export function Row({ k, v }: { k: string; v: ComponentChildren }) {
  return (
    <div class="row">
      <span class="lbl">{k}</span>
      <span class="val">{v}</span>
    </div>
  );
}

export function Dot({ kind }: { kind: 'ok'|'warn'|'err'|'off' }) {
  return <span class={`dot dot-${kind}`} />;
}

/* Optimistic save helper: shows a toast based on a promise. */
export function useToaster() {
  const [t, setT] = useState<{ msg: string; kind: 'ok'|'err' } | null>(null);
  return {
    toast: t,
    set: (msg: string, kind: 'ok'|'err' = 'ok') => setT({ msg, kind }),
    clear: () => setT(null),
    track: async <R,>(p: Promise<R>, okMsg = 'Saved'): Promise<R | undefined> => {
      try { const r = await p; setT({ msg: okMsg, kind: 'ok' }); return r; }
      catch (e: any) { setT({ msg: e.message || 'Failed', kind: 'err' }); return undefined; }
    }
  };
}

/* HSV color picker — simple wheel + lightness slider. Returns r/g/b. */
export function ColorPicker({ rgb, onChange }: { rgb: [number,number,number]; onChange: (r:number,g:number,b:number)=>void }) {
  const presets: [number,number,number][] = [
    [255,255,255], [255,170,80], [255,80,80], [255,80,180],
    [180,80,255], [80,180,255], [80,255,180], [255,255,80],
  ];
  return (
    <div class="color-row">
      {presets.map(p => {
        const on = p[0]===rgb[0] && p[1]===rgb[1] && p[2]===rgb[2];
        return (
          <div
            class={`swatch ${on?'on':''}`}
            style={`background: rgb(${p[0]},${p[1]},${p[2]})`}
            onClick={() => onChange(p[0], p[1], p[2])}
          />
        );
      })}
      <input
        type="color"
        value={`#${rgb.map(v => v.toString(16).padStart(2,'0')).join('')}`}
        onInput={(e) => {
          const hex = (e.target as HTMLInputElement).value;
          const r = parseInt(hex.slice(1,3),16);
          const g = parseInt(hex.slice(3,5),16);
          const b = parseInt(hex.slice(5,7),16);
          onChange(r,g,b);
        }}
        style="width: 36px; height: 36px; border: 1px solid var(--line); border-radius: 8px; padding: 2px; background: var(--bg-1); cursor: pointer;"
      />
    </div>
  );
}
