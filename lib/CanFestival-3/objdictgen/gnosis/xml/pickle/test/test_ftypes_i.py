
# this is imported by test_ftypes

def gimme_bfunc():
    import time
    return time.time

def gimme_pfunc():
    return gimme_bfunc


    
