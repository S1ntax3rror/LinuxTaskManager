document.addEventListener('DOMContentLoaded', () => {
  // ─── 1) ELEMENT REFERENCES ────────────────────────────────────────────────
  const timestampEl   = document.getElementById('timestamp');
  const loadAvgEl     = document.getElementById('loadAvg');
  const cpuUtilEl     = document.getElementById('cpuUtil');
  const tasksInfoEl   = document.getElementById('tasksInfo');
  const memoryInfoEl  = document.getElementById('memoryInfo');
  const buffersInfoEl = document.getElementById('buffersInfo');
  const swapInfoEl    = document.getElementById('swapInfo');

  // ─── 2) COMMON SETTINGS ───────────────────────────────────────────────────
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
  const pushTrim = (arr, v) => { arr.shift(); arr.push(v); };

  // ─── 3) CHART CONTEXTS ─────────────────────────────────────────────────────
  const netCtx   = document.getElementById('networkChart').getContext('2d');
  const diskCtx  = document.getElementById('diskChart').getContext('2d');
  const cpuCtx   = document.getElementById('cpuChart').getContext('2d');
  const memCtx   = document.getElementById('memoryChart').getContext('2d');

  // ─── 4) NETWORK CHART ─────────────────────────────────────────────────────
  let netDownload = Array(MAX_POINTS).fill(0);
  let netUpload   = Array(MAX_POINTS).fill(0);
  const netChart = new Chart(netCtx, {
    type: 'line',
    data: {
      labels,
      datasets: [
        { label: 'Download (Mb/s)', data: netDownload, fill: true, tension: 0.2 },
        { label: 'Upload (Mb/s)'  , data: netUpload  , fill: true, tension: 0.2 }
      ]
    },
    options: baseOpts
  });
  let lastNet = null;

  // ─── 5) DISK CHART (PER‐DISK + TOTAL) ─────────────────────────────────────
  // these will be populated on first update
  let diskDevices       = null;  // array of { key, name }
  const totalReadHist   = Array(MAX_POINTS).fill(0);
  const totalWriteHist  = Array(MAX_POINTS).fill(0);
  let diskReadHists     = {};    // keyed by stat‐key
  let diskWriteHists    = {};
  let diskChart         = null;
  let lastDiskStats     = null;

  // ─── 6) CPU CHART (AVG + PER‐CORE) ────────────────────────────────────────
  const NUM_CORES    = 8; // adjust if needed
  let avgHistory     = Array(MAX_POINTS).fill(0);
  let coreHistories  = Array.from({ length: NUM_CORES },
                                  () => Array(MAX_POINTS).fill(0));
  const palette = [
    '#007bff', '#e6194b', '#3cb44b', '#ffe119',
    '#4363d8', '#f58231', '#911eb4', '#46f0f0', '#f032e6'
  ];
  const cpuDatasets = [
    {
      label: 'Average CPU (%)',
      data: avgHistory,
      borderColor: palette[0],
      backgroundColor: palette[0] + '33',
      borderWidth: 2,
      fill: true,
      tension: 0.2
    },
    ...coreHistories.map((arr, i) => ({
      label: `Core ${i} (%)`,
      data: arr,
      borderColor:   palette[i+1],
      backgroundColor: 'transparent',
      pointRadius:   0,
      fill:          false,
      tension:       0.2
    }))
  ];
  const cpuChart = new Chart(cpuCtx, {
    type: 'line',
    data: { labels, datasets: cpuDatasets },
    options: baseOpts
  });

  // ─── 7) MEMORY & SWAP CHART ───────────────────────────────────────────────
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

  // ─── 8) FETCH + UPDATE LOOP ──────────────────────────────────────────────
  async function updateAll() {
    try {
      const res = await fetch('/api/stats/all');
      if (!res.ok) throw new Error(res.status);
      const d = await res.json();

      // ─── metadata ─────────────────────────────────────────────────────────
      timestampEl.textContent = new Date(d.net_stats.timestamp_ms)
                                 .toLocaleTimeString();
      const ls = d.load_stats;
      loadAvgEl.textContent   =
        `${ls.loadavg1.toFixed(2)}, ${ls.loadavg5.toFixed(2)}, ${ls.loadavg15.toFixed(2)}`;
      cpuUtilEl.textContent   = `${ls.cpu_util_percent.toFixed(1)}%`;
      tasksInfoEl.textContent = `total=${ls.tasks_total}, running=${ls.tasks_running}`;

      // ─── memory boxes ─────────────────────────────────────────────────────
      const m = d.memory;
      memoryInfoEl.textContent  =
        `total=${m.total_MB.toFixed(0)} MB, free=${m.free_MB.toFixed(0)} MB, avail=${m.available_MB.toFixed(0)} MB`;
      buffersInfoEl.textContent =
        `buffers=${m.buffers_MB.toFixed(0)} MB, cached=${m.cached_MB.toFixed(0)} MB`;
      swapInfoEl.textContent    =
        `total=${m.swap_total_MB.toFixed(0)} MB, free=${m.swap_free_MB.toFixed(0)} MB`;

      // ─── network deltas ───────────────────────────────────────────────────
      if (lastNet) {
        pushTrim(netDownload, d.net_stats.total_download_MB - lastNet.total_download_MB);
        pushTrim(netUpload,   d.net_stats.total_upload_MB   - lastNet.total_upload_MB);
        netChart.update();
      }
      lastNet = d.net_stats;

      // ─── disk deltas (per‐disk + total) ────────────────────────────────────
      const ds = d.disc_stats;

      // first time: discover devices & build chart
      if (!diskChart) {
        diskDevices = Object.values(ds)
          .filter(val => val && typeof val === 'object' && val.name)
          .map(val => ({ key: val.name, name: val.name }));

        diskDevices.forEach((dev,i) => {
          diskReadHists[dev.key]  = Array(MAX_POINTS).fill(0);
          diskWriteHists[dev.key] = Array(MAX_POINTS).fill(0);
        });

        const datasets = [];
        diskDevices.forEach((dev,i) => {
          const col = palette[(i+2) % palette.length];
          datasets.push({
            label: `${dev.name} Read`,
            data: diskReadHists[dev.key],
            fill: false,
            tension: 0.2,
            borderColor: col
          });
          datasets.push({
            label: `${dev.name} Write`,
            data: diskWriteHists[dev.key],
            fill: false,
            tension: 0.2,
            borderColor: col + '33'
          });
        });
        // finally, total lines
        datasets.push({
          label: 'Total Read',
          data: totalReadHist,
          fill: true,
          tension: 0.2,
          backgroundColor: palette[0] + '44',
          borderColor: palette[0]
        });
        datasets.push({
          label: 'Total Write',
          data: totalWriteHist,
          fill: true,
          tension: 0.2,
          backgroundColor: palette[1] + '44',
          borderColor: palette[1]
        });

        diskChart = new Chart(diskCtx, {
          type: 'line',
          data: { labels, datasets },
          options: baseOpts
        });
      }

      if (lastDiskStats) {
        // per‐disk delta
        diskDevices.forEach(dev => {
          const currR = ds[dev.key].read_MB;
          const prevR = lastDiskStats[dev.key].read_MB;
          pushTrim(diskReadHists[dev.key], currR - prevR);

          const currW = ds[dev.key].write_MB;
          const prevW = lastDiskStats[dev.key].write_MB;
          pushTrim(diskWriteHists[dev.key], currW - prevW);
        });
        // total delta
        pushTrim(totalReadHist,  ds.total_read_MB  - lastDiskStats.total_read_MB);
        pushTrim(totalWriteHist, ds.total_write_MB - lastDiskStats.total_write_MB);

        diskChart.update();
      }
      lastDiskStats = ds;

      // ─── cpu histories ────────────────────────────────────────────────────
      pushTrim(avgHistory, ls.cpu_util_percent);
      d.cores.forEach((core, i) => {
        pushTrim(coreHistories[i], core[`cpu${i}`] || 0);
      });
      cpuChart.update();

      // ─── memory/swap histories ────────────────────────────────────────────
      const usedMB   = m.total_MB - m.free_MB;
      const swapUsed = m.swap_total_MB - m.swap_free_MB;
      pushTrim(memUsedArr,  usedMB);
      pushTrim(memFreeArr,  m.free_MB);
      pushTrim(cachedArr,   m.cached_MB);
      pushTrim(bufferedArr, m.buffers_MB);
      pushTrim(swapUsedArr, swapUsed);
      pushTrim(swapFreeArr, m.swap_free_MB);
      memoryChart.update();

    } catch (e) {
      console.error('Failed to fetch all stats:', e);
    }
  }

  // kick it off + poll every second
  updateAll();
  setInterval(updateAll, 1000);
});
