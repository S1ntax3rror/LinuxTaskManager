// ─────────────────────────────────────────────────────────────────────────────
// js/history.js
// Poll both /api/stats/network and /api/stats/disk every second,
// compute instantaneous MB/s, keep a 60-second rolling window.
// Place this file in your web-root:  /static/js/history.js
// Make sure <script src="/js/history.js"></script> is on the /history page.
// ─────────────────────────────────────────────────────────────────────────────

document.addEventListener("DOMContentLoaded", () => {

  // ─── Global constants ───────────────────────────────────────────────────────
  const API_BASE   = "http://localhost:9000/api/stats";   // C-daemon endpoint
  const POLL_MS    = 1000;    // 1 second polling interval
  const MAX_POINTS = 60;      // 60 seconds window

  // ─── Helper: Format the current time as "HH:mm:ss" ───────────────────────────
  function nowHHMMSS() {
    const d = new Date();
    const pad = (n) => (n < 10 ? "0" + n : "" + n);
    return `${pad(d.getHours())}:${pad(d.getMinutes())}:${pad(d.getSeconds())}`;
  }

  // ─── 1) LAST‐UPDATED TIMESTAMP ELEMENT ───────────────────────────────────────
  const lastUpdatedEl = document.getElementById("timestamp");
  function updateTimestamp() {
    lastUpdatedEl.textContent = nowHHMMSS();
  }


  // ─── 2) NETWORK CHART SETUP ───────────────────────────────────────────────────
  const networkCtx = document.getElementById("networkChart").getContext("2d");
  const networkChart = new Chart(networkCtx, {
    type: "line",
    data: {
      labels: [],  // X labels: 0,1,2,…59
      datasets: [
        {
          label: "Download (MB/s)",
          data: [],
          borderColor: "rgba(0, 123, 255, 1)",
          backgroundColor: "rgba(0, 123, 255, 0.1)",
          fill: true,
          tension: 0.2,
          pointRadius: 2,
        },
        {
          label: "Upload (MB/s)",
          data: [],
          borderColor: "rgba(40, 167, 69, 1)",
          backgroundColor: "rgba(40, 167, 69, 0.1)",
          fill: true,
          tension: 0.2,
          pointRadius: 2,
        },
      ],
    },
    options: {
      animation: false,
      responsive: true,
      maintainAspectRatio: false,
      scales: {
        x: {
          title: { display: true, text: "Elapsed (s)" },
          ticks: {
            // Show ticks every 5 seconds (optional)
            callback: (val, idx) => (idx % 5 === 0 ? val : ""),
          },
        },
        y: {
          title: { display: true, text: "MB/s" },
          beginAtZero: true,
        },
      },
      plugins: {
        legend: { position: "top", labels: { boxWidth: 12 } }
      },
    },
  });

  // Keep track of previous cumulative values for network:
  let prevNetDownload = null;
  let prevNetUpload   = null;


  // ─── 3) DISK CHART SETUP ──────────────────────────────────────────────────────
  const diskCtx = document.getElementById("diskChart").getContext("2d");
  const diskChart = new Chart(diskCtx, {
    type: "line",
    data: {
      labels: [],  // 0…59
      datasets: [
        {
          label: "Read (MB/s)",
          data: [],
          borderColor: "rgba(255, 193, 7, 1)",      // amber
          backgroundColor: "rgba(255, 193, 7, 0.1)",
          fill: true,
          tension: 0.2,
          pointRadius: 2,
        },
        {
          label: "Write (MB/s)",
          data: [],
          borderColor: "rgba(220, 53, 69, 1)",       // red
          backgroundColor: "rgba(220, 53, 69, 0.1)",
          fill: true,
          tension: 0.2,
          pointRadius: 2,
        },
      ],
    },
    options: {
      animation: false,
      responsive: true,
      maintainAspectRatio: false,
      scales: {
        x: {
          title: { display: true, text: "Elapsed (s)" },
          ticks: {
            callback: (val, idx) => (idx % 5 === 0 ? val : ""),
          },
        },
        y: {
          title: { display: true, text: "MB/s" },
          beginAtZero: true,
        },
      },
      plugins: {
        legend: { position: "top", labels: { boxWidth: 12 } }
      },
    },
  });

  // Keep track of previous cumulative values for disk:
  let prevDiskRead  = null;
  let prevDiskWrite = null;


  // ─── 4) POLLING FUNCTION ─────────────────────────────────────────────────────
  async function pollAll() {
    updateTimestamp();

    // ── 4a) Fetch Network stats ─────────────────────────────────────────────────
    try {
      const netResp = await fetch(`${API_BASE}/network`);
      if (netResp.ok) {
        const netJson = await netResp.json();
        const curDownload = netJson.total_download_MB;
        const curUpload   = netJson.total_upload_MB;

        let downloadSpeed = 0;
        let uploadSpeed   = 0;
        if (prevNetDownload !== null && prevNetUpload !== null) {
          downloadSpeed = (curDownload - prevNetDownload); // MB over 1 second
          uploadSpeed   = (curUpload   - prevNetUpload);
          // (Our poll interval is exactly 1 second, so ΔMB = MB/s)
        }

        prevNetDownload = curDownload;
        prevNetUpload   = curUpload;

        // X‐axis label = “seconds since we started collecting”:
        const nextIndex = networkChart.data.labels.length;
        networkChart.data.labels.push(nextIndex);               // 0,1,2,…,60,…

        networkChart.data.datasets[0].data.push(downloadSpeed);
        networkChart.data.datasets[1].data.push(uploadSpeed);

        // If we exceed MAX_POINTS, drop the oldest value:
        if (networkChart.data.labels.length > MAX_POINTS) {
          networkChart.data.labels.shift();
          networkChart.data.datasets.forEach(ds => ds.data.shift());
        }
        networkChart.update("none");
      } else {
        console.error("Failed to fetch /api/stats/network:", netResp.status);
      }
    } catch (err) {
      console.error("Error fetching network stats:", err);
    }

    // ── 4b) Fetch Disk stats ────────────────────────────────────────────────────
    try {
      const diskResp = await fetch(`${API_BASE}/disk`);
      if (diskResp.ok) {
        const diskJson = await diskResp.json();
        const curRead  = diskJson.total_read_MB;
        const curWrite = diskJson.total_write_MB;

        let readSpeed  = 0;
        let writeSpeed = 0;
        if (prevDiskRead !== null && prevDiskWrite !== null) {
          readSpeed  = (curRead  - prevDiskRead);
          writeSpeed = (curWrite - prevDiskWrite);
        }

        prevDiskRead  = curRead;
        prevDiskWrite = curWrite;

        const nextIndex2 = diskChart.data.labels.length;
        diskChart.data.labels.push(nextIndex2);
        diskChart.data.datasets[0].data.push(readSpeed);
        diskChart.data.datasets[1].data.push(writeSpeed);

        if (diskChart.data.labels.length > MAX_POINTS) {
          diskChart.data.labels.shift();
          diskChart.data.datasets.forEach(ds => ds.data.shift());
        }
        diskChart.update("none");
      } else {
        console.error("Failed to fetch /api/stats/disk:", diskResp.status);
      }
    } catch (err) {
      console.error("Error fetching disk stats:", err);
    }

    // Schedule next tick in 1 second:
    setTimeout(pollAll, POLL_MS);
  }


  // ─── 5) INITIALIZE & START ───────────────────────────────────────────────────
  // We want the first “point” at time = 0 (speed = 0), so:
  for (let i = 0; i < MAX_POINTS; i++) {
    networkChart.data.labels.push(i);
    networkChart.data.datasets.forEach(ds => ds.data.push(0));
    diskChart.data.labels.push(i);
    diskChart.data.datasets.forEach(ds => ds.data.push(0));
  }
  networkChart.update("none");
  diskChart.update("none");

  // Initialize “previous” values by fetching one snapshot from the server,
  // so that on the *next* tick we can compute a Δ.
  (async () => {
    try {
      // Fetch network once:
      const netResp = await fetch(`${API_BASE}/network`);
      if (netResp.ok) {
        const netJson = await netResp.json();
        prevNetDownload = netJson.total_download_MB;
        prevNetUpload   = netJson.total_upload_MB;
      }
      // Fetch disk once:
      const diskResp = await fetch(`${API_BASE}/disk`);
      if (diskResp.ok) {
        const diskJson = await diskResp.json();
        prevDiskRead  = diskJson.total_read_MB;
        prevDiskWrite = diskJson.total_write_MB;
      }

      // Now that we have “previous,” start the continuous polling loop:
      pollAll();
    } catch (err) {
      console.error("Error during initial fetch:", err);
    }
  })();

});
