let sortField = 'cpu';

/**
 * Truncate a string to `maxLen` characters.
 * If the original string is longer, return the first maxLen
 * characters + an ellipsis (…).
 */
function truncate(str, maxLen) {
  if (!str) return "";
  return (str.length > maxLen)
    ? str.slice(0, maxLen) + "…"
    : str;
}

/**
 * Returns true if this trimmed_info represents a kernel thread.
 * Kernel threads have no cmdline, so proc.cmd === "".
 */
function isKernelThread(proc) {
  return (proc.cmd === "" || proc.cmd.trim() === "");
}

function setSort(f) {
  sortField = f;
  document.getElementById('btn-sort-cpu')
          .classList.toggle('active', f === 'cpu');
  document.getElementById('btn-sort-ram')
          .classList.toggle('active', f === 'ram');
  fetchProcs();
}

async function fetchProcs() {
  try {
    const res = await fetch('/api/processes');
    if (!res.ok) throw new Error(`HTTP ${res.status}`);
    let procs = await res.json();

    // 1) Sort by chosen field
    procs.sort((a, b) =>
      sortField === 'cpu'
        ? b.cpuPercent - a.cpuPercent
        : b.ramPercent - a.ramPercent
    );

    // 2) Hide kernel threads if checkbox is checked:
    const hideKernel = document.getElementById('chk-hide-kernel').checked;
    if (hideKernel) {
      procs = procs.filter(p => !isKernelThread(p));
    }

    // 3) Build table rows (columns must match index.html <th> order)
    document.getElementById('proc-table').innerHTML =
      procs.map(p => {
        // Truncate the “cmd” field to 50 characters for compact display
        const shortCmd = truncate(p.cmd, 50);

        return `
        <tr>
          <!-- 1) PID -->
          <td>${p.pid}</td>

          <!-- 2) User (username) -->
          <td>${p.username}</td>

          <!-- 3) Prio -->
          <td>${p.prio}</td>

          <!-- 4) Nice -->
          <td>${p.nice}</td>

          <!-- 5) VIRT (KiB) -->
          <td>${p.virt}</td>

          <!-- 6) RES (KiB) -->
          <td>${p.res}</td>

          <!-- 7) SHR (KiB) -->
          <td>${p.shared}</td>

          <!-- 8) COMMAND (truncated, with full command in title) -->
          <td title="${p.cmd.replace(/"/g, '&quot;')}">
            ${shortCmd}
          </td>

          <!-- 9) Up Time (s) -->
          <td>${p.upTime.toFixed(1)}</td>

          <!-- 10) Name (the “comm” field) -->
          <td>${p.comm}</td>

          <!-- 11) State -->
          <td>${p.state}</td>

          <!-- 12) % CPU -->
          <td>${p.cpuPercent.toFixed(1)}</td>

          <!-- 13) % RAM -->
          <td>${p.ramPercent.toFixed(1)}</td>

          <!-- 14) Action (Kill button) -->
          <td>
            <button class="btn btn-sm btn-danger me-1"
                    onclick="doSignal(${p.pid}, 'KILL')">
              Kill
            </button>
          </td>
        </tr>
        `;
      }).join('');
  } catch (e) {
    console.error('Failed to fetch processes:', e);
  }
}

async function doSignal(pid, cmd) {
  try {
    const res = await fetch(`/api/processes/${pid}/signal`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ cmd })
    });
    if (!res.ok) throw new Error(`HTTP ${res.status}`);
    fetchProcs();
  } catch (e) {
    alert(`Failed to send signal to PID ${pid}: ${e}`);
  }
}

async function doRenice() {
  const pid  = +document.getElementById('in-pid-renice').value;
  const nice = +document.getElementById('in-nice').value;
  try {
    const res = await fetch(`/api/processes/${pid}/renice`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ nice })
    });
    if (!res.ok) throw new Error(`HTTP ${res.status}`);
    fetchProcs();
  } catch (e) {
    alert(`Failed to renice PID ${pid}: ${e}`);
  }
}

async function doCpuLimit() {
  const pid  = +document.getElementById('in-pid-cpu').value;
  const secs = +document.getElementById('in-cpu-limit').value;
  try {
    const res = await fetch(`/api/processes/${pid}/limit/cpu`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ limit: secs })
    });
    if (!res.ok) throw new Error(`HTTP ${res.status}`);
    fetchProcs();
  } catch (e) {
    alert(`Failed to set CPU limit for PID ${pid}: ${e}`);
  }
}

async function doRamLimit() {
  const pid = +document.getElementById('in-pid-ram').value;
  const mb  = +document.getElementById('in-ram-limit').value;
  try {
    const res = await fetch(`/api/processes/${pid}/limit/ram`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ limit: mb })
    });
    if (!res.ok) throw new Error(`HTTP ${res.status}`);
    fetchProcs();
  } catch (e) {
    alert(`Failed to set RAM limit for PID ${pid}: ${e}`);
  }
}

window.addEventListener('load', () => {
  fetchProcs();
  setInterval(fetchProcs, 2000);
});
