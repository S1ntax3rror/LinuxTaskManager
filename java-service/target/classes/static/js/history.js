// js/history.js

document.addEventListener('DOMContentLoaded', () => {
  // ─── 1) DOM REFERENCES ────────────────────────────────────────────────────
  const timestampEl   = document.getElementById('timestamp');
  const loadAvgEl     = document.getElementById('loadAvg');
  const cpuUtilEl     = document.getElementById('cpuUtil');
  const tasksInfoEl   = document.getElementById('tasksInfo');
  const memoryInfoEl  = document.getElementById('memoryInfo');
  const buffersInfoEl = document.getElementById('buffersInfo');
  const swapInfoEl    = document.getElementById('swapInfo');

  // ─── 2) COMMON CHART SETUP ────────────────────────────────────────────────
  const MAX_POINTS = 60;
  const labels     = Array.from({ length: MAX_POINTS }, (_, i) => i + 1);
  const baseOpts = {
    responsive: true,
    maintainAspectRatio: false,
    scales: {
      x: {
        title: { display: true, text: 'Elapsed (s)' },
        ticks: { autoSkip: true, maxTicksLimit: 12 }
      },
      y: {
        beginAtZero: true
      }
    },
    plugins: { legend: { position: 'top' } }
  };
  function pushTrim(arr, v) {
    arr.shift();
    arr.push(v);
  }

  // ─── 3) NETWORK CHART (unchanged) ────────────────────────────────────────
  const netCtx     = document.getElementById('networkChart').getContext('2d');
  let netDownload  = Array(MAX_POINTS).fill(0);
  let netUpload    = Array(MAX_POINTS).fill(0);
  const netChart = new Chart(netCtx, {
    type: 'line',
    data: {
      labels,
      datasets: [
        {
          label: 'Download (MB/s)',
          data: netDownload,
          borderColor: 'rgba(54,162,235,1)',
          backgroundColor: 'rgba(54,162,235,0.1)',
          fill: true,
          tension: 0.2
        },
        {
          label: 'Upload (MB/s)',
          data: netUpload,
          borderColor: 'rgba(255,99,132,1)',
          backgroundColor: 'rgba(255,99,132,0.1)',
          fill: true,
          tension: 0.2
        }
      ]
    },
    options: {
      ...baseOpts,
      scales: {
        ...baseOpts.scales,
        y: { ...baseOpts.scales.y, title: { display: true, text: 'MB/s' } }
      }
    }
  });
  let lastNetTotals = null;

  // ─── 4) DISK CHART (unchanged) ───────────────────────────────────────────
  const diskCtx  = document.getElementById('diskChart').getContext('2d');
  let diskRead   = Array(MAX_POINTS).fill(0);
  let diskWrite  = Array(MAX_POINTS).fill(0);
  const diskChart = new Chart(diskCtx, {
    type: 'line',
    data: {
      labels,
      datasets: [
        {
          label: 'Read (MB/s)',
          data: diskRead,
          borderColor: 'rgba(54,162,235,1)',
          backgroundColor: 'rgba(54,162,235,0.1)',
          fill: true,
          tension: 0.2
        },
        {
          label: 'Write (MB/s)',
          data: diskWrite,
          borderColor: 'rgba(255,206,86,1)',
          backgroundColor: 'rgba(255,206,86,0.1)',
          fill: true,
          tension: 0.2
        }
      ]
    },
    options: {
      ...baseOpts,
      scales: {
        ...baseOpts.scales,
        y: { ...baseOpts.scales.y, title: { display: true, text: 'MB/s' } }
      }
    }
  });
  let lastDiskTotals = null;

  // ─── 5) CPU USAGE CHART (one‐line only) ──────────────────────────────────
  const cpuCtx       = document.getElementById('cpuChart').getContext('2d');
  let cpuHistory     = Array(MAX_POINTS).fill(0);
  const cpuChart = new Chart(cpuCtx, {
    type: 'line',
    data: {
      labels,
      datasets: [{
        label: 'CPU Util (%)',
        data: cpuHistory,
        borderColor: 'rgba(75,192,192,1)',
        backgroundColor: 'rgba(75,192,192,0.1)',
        fill: true,
        tension: 0.2
      }]
    },
    options: {
      ...baseOpts,
      scales: {
        ...baseOpts.scales,
        y: {
          ...baseOpts.scales.y,
          title: { display: true, text: '%' }
        }
      }
    }
  });

  // ─── 6) MEMORY & SWAP CHART ─────────────────────────────────────────────
  const memCtx    = document.getElementById('memoryChart').getContext('2d');
  let memUsedArr  = Array(MAX_POINTS).fill(0);
  let memFreeArr  = Array(MAX_POINTS).fill(0);
  let cachedArr   = Array(MAX_POINTS).fill(0);
  let bufferedArr = Array(MAX_POINTS).fill(0);
  let swapUsedArr = Array(MAX_POINTS).fill(0);
  let swapFreeArr = Array(MAX_POINTS).fill(0);

  const memoryChart = new Chart(memCtx, {
    type: 'line',
    data: {
      labels,
      datasets: [
        { label: 'Mem Used',  data: memUsedArr,  tension: 0.2, fill: true },
        { label: 'Mem Free',  data: memFreeArr,  tension: 0.2, fill: true },
        { label: 'Cached',    data: cachedArr,   tension: 0.2, fill: true },
        { label: 'Buffered',  data: bufferedArr, tension: 0.2, fill: true },
        { label: 'Swap Used', data: swapUsedArr, tension: 0.2, fill: true },
        { label: 'Swap Free', data: swapFreeArr, tension: 0.2, fill: true }
      ]
    },
    options: {
      ...baseOpts,
      scales: {
        ...baseOpts.scales,
        y: {
          ...baseOpts.scales.y,
          title: { display: true, text: 'MB' }
        }
      }
    }
  });

  // ─── 7) POLLING FUNCTIONS ─────────────────────────────────────────────────
  // (a) General stats + timestamp
  async function updateGeneralStats() {
    try {
      const res = await fetch('/api/stats/general');
      if (!res.ok) throw new Error(res.status);
      const d   = await res.json();

      // Timestamp
      timestampEl.textContent = new Date().toLocaleTimeString();

      // Fill spans
      loadAvgEl.textContent     = `${d.loadavg1.toFixed(2)}, ${d.loadavg5.toFixed(2)}, ${d.loadavg15.toFixed(2)}`;
      cpuUtilEl.textContent     = `${d.cpu_util_percent.toFixed(1)}%`;
      tasksInfoEl.textContent   = `total=${d.tasks_total}, running=${d.tasks_running}`;
      memoryInfoEl.textContent  = `total=${d.memory.total_MB.toFixed(0)} MB, free=${d.memory.free_MB.toFixed(0)} MB, avail=${d.memory.available_MB.toFixed(0)} MB`;
      buffersInfoEl.textContent = `buffers=${d.memory.buffers_MB.toFixed(0)} MB, cached=${d.memory.cached_MB.toFixed(0)} MB`;
      swapInfoEl.textContent    = `total=${d.memory.swap_total_MB.toFixed(0)} MB, free=${d.memory.swap_free_MB.toFixed(0)} MB`;
    } catch (e) {
      console.error('general stats fetch failed', e);
    }
  }

  // (b) Network
  async function updateNetwork() {
    try {
      const res = await fetch('/api/stats/network');
      if (!res.ok) throw new Error(res.status);
      const d   = await res.json();
      if (lastNetTotals) {
        pushTrim(netDownload, d.total_download_MB - lastNetTotals.total_download_MB);
        pushTrim(netUpload,   d.total_upload_MB   - lastNetTotals.total_upload_MB);
        netChart.update();
      }
      lastNetTotals = d;
    } catch (e) {
      console.error('network fetch failed', e);
    }
  }

  // (c) Disk
  async function updateDisk() {
    try {
      const res = await fetch('/api/stats/disk');
      if (!res.ok) throw new Error(res.status);
      const d   = await res.json();
      if (lastDiskTotals) {
        pushTrim(diskRead,  d.total_read_MB  - lastDiskTotals.total_read_MB);
        pushTrim(diskWrite, d.total_write_MB - lastDiskTotals.total_write_MB);
        diskChart.update();
      }
      lastDiskTotals = d;
    } catch (e) {
      console.error('disk fetch failed', e);
    }
  }

  // (d) CPU (single series)
  async function updateCpuHistory() {
    try {
      const res = await fetch('/api/stats/general');
      if (!res.ok) throw new Error(res.status);
      const d   = await res.json();
      const util = d.cpu_util_percent;
      pushTrim(cpuHistory, util);
      cpuChart.update();
    } catch (e) {
      console.error('cpu history fetch failed', e);
    }
  }

  // (e) Memory & Swap (all from general)
  async function updateMemoryHistory() {
    try {
      const res = await fetch('/api/stats/general');
      if (!res.ok) throw new Error(res.status);
      const d   = await res.json();
      const mem = d.memory;
      const used = mem.total_MB - mem.free_MB;
      const swapUsed = mem.swap_total_MB - mem.swap_free_MB;

      pushTrim(memUsedArr,  used);
      pushTrim(memFreeArr,  mem.free_MB);
      pushTrim(cachedArr,   mem.cached_MB);
      pushTrim(bufferedArr, mem.buffers_MB);
      pushTrim(swapUsedArr, swapUsed);
      pushTrim(swapFreeArr, mem.swap_free_MB);

      memoryChart.update();
    } catch (e) {
      console.error('memory history fetch failed', e);
    }
  }

  // ─── 8) START EVERYTHING ─────────────────────────────────────────────────
  updateGeneralStats();
  updateNetwork();
  updateDisk();
  updateCpuHistory();
  updateMemoryHistory();

  setInterval(updateGeneralStats,    1000);
  setInterval(updateNetwork,         1000);
  setInterval(updateDisk,            1000);
  setInterval(updateCpuHistory,      1000);
  setInterval(updateMemoryHistory,   1000);
});
