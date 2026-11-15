import os, sys; sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from e2e import *
print("== INVENTORY / ALLOCATION / TRANSFER WORKFLOW ==")
fresh()
setup=ADMIN_FIRST+[
 1, 1,1,'Water','Liquid','d','Litre',10, 1,2,'Blanket','Relief','d','Piece',5, 0,
 2, 1,1,'Central','Dehradun','Rajpur','12 Main Road','999', 1,2,'North','Haridwar','Jwalapur','5 Canal Rd','888',
    1,1,'Dup','x','y','z','1',                       # duplicate warehouse id
    1,3,'NoCity','','a','b',                   # missing city
    0,
 3, 1,1,1,100,10, 1,1,2,20,5, 1,2,1,50,5,
    1,1,1,5,1,                                       # duplicate pair
    1,9,1,5,1,                                       # unknown resource
    3,1,50, 4,1,500, 4,1,10, 3,1,0, 3,1,-5, 12, 13,1,
 0,
 4, 1,'Flood','Dehradun','x',3,100,30.3,78.0, 1,'Fire','Haridwar','y',4,50,29.9,78.1, 0,
 5, 1,1,1,1,60, 1,1,1,2,50, 3,2,10, 4,1,40, 5,1,10, 6,1, 7,2, 4,1,1000, 5,1,99, 11,1,
 0]
out,rc=run(setup)
check("exit 0", rc==0, rc)
check("duplicate warehouse rejected", "Warehouse ID already exists" in out)
check("missing city rejected", "City cannot be empty" in out)
wh=q("select warehouse_name,address,latitude,longitude from warehouses where warehouse_id=1")
check("warehouse stores city/area/address, no lat/lon asked", wh==[('Central','City: Dehradun, Area: Rajpur, Address: 12 Main Road',0,0)], wh)
check("duplicate inventory pair rejected", "Could not create inventory" in out)
check("negative inventory refused", "negative inventory is not allowed" in out)
check("zero/negative qty refused", out.count("Quantity must be greater than zero")>=2)
inv=q("select inventory_id,available_quantity,reserved_quantity from inventory order by inventory_id")
check("inventory in MySQL", inv==[(1,90,20),(2,20,0),(3,50,0)], inv)
al=q("select allocation_id,requested_quantity,allocated_quantity,dispatched_quantity,returned_quantity,status from allocations order by allocation_id")
check("allocations in MySQL", al==[(1,60,60,40,10,'DISPATCHED'),(2,50,20,0,0,'CANCELLED')], al)
check("partial allocation reported", "partial" in out)
check("complete rejected while not fully dispatched", "Rejected" in out)
check("requirement synced", q("select resource_id,required_quantity,fulfilled_quantity from emergency_requirements where emergency_id=1")==[(1,60,60)], q("select * from emergency_requirements"))
led=q("select transaction_type,quantity from resource_transactions order by transaction_id")
check("stock ledger written", ('STOCK_IN',100) in led and ('RESERVE',60) in led and ('DISPATCH',40) in led and ('RETURN',10) in led and ('RELEASE',20) in led, led)
check("low stock warning shown", "WARNING: low stock" in out)
tot=q("select sum(available_quantity+reserved_quantity) from inventory where resource_id=1")
check("conservation: 100+50-10+20 stock - 40 net dispatched + 10 returned = 130", tot==[(130,)], tot)
# ---- transfers
out,rc=run(ADMIN+[7,
 1,1,2,2,5,'no inv at dest',     # dest has no record of resource 2
 1,1,1,1,5,'same wh',            # same warehouse
 1,1,2,1,1000,'too much',        # insufficient
 1,1,2,1,30,'rebalance',         # id 1
 3,1, 3,1,                       # execute, execute again
 1,1,2,1,10,'to cancel',         # id 2
 4,2, 4,2, 3,2,                  # cancel, cancel again, execute cancelled
 4,1,                            # cancel completed
 5, 6,2, 2,1, 0, 0,0])
check("transfer: missing dest record rejected", out.count("Transfer request rejected")+out.count("must differ")>=3)
check("transfer executed", "Transfer executed" in out)
check("double execute rejected", out.count("Rejected: only PENDING")>=3, out.count("Rejected: only PENDING"))
inv=q("select inventory_id,available_quantity,reserved_quantity from inventory order by inventory_id")
check("source/destination stock updated in MySQL", inv==[(1,60,20),(2,50,0),(3,50,0)], inv)
tr=q("select transfer_id,status from warehouse_transfers order by transfer_id")
check("transfer statuses persisted", tr==[(1,'COMPLETED'),(2,'CANCELLED')], tr)
led=q("select transaction_type,quantity,reference_type,reference_id from resource_transactions where reference_type='TRANSFER'")
check("transfer ledger both sides", sorted(led)==[('TRANSFER_IN',30,'TRANSFER',1),('TRANSFER_OUT',30,'TRANSFER',1)], led)
check("transfer activity persisted", q("select count(*) from activity_history where activity_type='WAREHOUSE_TRANSFERRED'")[0][0]>=1)
# ---- multi requirement + auto allocate + cancel emergency releases stock
out,rc=run(ADMIN+[5, 10,2, 2,1,20, 1,2,10, 0,  11,2, 12,2,1,70, 0,
 4, 9,2,'resolved elsewhere', 0, 5, 8, 0, 0,0])
al=q("select allocation_id,emergency_id,resource_id,warehouse_id,allocated_quantity,status from allocations where emergency_id=2 order by allocation_id")
check("multiple requirements created", len(al)>=3, al)
check("emergency cancel cancelled open allocations", all(a[5]=='CANCELLED' for a in al), al)
inv=q("select inventory_id,available_quantity,reserved_quantity from inventory order by inventory_id")
check("cancel returned all reserved stock", inv==[(1,60,20),(2,50,0),(3,50,0)], inv)
check("requirements display", "RESOURCE REQUIREMENTS FOR EMERGENCY 2" in out)
summary()
