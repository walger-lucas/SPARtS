// src/services/apiSparts.js
const sleep = ms => new Promise(r => setTimeout(r, ms));

export default class ApiSpartsClient {
  /**
   * @param {string} baseUrl - ex: 'http://192.168.4.1' ou '/api/sparts' (proxy)
   * @param {{pollInterval?: number, timeout?: number}} opts
   */
  constructor(baseUrl = '/api/sparts', opts = {}) {
    this.baseUrl = baseUrl.replace(/\/$/, '');
    this.status = 'OK';
    this.imageItemName = '';
    this.bins = [];
    this.pollInterval = opts.pollInterval ?? 2000; // ms
    this.timeout = opts.timeout ?? 1000000; // ms
  }

  // Internal helper: fetch with JSON and error handling
  async _fetchJson(path, options = {}) {
    const url = `${this.baseUrl}${path}`;
    const res = await fetch(url, options);
    const text = await res.text();
    let json = null;
    try { json = text ? JSON.parse(text) : null; } catch (e) { /* ignore parse error */ }
    return { ok: res.ok, status: res.status, json, text };
  }

  async saveJSON(json){
  
  }

  // Poll /status until state === 'FINISHED' (or timeout)
  async awaitFinish({ interval = this.pollInterval, timeout = this.timeout } = {}) {
    const start = Date.now();
    let finished = false;
    let lastData = null;

    while (!finished) {
      if (Date.now() - start > timeout) {
        throw new Error('Timeout waiting for device to finish');
      }

      const { ok, json } = await this._fetchJson('/status', { method: 'GET' });
      if (ok && json) {
        lastData = json;
        finished = json.state === 'FINISHED';
        if (finished) {
          // set client state
          this.status = json.status;
          this.imageItemName = json.item_name ?? this.imageItemName;
          return json;
        }
      } else {
        // se o endpoint falhar, aguarde e tente de novo (pode ser temporário)
      }

      await sleep(interval);
    }

    // should not reach here
    return lastData;
  }

  // POST helper
  async _post(path, body = {}) {
    const opts = {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: Object.keys(body).length ? JSON.stringify(body) : '{}'
    };
    return this._fetchJson(path, opts);
  }

  // Endpoints equivalentes
  async setup(url) {
    const { ok, status, json } = await this._post('/setup', { image_processing_uri: url });
    console.log(url)
    this.getBins()
    if (!ok) throw new Error(`setup failed (${status})`);
    await this.awaitFinish();
    return { ok: true, status: this.status, item_name: this.imageItemName };
  }

  async map() {
    const { ok, status } = await this._post('/remap');
    if (!ok) throw new Error(`remap failed (${status})`);
    await this.awaitFinish();
    return { ok: true, status: this.status };
  }

  async organize(reweight) {
    const body = { reweight: reweight }
    const { ok, status } = await this._post('/reorganize', body);
    if (!ok) throw new Error(`reorganize failed (${status})`);
    await this.awaitFinish();
    return { ok: true, status: this.status };
  }

  async autoStore() {
    const { ok, status } = await this._post('/auto_store');
    if (!ok) throw new Error(`auto_store failed (${status})`);
    await this.awaitFinish();
    return { ok: true, status: this.status };
  }

  async image() {
    const { ok, status } = await this._post('/capture_image');
    if (!ok) throw new Error(`capture_image failed (${status})`);
    await this.awaitFinish();
    return { ok: true, status: this.status, itemName: this.imageItemName };
  }

  async getBins() {
    const { ok, status, json } = await this._fetchJson('/bins', { method: 'GET' });
    if (!ok) throw new Error(`getBins failed (${status})`);
    this.bins = Array.isArray(json?.bins) ? json.bins : [];
    return { ok: true, bins: this.bins };
  }

  async store(changeType = false, itemNameToChange = 'None') {
    const body = changeType ? { item_name: itemNameToChange } : {};
    const { ok, status } = await this._post('/store', body);
    if (!ok) throw new Error(`store failed (${status})`);
    await this.awaitFinish();
    return { ok: true, status: this.status };
  }

  async retrieve(rfidText = '000000000000000000000000') {
    const { ok, status } = await this._post('/retrieve', { rfid: rfidText });
    if (!ok) throw new Error(`retrieve failed (${status})`);
    await this.awaitFinish();
    return { ok: true, status: this.status };
  }

  async read(id = 255) {
    const { ok, status } = await this._post('/read', { id });
    if (!ok) throw new Error(`read failed (${status})`);
    await this.awaitFinish();
    return { ok: true, status: this.status };
  }
}
