import os, sys; sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from e2e import *
print("== RESPONDER / USER / AUTH WORKFLOW ==")
fresh()
L=ADMIN_FIRST+[4,1,'Flood','Dehradun','x',3,100,30.3,78.0, 0,
 6, 1,1,'Asha','Medic','999','Paramedic',30.3,78.0,
    1,2,'Ravi','Rescue','888','Diver',30.2,78.1,
    1,1,'Dup','r','1','s',1,1,                   # duplicate id
    1,0,'Zero','r','1','s',1,1,                  # invalid id (positive-number prompt, still reads fields)
    3,1,1, 3,1,1, 3,2,99, 3,99,1,               # assign ok / not available / unknown emergency / unknown responder
    6,1, 5,1, 5,1,                               # off-duty denied(assigned) / release / release again
    4,1, 10, 7,1, 6,2, 7,2, 8, 9, 0,
 4, 6,1,2, 6,1,3, 0,
 6, 3,2,1, 0,
 4, 6,1,4, 0,                                    # resolve -> responders auto-released
 0, 0,0]
out,rc=run(L)
check("exit 0", rc==0, rc)
check("responders saved", q("select count(*) from responders")==[(2,)], q("select * from responders"))
check("duplicate responder rejected", "Responder ID already exists" in out)
check("assign persisted", "Responder 1 assigned to emergency 1" in out)
check("assign unavailable denied", "not available" in out)
check("assign unknown emergency denied", "Emergency not found" in out)
check("unknown responder denied", "Responder not found" in out)
check("off duty denied while assigned", "assigned responders must be released first" in out)
check("release works", "Responder released and available again" in out)
check("round robin assign", "assigned to emergency 1" in out)
a=q("select responder_id,assignment_status from emergency_responder_assignments order by assignment_id")
check("assignment history kept", len(a)>=3 and a[0]==(1,'RELEASED'), a)
check("auto-release when emergency resolved", all(r[0]!='ASSIGNED' for r in q("select status from responders")), q("select responder_id,status from responders"))
check("no ACTIVE assignment after resolve", q("select count(*) from emergency_responder_assignments where assignment_status='ACTIVE'")==[(0,)], a)
# restart must load responders without error (assignment join)
out,rc=run(ADMIN+[0,0])
check("restart loads cleanly", "[ERROR]" not in out and rc==0, out[:400])
# ---- users, roles
L=ADMIN+[10,2,'Coord One','coord1','c@x.in','123','COORDINATOR','CoordPass12345','CoordPass12345',
 2,'Bad Pw','badpw','','','COORDINATOR','short','short',
 2,'Dup','coord1','','','ADMIN','CoordPass12345','CoordPass12345',
 1, 0, 0, 0]
out,rc=run(L)
check("coordinator created", "User account created successfully" in out)
check("short password refused", "Password too short" in out)
check("duplicate username refused", "username already exists" in out)
check("password stored hashed", q("select password_hash from users where username='coord1'")[0][0].startswith("pbkdf2_sha256$"))
out,rc=run([1,'coord1','CoordPass12345', 9, 10, 4,0, 0, 0])
check("coordinator denied activity history", "ACCESS DENIED" in out)
check("coordinator menu hides admin items", "10. User Management" not in out)
out,rc=run([1,'coord1','wrongpass',  'coord1','wrongpass',  'coord1','wrongpass', 0])
check("3 bad logins locked out", "Maximum login attempts exceeded" in out)
# responder self registration -> approval -> login
L=[2,'Meera Rao','meera','m@x.in','777','Medical','Surgeon','Dehradun','Rajpur','12 Base Rd','MeeraPass12345','MeeraPass12345',
   2,'Meera Rao','meera','','777','Medical','Surgeon','Dehradun','Rajpur','12 Base Rd','MeeraPass12345','MeeraPass12345',  # duplicate pending
   0]
out,rc=run(L)
check("self-registration pending", "PENDING ADMIN APPROVAL" in out)
check("duplicate pending refused", "Could not submit registration" in out)
check("request stored with city/area/address", q("select status,city,area,base_address from responder_registration_requests")==[('PENDING','Dehradun','Rajpur','12 Base Rd')])
out,rc=run([1,'meera','MeeraPass12345',0])
check("login blocked before approval", "Authentication failed" in out or "Invalid username" in out)
out,rc=run(ADMIN+[10,5,1,2,1,3,99,0,0,0,0])
check("pending listed", "PENDING RESPONDER REGISTRATIONS" in out)
check("approval creates user+responder", "Registration approved" in out, out[-600:])
ur=q("select u.username,u.role,r.responder_name,r.status,r.city from users u join responders r on r.user_id=u.user_id where u.username='meera'")
check("responder linked to user", ur==[('meera','RESPONDER','Meera Rao','AVAILABLE','Dehradun')], ur)
check("request marked APPROVED", q("select status from responder_registration_requests")==[('APPROVED',)])
out,rc=run([1,'meera','MeeraPass12345',1,2,1,3,0,0,0])
check("responder dashboard works", "RESPONDER DASHBOARD" in out and "Availability updated" in out, out[-500:])
check("responder status saved", q("select status from responders where responder_name='Meera Rao'")==[('AVAILABLE',)])
out,rc=run([1,"meera","MeeraPass12345",7,0,0])
check("responder has no admin modules", "Invalid choice" in out)
summary()
