// default sort field
let sortField = 'cpu';

// change sort and update button states
function setSort(field) {
  sortField = field;

  // toggle active classes
  document.getElementById('btn-sort-cpu')
          .classList.toggle('active', field === 'cpu');
  document.getElementById('btn-sort-ram')
          .classList.toggle('active', field === 'ram');

  // immediately refresh
  fetchProcs();
}

async function fetchProcs() {
  try {
    const res = await fetch('/api/processes');
    if (!res.ok) {
      throw new Error(`Server error: ${res.status}`);
    }

    let procs = await res.json();

    // sort by chosen field descending
    procs.sort((a, b) => {
      if (sortField === 'cpu') {
        return b.cpuPercent - a.cpuPercent;
      } else {
        return b.ramPercent - a.ramPercent;
      }
    });

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
    console.error('fetchProcs failed:', e);
  }
}

// signal helper stays the same
async function signal(pid, cmd) {
  try {
    const res = await fetch(`/api/processes/${pid}/signal`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ cmd })
    });
    if (!res.ok) {
      throw new Error(`Signal failed: ${res.status}`);
    }
  } catch (e) {
    console.error('signal error:', e);
  }
}

// initial load + refresh every second
fetchProcs();
setInterval(fetchProcs, 1000);
