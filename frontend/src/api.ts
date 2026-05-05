/* Thin fetch wrapper. All API endpoints relative; works in dev (Vite proxies
 * /api → device IP) and in prod (UI is served from same origin as the API). */

export async function getJSON<T = any>(url: string): Promise<T> {
  const r = await fetch(url, { credentials: 'same-origin' });
  if (!r.ok) throw new Error(`${r.status}: ${url}`);
  return r.json();
}

export async function postJSON<T = any>(url: string, body: any): Promise<T> {
  const r = await fetch(url, {
    method: 'POST',
    credentials: 'same-origin',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(body),
  });
  if (!r.ok) {
    let msg = String(r.status);
    try { const j = await r.json(); if (j.error) msg = j.error; } catch {}
    throw new Error(msg);
  }
  return r.json();
}

export async function postBinary(url: string, blob: Blob, onProgress?: (n: number) => void): Promise<any> {
  return new Promise((resolve, reject) => {
    const xhr = new XMLHttpRequest();
    xhr.open('POST', url, true);
    xhr.setRequestHeader('Content-Type', 'application/octet-stream');
    xhr.upload.onprogress = (e) => onProgress && onProgress(e.loaded / e.total);
    xhr.onload = () => {
      if (xhr.status >= 200 && xhr.status < 300) {
        try { resolve(JSON.parse(xhr.responseText)); } catch { resolve({}); }
      } else {
        let msg = String(xhr.status);
        try { const j = JSON.parse(xhr.responseText); if (j.error) msg = j.error; } catch {}
        reject(new Error(msg));
      }
    };
    xhr.onerror = () => reject(new Error('network'));
    xhr.send(blob);
  });
}

/* WebSocket live data — auto-reconnects with backoff. */
export function liveSocket(onMsg: (data: any) => void): () => void {
  let stop = false;
  let ws: WebSocket | null = null;
  let backoff = 500;
  const url = (location.protocol === 'https:' ? 'wss://' : 'ws://') + location.host + '/api/live';
  function connect() {
    if (stop) return;
    ws = new WebSocket(url);
    ws.onopen = () => { backoff = 500; };
    ws.onmessage = (e) => {
      try { onMsg(JSON.parse(e.data)); } catch {}
    };
    ws.onclose = () => {
      if (stop) return;
      setTimeout(connect, backoff);
      backoff = Math.min(backoff * 2, 5000);
    };
    ws.onerror = () => ws?.close();
  }
  connect();
  return () => { stop = true; ws?.close(); };
}
