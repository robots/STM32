"Multiple dispatch in Python with configurable dispatch resolution"

_AUTHOR=["David Mertz (mertz@gnosis.cx)",]
_THANKS_TO=[
    "Tim Hochberg (tim.hochberg@ieee.org)",
    "Samuele Pedroni (pedronis@bluewin.ch)",
 ]
_COPYRIGHT="""
    This file is released to the public domain.  I (dqm) would
    appreciate it if you choose to keep derived works under terms
    that promote freedom, but obviously am giving up any rights
    to compel such.
"""
_HISTORY="""
12/02  Initial "proof-of-concept"

01/03  Changed Dispatcher to Dispatch.

       Changed 'next_meth' argument for Dispatch.add_rule() to
       'propagate'.

       Added a Dispatch.next_method() method that can be called
       within a dispatched function (if the Dispatch instance
       is passed in).

       Provided example for using positional and keyword args.

02/03  Added a Dispatch.clone() method.  Added additional
        examples.
"""

#--- Constants (including Booleans if not defined in Python version)
AT_START = -1   # names for when to run other matching methods
SKIP     = 0
AT_END   = 1
if not globals().has_key('True'):  True = 1
if not globals().has_key('False'): False = 0

#-- Sample functions linearize match tables in various ways
# 'signature' passed to each func is seq of classes in current call
# 'matches' is list of (sig,fn,nm) tups, already pruned for sig len/types

def def_order(signature, matches):
    "Pass control according to first-defined-first-executed"
    return matches

def reverse_def(signature, matches):
    "Pass control according to first-defined-last-executed"
    matches.reverse()
    return matches

def unambiguous_lexicographic_mro(signature, matches):
    "Use dispatch ranking similar to Dylan"
    # I suppose this would need a topological sort
    raise NotImplementedError, "Someone want to help?"

def lexicographic_mro(signature, matches):
    "Use dispatch ranking similar to CLOS"
    # Schwartzian transform to weight match sigs, left-to-right"
    proximity = lambda klass, mro: mro.index(klass)
    mros = [klass.mro() for klass in signature]
    for (sig,func,nm),i in zip(matches,xrange(1000)):
        matches[i] = (map(proximity, sig, mros), matches[i])
    matches.sort()
    return map(lambda t:t[1], matches)

def weighted_mro(signature, matches):
    "Use dispatch ranking similar to Perl Class::Multimethods"
    # Schwartzian transform to weight match sigs, unbiased sum"
    from operator import add
    proximity = lambda klass, mro: mro.index(klass)
    sum = lambda lst: reduce(add, lst)
    mros = [klass.mro() for klass in signature]
    for (sig,func,nm),i in zip(matches,xrange(1000)):
        matches[i] = (sum(map(proximity,sig,mros)), matches[i])
    matches.sort()
    return map(lambda t:t[1], matches)

#-- Dispatch class
class Dispatch(object):
    def __init__(self, table=(), order=lexicographic_mro):
        self.table = []
        for rule in table:
            self.add_rule(*rule)
        self.order = order

    def clone(self):
        "Create a new dispatcher (usually for a new thread)"
        return Dispatch(self.table, self.order)

    def __call__(self, *args):
        self.results = []
        self.args = args
        signature = [o.__class__ for o in args]
        self.linearization = self.linearize_table(signature)
        self.pos = 0
        self.run()
        return self.results

    def with_dispatch(self, *args):
        return self(*(args+(self,)))

    def run(self):
        if self.pos >= len(self.linearization):
            return
        func, propagate = self.linearization[self.pos]
        self.pos += 1
        if propagate < SKIP: self.run()
        self.results.append(func(*self.args))
        if propagate > SKIP: self.run()

    def add_rule(self, signature, func, propagate=SKIP):
        self.table.append((signature, func, propagate))

    def add_dispatchable(self, signature, func, propagate=SKIP):
        self.add_rule(signature+(Dispatch,), func, propagate)

    def remove_rule(self, signature):
        "Remove any rule with the given signature."
        for s,f,nm in self.table:
            if s == signature:  self.table.remove((s,f,nm))

    def next_method(self):
        func, _ = self.linearization[self.pos]
        self.pos +=1
        result = func(*self.args)
        self.pos -= 1
        return result

    def linearize_table(self, sig):
        from operator import mul
        len_match = lambda (s,f,nm): len(s) == len(sig)
        typ_match = lambda (s,f,nm): reduce(mul, map(issubclass, sig, s))
        all_match = lambda x: len_match(x) and typ_match(x)
        table = filter(all_match, self.table)
        if not table:
            def nomatch(*a):
                raise TypeError, \
                  "%s instance: no defined call signature <%s>" %\
                   (self.__class__.__name__, ",".join(map(type, sig)))
            return [(nomatch,SKIP)]
        return map(lambda l:l[1:], self.order(sig, table))


def beats_game():
    class Thing(object):   pass
    class Rock(Thing):     pass
    class Paper(Thing):    pass
    class Scissors(Thing): pass
    rock, paper, scissors = Rock(), Paper(), Scissors()

    def true_func(*a):  return True
    def false_func(*a): return False

    beats = Dispatch([((Rock, Scissors), true_func),
                      ((Scissors, Paper), true_func),
                      ((Thing, Thing), false_func),
                      ((Paper, Rock), true_func),
                     ])
    beats.order = weighted_mro

    print "<rock, scissors>   ", beats(rock, scissors)
    print "<paper, scissors>  ", beats(paper, scissors)

    print "----- Add fire rule ------"
    class Fire(Thing): pass
    def firepower(a,b): return "Fire always wins!"
    beats.add_rule((Fire, Thing), firepower, AT_END)
    beats.add_rule((Thing, Fire), firepower, AT_END)
    fire = Fire()

    print "<fire, rock>       ", beats(fire, rock)
    print "<fire, paper>      ", beats(fire, paper)
    print "<fire, fire>       ", beats(fire, fire)
    print "<scissors, rock>   ", beats(scissors, rock)

    print "---- Remove fire rule ----"
    beats.remove_rule((Fire, Thing))
    print "<fire, rock>       ", beats(fire, rock)
    print "<fire, paper>      ", beats(fire, paper)
    print "<scissors, rock>   ", beats(scissors, rock)

    #print beats(scissors, "") # Raises a TypeError

    #for sig, func, propagate in beats.table:
    #    print map(lambda k: k.__name__, sig),
    #    print func.__name__,
    #    print propagate

def auto_dispatch():
    "Demonstrate dispatch propagation"
    class General(object): pass
    class Between(General): pass
    class Specific(Between): pass
    dispatch = Dispatch()
    dispatch.add_rule((General,), lambda _:"General", AT_END)
    dispatch.add_rule((Between,), lambda _:"Between", AT_END)
    dispatch.add_rule((Specific,), lambda _:"Specific", AT_END)
    print dispatch(General())
    print dispatch(Specific())

def manual_dispatch():
    "Enable Dispatch.next_method() call within function bodies"
    class General(object): pass
    class Between(General): pass
    class Specific(Between): pass

    def do_general(x, dispatch):
        print "start general"
        return "-> general"

    def do_between(x, dispatch):
        print "start between"
        print dispatch.next_method()
        print "still between"
        return "-> between"

    def do_specific(x, dispatch):
        print "start specific"
        print dispatch.next_method()
        print "still specific"
        print dispatch.next_method()    # same dispatch as prior call
        return "-> specific"

    # Either specify Dispatch in .add_rule(), or use .add_dipatchable()
    multi = Dispatch()
    multi.add_dispatchable((General,), do_general)
    multi.add_dispatchable((Between,), do_between)
    multi.add_rule((Specific, Dispatch), do_specific)

    o = Specific()
    x = multi.with_dispatch(o)
    # Or: multi(o, multi)
    print x

def function_args():
    "Call a multimethod with (simulated) positional or keyword arguments"
    class Foo(object): pass

    def say_args(foo, args=(), kw={}):
        print "Arguments:", args
        print " Keywords:", kw
        print "-"*72

    multi = Dispatch()
    multi.add_rule((Foo,), say_args)
    multi.add_rule((Foo,tuple), say_args)
    multi.add_rule((Foo,tuple,dict), say_args)

    foo = Foo()
    multi(foo)              # no arguments
    multi(foo, ('list','of','arguments'))
    multi(foo, (), {'keyword':'arguments'})

if __name__=='__main__':
    #beats_game()
    #auto_dispatch()
    manual_dispatch()
    #function_args()

