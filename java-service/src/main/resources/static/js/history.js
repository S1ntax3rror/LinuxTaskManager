// File: /js/history.js
// ------------------------------------------------------

document.addEventListener('DOMContentLoaded', () => {
  //
  // ─── 1) DOM ELEMENT REFERENCES ───────────────────────────────────────────
  //
  const timestampEl    = document.getElementById('timestamp');
  const loadAvgEl      = document.getElementById('loadAvg');
  const cpuUtilEl      = document.getElementById('cpuUtil');
  const tasksInfoEl    = document.getElementById('tasksInfo');
  const memoryInfoEl   = document.getElementById('memoryInfo');
  const buffersInfoEl  = document.getElementById('buffersInfo');
  const swapInfoEl     = document.getElementById('swapInfo');

  //
  // ─── 2) SET UP THE NETWORK CHART (60‐POINT WINDOW) ────────────────────────
  //
  const netCtx = document.getElementById('networkChart').getContext('2d');
  // Pre‐fill arrays with 60 zeros
  let netDownload = Array(60).fill(0);
  let netUpload   = Array(60).fill(0);

  // X‐axis labels: [1, 2, 3, …, 60]
  const labels = Array.from({ length: 60 }, (_, i) => i + 1);

  const netChart = new Chart(netCtx, {
    type: 'line',
    data: {
      labels: labels,
      datasets: [
        {
          label: 'Download (MB/s)',
          data: netDownload,
          borderColor: 'rgba(54, 162, 235, 1)',       // blue line
          backgroundColor: 'rgba(54, 162, 235, 0.1)', // light‐blue fill
          fill: true,
          tension: 0.2,
        },
        {
          label: 'Upload (MB/s)',
          data: netUpload,
          borderColor: 'rgba(75, 192, 192, 1)',       // green line
          backgroundColor: 'rgba(75, 192, 192, 0.1)',  // light‐green fill
          fill: true,
          tension: 0.2,
        },
      ],
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      scales: {
        x: {
          title: { display: true, text: 'Elapsed (s)' },
          ticks: { autoSkip: true, maxTicksLimit: 12 },
        },
        y: {
          title: { display: true, text: 'MB/s' },
          beginAtZero: true,
        },
      },
      plugins: {
        legend: { position: 'top' },
      },
    },
  });

  // We'll store the *previous* total_download_MB / total_upload_MB
  let lastNetTotals = { download: null, upload: null };

  //
  // ─── 3) SET UP THE DISK CHART (60‐POINT WINDOW) ───────────────────────────
  //
  const diskCtx = document.getElementById('diskChart').getContext('2d');
  let diskRead  = Array(60).fill(0);
  let diskWrite = Array(60).fill(0);

  const diskChart = new Chart(diskCtx, {
    type: 'line',
    data: {
      labels: labels,
      datasets: [
        {
          label: 'Read (MB/s)',
          data: diskRead,
          borderColor: 'rgba(255, 99, 132, 1)',       // red line
          backgroundColor: 'rgba(255, 99, 132, 0.1)',  // light‐red fill
          fill: true,
          tension: 0.2,
        },
        {
          label: 'Write (MB/s)',
          data: diskWrite,
          borderColor: 'rgba(255, 206, 86, 1)',       // orange line
          backgroundColor: 'rgba(255, 206, 86, 0.1)',  // light‐orange fill
          fill: true,
          tension: 0.2,
        },
      ],
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      scales: {
        x: {
          title: { display: true, text: 'Elapsed (s)' },
          ticks: { autoSkip: true, maxTicksLimit: 12 },
        },
        y: {
          title: { display: true, text: 'MB/s' },
          beginAtZero: true,
        },
      },
      plugins: {
        legend: { position: 'top' },
      },
    },
  });

  // We'll store the *previous* total_read_MB / total_write_MB
  let lastDiskTotals = { read: null, write: null };

  //
  // ─── 4) POLLING FUNCTIONS ─────────────────────────────────────────────────
  //
  // (a) Poll General Stats
  async function updateGeneralStats() {
    try {
      const res = await fetch('http://localhost:9000/api/stats/general');
      if (!res.ok) throw new Error(`HTTP ${res.status}`);
      const data = await res.json();

      // 1) Update “Last updated” clock
      const now = new Date();
      timestampEl.textContent = now.toLocaleTimeString();

      // 2) Load Averages (1m, 5m, 15m)
      loadAvgEl.textContent = `${data.loadavg1.toFixed(2)}, ${data.loadavg5.toFixed(2)}, ${data.loadavg15.toFixed(2)}`;

      // 3) CPU Util %
      cpuUtilEl.textContent = `${data.cpu_util_percent.toFixed(1)}%`;

      // 4) Tasks: total/running
      tasksInfoEl.textContent = `total=${data.tasks_total}, running=${data.tasks_running}`;

      // 5) Memory (MB)
      memoryInfoEl.textContent =
        `total=${data.memory.total_MB.toFixed(0)} MB, ` +
        `free=${data.memory.free_MB.toFixed(0)} MB, ` +
        `avail=${data.memory.available_MB.toFixed(0)} MB`;

      // 6) Buffers + Cached (MB)
      buffersInfoEl.textContent =
        `buffers=${data.memory.buffers_MB.toFixed(0)} MB, ` +
        `cached=${data.memory.cached_MB.toFixed(0)} MB`;

      // 7) Swap (MB)
      swapInfoEl.textContent =
        `total=${data.memory.swap_total_MB.toFixed(0)} MB, ` +
        `free=${data.memory.swap_free_MB.toFixed(0)} MB`;

    } catch (err) {
      console.error('Error fetching general stats:', err);
    }
  }

  // (b) Poll Network Stats → compute “MB/s” deltas
  async function updateNetworkStats() {
    try {
      const res = await fetch('http://localhost:9000/api/stats/network');
      if (!res.ok) throw new Error(`HTTP ${res.status}`);
      const data = await res.json();

      const totDown = data.total_download_MB;
      const totUp   = data.total_upload_MB;

      // If we have a “previous” snapshot, compute delta
      if (lastNetTotals.download !== null) {
        const deltaDown = totDown - lastNetTotals.download;
        const deltaUp   = totUp   - lastNetTotals.upload;

        // Push into rolling arrays, drop oldest sample
        netDownload.shift();
        netDownload.push(deltaDown);

        netUpload.shift();
        netUpload.push(deltaUp);

        // Tell Chart.js to use the new data
        netChart.data.datasets[0].data = netDownload;
        netChart.data.datasets[1].data = netUpload;
        netChart.update();
      }

      // Save this cycle as “last”
      lastNetTotals.download = totDown;
      lastNetTotals.upload   = totUp;

    } catch (err) {
      console.error('Error fetching network stats:', err);
    }
  }

  // (c) Poll Disk Stats → compute “MB/s” deltas
  async function updateDiskStats() {
    try {
      const res = await fetch('http://localhost:9000/api/stats/disk');
      if (!res.ok) throw new Error(`HTTP ${res.status}`);
      const data = await res.json();

      const totRead  = data.total_read_MB;
      const totWrite = data.total_write_MB;

      if (lastDiskTotals.read !== null) {
        const deltaRead  = totRead  - lastDiskTotals.read;
        const deltaWrite = totWrite - lastDiskTotals.write;

        diskRead.shift();
        diskRead.push(deltaRead);

        diskWrite.shift();
        diskWrite.push(deltaWrite);

        diskChart.data.datasets[0].data = diskRead;
        diskChart.data.datasets[1].data = diskWrite;
        diskChart.update();
      }

      lastDiskTotals.read  = totRead;
      lastDiskTotals.write = totWrite;

    } catch (err) {
      console.error('Error fetching disk stats:', err);
    }
  }

  //
  // ─── 5) INITIALIZE “LAST” VALUES, THEN START INTERVALS ───────────────────
  //
  // Do one initial fetch so that lastNetTotals / lastDiskTotals are set
  updateGeneralStats();
  updateNetworkStats();
  updateDiskStats();

  // Then repeat every 1 second
  setInterval(updateGeneralStats, 1000);
  setInterval(updateNetworkStats, 1000);
  setInterval(updateDiskStats, 1000);
});
