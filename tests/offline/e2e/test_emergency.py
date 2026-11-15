import os, sys; sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from e2e import *
print("== EMERGENCY WORKFLOW ==")
fresh()
L=ADMIN_FIRST+[4,
 1,'Flood','Dehradun','River overflow',3,100,30.3,78.0,       # id1
 1,'Fire','Haridwar','Warehouse fire',5,10,29.9,78.1,          # id2 (higher severity)
 1,'Bad','X','x',9,5,10,10,                                    # severity 9
 1,'Bad','X','x',3,0,10,10,                                    # people 0
 1,'Bad','X','x',3,5,95,10,                                    # lat 95
 2,1,
 3,1,'Flood updated','Dehradun City','Updated description',    # details update (the reported failure)
 4,1,5,                                                        # severity -> 5
 5,1,5000,                                                     # people
 11,                                                           # priority queue
 6,1,2,                                                        # REPORTED->VERIFIED
 6,1,4,                                                        # VERIFIED->RESOLVED invalid
 6,1,3,                                                        # ->IN_PROGRESS
 6,1,1,                                                        # IN_PROGRESS->REPORTED invalid
 10,1,                                                         # history
 6,1,4,                                                        # ->RESOLVED
 6,1,5,                                                        # RESOLVED->CANCELLED invalid (final)
  3,1,                                       # edit final denied
 9,2,'False alarm',                                            # cancel #2
 9,2,'again',                                                  # cancel twice
 8,5,
 7,
 0,0,0]
out,rc=run(L)
check("exit 0", rc==0, rc)
check("two registered ids 1,2", "ID: 1" in out and "ID: 2" in out)
check("bad severity rejected", "Severity must be between 1 and 5" in out)
check("bad people rejected", "Affected people must be at least 1" in out)
check("bad coordinates rejected", "Coordinates out of range" in out)
check("details update persisted (no repository error)", "Details updated and saved to MySQL" in out and "MySQL Repository failed" not in out)
row=q("select emergency_type,location_name,description from emergencies where emergency_id=1")
check("location_name column updated", row==[('Flood updated','Dehradun City','Updated description')], row)
check("severity+people persisted", q("select severity,affected_people from emergencies where emergency_id=1")==[(5,5000)])
check("queue shows highest first", "Next emergency to serve" in out)
check("invalid transitions refused", out.count("Invalid lifecycle transition")>=3, out.count("Invalid lifecycle transition"))
check("final edit denied", "emergencies are final and cannot be edited" in out)
check("status RESOLVED persisted", q("select status from emergencies where emergency_id=1")==[('RESOLVED',)])
check("status CANCELLED persisted", q("select status from emergencies where emergency_id=2")==[('CANCELLED',)])
h=q("select old_status,new_status from emergency_status_history where emergency_id=1 order by status_history_id")
check("history rows preserved", h==[(None,'REPORTED'),('REPORTED','VERIFIED'),('VERIFIED','IN_PROGRESS'),('IN_PROGRESS','RESOLVED')], h)
check("history displayed", "STATUS HISTORY FOR EMERGENCY 1" in out and "REPORTED -> VERIFIED" in out)
check("activity logged to MySQL", q("select count(*) from activity_history where emergency_id=1")[0][0]==4, q("select count(*) from activity_history"))
# restart: heap rebuilt only with active emergencies, ids continue
out,rc=run(ADMIN+[4,11,1,'Quake','Rishikesh','x',2,3,30.1,78.3,0,0,0])
check("no active before new one", "No active emergencies in the priority queue" in out)
check("new id continues at 3", q("select max(emergency_id) from emergencies")==[(3,)], q("select emergency_id from emergencies"))
summary()
