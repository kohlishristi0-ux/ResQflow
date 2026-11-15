"""Offline end-to-end test helpers: drive the real ResQflow binary through its menus and verify the database."""
import subprocess, os, sqlite3, sys
R={'p':0,'f':0}
BIN=os.environ.get('RQ_BIN','/tmp/rq_off')
SCHEMA=os.environ.get('RQ_SCHEMA','/tmp/schema_sqlite.sql')
DB=os.environ.get('RQ_DB','/tmp/e2e.db')
ENV=dict(os.environ, RESQFLOW_DB_PASSWORD='x', RESQFLOW_SQLITE_DB=DB, RESQFLOW_SQLITE_SCHEMA=SCHEMA)
def fresh():
    if os.path.exists(DB): os.remove(DB)
def run(lines, timeout=60):
    p=subprocess.run([BIN], input="\n".join(map(str,lines))+"\n", capture_output=True, text=True, env=ENV, timeout=timeout)
    out=p.stdout+p.stderr
    if 'Sanitizer' in out or 'runtime error' in out:
        print('  FAIL sanitizer report:', out[out.find('=='):out.find('==')+400]); R['f']+=1
    return out, p.returncode
def q(sql, *a):
    c=sqlite3.connect(DB); r=c.execute(sql,a).fetchall(); c.close(); return r
ADMIN_FIRST=[1,'Admin One','admin','AdminPass12345','AdminPass12345','admin','AdminPass12345']
ADMIN=[1,'admin','AdminPass12345']
def check(name, cond, detail=""):
    if cond: R['p']+=1; print("  PASS", name)
    else: R['f']+=1; print("  FAIL", name, detail)
def summary(): print('TOTAL passed',R['p'],'failed',R['f'])
