import zds_py

entries = zds_py.list_data_sets("SYS1.PARMLIB.*")

for x in entries:
    print(x.name)