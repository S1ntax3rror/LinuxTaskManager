let sortField = 'cpu';

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
    const procs = await res.json();

    procs.sort((a, b) =>
      sortField === 'cpu'
        ? b.cpuPercent - a.cpuPercent
        : b.ramPercent - a.ramPercent
    );

    document.getElementById('proc-table').innerHTML =
      procs.map(p => `
        <tr>
          <td>${p.pid}</td>
          <td>${p.comm}</td>
          <td>${p.state}</td>
          <td>${p.nice}</td>
          <td>${p.cpuPercent.toFixed(1)}</td>
          <td>${p.ramPercent.toFixed(1)}</td>
          <td>
            <button class="btn btn-sm btn-danger me-1"
                    onclick="doSignal(${p.pid}, 'KILL')">
              Kill
            </button>
          </td>
        </tr>
      `).join('');
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
