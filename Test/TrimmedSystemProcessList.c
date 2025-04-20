#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>
#include <string.h>

// -----------------------------------------------------------------------------
// Basic data structure matching /proc/[pid]/stat fields.
// First only a handful of fields to make the development easier
// we can extend the fields later if needed
// -----------------------------------------------------------------------------
typedef struct proc_stat {
    int   pid;         // process ID
    char  comm[256];   // filename of the executable, without '(' or ')'
    char  state;       // single-character process state
    int   ppid;        // parent PID
    unsigned long utime, stime; // user & system CPU time
    long  rss;         // resident set size
    // … we can add more fields later …
} proc_stat;

// -----------------------------------------------------------------------------
// Returns 1 if str consists entirely of digits, else 0.
// -----------------------------------------------------------------------------
int is_number(const char *str) { // Here I fixed something small from your code @Jiri 
    for (int i = 0; str[i] != '\0'; i++) { // instead of (*str)) we have to do str[i]
        if (!isdigit((unsigned char)str[i])) {
            return 0;
        }
    }
    return 1;
}

// -----------------------------------------------------------------------------
// split_stat_line: 
//   Parses a single line from /proc/[pid]/stat into tokens, ensuring that
//   the 'comm' field (in parentheses) remains one token even if it contains spaces.
//   Returns a malloc’d array of strings, and sets *num_tokens.
//   Caller must free each string in the array and the array itself.
// -----------------------------------------------------------------------------
char **split_stat_line(const char *line, int *num_tokens) {
    // Maximum fields in /proc/[pid]/stat (approximate)
    const int MAX_FIELDS = 64;
    char **tokens = calloc(MAX_FIELDS, sizeof(char *));
    int   count  = 0;

    // 1) Extract PID (first number)
    const char *p = line;
    tokens[count++] = strndup(p, strcspn(p, " "));
    p += strlen(tokens[0]) + 1;

    // 2) Extract comm (between first '(' and last ')')
    const char *lpar = strchr(line, '(');
    const char *rpar = strrchr(line, ')');
    int comm_len = rpar - lpar - 1;
    tokens[count] = malloc(comm_len + 1);
    memcpy(tokens[count], lpar + 1, comm_len);
    tokens[count][comm_len] = '\0';
    count++;
    p = rpar + 2;  // move just past ") "

    // 3) Tokenize the rest by spaces
    while (*p != '\0' && count < MAX_FIELDS) {
        // skip any extra spaces
        while (*p == ' ') p++;
        if (*p == '\0') break;
        const char *start = p;
        while (*p != ' ' && *p != '\0') p++;
        tokens[count++] = strndup(start, p - start);
    }

    *num_tokens = count;
    return tokens;
}

// -----------------------------------------------------------------------------
// read_stat: open the given /proc file, read one line into buf, then close.
// Centralizes error‑checked file open/read/close logic so the parsing function can focus on tokenization.
// -----------------------------------------------------------------------------
static int read_stat_file(const char *path, char *buf, size_t bufsize) {
    FILE *fp = fopen(path, "r");
    if (!fp) return -1;
    if (!fgets(buf, bufsize, fp)) {
        fclose(fp);
        return -1;
    }
    fclose(fp);
    return 0;
}

// -----------------------------------------------------------------------------
// parse_proc_stat: given a path to /proc/[pid]/stat, fills a proc_stat.
// -----------------------------------------------------------------------------
int parse_proc_stat(const char *stat_path, proc_stat *st) {
    char buf[1024];
    if (read_stat_file(stat_path, buf, sizeof(buf)) < 0) {
        return -1;
    }

    int n;
    char **tok = split_stat_line(buf, &n);
    if (n < 6) {  // we need at least pid, comm, state, ppid, utime, stime
        // Cleanup
        for (int i = 0; i < n; i++) free(tok[i]);
        free(tok);
        return -1;
    }

    // Map tokens into our struct
    st->pid   = atoi(tok[0]);
    strncpy(st->comm, tok[1], sizeof(st->comm)-1);
    st->comm[sizeof(st->comm)-1] = '\0';
    st->state = tok[2][0];
    st->ppid  = atoi(tok[3]);
    st->utime = strtoul(tok[13], NULL, 10);  // user jiffies
    st->stime = strtoul(tok[14], NULL, 10);  // sys  jiffies
    st->rss   = atol(tok[23]);               // resident set size

    // Free tokens
    for (int i = 0; i < n; i++) free(tok[i]);
    free(tok);
    return 0;
}

// -----------------------------------------------------------------------------
// main: iterate over /proc, parse each stat, and print summary.
// -----------------------------------------------------------------------------
int main(void) {
    DIR *dp = opendir("/proc");
    if (!dp) {
        perror("opendir(/proc)");
        return 1;
    }

    struct dirent *entry;
    while ((entry = readdir(dp)) != NULL) {
        // Only numeric directories are PIDs
        if (!is_number(entry->d_name)) continue;

        // Build the path to stat
        char stat_path[64];
        snprintf(stat_path, sizeof(stat_path),
                 "/proc/%s/stat", entry->d_name);

        proc_stat st;
        if (parse_proc_stat(stat_path, &st) == 0) {
            // Print a concise summary: PID | COMM | STATE | CPU- & MEM- jiffies
            printf("%5d  %-20s  %c   u=%5lu  s=%5lu  rss=%8ld\n",
                   st.pid, st.comm, st.state,
                   st.utime, st.stime, st.rss);
        }
    }

    closedir(dp);
    return 0;
}
