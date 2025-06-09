let sortField    = 'cpu';
let frozen       = false;
let refreshTimer = null;

/**
 * Truncate a string to `maxLen` characters.
 * If the original string is longer, return the first maxLen
 * characters plus an ellipsis (…).
 */
function truncate(str, maxLen) {
  if (!str) return "";
  return str.length > maxLen
    ? str.slice(0, maxLen) + "…"
    : str;
}

/**
 * Returns true if this process is a kernel thread.
 * Kernel threads have no cmdline (proc.cmd === "").
 */
function isKernelThread(proc) {
  return !proc.cmd || proc.cmd.trim() === "";
}

/**
 * Switch sorting field between 'cpu' and 'ram',
 * update button active states, and refresh the table.
 */
function setSort(field) {
  sortField = field;
  document.getElementById('btn-sort-cpu')
          .classList.toggle('active', field === 'cpu');
  document.getElementById('btn-sort-ram')
          .classList.toggle('active', field === 'ram');
  fetchProcs();
}

/**
 * Toggles auto-refresh on or off based on the Freeze checkbox.
 */
function toggleFreeze() {
  frozen = document.getElementById('chk-freeze').checked;
  if (frozen) {
    clearInterval(refreshTimer);
  } else {
    // resume auto-refresh every 2s
    refreshTimer = setInterval(fetchProcs, 2000);
  }
}

/**
 * Fetch the current list of processes from the backend,
 * apply filters/sorting, and rebuild the table body.
 */
async function fetchProcs() {
  try {
    let procs = await fetch('/api/processes').then(r => r.json());

    // Sort
    procs.sort((a, b) =>
      sortField === 'cpu'
        ? b.cpuPercent - a.cpuPercent
        : b.ramPercent - a.ramPercent
    );

    // Hide kernel if desired
    if (document.getElementById('chk-hide-kernel').checked) {
      procs = procs.filter(p => !isKernelThread(p));
    }
    // Show sleepers only if desired
    if (document.getElementById('chk-show-sleepers').checked) {
      procs = procs.filter(p => p.is_sleeper === 1);
    }

    // Column-based text filters
    const filters = {
      pid:       document.getElementById('filter-pid').value.trim(),
      username:  document.getElementById('filter-username').value.trim().toLowerCase(),
      prio:      document.getElementById('filter-prio').value.trim(),
      nice:      document.getElementById('filter-nice').value.trim(),
      virt:      document.getElementById('filter-virt').value.trim(),
      res:       document.getElementById('filter-res').value.trim(),
      shared:    document.getElementById('filter-shared').value.trim(),
      cmd:       document.getElementById('filter-cmd').value.trim().toLowerCase(),
      upTime:    document.getElementById('filter-uptime').value.trim(),
      name:      document.getElementById('filter-name').value.trim().toLowerCase(),
      state:     document.getElementById('filter-state').value.trim().toLowerCase(),
      cpu:       document.getElementById('filter-cpu').value.trim(),
      ram:       document.getElementById('filter-ram').value.trim()
    };

    procs = procs.filter(p => {
      if (filters.pid       && !p.pid.toString().includes(filters.pid))               return false;
      if (filters.username  && !p.username.toLowerCase().includes(filters.username))    return false;
      if (filters.prio      && !p.prio.toString().includes(filters.prio))               return false;
      if (filters.nice      && !p.nice.toString().includes(filters.nice))               return false;
      if (filters.virt      && !p.virt.toString().includes(filters.virt))               return false;
      if (filters.res       && !p.res.toString().includes(filters.res))                 return false;
      if (filters.shared    && !p.shared.toString().includes(filters.shared))           return false;
      if (filters.cmd       && !p.cmd.toLowerCase().includes(filters.cmd))             return false;
      if (filters.upTime    && !p.upTime.toFixed(1).includes(filters.upTime))           return false;
      if (filters.name      && !p.comm.toLowerCase().includes(filters.name))            return false;
      if (filters.state     && !p.state.toLowerCase().includes(filters.state))          return false;
      if (filters.cpu       && !p.cpuPercent.toFixed(1).includes(filters.cpu))          return false;
      if (filters.ram       && !p.ramPercent.toFixed(1).includes(filters.ram))          return false;
      return true;
    });

    // Build table rows
    document.getElementById('proc-table').innerHTML = procs.map(p => {
      const shortCmd = truncate(p.cmd, 50);
      return `
      <tr>
        <td>${p.pid}</td>
        <td>${p.username}</td>
        <td>${p.prio}</td>
        <td>${p.nice}</td>
        <td>${p.virt}</td>
        <td>${p.res}</td>
        <td>${p.shared}</td>
        <td title="${p.cmd.replace(/"/g, '&quot;')}">${shortCmd}</td>
        <td>${p.upTime.toFixed(1)}</td>
        <td>${p.comm}</td>
        <td>${p.state}</td>
        <td>${p.cpuPercent.toFixed(1)}</td>
        <td>${p.ramPercent.toFixed(1)}</td>
        <td>
          <button class="btn btn-sm btn-danger me-1"
                  onclick="doSignal(${p.pid}, 'KILL')">
            Kill
          </button>
        </td>
      </tr>`;
    }).join('');
  } catch (err) {
    console.error('Failed to fetch processes:', err);
  }
}

/** Send a UNIX signal to a process, then refresh. */
async function doSignal(pid, cmd) {
  try {
    const res = await fetch(`/api/processes/${pid}/signal`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ cmd })
    });
    if (!res.ok) throw new Error(`HTTP ${res.status}`);
    fetchProcs();
  } catch (err) {
    alert(`Failed to send signal to PID ${pid}: ${err}`);
  }
}

/** Change the nice value of a process, then refresh. */
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
  } catch (err) {
    alert(`Failed to renice PID ${pid}: ${err}`);
  }
}

/** Apply a CPU-time limit to a process, then refresh. */
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
  } catch (err) {
    alert(`Failed to set CPU limit for PID ${pid}: ${err}`);
  }
}

/** Apply a RAM limit (in MB) to a process, then refresh. */
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
  } catch (err) {
    alert(`Failed to set RAM limit for PID ${pid}: ${err}`);
  }
}

// Initialize on page load and start auto-refresh
window.addEventListener('load', () => {
  fetchProcs();
  refreshTimer = setInterval(fetchProcs, 2000);
});
