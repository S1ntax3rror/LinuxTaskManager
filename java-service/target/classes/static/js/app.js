// custom.css will handle fonts, colors, spacing

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
            <button class="btn btn-sm btn-danger"
                    onclick="doSignal(${p.pid}, 'KILL')">
              Kill
            </button>
          </td>
        </tr>
      `).join('');
  } catch (e) {
    console.error('Failed to fetch processes', e);
  }
}

async function doSignal(pid, cmd) {
  await fetch(`/api/processes/${pid}/signal`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ cmd })
  });
  fetchProcs();
}

async function doRenice() {
  const pid  = +document.getElementById('in-pid-renice').value;
  const nice = +document.getElementById('in-nice').value;
  await fetch(`/api/processes/${pid}/renice`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ nice })
  });
  fetchProcs();
}

async function doCpuLimit() {
  const pid = +document.getElementById('in-pid-cpu').value;
  const secs = +document.getElementById('in-cpu-limit').value;
  await fetch(`/api/processes/${pid}/limit/cpu`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ limit: secs })
  });
  fetchProcs();
}

async function doRamLimit() {
  const pid = +document.getElementById('in-pid-ram').value;
  const mb  = +document.getElementById('in-ram-limit').value;
  await fetch(`/api/processes/${pid}/limit/ram`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ limit: mb })
  });
  fetchProcs();
}

window.addEventListener('load', () => {
  fetchProcs();
  setInterval(fetchProcs, 2000);
});
