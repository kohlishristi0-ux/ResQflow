import os, sys, sqlite3; sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from e2e import *
print("== ROUTE / GRAPH WORKFLOW ==")
fresh()
L=ADMIN_FIRST+[8,
 1,1,'A',30.0,78.0, 1,2,'B',30.1,78.1, 1,3,'C',30.2,78.2, 1,4,'Island',31.0,79.0, 1,5,'E',30.3,78.3,
 1,1,'Dup',1,1, 1,6,'Bad',99,10,
 4,1,2,10, 4,2,3,5, 4,1,3,100, 4,3,5,0, 4,1,1,5, 4,1,99,5, 4,1,2,7,
 8,1,3, 8,1,4, 8,1,99, 8,4,4,
 6,1, 7,1, 9,
 5,2,3, 8,1,3, 5,2,3,
 4,2,3,5, 8,1,3,
 3,2, 8,1,3, 8,1,2,
 10,5,'E renamed',30.31,78.31,
 0,0,0]
out,rc=run(L)
check("exit 0", rc==0, rc)
check("duplicate/invalid location refused", "Location ID already exists" in out and "Coordinates out of range" in out)
check("self/unknown connection refused", "must connect two different" in out and "Both locations must exist" in out)
check("duplicate connection refused", "already connected" in out)
check("haversine distance calculated", "Calculated distance:" in out)
check("shortest path via B (15 km)", "Shortest Path: A -> B -> C" in out and "Total Distance: 15 km" in out, out[out.find("Shortest"):out.find("Shortest")+120])
check("disconnected vertex reported", "No path exists" in out)
check("unknown location reported", "Invalid source or destination location" in out)
check("same source/destination", "Shortest Path: Island" in out)
check("BFS/DFS ran", "BFS Traversal:" in out and "DFS Traversal:" in out)
check("after removing B-C route is direct 100 km", "Total Distance: 100 km" in out)
check("route restored after reconnect (15 km again)", out.count("Total Distance: 15 km")>=2, out.count("Total Distance: 15 km"))
check("location removal drops its roads", "No path exists" in out and out.count("Location and its roads removed")==1)
check("edge rows deactivated in MySQL", q("select count(*) from route_edges where is_active=1 and (source_location_id=2 or destination_location_id=2)")==[(0,)], q("select * from route_edges"))
check("location update persisted", q("select location_name,latitude from locations where location_id=5")==[('E renamed',30.31)])
out,rc=run(ADMIN+[8,9,0,0,0])
check("graph reloads from MySQL", "[ERROR]" not in out and "A (1)" in out and "B (2)" not in out, out[:600])

print("== FAILURE INJECTION: database errors must roll back memory ==")
fresh()
out,rc=run(ADMIN_FIRST+[1,1,1,'Water','L','d','Litre',10,0, 2,1,1,'W1','C','A','Addr','1',0,
  3,1,1,1,100,10,0, 4,1,'Flood','Dehradun','orig',3,10,30,78,0,0,0])
c=sqlite3.connect(DB)
c.executescript("""CREATE TRIGGER fail_ledger BEFORE INSERT ON resource_transactions BEGIN SELECT RAISE(ABORT,'forced failure'); END;
CREATE TRIGGER fail_emerg_upd BEFORE UPDATE OF description ON emergencies BEGIN SELECT RAISE(ABORT,'forced failure'); END;
CREATE TRIGGER fail_res_upd BEFORE UPDATE OF resource_name ON resources BEGIN SELECT RAISE(ABORT,'forced failure'); END;""")
c.commit(); c.close()
out,rc=run(ADMIN+[3,3,1,50, 2,1, 0,
  4,3,1,'New','Loc','changed', 2,1, 0,
  1,4,1,'Renamed','L','d2','Litre',99, 2,1, 0, 0,0])
check("inventory op failure reported", "could not be saved to MySQL; the change was rolled back" in out)
check("memory restored: stock still 100", "Available Quantity: 100" in out and "Available Quantity: 150" not in out)
check("MySQL untouched", q("select available_quantity from inventory")==[(100,)])
check("emergency detail failure rolled back in memory", "Description: orig" in out and "Description: changed" not in out)
check("resource update failure rolled back in memory", "Name: Water" in out and "Renamed" not in out.split("Name: Water")[-1][:200] or "Name: Renamed" not in out)
check("no orphan activity rows for failed ops", q("select count(*) from activity_history where description like '%Stock added%'")==[(0,)], q("select description from activity_history"))
summary()
