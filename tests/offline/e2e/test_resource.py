import os, sys; sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from e2e import *
print("== RESOURCE WORKFLOW ==")
fresh()
# menu 1 = Resource. add, duplicate, invalid, search id, search name, update, display, deactivate, re-add inactive id, reactivate
L=ADMIN_FIRST+[1,
 1,101,'Water','Liquid','Bottled water','Litre',10,       # add ok
 1,101,'Dup','x','x','x',1,                                # duplicate id
 1,0,'Bad','x','x','x',1,                                  # id 0
 1,102,'Rice','Food','Bag','Kg',-5,                        # negative min
 1,103,'','Food','','Kg',5,                                # empty name
 2,101,                                                    # search id
 3,'wat',                                                  # search name
 4,101,'Drinking Water','Liquid','Updated','Litre',25,     # update
 7,                                                        # display all
 5,101,                                                    # deactivate
 2,101,                                                    # now not found
 1,101,'Again','Liquid','x','Litre',1,                     # id of inactive row
 6,101,                                                    # reactivate
 2,101,
 0,0,0]                                                    # back, logout, exit
out,rc=run(L)
check("exit code 0", rc==0, rc)
check("add saved", "Resource 101 added and saved to MySQL" in out)
check("duplicate rejected", "Resource ID 101 already exists" in out)
check("id 0 rejected", "must be a positive number" in out)
check("negative min rejected", "cannot be negative" in out)
check("empty name rejected", "Name cannot be empty" in out)
check("search by name", "Water" in out and "ID: 101" in out)
check("update saved", "Resource updated and saved to MySQL" in out)
check("deactivated", "Resource deactivated" in out)
check("inactive id message", "DEACTIVATED resource" in out)
check("reactivated", "Resource reactivated." in out)
row=q("select resource_name,category,description,unit,minimum_stock_level,is_active from resources where resource_id=101")
check("MySQL row correct", row==[('Drinking Water','Liquid','Updated','Litre',25,1)], row)
check("only one row", q("select count(*) from resources")==[(1,)])
# persistence across restart
out,rc=run(ADMIN+[1,2,101,0,0,0])
check("persists after restart", "Drinking Water" in out)
summary()
