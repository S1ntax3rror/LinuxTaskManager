async function fetchProcs() {
    try {
      const res = await fetch('/api/processes');
      const procs = await res.json();
      const tbody = document.getElementById('proc-table');
      tbody.innerHTML = procs.map(p => `
        <tr>
          <td>${p.pid}</td>
          <td>${p.comm}</td>
          <td>${p.state}</td>
          <td>${p.cpuPercent.toFixed(1)}</td>
          <td>${p.ramPercent.toFixed(1)}</td>
          <td>
            <button class="btn btn-sm btn-danger me-1"
                    onclick="signal(${p.pid}, 'KILL')">Kill</button>
            <button class="btn btn-sm btn-warning"
                    onclick="signal(${p.pid}, 'RENICE')">Renice</button>
          </td>
        </tr>
      `).join('');
    } catch (e) {
      console.error('fetch failed', e);
    }
  }
  
  async function signal(pid, cmd) {
    await fetch(`/api/processes/${pid}/signal?cmd=${cmd}`, { method: 'POST' });
  }
  
  fetchProcs();
  setInterval(fetchProcs, 1000);
  