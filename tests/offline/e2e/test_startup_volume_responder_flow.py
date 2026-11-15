import os, sys, subprocess; sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from e2e import *
print("== STARTUP FAILURES / VOLUME / RESPONDER FLOW ==")
fresh()
env=dict(os.environ); env.pop('RESQFLOW_DB_PASSWORD',None)
p=subprocess.run([BIN],input="0\n",capture_output=True,text=True,env=env)
check("missing password variable -> clear message, exit 1", p.returncode==1 and "RESQFLOW_DB_PASSWORD" in p.stdout)
env=dict(ENV); env.pop('RESQFLOW_SQLITE_SCHEMA')
if os.path.exists(DB): os.remove(DB)
p=subprocess.run([BIN],input="0\n",capture_output=True,text=True,env=env)
check("empty/unmigrated database -> refuses to start", p.returncode==1 and "cannot start safely" in p.stdout, p.stdout[-300:])
fresh()
# --- volume: 120 resources, 120 emergencies in one session
L=ADMIN_FIRST+[1]
for i in range(1,121): L+= [1,i,'Item%d'%i,'Cat','d','Unit',i%7]
L+=[0,4]
for i in range(1,121): L+= [1,'Type%d'%i,'Loc%d'%i,'d',1+i%5,1+i*13%900,10+(i%50)*0.1,70+(i%40)*0.1]
L+=[11,0,0,0]
out,rc=run(L, timeout=120)
check("120 resources stored", q("select count(*) from resources")==[(120,)])
check("120 emergencies stored", q("select count(*) from emergencies")==[(120,)])
check("120 initial history rows", q("select count(*) from emergency_status_history")==[(120,)])
out,rc=run(ADMIN+[4,11,0,0,0])
import re
m=re.search(r"Next emergency to serve: ID (\d+)", out)
top=int(m.group(1)) if m else -1
best=q("select emergency_id from emergencies order by severity desc, affected_people desc, registration_order asc limit 1")[0][0]
check("heap rebuilt from MySQL; top = highest severity/people", top==best, (top,best))
check("restart loaded 120 without error", "[ERROR]" not in out and rc==0)
# --- responder resolves own emergency
fresh()
L=ADMIN_FIRST+[4,1,'Flood','Dehradun','x',3,100,30.3,78.0, 0,  6,1,1,'Asha','Medic','999','Paramedic',30.3,78.0, 0, 0, 0]
run(L)
# link responder 1 to a login: create RESPONDER user, then approve-less link via SQL (admin tooling not in menu)
c=__import__('sqlite3').connect(DB)
from subprocess import check_output
c.close()
out,rc=run(ADMIN+[10,2,'Asha Responder','asha','','999','RESPONDER','AshaPass123456','AshaPass123456',0,0,0])
check("responder-role user created by admin", "User account created successfully" in out)
c=__import__('sqlite3').connect(DB); c.execute("update responders set user_id=(select user_id from users where username='asha') where responder_id=1"); c.commit(); c.close()
out,rc=run(ADMIN+[4,6,1,2,0, 6,3,1,1,0, 0,0])   # verify, then assign responder 1
check("verified + assigned", "Responder 1 assigned to emergency 1" in out, out[-400:])
out,rc=run([1,'asha','AshaPass123456', 3,1, 3,2, 1, 0, 0])
check("responder sets IN_PROGRESS then RESOLVED", out.count("Emergency status updated")==2, out[-700:])
check("emergency RESOLVED in MySQL", q("select status from emergencies where emergency_id=1")==[('RESOLVED',)])
check("responder auto-released", q("select status from responders where responder_id=1")==[('AVAILABLE',)] and
      q("select count(*) from emergency_responder_assignments where assignment_status='ACTIVE'")==[(0,)])
h=q("select new_status,changed_by from emergency_status_history where emergency_id=1 order by status_history_id")
check("history records who changed it", [x[0] for x in h]==['REPORTED','VERIFIED','IN_PROGRESS','RESOLVED'] and h[-1][1] is not None, h)
summary()
