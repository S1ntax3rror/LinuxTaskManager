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
      y: { beginAtZero: true }
    },
    plugins: { legend: { position: 'top' } }
  };
  function pushTrim(arr, v) {
    arr.shift();
    arr.push(v);
  }

  // ─── CHART INITIALIZATIONS ───────────────────────────────────────────────
  // Network
  const netCtx = document.getElementById('networkChart').getContext('2d');
  let netDownload = Array(MAX_POINTS).fill(0);
  let netUpload   = Array(MAX_POINTS).fill(0);
  const netChart = new Chart(netCtx, {
    type: 'line',
    data: {
      labels,
      datasets: [
        { label: 'Download (Mb/s)', data: netDownload, fill: true, tension: 0.2 },
        { label: 'Upload (Mb/s)',   data: netUpload,   fill: true, tension: 0.2 }
      ]
    },
    options: baseOpts
  });
  let lastNetTotals = null;

  // Disk
  const diskCtx = document.getElementById('diskChart').getContext('2d');
  let diskRead  = Array(MAX_POINTS).fill(0);
  let diskWrite = Array(MAX_POINTS).fill(0);
  const diskChart = new Chart(diskCtx, {
    type: 'line',
    data: {
      labels,
      datasets: [
        { label: 'Read (MB/s)',  data: diskRead,  fill: true, tension: 0.2 },
        { label: 'Write (MB/s)', data: diskWrite, fill: true, tension: 0.2 }
      ]
    },
    options: baseOpts
  });
  let lastDiskTotals = null;

  // CPU Util history
  const cpuCtx   = document.getElementById('cpuChart').getContext('2d');
  let cpuHistory = Array(MAX_POINTS).fill(0);
  const cpuChart = new Chart(cpuCtx, {
    type: 'line',
    data: { labels, datasets: [{ label: 'CPU Util (%)', data: cpuHistory, fill: true, tension: 0.2 }] },
    options: baseOpts
  });

  // Memory & Swap
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
        { label: 'Mem Used',  data: memUsedArr,  fill: true, tension: 0.2 },
        { label: 'Mem Free',  data: memFreeArr,  fill: true, tension: 0.2 },
        { label: 'Cached',    data: cachedArr,   fill: true, tension: 0.2 },
        { label: 'Buffered',  data: bufferedArr, fill: true, tension: 0.2 },
        { label: 'Swap Used', data: swapUsedArr, fill: true, tension: 0.2 },
        { label: 'Swap Free', data: swapFreeArr, fill: true, tension: 0.2 }
      ]
    },
    options: baseOpts
  });

  // ─── SINGLE ENDPOINT POLLER ──────────────────────────────────────────────
  async function updateAll() {
    try {
      const res = await fetch('/api/stats/all');
      if (!res.ok) throw new Error(res.status);
      const d = await res.json();

      // Timestamp from net_stats
      timestampEl.textContent = new Date(d.net_stats.timestamp_ms).toLocaleTimeString();

      // General stats
      const ls = d.load_stats;
      loadAvgEl.textContent     = `${ls.loadavg1.toFixed(2)}, ${ls.loadavg5.toFixed(2)}, ${ls.loadavg15.toFixed(2)}`;
      cpuUtilEl.textContent     = `${ls.cpu_util_percent.toFixed(1)}%`;
      tasksInfoEl.textContent   = `total=${ls.tasks_total}, running=${ls.tasks_running}`;

      // Memory boxes
      const m = d.memory;
      memoryInfoEl.textContent  = `total=${m.total_MB.toFixed(0)} MB, free=${m.free_MB.toFixed(0)} MB, avail=${m.available_MB.toFixed(0)} MB`;
      buffersInfoEl.textContent = `buffers=${m.buffers_MB.toFixed(0)} MB, cached=${m.cached_MB.toFixed(0)} MB`;
      swapInfoEl.textContent    = `total=${m.swap_total_MB.toFixed(0)} MB, free=${m.swap_free_MB.toFixed(0)} MB`;

      // Network speeds
      if (lastNetTotals) {
        pushTrim(netDownload, d.net_stats.total_download_MB - lastNetTotals.total_download_MB);
        pushTrim(netUpload,   d.net_stats.total_upload_MB   - lastNetTotals.total_upload_MB);
        netChart.update();
      }
      lastNetTotals = d.net_stats;

      // Disk I/O speeds
      if (lastDiskTotals) {
        pushTrim(diskRead,  d.disc_stats.total_read_MB  - lastDiskTotals.total_read_MB);
        pushTrim(diskWrite, d.disc_stats.total_write_MB - lastDiskTotals.total_write_MB);
        diskChart.update();
      }
      lastDiskTotals = d.disc_stats;

      // CPU history
      pushTrim(cpuHistory, ls.cpu_util_percent);
      cpuChart.update();

      // Memory history
      const usedMB = m.total_MB - m.free_MB;
      const swapUsedMB = m.swap_total_MB - m.swap_free_MB;
      pushTrim(memUsedArr,  usedMB);
      pushTrim(memFreeArr,  m.free_MB);
      pushTrim(cachedArr,   m.cached_MB);
      pushTrim(bufferedArr, m.buffers_MB);
      pushTrim(swapUsedArr, swapUsedMB);
      pushTrim(swapFreeArr, m.swap_free_MB);
      memoryChart.update();

    } catch (e) {
      console.error('Failed to fetch all stats:', e);
    }
  }

  // Initialize and poll every second
  updateAll();
  setInterval(updateAll, 1000);
});
