from gnosis.util.introspect import instance_noinit

class Old_noinit: pass

class Old_init:
    def __init__(self): print "Init in Old"

class New_slots_and_init(int):
    __slots__ = ('this','that')
    def __init__(self): print "Init in New w/ slots"

class New_init_no_slots(int):
    def __init__(self): print "Init in New w/o slots"

class New_slots_no_init(int):
    __slots__ = ('this','that')

class New_no_slots_no_init(int):
    pass

print "----- This should be the only line -----"
instance_noinit(Old_noinit)
instance_noinit(Old_init)
instance_noinit(New_slots_and_init)
instance_noinit(New_slots_no_init)
instance_noinit(New_init_no_slots)
instance_noinit(New_no_slots_no_init)

